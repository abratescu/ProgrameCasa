print("Starting service..")
from subprocess import call
from socket import *
import time
import MySQLdb
import json
import casa

def convert_to_bool(bstring):
    return False if bstring == b'\x00' else True
	

#MAIN PROGRAM STARTS HERE
addressCentrala= ( '192.168.2.204', 5000) #define server IP and port
addressPanouHol= ( '192.168.2.205', 5000) #define server IP and port
addressPanouScara= ( '192.168.2.207', 5000) #define server IP and port
client_socket =socket(AF_INET, SOCK_DGRAM) #Set up the Socket
client_socket.settimeout(2) #Only wait 1 second for a response
nrCycles=0
while(1):
	if nrCycles%20==0:
		print("Getting data..")
		(releeP,releeS,releeC)=casa.getRelee(False)
	nrCycles+=1	
	print("Sending data to devices..")
	client_socket.sendto("".join(releeC), addressCentrala) #Send the data request
	client_socket.sendto("".join(releeP), addressPanouHol) #Send the data request
	client_socket.sendto("".join(releeS), addressPanouScara) #Send the data request	
	print("Cycle complete.Waiting..")
	time.sleep(5)
