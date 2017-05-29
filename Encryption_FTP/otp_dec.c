/*************************************************
 * Teage Silbert
 * Assignment 4
 * 6/6/16
 * opt_dec: This program takes and argument of two files, a text file to be deciphered
 * and a file with the key along with the port to communicate between the client
 * side and server side, it then outputs the deciphered message
 * ***********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


int main(int argc, char *argv[])
{
	//to hold socket file descriptors, port number
	int sockfd, portno, n;
	//structs to create connections
	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	//to accept input
	char buffer[256];
	
	//to check for a valid number of arguments
	if(argc < 3)
	{
		perror("Invalid number of arguments");
		exit(1);
	}
	
	//to open file to check for invalid characters
	FILE *fp;
	FILE *keyFp;
	//to open the file passed as an arg
	fp = fopen(argv[1], "r");
	//if the file doesn't open properly
	if(fp == NULL)
	{
		perror("Error Opening File");
		exit(1);
	}

	//to assist in checking for valid values in file
	char checker;
	//to get the length of the characters in the file
	int fLength = 0;
	//to read the file for any invalid arguments
	while(1)
	{
		checker = fgetc(fp);
		//printf("%d", checker);
		if(checker == EOF)
		{
			break;
		}
		if((checker != ' ' && checker != '\n') && (checker < 65 || checker > 90))
		{
			perror("ERROR Invalid Data Types in File");
			exit(2);	
		}
		fLength++;
	}
	
	char keyCheck;
	int endCheck;
	int kCount = 0;
	int keyVal;
	fclose(fp);
	//open the key file
	keyFp = fopen(argv[2], "r");
	if(keyFp == NULL)
	{
		perror("Error opening file");
		exit(1);
	}
	//to get the elements of the key into a file
	while(1)
	{
		keyCheck = fgetc(keyFp);
		if(keyCheck == EOF)
		{	
			break;
		}
	//	printf("%c ", keyCheck);
		kCount++;
	//	printf("%d ", kCount);
	}
	kCount -= 1;
	//to make sure the key is longer than the text to decipher
	if(kCount < fLength)
	{
		perror("Key is too short");
		exit(1);
	}
	//reopen the file to read again
	//fclose(fp);
	fclose(keyFp);
	fp = fopen(argv[1], "r");

	//to set port number into int form
	portno = atoi(argv[3]);
	//to create socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("ERROR opening socket");
		exit(1);
	}
	
	//to create the host 
	if((server = gethostbyname("localhost")) == NULL)
	{
		perror("ERROR Invalid host name");
		exit(1);
	}
	//to set memory of serv_addr
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	
	//to copy memory
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	//to give serv_addr the port number in network byte order
	serv_addr.sin_port = htons(portno);	
	
	//to check the connection
	if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR cannot connect");
	}
	
	//to clear the buffer
	bzero(buffer, 256);

	char sockOut;	
	int i = 0;
	int count = 0;
	//to copy data over to server
	while(1)
	{	
		sockOut = fgetc(fp);

		if(sockOut == EOF)
		{
			//character to inform server to stop reading
			break;
		}
		//if buffer is full send data through socket and reset the buffer 
		else if(i == 255)
		{
			n = write(sockfd, buffer, strlen(buffer));
			if(n < 0)
			{
				perror("ERROR writing to the socket");
			}
			//clear the buffer and set i to 0 
			bzero(buffer, 256);	
			i = 0;
		}
		buffer[i] = sockOut;
		count++;
		i++;
		
		//printf("%c", sockOut);	
	
	}
	//write any remaining data to the servier	
	n = write(sockfd, buffer, strlen(buffer));	
	if(n < 0)
	{
		perror("ERROR writing to the socket");
	}
	//create memory for buffer to read	
	bzero(buffer, 256);

	//create a temp file to hold the data from the client
	FILE *tmpF;
	FILE *keyTmp;
	tmpF = tmpfile();
	//to use as a breakpoint
	char breakPoint[1];	
	while(1)
	{	
		//read data from the server
		n = read(sockfd, breakPoint, 2);
		if(n < 0)
		{
			perror("ERROR reading breakpoint");
		}
		if(strlen(breakPoint) != 2)
		{
			break;	
		}

	}
	if(n < 0)
	{
		perror("ERROR BREAKPOINT\n");	
	}
	i = 0;
	//to reopen the key file
	keyFp = fopen(argv[2], "r");
	//to check if the breakpoint has been met
	if(breakPoint[0] == 'b')
	{
		while(1)
		{	
			//read the key from the file to send to the server
			sockOut = fgetc(keyFp);

			if(sockOut == EOF)
			{
				//character to inform server to stop reading
				break;
			}
			//if buffer is full send data through socket and reset the buffer 
			else if(i == 255)
			{
				n = write(sockfd, buffer, strlen(buffer));
				if(n < 0)
				{
					perror("ERROR writing to the socket");
				}
				//clear the buffer and set i to 0 
				bzero(buffer, 256);	
				i = 0;
			}
			buffer[i] = sockOut;
			i++;
		
		//printf("%c", sockOut);	
	
		}	
		n = write(sockfd, buffer, strlen(buffer));	
		if(n < 0)
		{
			perror("ERROR writing to the socket");
		}
	} 
	//to read the data back from the server
	while(1)
	{
		n = read(sockfd, buffer, 255);
		fputs(buffer, tmpF);
		if(n < 0)
		{
			perror("Error reading from socket (clientside)");
		}
		if(strlen(buffer) !=255)
		{
			break;
		}
	
		//clear buffer if it is maxed out
		bzero(buffer, 255);
	}

	char fileRead;
	//to get to the beginning of temp file
	rewind(tmpF);
	
	//to print cyphered contents
	for(i = 0; i < count; i++)
	{       
		//printf("INLOOP\n");
		fileRead = fgetc(tmpF);
		
		if(fileRead == EOF)
		{
			break;
		}
		//to check if there is no more valid input
		if(fileRead == 'a')
		{
			break;
		}

		printf("%c", fileRead);

	}
	printf("\n");
	
	if(n < 0)
	{
		perror("ERROR reading from socket");
	}

	//close socket and file
	close(sockfd);
	fclose(fp);
	fclose(keyFp);
	fclose(tmpF);	
	return 0;
}
