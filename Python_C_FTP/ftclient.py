# Teage Silbert
# 3/7/17
# CS372 Project 2
# This file serves as the client side for a simple FTP program
# it connects to the server and either requests the directory listing or
# for a specific file.  
# most code based off of 'Computer Networking A Top-Down Approach'-Kurose-Ross Chapter 2.7 unless otherwise noted

#necessary libraries
from socket import*
import sys
import os

#Function: getFile
#Purpose: to make proper checks if the user would like to have a file sent
#and if requirements are met send receive it
#def getFile()

def main():
	#to ensure proper number of arguments
	if len(sys.argv) < 4:
		print "ERROR ON NO OF ARGUMENTS PROVIDED:TERMINATING"
		exit()

	initiateControl()

#Function: initiateControl
#purpose: to initiate a client side interaction of the FTP program
def initiateControl():
		
	#to get the local hostname
	test = gethostname()
	#print  test
	serverName = sys.argv[1]
	serverPort = int(sys.argv[2])
	userInput = sys.argv[3]
	command = userInput + " " + test
	#to send file name if necessary
	if len(sys.argv) == 6:
		command = command + " " + sys.argv[4] + " " + sys.argv[5]
	else:
		command = command + " " + sys.argv[4] 
	clientControl = socket(AF_INET, SOCK_STREAM)
	try:
		clientControl.connect((serverName, serverPort))
	except:
		print "FAILED TO MAKE INITIAL CONNECTION TO SERVER"
		exit()
	try:
		clientControl.send(command.encode())
	except:
		print"FAILED TO SUCCESSFULLY SEND DATA TO SERVER"
		exit()

	print "SENDING REQUEST"
	#check for error from server
	possibleErrorMsg = clientControl.recv(1024)
	#if invalid request made
	if possibleErrorMsg != "success":
		print possibleErrorMsg
		exit()
	
	#to move onto the Data connection		
	initiateData(userInput, serverName)
	
	#close socket once everything is completed
	clientControl.close()

#Function: initiateData
#Purpose: this function serves as a data connection for the FTP program
#it is a server side TCP connection that receives data and places it into
#a file or lists the contentst of the current directory

def initiateData(userInput, server):

	#to get data portnumber
	portNumber = int(sys.argv[4])
	#to create the server side TCP connection
	clientData = socket(AF_INET, SOCK_STREAM)
	
	try:
		#bind the port number
		clientData.bind(('', portNumber))
	except:
		print "ERROR BINDING TO PORT %d" %portNumber
		exit()
	#listen for a request
	clientData.listen(1)
	try:
		dataSocket, addr = clientData.accept() 
	except:
		print "ERROR CREATING DATA CONNECTION"
		exit();
	print "CONNECTION MADE"
	
	
	#if list request
	if userInput == "-l":
		print "Receiving directory from %s on port %d" % (server, portNumber)
		fileList = ""
		#to continue to receive data until wildcard is found
		while "end4FiLeS" not in fileList:
			#dataSocket, addr = clientData.accept()
			fileList += dataSocket.recv(1024).decode()
		dataSocket.close()
		print "CURRENT DIRECTORY:"	
		#to get rid of the wildcard
		fileList = fileList[:-9]
		print fileList
	
	#if get request
	elif userInput == "-g":
		if len(sys.argv) != 6:
			print "NOT ENOUGH ARGUMENTS FOR FILE REQUEST"
			exit()

		print "Receiving %s from %s on %d" % (sys.argv[5], server, portNumber)
	#	try:
	#		dataSocket, addr, = clientData.accept()
	#	except:
	#		print "ERROR MAKING DATA CONNECTION"
	#		exit()
		print "CONNECTION MADE"
		#initial connection and data transfer
		fileData = dataSocket.recv(1024).decode()
		totalData = fileData	
		#to continue to receive packets until the packet is not completely full
		while len(fileData) == 1024:
			fileData =""
			fileData = dataSocket.recv(1024).decode()
			#add additional data
			totalData += fileData
		dataSocket.close()
		if fileData == "FILE NOT FOUND IN CURRENT DIRECTORY":
			print fileData
		#SOURCE for file check www.stackabuse.com/python-check-if-a-file-or-directory-exists:
	 	if os.path.isfile(sys.argv[5]):	
			print "FILE ALREADY EXISTS, WOULD YOU LIKE TO OVERWRITE(y/n)?"
			choice = raw_input()
			if choice == 'n':
				print "FILE NOT COPIED, TERMINATING"
			elif choice == 'y':
				#Write data to file
				#source:www.tutorialspoint.com/python/python_files_io.thm
				try:
					copy = open(sys.argv[5], "w")
					copy.write(totalData);
					copy.close()
					print "%s COPIED SUCCESSFULLY" % sys.argv[5]
				except:
					print "ERROR CREATING FILE COPY"
					exit()
			else:
				print "INVALID CHOICE, TERMINATING WITHOUT COPYING"
				exit()
		else:	
			try:
				copy = open(sys.argv[5], "w")
				copy.write(totalData);
				copy.close()
				print "%s COPIED SUCCESSFULLY" % sys.argv[5]
			except:
				print "ERROR CREATING FILE COPY"
				exit()



#program start point
if __name__ == "__main__":
	main()
