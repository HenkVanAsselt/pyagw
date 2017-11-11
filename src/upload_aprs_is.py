# Below is a Python script to upload packets to APRS-IS.
# It is quick and dirty but it works.  I test it on
# WinXP
# and SuSE linux.
#=====================================================
import sys, time
from socket import *

serverHost = 'second.aprs.net'
serverPort = 10151
#password = '12345'
password = '-1'
address = 'N9TN-1>APRS,qAR,N9TN-VS:'
position = '=3349.13N/11153.55W-'
# cmment length is supposed to be 0 to 43 char. long-this is 53 char. but it works
comment = 'N9TN Python script -- Sky Ranch airport - Carefree AZ'
packet = ''
delay = 15 # delay in seconds - 15 sec. is for testing - should be 20 to 30 min for fixed QTH

def send_packet():
        # create socket & connect to server
        sSock = socket(AF_INET, SOCK_STREAM)
        sSock.connect((serverHost, serverPort))
        # logon
        sSock.send('user N9TN-VS pass ' + password + 'vers "PA3BTL Python"\n')
        # send packet
        #sSock.send(address + position + comment + '\n')
        #print("packet sent: " + time.ctime() )
        # close socket -- must be closed to avoid buffer overflow
        time.sleep(15) # 15 sec. delay
        sSock.shutdown(0)
        sSock.close()

packet = address + position + comment
print (packet) # prints the packet being sent
print (len(comment)) # prints the length of the comment part of the packet
while 1:
        send_packet()
        time.sleep(delay)