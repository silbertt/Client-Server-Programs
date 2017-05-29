/***************************************
 * Teage Silbert
 * Program 1
 * 2/6/17
 * chatclient.c: This program will serve as the client side portion of a 
 * simple chat interface between two different programming languages.  It
 * will take two args from the command line, one for the port number and one
 * for the host address
 * main source is www.linuxhottos.org/C_C++/socket.htm
 *************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

/*function:initiate
 * description: this function makes the connection to the server and if not exits
 *paramter: socket, struct of sockaddr_in type
 *preconditions: not connected to the server
 *postconditions: connected to the server
 * ******************/
void initiate(int newConnector, struct sockaddr_in serv_addr)
{
	if(connect(newConnector, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("CANNOT CONNECT, TERMINATING");
		exit(1);
	}
}

/*function:messageReceived
* description: this function checks for incoming messages from the server side
 *it then checks for whether the server has left the conversation and if so 
 *terminates the program
 *parameter: socket being used, and port number
 *preconditions: no information recevied
 *postconditions: message is received from server
 ************************** */
void messageReceived(int incoming, int portNo)
{
	//to create char for data to be received
	char msgRecd[511];
	bzero(msgRecd, 511);
	recv(incoming, msgRecd, portNo, 0);
	//printf("IN FUNCTION");

	//to check if the end of the incoming string ends with \quit
	char *compare = "\\quit";
	int incomingLength, testLength;
	incomingLength  = strlen(msgRecd);
	testLength = strlen(compare);
	
	//to perform check on incoming string and check for \quit
	//stackoverflow.com/questions/744766/how-to-compare-ends-of-strings-in-c
	if(strcmp(msgRecd + incomingLength - testLength, compare) == 0)
	{
		printf("SERVER HAS CLOSED THE CONNECTION TERMINATING PROGRAM\n");
		exit(0);
	}	

	printf("%s\n", msgRecd);
}

/*Function: messageSend
 * description: this function gets suer input checks if the user would like to quit
 * and then sends data to the server side
 *parameter: socket being uused
 *preconditions: no information sned
 *postconditions: message is sento the server and the program possibly terminates
 ************************** */
void messageSend(int outgoing, char *handle)
{
	//output users handle prior to obtaining input
	printf("%s>", handle);
	//create variable and clear buffer
	char buffer[511];
	bzero(buffer, 511);
	//create variable to get user input to concatenate with handle
	char userInput[500];
	bzero(userInput, 500);
	fgets(userInput, 500, stdin);
	
	//for comparision purposes
	char *compare = "\\quit";

	//to check if user would like to quit the program
	if(strncmp(userInput, compare, 5) == 0)
	{
		printf("CLIENT HAS CLOSED THE CONNECTION\n");
		write(outgoing, userInput, strlen(userInput));
		exit(0);
	}	

	//to append the user hand and send to server	
	strcat(buffer, handle);
	strcat(buffer, ">");
	strcat(buffer, userInput);
	//to eliminate having double \n at the end of the strings
	buffer[strlen(buffer) - 1] = 0;
	write(outgoing, buffer, strlen(buffer));
}

int main(int argc, char *argv[])
{
	int portNumber, hostLength;

	//to check for the proper amount of arguments 
	if(argc != 3)
	{
		printf("INVALID NUMBER OF ARGUMENTS ON COMMAND LINE, TERMINATING\n");
		exit(1);
	}
	else
	{
		//to get the length of the of host to allocate memory
		hostLength = strlen(argv[1]);
	}

	//allocate necessary memory and copy arg into variable
	char *hostName = (char *)malloc(hostLength);
	strcat(hostName, argv[1]);	
	//set port number in int form
	portNumber = atoi(argv[2]);

	printf("PORT %d\n", portNumber);	
	printf("HOSTNAME %s\n", hostName);
	
	//to assist in socket data
	int connectorfd;
	
	//struts to assist in TCP connection
	struct sockaddr_in serv_addr;
	struct hostent *server;

	//to create socket and perform error checking
	if((connectorfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("ERROR OPENING SOCKET, TERMINATING");
		exit(1);
	}	
	//to check the server and do error checking
	if((server = gethostbyname(hostName)) == NULL)
	{
		perror("ERROR INVALID HOST NAME, TERMINATING");
		exit(1);
	}

 
	//to set memory for serv_addr
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;

	//to copy memory
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

	//to give th serv_add the port number in network byte order
	serv_addr.sin_port = htons(portNumber);

	initiate(connectorfd, serv_addr);
	//for user handle
	char userHandle[11];
	//to be able to check for overflow and assist in clearing it if necessary
	int overBool;
	char *overChecker;
	printf("PLEASE ENTER A USER HANDLE, MAX 10 CHARACTERS\n");
	
	//to check for correct input and eliminate and overflow in stdin
	//assistnace in below if statement from www.informit.com/articles/article.aspx?=2036582&seqNum=5
	if(fgets(userHandle, 11, stdin))
	{
		//to check for a newline char in userHandle variable
		overChecker = strchr(userHandle, '\n');
		//if newline exists replace with \0
		if(overChecker)
		{
			*overChecker = '\0';
		}
		//if newline character is not found
		else
		{
			//to flush stdin
			while(((overBool = getchar()) != '\n') && !feof(stdin) && !ferror(stdin))
			{
			
			}
		}

	}



	//to send and receive messages
	while(1)
	{
		messageSend(connectorfd, userHandle);
		messageReceived(connectorfd, portNumber);	
	}
	return 0;
}
