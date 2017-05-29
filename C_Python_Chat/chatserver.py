#Teage Silbert
#CS372 Project 1
#This program will serve as the server for a simple chat interface
#The user will need to input a port number
#this is based off of the below simple implementation in:
#'Computer Networking A Top-Down Approach'-Kurose-Ross Chapter 2.7
#functions were derived from the original content with an emphasis
#on modularization

#to get the necessary socket library
from socket import*
import sys

#function: initiateSocket
#description: creates the necessary aspects of the socket api to use in a 
#server/client connection.  This is the server side
#parameter: none
#pre-conditions: no socket created
#post-conditions: socket created and returned as a result of the function call
def initiateSocket():
	#to hold the portnumber
	portNumber = int(sys.argv[1])

	#to create a TCP socket
	serverSocket = socket(AF_INET, SOCK_STREAM)

	#to bind the port number
	serverSocket.bind(('', portNumber))

	#to listen for a request
	serverSocket.listen(1)

	print "SERVER IS LISTENING ON PORT ", portNumber

	return serverSocket


#function: messageRecieved
#description: takes a socket as a parameter and receives the message sent from
#the client
#parameter: socket
#pre-conditions: socket is created but unable to recieve messages
#post-conditions: function recieves messages from clients and outputs to screen
def messageReceived(recSocket):
	#to be able to recieve a message
	sentence = recSocket.recv(511).decode()
	return sentence

#function: messageSend
#description: takes a socket as a parameter and sends a user inputted string to the 
#client
#parameter: socket
#pre-conditions: socket is created but unable to send
#post-conditions: function send messages from the server to the client also
#sends the message back to check if the user would like to end the connection
def messageSend(sentSocket):
	#to set the handle for the server 
	serverHandle = "server>"
	#prompt the user on the server side
	sys.stdout.write(str(serverHandle))
	toClient = raw_input()
	#combine servier handle and request
	sentenceToSend = serverHandle + toClient
	#send to client
	sentSocket.send(sentenceToSend.encode())
	return toClient

#to check to ensure there are the proper amount of argruments
if len(sys.argv) != 2:
	print "ERROR ON PORT NO, PROGRAM TERMINATING"
	exit()
else:
	print "ATTEMPTING TO CONNECT ON PORT" ,sys.argv[1]

#to initialize the socket
serverSocket = initiateSocket()

#to open and receive information over the tcp connection
while 1:
	#to determine if user would like to quit
	keepRunning = 0
	print "LISTENING"
	#to create connection
	connectionSocket, addr = serverSocket.accept()
	#keep connection open until server/client decideds to end it
	while (keepRunning != "\quit"): 
		incomingTest = messageReceived(connectionSocket)
		#check to see if client chose to end connection 
		if "\quit" in incomingTest:
			print "THE CLIENT HAS CLOSED THE CONNECTION"
			break
		else:
			print incomingTest
		keepRunning = messageSend(connectionSocket)
		#to check if the user on the server side would like to end connection
		if keepRunning == "\quit":
			break
	#to close the connection but listen for more connections
	connectionSocket.close()
