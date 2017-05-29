/*****************************************
 * Teage Silbert
 * CS344
 * 6/1/16
 * keygen: This program will create a key to use as a one time pad for encryption
 * purposes of the length requested by the user using a variety of 27 numbers
 *****************************************/

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	//to seed random number generator
	srand(time(NULL));

	if(argc < 2)
	{
		perror("ERROR not enough arguments");
		exit(1);
	}
	
	//to get the requested length of the key
	int length  = atoi(argv[1]);
	//to pull a random char value from
	char keyOpts[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";	
	//to hold random number
	int rChar;
	int i;
	//to print out the key 
	for(i = 0; i < length; i++)
	{
		rChar = rand() % 27;
		printf("%c", keyOpts[rChar]);	
	}
	printf("\n");
	
	return 0;

}
