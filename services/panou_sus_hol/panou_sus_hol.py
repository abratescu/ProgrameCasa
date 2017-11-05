#!/usr/bin/python
print("Starting service_camera_centrala..")
from subprocess import call
from socket import *
import time
import MySQLdb
import json

def test_connection():
	global API_KEYS
	print("Testing if device is alive.")
	return_code = call("ping 192.168.2.205 -c 1 > /dev/null", shell=True)
	if return_code == 0:
		print ("SUCCESS")
	else:
		print ("FAILED")
		time.sleep(60)
		test_connection()	

def convert_to_bool(bstring):
    return False if bstring == b'\x00' else True
#MAIN PROGRAM STARTS HERE
	
test_connection()
address= ( '192.168.2.205', 5000) #define server IP and port
client_socket =socket(AF_INET, SOCK_DGRAM) #Set up the Socket
client_socket.settimeout(2) #Only wait 1 second for a response
while(1):
	print("Cycle started.Checking temps.")

	data="data" 
	client_socket.sendto(data, address) #Send the data request
	print("Getting data..")
	time.sleep(1)
	print("Cycle complete.Waiting..")
	time.sleep(300)
	test_connection()
