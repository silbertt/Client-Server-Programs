/****************************************************
 * Teage Silbert
 * Assignment 4
 * 6/6/16
 * otp_enc_d: This program will take an argument of a port number to 
 * connect with a client and should be run in the backgrouns (&) it will
 * then read two files from the otp_enc prog use that data to cipher one of the
 * files passed by that prog and send back a ciphered version of the text
 * *************************************************/

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
	//create fork to split processes
	int i = 0;
	//to create parent and continue running in the background
	//SOURCE:www.tutorialspoint.com/unix_sockets/socket_server_example.htm
	while(1)	
	{	
		//to be able to accept data from socket
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		//to check for errors
		if(newsockfd < 0)
		{
			perror("ERROR on accept");
			exit(1);
		}
		//create memory for buffer to read	
		bzero(buffer, 256);
		//to fork into two progs
		pid_t splitter = fork();
		
		//in child process
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
				//printf("length of %d\n", strlen(buffer));
				n = read(newsockfd, buffer, 255);
				fputs(buffer, fp);
				if(n < 0)
				{
					perror("ERROR reading from socket");
				}
				if(strlen(buffer) != 255)
				{
					//printf("BUFFER %s\n", buffer);
					break;
				}
				
				bzero(buffer, 256);	
				//printf("%c\n", buffer[i]);
				i++;
			}
			char checker;
			//to get to the beginning of the file
			rewind(fp);
			//to use as a breakpoint 
			n = write(newsockfd, "b", 1);
			//create temp file to hold key
			FILE *keyTmp;
			keyTmp = tmpfile();
			//to get key file from client
			while(1)
			{
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
			
			//to get length of the plaintext file
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
			//to read the key from the client
			while(i < fileLength)
			{
				checker = fgetc(keyTmp);
				keyBuf[i] = checker - 65;
				if(checker == EOF)
				{
					break;
				}
				//to check for a space as the key
				if(keyBuf[i] == -33)
				{
					keyBuf[i] = 26;
				}
				if(keyBuf[i] == -55)
				{
					break;
				}
				//printf("%d ", keyBuf[i]);
				i++;
			}
			//to get to the beginning of the file and clear buffer
			rewind(fp);
			bzero(buffer, 256);
			
			i = 0;
			//to get the numerical value of the digit for encryption
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
							
				i++;

			}
			
			//to count against number of letters from plaintext file
			int k;
			//to get numerical value of the ecnrypted data
			int encrypt[fileLength];
			for(k = 0; k < fileLength; k++)
			{
				encrypt[k] = numBuffer[k] + keyBuf[k];
				
			}
			
			//to hold cyphered data
			char cypher[fileLength];
			
			//to encrypt the data using the key
			for(k = 0; k < fileLength; k++)
			{
				if(encrypt[k] % 27 +65 != 91)
				{
					cypher[k] = encrypt[k] % 27 + 65;
				}		
				else
				{
					cypher[k] = 32;
				}
			}
			char newBuffer[256];
			//to clear buffer
			bzero(newBuffer, 256);
			i = 0;
			//to send the ciphered data back to the client
			for(k = 0; k < fileLength; k++)
			{
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
				newBuffer[i] = cypher[k];
				
				i++;

			}
			//to send a breakpoint to client
			newBuffer[i] = 'a';
			//to send data back to client
			n = write(newsockfd, newBuffer, strlen(newBuffer));
			if(n < 0)
			{
				perror("ERROR writing to socket");
			}
			close(sockfd);
			fclose(fp);
			exit(0);
		}
		//to check if the child failed
		if(splitter < 0)
		{
			perror("Child process failed");
			exit(1);
		}
		//to close the parent 
		else
		{
			i++;
			close(newsockfd);
		}
	}
	return 0;

}  
