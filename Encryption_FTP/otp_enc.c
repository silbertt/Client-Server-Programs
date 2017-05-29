/***************************************************
 * Teage Silbert
 * Assignment 4
 * 6/6/16
 * otp_enc: This program takes two files as parameters and passed the them to the
 * server, one is a file to be ciphered and the other is the key, the last arg
 * is a port number to connect with the server which does the ciphering and sends 
 * the data back
 * ************************************************/

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
	//to open the test file to be ciphered
	fp = fopen(argv[1], "r");
	if(fp == NULL)
	{
		perror("Error Opening File");
		exit(1);
	}

	//to assist in checking for valid values in file
	char checker;
	//to get the length of the characters in the file
	int fLength = 0;
	while(1)
	{	
		//to read in the text file to be ciphered and check for bad chars
		checker = fgetc(fp);
		//printf("%d", checker);
		if(checker == EOF)
		{
			break;
		}
		//to check for invalid data types
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
	//to read in the key
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
	//to ensure the key is long enough
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
	//to pass the remaining data to server side	
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
	int newCount = 0;
	//to reopen the key file to send to server side for ciphering
	keyFp = fopen(argv[2], "r");
	//to check if the breakpoint is met to know to send data
	if(breakPoint[0] == 'b')
	{
		while(1)
		{	
			sockOut = fgetc(keyFp);
			//printf("SEND WHILE ENC\n");
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
			newCount++;
			//printf("%c", sockOut);	
	
		}
		//write remaining data to server	
		n = write(sockfd, buffer, strlen(buffer));	
		if(n < 0)
		{
			perror("ERROR writing to the socket");
		}
	} 
	//printf("NEWCOUNT %d\n", newCount);
	i = 0;
	//to read back the ciphered data from the server
	while(1)
	{
		n = read(sockfd, buffer, 255);
		//to put data from server into a temp file
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
		//pull data from tmp file and print
		fileRead = fgetc(tmpF);
		
		if(fileRead == EOF)
		{
			//printf("EOF\n");
			break;
		}
		else if(fileRead == 'a')
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
