/***************************************************
 * Teage Silbert
 * Assignment 4
 * 6/6/16
 * opt_dec_d: This program takes a port number to connect to a client and should
 * be run in the background (&), it connects to the client side and recieves
 * two files, a key text file, using the key it deciphers the text file and 
 * sends it back to the cient
 * **************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	//for sockets and port number
	int sockfd, newsockfd, portno;
	socklen_t clilen;

	//to hold data
	char buffer[256];
	//to create necessary structs for TCP socket communication
	struct sockaddr_in serv_addr, cli_addr;
	//for error checking
	int n;
	
	//to ensure proper amount of arguments
	if(argc < 2)
	{
		perror("ERROR, no port porvided");
		exit(1);
	}
	
	//to create socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("ERROR opening socket");	
	}
	//set mem for data
	bzero((char *) &serv_addr, sizeof(serv_addr));
	//set port number converting to integer
	portno = atoi(argv[1]);
	//to set proper values for serv_addr struct
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	//to bind the socket and check for errors
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("Error on binding");
	}
	
	//to be able to accept up to five operations
	listen(sockfd, 5);
	//set clilen var to size of cli-addr
	clilen = sizeof(cli_addr);
	//to keep parent process running
	//SOURCE: www.tutorialspoint.com/unix_sockets/socket_server_example.htm
	while(1)
	{
		//to be able to accept data from socket
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		//to check for errors
		if(newsockfd < 0)
		{
			perror("ERROR on accept");
		}
		//create memory for buffer to read	
		bzero(buffer, 256);
		//to create child process
		pid_t splitter = fork();

		//to check if child process succeeded
		if(splitter == 0)	
		{
			//create temp file to hold data
			FILE *fp;
			fp = tmpfile();
			char sockIn;
			if(fp == NULL)
			{
				perror("ERROR opening file");
			}
			int i = 0;
			//to read the plaintext file and place in a temp file
			while(1)
			{
				//to get the plaintext file to decipher
				n = read(newsockfd, buffer, 255);
				fputs(buffer, fp);
				if(n < 0)
				{
					perror("ERROR reading from socket");
				}
				//if the buffer is not full
				if(strlen(buffer) != 255)
				{
					break;
				}
				
				bzero(buffer, 256);	
				//printf("%c\n", buffer[i]);
				i++;
			}
			char checker;
			//to get to the beginning of the file
			rewind(fp);
			//to write a breakpoint
			n = write(newsockfd, "b", 1);
			//create temp file to hold key
			FILE *keyTmp;
			keyTmp = tmpfile();
			//to get key file from client
			while(1)
			{
				//to read the key from the client
				n = read(newsockfd, buffer, 255);
				fputs(buffer, keyTmp);
				if(n < 0)
				{
					perror("ERROR reading from socket");
				}
				if(strlen(buffer) != 255)
				{
					break;
				}
				bzero(buffer, 256);

			}	
			i = 0;
			
			//to get length of the encrypted file
			while(1)
			{
				checker = fgetc(fp);
				if(checker == EOF)
				{
					break;
				}
				i++;
			}
			//to hold the length of the file
			int fileLength = i - 1;
				
			//to get to beginning of file and clear data in buffer
			rewind(keyTmp);
			bzero(buffer, 256);
			//to hold numerical values of key
			int keyBuf[fileLength];
			i = 0;
			//to read the key from the tmpfile
			while(i < fileLength)
			{
				checker = fgetc(keyTmp);
				keyBuf[i] = checker - 65;
				if(checker == EOF)
				{
					break;
				}
				if(keyBuf[i] == -33)
				{
					keyBuf[i] = 26;
				}
				
				i++;
			}
			//to get to the beginning of the file and to clear buffer
			rewind(fp);
			bzero(buffer, 256);
			//printf("NUMBUFFER\n");
			i = 0;
			//to get the numerical value of the digit for de-encryption
			int numBuffer[fileLength];		
			while(1)
			{
				checker = fgetc(fp);
				if(checker == EOF)
				{
					break;
				}	
				if(checker == ' ')
				{
					numBuffer[i] = 26;
				}
				else
				{
					numBuffer[i] = checker - 65;
				}
				
				//printf("%d: %d\n ", i, numBuffer[i]); 
				i++;

			}
			
			//to count against number of letters from plaintext file
			int k;
			//to get numerical value of the ecnrypted data
			int dencrypt[fileLength];
			for(k = 0; k < fileLength; k++)
			{
				dencrypt[k] = numBuffer[k] - keyBuf[k];
			}
			
			//to hold cyphered data
			char decypher[fileLength];
			
			//to dencrypt the data using the key
			for(k = 0; k < fileLength; k++)
			{
				//to ensure the modulus operation will work correctly
				if(dencrypt[k] < 0)
				{
					while(dencrypt[k] < 0)
					{
						dencrypt[k] = dencrypt[k] + 27;
					}
				}
				//if original value was a spaace
				if(dencrypt[k] % 27 == 26)
				{
					decypher[k] = 32;
				}		
				else
				{
					decypher[k] = dencrypt[k] % 27 + 65;
				}
				//printf("Dec %d\n", dencrypt[k] % 27 +65);

			}

			char newBuffer[256];
			//to clear buffer
			bzero(newBuffer, 256);
			i = 0;
			//to send the decpihered data back to the client
			for(k = 0; k < fileLength; k++)
			{
				//if the buffer is full send it to the client
				if(i == 255)
				{	
					n = write(newsockfd, newBuffer, strlen(newBuffer));
					if(n < 0)
					{
						perror("ERROR writing to socket");
					}
					bzero(newBuffer, 256);
					i = 0;
				}
				newBuffer[i] = decypher[k];
				
				i++;

			}
			//to create a breakpoint to send to the client
			newBuffer[i] = 'a';
			//to send remaining data back to client
			n = write(newsockfd, newBuffer, strlen(newBuffer));
			if(n < 0)
			{
				perror("ERROR writing to socket");
			}
			close(sockfd);
			fclose(fp);
			exit(0);
		}
		if(splitter < 0)
		{
			perror("Child process failed");
		}
		//to close the parent process
		else 
		{
			close(newsockfd);
		}
	}

	return 0;

}  
