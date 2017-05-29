README 
PROJECT 1
CS 372
Teage Silbert

These programs were written in VIM and tested on the ENGR.oregonstate.edu server

There are two files, chatclient.c and chatserver.py

to compile chatlient.c enter the below command in the command line when you are in the directory the file is saved in

gcc -Wall -o chatclient chatclient.c

to run the programs first find the hostname of the server side program by entering hostname in the instance you will
be running the server side on (you will use this to run the chatclient program)

start the server program first by entering the below (portNumber is a user defined port, please use numbers of 5 digit length)

python chatserver.py <portNumber> 

next start the client program using the below (in a separate instance than the server)

chatclient <hostName> <portNumber>

<hostName> is the hostname gathered in the above step and the port numbers for the server and client must match

