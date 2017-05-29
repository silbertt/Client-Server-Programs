README 
PROJECT 2
CS 372
Teage Silbert

These programs were written in VIM and tested on the ENGR.oregonstate.edu server

There are two files, ftserver.c and ftclient.py

to compile ftserver.c enter the below command in the command line when you are in the directory the file is saved in

gcc -Wall -o ftserver ftserver.c

to run the programs first find the hostname of the server side program by entering hostname in the instance you will
be running the server side on (you will use this to run the ftclient program)

start the server program first by entering the below (portNumber is a user defined port, please use numbers of 5 digit length)

ftserver <portNumber> 

next start the client program using the below (in a separate instance than the server)

python ftclient.py <hostname> <controlport> <request> <dataport> <filename>

<hostName> is the hostname gathered in the above step and the port numbers for the server and client must match
<controlport> is the port should match the server <portnumber>
<request> this can be -l for a directory listing of the current location of the server
or -g to request a file to be copied (.txt only)
<dataport> select a port to open the data connection on
<filename> used if -g is the request.  This is where the requested filename goes.
