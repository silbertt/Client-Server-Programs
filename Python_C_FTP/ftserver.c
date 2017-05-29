/***********************************
 * Teage Silbert
 * CS372 Project 2
 * 3/7/17
 * This program will server as the server for a simple FTP 
 * the user will need to input the port to open the server on and then 
 * receive and interpret data from the client side, including sending the
 * contents (files) of a directory, or sending a copy of a file
 * MAIN SOURCE:www.linuxhowtos.org/C_C++/socket.htm
 * **********************************/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<dirent.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<errno.h>

//fucntion declarations
void initializeControl(char *portArg);
char **directoryList();
void sendDir(int dataSocket, char** directory);
void initiateDataReq(int fileNum, char **fullData, char **newTest);
char **clientArgs(char *buffer, int *args);
int fileCheck(char **localFiles, char ** argsFromClient, int noFiles, int args);

//to test pulling the directory contents
int main(int argc, char *argv[])
{	
	//check for proper amount of args, exit if not correct
	if(argc !=2)
	{
		printf("NO PORT NUMBER PROVIDED:TERMINATING\n");
		exit(0);	
	}

	initializeControl(argv[1]);	

	return 0;
}

/***********************
 * Function: initailzeFTP
 * Purpose: This function starts the intial interaction between
 * client and server and stay alive until the user manually kille
 * the program unless an error occurs
************************/
void initializeControl(char *portArg)
{
	//necessary items for the socket
	int sockfd, newsockfd, portNo; 
	socklen_t clilen;	
	
	//size of items that can be sent to the client
	char buffer[256];

	//to create the socket
	struct sockaddr_in serv_addr, cli_addr;

	//to create actual socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	//ensure socket opened properly
	if(sockfd < 0)
	{
		perror("ERROR OPENING SOCKET:TERMINATING");
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portNo = atoi(portArg);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portNo);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR ON BINDING:TERMINATING\n");
		exit(1);
	}
	//to keep the server lisenting
	while(1)
	{
		listen(sockfd, 5);
		clilen = sizeof(cli_addr);
		printf("SERVER OPEN ON %s\n", portArg);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if(newsockfd < 0)
		{
			perror("CONNECTION FAILED ON ACCEPT\n");
			exit(1);
		}	

		bzero(buffer, 256);
		//recieve the request
		recv(newsockfd, buffer, 255, 0);
		int arguments;
		char **fromClient = clientArgs(buffer, &arguments); 
		//to ensure correct request
		if((strncmp(buffer, "-l", 2) != 0) && (strncmp(buffer, "-g", 2) !=0))
		{
			char *error = "INVALID REQUEST SENT";
			send(newsockfd, error, strlen(error), 0);
			
		}
		//for use in retrieving the files in the directory
		int fileNos;
		//to get the items in the directory for comparison or sending
		char **localFileList = directoryList(&fileNos);
		//to check to see if file exists
		int exists = fileCheck(localFileList, fromClient, fileNos, arguments);
		//to check if the file exists and send error if not
		if((strncmp(buffer, "-g", 2)) == 0  && (exists == 0))
		{
			char *err = "FILE NOT FOUND IN CURRENT DIRECTORY";
			send(newsockfd, err, strlen(err), 0);
		}
		else
		{
			//if valid request send succes message to client
			char *success = "success";
			send(newsockfd, success, strlen(success), 0);
			//initiate data connection
			initiateDataReq(fileNos, fromClient, localFileList);
		}
	}
}

void initiateDataReq(int fileNum, char **fullData, char **newTest)
{
	int portNo = atoi(fullData[2]);
	char *hostName = fullData[1];
	
	printf("OPENING DATA CONNECTION ON PORT: %d WITH HOST %s\n", portNo, hostName);

	int dataFD;

	struct sockaddr_in serv_addr;
	struct hostent *server;

	//to create socket and create error check
	if((dataFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("ERROR OPENING DATA SOCKET, TERMINATING");
		exit(1);
	}
	//server error checking
	if((server = gethostbyname(hostName)) == NULL)
	{
		perror("ERROR INVALID DATA HOST NAME, TERMINATING");
		return;
	}

	//set mem for serv_add
	bzero((char *) & serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;

	//copy memory
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

	//to get port num in network byte order
	serv_addr.sin_port = htons(portNo);

	//connect for data 
	if(connect(dataFD, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
	{
		perror("CANNOT CONNECT, TERMINATING");
		return;
	}
	else
	{
		printf("DATA CONNECTION SUCCESSFUL ON PORT %d\n", portNo);
	}
	
 	//to determine what the user is requesting
	if(strncmp(fullData[0], "-g", 2) == 0)
	{
		//to get file name that was requested
		char *fileName = fullData[3];

		printf("FILE REQUESTED %s\n", fileName);
		//to open file and copy to client
		FILE *fp;
		fp = fopen(fileName, "r");
		if(fp == NULL)
		{	
			perror("ERROR OPENING FILE");
			return;
		}
			
		//variables needed to send data to client
		int length = 0;
		int n;
		const int BUFFERSIZE = 1024;
		char buffer[BUFFERSIZE];
		//clear the memory of the buffer
		bzero(buffer, 1024);
		while(1)
		{
			buffer[length] = getc(fp);
			//check for end of file
			if(buffer[length] == EOF)
			{	
				//send last packet
				n = write(dataFD, buffer, length);
				if(n < 0)
				{		
					perror("ERROR WRITING TO SOCKET");
				}
					//to break out of while loop after all data sent
				break;
			}
			if(length == 1024)
			{
				//send full packet
				n = write(dataFD, buffer, length);
				//error check
				if(n < 0)
				{
					perror("ERROR WRITING TO SOCKET");
					return;
				}	
				
				//reset length of packet to zero and reset buffer
				length = 0;
				bzero(buffer, 1024);
				
			}
			//to be able to inform client of package size
			length++;
		}			
	}
	//to send the directory back to the user
	else
	{
			int i;
			char *wow = "\n";
			//send(dataFD, newTest[0], strlen(newTest[0]), 0);
			//send(newsockfd, &buffer, sizeof(buffer), 0);
			for(i = 0; i < fileNum; i++)
			{
				send(dataFD, newTest[i], strlen(newTest[i]), 0);
				send(dataFD, wow, strlen(wow), 0);
			}
			char *end = "end4FiLeS";
			send(dataFD, end, strlen(end), 0);
		printf("FILE LISTING SENT\n");
	}
}	

/******************************
 * Function:directoryList
 * Purpose: this file will return a list of the files in
 * the current directory
 *****************************/
char **directoryList(int *fileNum)
{	
	//SOURCE: stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
	//to create necessary items for directory
	DIR *currentDir;
	struct dirent *dir;
	
	//to set directory to current location
	currentDir = opendir(".");
	
	//to hold number of files for memory allocation 
	*fileNum = 0;
		
	//create initial pointer to pointer to be array of file names
	char **files;

	//original memory allocation
	files = malloc(sizeof(char *));
	
	//error check
	if(files == NULL)
	{
		perror("MEMORY NOT ALLOCATED CORRECTY");
		exit(1);
	}
	
	//to ensure data in directory
	if(currentDir)
	{
		
		while((dir = readdir(currentDir)) != NULL)
		{
			//to only pull actualy files, not directories
			if(dir->d_type == DT_REG)
			{
				//allocate necessary memory and add to array
				files = realloc(files, (*fileNum + 1) * sizeof(char *));
				files[*fileNum] = dir->d_name;
				(*fileNum)++;
			}
			
		} 
	} 

	//close out the directory
	closedir(currentDir);
	return files;
}

/************************
 * Function:clientArgs
 * purpose: to take the arguments from teh client connection and parse them out
 * ********************/ 
char** clientArgs(char *buffer, int *args)			
{
	//to hold args from client for data connection
	char **clientData;
	clientData = malloc(sizeof(char *));
	//error check
	if(clientData == NULL)
	{
		perror("CLIENT DATA MEMORY NOT ALLOCATED PROPERLY");
		exit(1);
	}
	//tp assist in separating the data
	*args = 0;
	char *test;
	//to parse the command line from client
	test = strtok(buffer, " ");
	while(test != NULL)
	{
		//reallocate mem
		clientData = realloc(clientData, (*args + 1) * sizeof(char *));
		clientData[*args] = test;
		//printf("TEST %s\n", test);
		test = strtok(NULL, " ");
		(*args)++;
	}
	
	return clientData;
}

/*******************
 * Function:fileCheck
 * Purpose: this will check to see if a requested file exists
 * in the current directory and return a 0 or 1
 * ****************/
int fileCheck(char **localFiles, char ** argsFromClient, int noFiles, int args)
{
	//ensure there is a filename provided
	if(args < 4)
	{
		return 0;	
	}
	char *fileName = argsFromClient[3];

	//to check to see if the requested file exists
	int i;
	for(i = 0; i < noFiles; i++)
	{
		if(strncmp(fileName, localFiles[i], strlen(fileName)) == 0)
		{
			return 1;
		}
	}

	return 0;
}

