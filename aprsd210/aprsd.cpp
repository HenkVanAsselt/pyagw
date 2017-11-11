/* aprsd.cpp  2.1.0 06-22-1999



Freeware Linux APRS tcpip server.  
Originally ported from OS/2 on Aug. 25 1997
 
The source was edited with Visual SlickEdit for Linux with tabs every 3 chars.

Copyright 1997-1999 by Dale A. Heatherington, WA4DSY
email: wa4dsy@wa4dsy.radio.org
http://www.wa4dsy.ampr.org

See the aprsddoc.html file for detailed instructions.
(Use your web browser)


---------------------------------------------------

This program is free software; you can redistribute it and/or modify  
it under the terms of the GNU General Public License as published by      
the Free Software Foundation; either version 2 of the License, or         
(at your option) any later version.                                       
                                                                          
This program is distributed in the hope that it will be useful,       
but WITHOUT ANY WARRANTY; without even the implied warranty of            
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             
GNU General Public License for more details.                              
                                                                          
You should have received a copy of the GNU General Public License     
along with this program; if not, write to the Free Software               
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA                  
                                                                          
---------------------------------------------------

---BOB BRUNINGAS APRS PROTOCOL LICENSE REQUIRMENTS---

APRS is a registered trademark, and APRS protocols are copyrighted
by  Bob Bruninga, WB4APR.  The owner reserves all rights and
privileges to their use.

HAMS may apply the APRS formats in the TRANSMISSION of position, 
weather, and status packets.  However, the author reserves the 
ownership of these protocols for exclusive commercial application and for 
all reception and plotting applications. Other persons desiring to include 
APRS RECEPTION in their software  for sale within or outside of the 
amateur community will require a license from the author.  Also 
TRANSMISSION of APRS protocols in any NON-AMATEUR commercial application 
or software will require a license from the author. 

Bob Bruninga, WB4APR
115 Old Farm CT
Glen Burnie, MD 21060
  
  
  
-------------------------------------------------------

 
*/

/*

FUNCTIONS PERFORMED:

This program gets data from a TNC connected to a serial port
and sends it to all clients who have connected via tcpip to 
ports defined in the /home/aprsd2/aprsd.conf file.

Clients can use telnet to watch raw TNC data or
other APRS specfic clients such as JavAPRS or Mac/WinAPRS
to view the data plotted on a map.

A history of TNC data going back 30 minutes is kept in
memory and delivered to each user when he connects.  This
history data is filtered to remove duplicates and certain
other unwanted information.  The data is saved to history.txt
every 10 minutes and on exit with ctrl-C or "q" .  



REMOTE CONTROL of the TNC
The server system operator can access the TNC remotely via
telnet.  Simply telnet to the server and, after the server
finishes sending the history file, hit the <Esc> key .
The user will be prompted for a user name and password.  If these
are valid and the user is a member of the "aprsd" group the user
will be able to type commands to the TNC.	Type <Esc> to quit
remote mode or ctrl-d to disconnect.  Be sure to give the
TNC a "K" command to put it back on line before you exit.

To logon in remote TNC control mode you must have a
aprsd group defined in /etc/group and your Linux login
name must appear in the entry.  Here is an example.

aprsd::102:root,wa4dsy,bozo

Note: Once you have entered remote control mode the TNC
is off-line and no TNC data will go out to users.

See README file for more instructions.



-------------------------------------------------------------
REVISIONS

2.1.0  6-22-1999

1) Fixed the code so it runs properly as a daemon.   
   See added function "daemonInit()" in code.
   Also modified aprsd.init .
   To run as a daemon enter: ./aprsd -d 
   To run as a program enter: ./aprsd
   

2) Added logic to handle MD5 password encryption in the validate module.
   Now it works with RedHat 6.0 shadowed MD5 passwords.
   
3) Added "#include <errno.h>" statements in history.cpp and aprsd.cpp.
   Now it now compiles under RedHat 6.0.
   
4) Changed logic so messages sent on RF also are sent to other Internet users.
   Needed to do this so messages to wild card groups such as "SCOUTS"
   go both to RF and the remainder of the users and igates.
   
5) Fixed 2 bugs in mic_e and raw user port code.  Now mic_e packets
   come out in raw format on port 14580 .
   
6) Fixed bug that caused the history list dump to abort and disconnect user
   after about 700 items had been sent.
   
7) New Feature: The server now responds to the "?IGATE?" query.

8) New Feature: You can put system abusers in the "user.deny" file and
   deny access to  RF or prevent them from logging on.
   
9) New Feature: You can configure the server to ignore  packets which were
   transmitted from your own TNC.  The previous versions always igated
   your own TNCs packets. Use the command line "igateMyCall no" in aprsd.conf file
   to turn off igateing of your own TNCs packets.
    
   
10) Changed the servers status messages to the new format.
    eg: "aprsdATL>APRS,TCPIP*::USERLIST :Verified user W4HAM has logged on."
    Note the extra colon after TCPIP* and the lack of a message number on the end
    to indicate we don't expect an ack.
    
11) Fixed speed related problem in SendToAllClients() function that caused
    overflows of the Internet send queue.  The server can now handle many
    more users at higher data rates.
    
12) Added command "TncPktSpacing" to the aprsd.conf file. This command takes
    one parameter which is the number of milliseconds between packets sent to the
    TNC.  Queued packets to be transmitted will be spaced out at least by
    this number of milliseconds.  The default is 1500 (1.5 seconds).






-------------------------------------------------------------

2.0.9  05-23-1999

Compiled the "validate" program as an library object file
and put it in subdirectory lib/validate.o  .
Added lib/validate.o to linker list in the makefile.
Changed the code so it just calls validate instead of
running the external program. This is much safer than the
external program and still hides the source as Steve Dimse requires.

Changed the docs to reflect this change.

-------------------------------------------------------------

2.0.8 05-12-1999

1) Fixed a security problem with logons. It was possible
   in earlier versions to "be creative" with user and password
   strings to pass commands to a Linux shell.
   Replaced "system()" call with execl() and added filtering of 
   shell characters such as ; <> ~ $ | etc.

2) Cleaned up some serial interface code which may 
   have been causing trouble with raw packets on some systems.
   
3) Added msgdest2rf keyword in aprsd.conf file.

--------------------------------------------------------------
2.0.7a  04-05-1999
Replaced the Mic-E decoder/converter (mic_e.cpp) with the latest version
from Alan Crosswell.  
-------------------------------------------------

2.0.7  02-11-1999
Fixed bug in the mic-e conversion code which produced incorrect
lat/lon under some conditions (1 degree off).   See mic_e.cpp

Fixed the user validation code so it now works with shadowed passwords.

Added code to send redundent acks to the TNC at the suggestion
of Bob Bruninga.  
New keywords for aprsd.conf: "ackrepeats" and "ackrepeattime".
 

---------------------------------------------------

2.0.6a fixes a bug in 2.0.6 which causes packets which were
written to rf.log to have their CR-LF removed before being
sent on the internet.  This only showed up on packets entering
from the UDP port.


** Major functional upgrade 2.0.x  (July-Aug-Sep-Oct 1998) **

2.0.6 added tcpip port for raw tnc data to  maintain compatibiltiy
with older 1.x versions.

Added duplicate packet filter.  Duplicate packets (same "ax25 source call" and data)
are discarded if they are heard less than 30 seconds
after the original.  Duplicates are also removed from the history file.
Each station can can have only one posit, one weather, and one "other" packet
in the history buffer.  Only the latest packets are kept, older ones are deleted.

Added configuration file /home/aprsd2/aprsd.conf
to allow easy setting of tcpip ports, the serial port
and other custom parameters.

Added two-way Igating of messages and corresponding posits.

Added means for stations in a list to be gated to RF full time
or just their posits every 15 minutes.

Added user log-on and validation for Mac/WinAPRS.  Also
accepts Linux user/passwords.

Added UDP port for sending data directly to the TNC or tcpip users
  I use it to inject my weather data into the aprs stream.
  
Improved Remote TNC sysop mode.  TNC now goes off line to
give the system operator a private connection to issue
TNC commands via telnet.
  
Fixed shutdown bug.  Now /home/aprsd2/history.txt is properly saved
and it doesn't dump core on shutdown.

Fixed segmentation fault bug.  Now it only runs on RedHat 5.1 or later
because it needs libstdc++ .



--------------------------------------------------------

Modification 1.0.5 (July 1998):  

Stopped echo of Internet user data back to originator.
This will prevent infinite echo loops when two servers are linked.



--------------------------------------------------------
Modification 1.0.4 (MAR 1998) : Added code for an additional server port
14580 which does NOT echo any user data from the internet.
This allows Steves AprsServ to use the local VHF APRS data
but not get any Internet user data.

--------------------------------------------------------
bug fix 1.0.3 (Nov 1997):  No longer crashes when logged on users data
line length exceeds 1024 bytes.  All chars past 1021 are truncated.

---------------------------------------------------------


/*-------------------------------------------------------------------------------*/
/* Here's some off the air data for reference */
//
//$GPRMC,013426,A,3405.3127,N,08422.4680,W,1.7,322.5,280796,003.5,W*73
/*
KD4GVX>APRS,WB4VNT-2,N4NEQ-2*,WIDE:@071425/Steve in Athens, Ga.
N4NEQ-9>APRS,RELAY*,WIDE:/211121z3354.00N/08418.04W/GGA/NUL/APRS/good GGA FIX/A=
000708
N4NEQ-9>APRS,RELAY,WIDE*:/211121z3354.00N/08418.04W/GGA/NUL/APRS/good GGA FIX/A=
000708
KE4FNU>APRS,KD4DLT-7,N4NEQ-2*,N4KMJ-7,WIDE:@311659/South Atlanta via v7.5a@Stock
bridge@DavesWorld
KC4ELV-2>APRS,KD4DLT-7,N4NEQ-2*,WIDE:@262026/APRS 7.4 on line.
KC4ELV-2>APRS,KD4DLT-7,N4NEQ-2,WIDE*:@262026/APRS 7.4 on line.
N4QEA>APRS,SEV,WIDE,WIDE*:@251654/John in Knoxville, TN.  U-2000
WD4JEM>APRS,N4NEQ-2,WIDE*:@170830/APRS 7.4 on line.
KD4DLT>APRS,KD4DLT-7,N4NEQ-2*,WIDE:@201632/APRS 7.6f on line.
N4NEQ-3>BEACON,WIDE,WIDE:!3515.46N/08347.70W#PHG4970/WIDE-RELAY Up High!!!
N4NEQ-3>BEACON,WIDE*,WIDE:!3515.46N/08347.70W#PHG4970/WIDE-RELAY Up High!!!
KD4DKW>APRS:@151615/APRS 7.6f on line.
KE4KQB>APRS,KD4DLT-7,WIDE*:@111950/APRS 7.6 on line.
WB4VNT>APRS,WB4VNT-2,N4NEQ-2*,WIDE:@272238/7.5a on line UGA rptr 147.000+
N4YTR>APRS,AB4KN-2*,WIDE:@111443zEd - ARES DEC National Weather Service, GA
N4YTR>APRS,AB4KN-2,WIDE*:@111443zEd - ARES DEC National Weather Service, GA
W6PNC>APRS,N4NEQ-2,WIDE:@272145/3358.60N/08417.84WyJohn in Dunwoody, GA
WA4DSY-9>APRS,WIDE:$GPRMC,014441,A,3405.3251,N,08422.5074,W,0.0,359.3,280796,003
.5,W*77
N6OAA>APRS,GATE,WIDE*:@280144z4425.56N/08513.11W/ "Mitch", Lake City, MI



-------------------------------------------------------------------------------------*/

#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#define _REENTRANT 1

#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>


#include <assert.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/time.h>

#include <stdio.h>
#include <ctype.h>

#include <sys/stat.h>
#include <fcntl.h>


#include <string.h>
#include <string>

#include <math.h>
#include <stdlib.h>

#include <fstream.h>
#include <iostream.h>
#include <strstream.h>
#include <iomanip.h>

#include <linux/kernel.h>
#include <linux/sys.h>
//tcpip header files

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <errno.h>


#include "cpqueue.h"
#include "utils.h"
#include "constant.h"
#include "history.h"
#include "serialp.h"
#include "aprsString.h"
#include "validate.h"
#include "queryResp.h"



//--------------------------------------------------



#define BASE 0
#define USER 1
#define PASS 2
#define REMOTE 3
#define MAX 256
#define UMSG_SIZE MAX+SBUFSIZE+3


//---------------------------------------------------
extern int dumpAborts;              //Number of history dumps aborted
extern int ItemCount;	           //number of items in History list
cpQueue sendQueue(256,TRUE);		  //Internet transmit queue
cpQueue tncQueue(64,TRUE);         //TNC RF transmit queue
cpQueue charQueue(256,FALSE);      //A queue of single characters
cpQueue conQueue(256,TRUE);         //data going to the console from various threads


char *MyCall;
char *MyLocation;
char *MyEmail;
char *NetBeacon;
char *TncBeacon;
int TncBeaconInterval, NetBeaconInterval;
long tncPktSpacing;
BOOL igateMyCall;       //Set TRUE if server will gate packets to inet with "MyCall" in the ax25 source field.
extern int ttlDefault;
extern BOOL TncSysopMode;

int ackRepeats,ackRepeatTime;    //Used by the ACK repeat thread


BOOL sendOnRF(aprsString& atemp,  char* szPeer, char* userCall, const int src);
int WriteLog(const char *cp, char *LogFile);





//-------------------------------------------------------------------
int SendFiletoClient(int session, char *szName);
void PrintStats(ostream &os) ;
void dequeueTNC(void);

void (*p_func)(void*);

char*          szServerCall; //  This servers "AX25 Source Call" (user defined)
const char		szServerID[] = "Linux APRS Server: ";
const char		szJAVAMSG[] = ">JAVA::javaMSG  :";
const char     szAPRSDPATH[] = ">APRS,TCPIP*:" ;
const char     szUSERLISTMSG[] = ">APRS,TCPIP*::USERLIST :";
const char     szUNVERIFIED[] = "Unverified ";
const char     szVERIFIED[]  = "Verified ";
const char     szRM1[]        = "You are an unverified user, Internet access only.\r\n";
const char     szRM2[]        = "Send Access Denied. Access is Read-Only.\r\n";
const char     szRM3[]        = "RF access denied, Internet access only.\r\n";
const char     szACCESSDENIED[] = "Access Denied ";


pthread_mutex_t*				pmtxSendFile;
pthread_mutex_t*				pmtxSend;		//protect socket send() 
pthread_mutex_t*				pmtxAddDelSess;	//protects AddSession() and DeleteSession()
pthread_mutex_t*				pmtxSesStr;




BOOL ShutDownServer, configComplete;
int ConnectedClients;
int msg_cnt;
int posit_cnt;
ULONG WatchDog, tickcount, TotalConnects, TotalTncChars, TotalLines, MaxConnects;
ULONG TotalIgateChars, TotalUserChars, bytesSent;
time_t serverStartTime;
ULONG TotalTNCtxChars;
int msgsn;
char* szHostIP;

char *szComPort;



//----------------------------

struct ConnectParams{
      int RemoteSocket;
      char* RemoteName;
      int EchoMask;
      char* user;
      char* pass;
      pthread_t tid;
      pid_t pid;
};

struct ServerParams{
      int ServerPort;
      int ServerSocket;
      int EchoMask;
      pthread_t tid;
      pid_t pid;
      
};

struct SessionParams{
      int   Socket;
      int   EchoMask;
      int   ServerPort;
      int   overruns;
      pid_t pid;
};

struct UdpParams{
      int   ServerPort;
      pthread_t tid;
      pid_t pid;
};

struct pidList{
   pid_t main;
   pid_t SerialInp;
   pid_t TncQueue;
   pid_t InetQueue;
};


//Constants for EchoMask.  Each aprsSting object has this variable.
//These allow each Internet listen port to filter
//data it needs to send.
extern const int srcTNC = 1;            //data from TNC
extern const int srcUSER = 2;           //data from any logged on internet user
extern const int srcUSERVALID = 4;      //data from validated internet user
extern const int srcIGATE = 8;          //data from another IGATE
extern const int srcSYSTEM = 16;        //data from this server program
extern const int srcUDP = 32;           //data from UDP port
extern const int srcHISTORY = 64;       //data fetched from history list
extern const int src3RDPARTY = 128;     //data is a 3rd party station to station message
extern const int srcSTATS = 0x100;      //data is server statistics
extern const int wantRAW = 0x2000;      //User wants raw data only
extern const int sendDUPS = 0x4000;     //if set then don't filter duplicates
extern const int sendHISTORY = 0x8000;  //if set then history list is sent on connect

//------------------------------

SessionParams *sessions;    //points to array of active socket descriptors

int MaxClients;            //defines how many can log on at once

ULONG ulNScnt = 0;
BOOL tncPresent;            //TRUE if TNC com port has been specified
BOOL tncMute;                //TRUE stops messages from going to the TNC

int  nObjects = 0;   //number of aprsString objects that exist now.


ServerParams spMainServer, spMainServer_NH, spLinkServer, spLocalServer, spMsgServer;
ServerParams spRawTNCServer;
UdpParams upUdpServer;

extern const int maxIGATES = 20;         //Defines max number of IGATES you can connect to
int nIGATES = 0;                          //Actual number of IGATES you have defined

//------------------------------------------------------------------------------------------
//Array to hold list of stations
//allowed to full time gate from
//Internet to RF
#define MAXRFCALL 65                       //allow 64 of 'em including a NULL for last entry.
string* rfcall[MAXRFCALL];                 //list of stations gated to RF full time (all packets)
int rfcall_idx;

string* posit_rfcall[MAXRFCALL];           //Stations whose posits are always gated to RF
int posit_rfcall_idx;

string* stsmDest_rfcall[MAXRFCALL];       //Station to station messages with these 
                                          // DESTINATION call signs are always gated to RF
int stsmDest_rfcall_idx;

BOOL RF_ALLOW = FALSE;                    //TRUE to allow Internet to RF message passing.

//--------------------------------------------------------------------------------------------
ConnectParams cpIGATE[maxIGATES];



//Stuff for trusted UDP source IPs
struct sTrusted{  in_addr sin_addr;   //ip address
                  in_addr sin_mask;   //subnet mask
};
                  
const int maxTRUSTED = 20;        //Max number of trusted UDP users
sTrusted Trusted[maxTRUSTED];     //Array to store their IP addresses        
int nTrusted = 0;                 //Number of trusted UDP users defined

//Debug stuff
pidList pidlist;
char* DBstring;      //for debugging
aprsString* lastPacket; //for debugging

	
//---------------------------------------------------------------------
//Add a new user to the list of active sessions
BOOL AddSession(int s, int echo)
{	BOOL rv = FALSE;
	int i;
  

	pthread_mutex_lock(pmtxAddDelSess);

	for(i=0;i<MaxClients;i++) if (sessions[i].Socket == -1) break;

	if(i < MaxClients)
		{	sessions[i].Socket = s;
         sessions[i].EchoMask = echo;
			ConnectedClients++;
         sessions[i].overruns = 0;
			rv = TRUE;
		}

  pthread_mutex_unlock(pmtxAddDelSess);

	return rv;
}

//--------------------------------------------------------------------
//Remove a user
BOOL DeleteSession(int s)
{
	int i = 0;
	BOOL rv = FALSE;

   pthread_mutex_lock(pmtxAddDelSess);
   for(i=0; i<MaxClients; i++) 
      {  if (sessions[i].Socket == s )
	        {
	    	      sessions[i].Socket = -1;
               sessions[i].EchoMask = 0;
			      ConnectedClients--;
			      if ( ConnectedClients < 0) ConnectedClients = 0;
			      rv = TRUE;
           }
		}
   pthread_mutex_unlock(pmtxAddDelSess);
	return rv;

}
//---------------------------------------------------------------------
void CloseAllSessions(void)
{

	for(int i=0;i<MaxClients;i++)
		  if (sessions[i].Socket != -1 )
			  {	shutdown(sessions[i].Socket,2);
					close(sessions[i].Socket);
					sessions[i].Socket = -1;
               sessions[i].EchoMask = 0;
					
			  }
}
//---------------------------------------------------------------------
//Send data at "p" to all logged on clients listed in the sessions array.


void SendToAllClients( aprsString* p)
{
	
	int ccount = 0;
	int rc,n,nraw;
   char *d=NULL;
   struct timeval tv;
   struct timezone tz;
   long t0,t1;
   
   
	
   if (p == NULL) return;
   if ( (p->length() > 1023) || (p->length() < 2) ) return;
   
   /*
   gettimeofday(&tv,&tz);  //Get time of day in microseconds to tv.tv_usec
   t0 = tv.tv_usec + (tv.tv_sec * 1000000);
   */

   pthread_mutex_lock(pmtxSend);
   

   n = p->length();
   nraw = p->raw.length();

	for(int i=0;i<MaxClients;i++)
		if (sessions[i].Socket != -1)
           {  
              BOOL dup, wantdups;
              dup = wantdups = FALSE ;
              if(p->EchoMask & sendDUPS) dup = TRUE;
              if(sessions[i].EchoMask & sendDUPS) wantdups = TRUE;

              int Em = p->EchoMask & 0x1ff;   //Mask off non-source determining bits


               if ((sessions[i].EchoMask & Em)   //Echo inet data if source mask bits match
                      && (p->sourceSock != sessions[i].Socket) // no echo to original sender
                      && (ShutDownServer == FALSE)
                      && ((dup == FALSE) || (wantdups)))  //dups filtered (or not)
                      
                   { 
                       if(sessions[i].EchoMask & wantRAW) {
                            rc = send(sessions[i].Socket,p->raw.c_str(),nraw,0); //Raw data to clients
                            
                       } else{
                           rc = send(sessions[i].Socket,p->c_str(),n,0); //Cooked data to clients
                         }

                     /* Dump user if socket error or he failed
                        to accept 50 consecutive packets due to
                        resource temporarally unavailable errors */

			            if (rc == -1)    
						      { 
                          if(errno == EAGAIN) sessions[i].overruns++;

                          if((errno != EAGAIN) || (sessions[i].overruns >= 50)){
                           perror("SendToAllClients(): ");
                           DeleteSession(sessions[i].Socket);   //Take it out of the list
                           shutdown(sessions[i].Socket,2);		//Close the offending socket
						         close(sessions[i].Socket);				//
                          }
						                                   
						      } else sessions[i].overruns = 0;   //Clear users counter if he accepted packet
                     ccount++;
                   }
				  					
				  
				}

   
   pthread_mutex_unlock(pmtxSend);

      
   if((ccount > 0) && ((p->EchoMask & srcSTATS) == 0))
      {  char *cp = new char[256];
         ostrstream msg(cp,256);
         
	      msg   << "Sent " << setw(4) << n << " bytes to " << setw(3) << ccount << " clients"
               <<	endl
			      <<	ends ;
         conQueue.write(cp,0);    //cp deleted by queue reader
      }
         
   bytesSent += (n * ccount);
  /*
   gettimeofday(&tv,&tz);  //Get time of day in microseconds to tv.tv_usec
   t1 = tv.tv_usec + (tv.tv_sec * 1000000);
   cout << "t= " << t1-t0 << endl;
   */
	return;
}


//---------------------------------------------------------------------
// Pushes a character string into the server send queue.
void BroadcastString(char *cp)
{

	
	aprsString *msgbuf = new aprsString(cp,SRC_INTERNAL,srcSYSTEM);
	sendQueue.write(msgbuf); //DeQueue() deletes the *msgbuf
	return ;
}

//---------------------------------------------------------------------
//This is a thread.  It removes items from the send queue and transmits
//them to all the clients.  Also handles checking for items in the TNC queue
//and calling the tnc dequeue routine.
void *DeQueue(void* vp)
{
 	APIRET rc;
   BOOL noHist;
	int		cmd;
	char		*buff;
   aprsString* abuff;
	int MaxAge,MaxCount;
   struct timeval tv;
   struct timezone tz;
   long usLastTime, usNow;
   long t0,t1,t2;

 
 usLastTime = usNow = 0;

 pidlist.InetQueue = getpid();
 
 nice(-10);   //Increase priority of this thread by 10 (only works if run as root)

 while(!ShutDownServer)
  {
    
   gettimeofday(&tv,&tz);  //Get time of day in microseconds to tv.tv_usec
   usNow = tv.tv_usec + (tv.tv_sec * 1000000);
   if(usNow < usLastTime) usLastTime = usNow;
   t0 = usNow;
     if((usNow - usLastTime) >= tncPktSpacing) {  //Once every 1.5 second or user defined
          usLastTime = usNow;
          if(tncQueue.ready()) dequeueTNC(); //Check the TNC queue
     }

   


 	while(!sendQueue.ready())     //Loop here till somethings in the send queue
	   {	gettimeofday(&tv,&tz);  //Get time of day in microseconds to tv.tv_usec
         usNow = tv.tv_usec + (tv.tv_sec * 1000000);
         if(usNow < usLastTime) usLastTime = usNow;
         t0 = usNow;
         if((usNow - usLastTime) >= tncPktSpacing) {  //Once every 1.5 second or user defined
            usLastTime = usNow;
            if(tncQueue.ready()) dequeueTNC(); //Check the TNC queue
         }

         usleep(1000);    //1ms
			if(ShutDownServer) pthread_exit(0);
		}

	abuff = (aprsString*)sendQueue.read(&cmd);      //Read an aprsString from the queue to abuff
   

   lastPacket = abuff;
	  
   if (abuff != NULL)
	  {
	      nObjects = abuff->NN;
         abuff->dest = destINET;
         BOOL dup;
         if(((abuff->EchoMask & src3RDPARTY)&&((abuff->aprsType == APRSPOS))
               || (abuff->aprsType == COMMENT)
               || (abuff->EchoMask & (srcSTATS | srcSYSTEM))  ))
            {  dup = FALSE; noHist = TRUE;
          }else
            {
	            GetMaxAgeAndCount(&MaxAge,&MaxCount);	//Set max ttl and count values
               abuff->ttl = MaxAge;
               dup = AddHistoryItem(abuff);   	//Put item in history list.
               
               noHist = false;      

            }
          
			
			        
         if (dup) abuff->EchoMask |= sendDUPS; //If it's a duplicate mark it.
         SendToAllClients(abuff);	// Send item out on internet
               
            

                                                                                               
         if(noHist || dup) 
            { delete abuff;   //delete it if it didn't go to the history list.
              abuff = NULL;
            }
	  }else
		  cerr << "Error in DeQueue: abuff is NULL" << endl << flush;

  }

 
 return NULL;

}
//----------------------------------------------------------------------
/* This thread is started by code in  dequeueTNC when an ACK packet
is detected being sent to the TNC.  A pointer to the ack packet is passed
to this thread.  This thread puts additional identical ack packets into the
TNC queue.  The allowdup attribute is set so the dup detector won't kill 'em.
*/
void *ACKrepeaterThread(void *p)
{
   
   aprsString *paprs;
   paprs = (aprsString*)p;
   aprsString abuff(*paprs);
   abuff.allowdup = TRUE; //Bypass the dup filter!

   for(int i=0 ;i<ackRepeats;i++)
   {
   
   sleep(ackRepeatTime);
   aprsString *ack =  new aprsString(abuff);
   
   tncQueue.write(ack);   //ack will be deleted later by the history list manager
   
   }

}

//----------------------------------------------------------------------

// Pulls aprsString object pointers off the tncQueue
// and sends the data to the TNC.
void dequeueTNC(void)
{
 BOOL dup;
 char		*rfbuf;
 aprsString* abuff;
 char szUserMsg[UMSG_SIZE];
 int MaxAge, MaxCount;
 pthread_t tid;


 

    rfbuf = new char[300] ;

 
 
 
   abuff = (aprsString*)tncQueue.read(NULL);  //Get pointer to a packet off the queue

   if ((RF_ALLOW == FALSE) //See if sysop allows Internet to RF traffic
        && ((abuff->EchoMask & srcUDP) == FALSE))  //UDP packets excepted
      { if(abuff) delete abuff;
        delete rfbuf;
        return;
      }

   //abuff->print(cout); //debug
   
   if(abuff != NULL)
      {  abuff->dest = destTNC;

            abuff->ttl = 5;               //This only exist  5 minutes for dup checking
            abuff->EchoMask = 0;          //and will not be sent anywhere else
            dup = AddHistoryItem(abuff);  //Put item in history list and check for duplicates.
            if(dup){ delete abuff; abuff = NULL;}  //kill the duplicate here 
         
      }
    
	
   if (abuff != NULL)
	  {
                
                  
         if(tncPresent ) 

            {   
               if ((abuff->allowdup == FALSE) 
                     && (abuff->msgType == APRSMSGACK)
                     && (ackRepeats > 0) ) {
               int rc = pthread_create(&tid, NULL,ACKrepeaterThread,abuff);
	            if (rc != 0)
		            {  cerr << "Error: ACKrepeaterThread failed to start\n";
			            
		            }
                  else pthread_detach(tid);  //Run detached to free resources.

                  
               }

                strncpy(rfbuf,abuff->data.c_str(),256);  //copy only data portion to rf buffer
                                                         //and truncate to 256 bytes
                rfbuf[256] = '\0';                       //Make sure there's a null on the end

                RemoveCtlCodes(rfbuf);     //remove all control codes and set 8th bit to zero.
                strcat(rfbuf,"\r");         //append a CR to the end

                char* cp = new char[300];
                ostrstream msg(cp,300);
                msg << "Sending to TNC: " << rfbuf << endl << ends; //debug only
                conQueue.write(cp,0);

                            
               TotalTNCtxChars += strlen(rfbuf);
               
               if(!tncMute){ if(abuff->reformatted)
                                 {  msg_cnt++;
                                    ostrstream umsg(szUserMsg,UMSG_SIZE-1);                                                                                
                                    umsg  << abuff->peer << " " << abuff->user                                                                                
                                    << ": "                                                                                                                          
                                    << abuff->getChar()                                                                                             
                                    << ends;
                                    //Save the station-to-station message in the log
                                    WriteLog(szUserMsg,STSMLOG);                                         
                                 }
                             WriteCom(rfbuf);	 // Send string out on RF via TNC
                             
                           }

            }          
         

	  }    
  

 delete rfbuf;
 return ;

}





//-----------------------------------------------------------------------
int SendSessionStr(int session, char *s)
{  int rc, retrys;
   pthread_mutex_lock(pmtxSend);
   retrys = 0;
   do{
      rc = send(session,s,strlen(s),0);
      if (rc < 0) { usleep(50000) ; retrys++;}   //try again 50ms later
   }while((rc < 0) && (errno == EAGAIN) && (retrys <= MAXRETRYS));
   pthread_mutex_unlock(pmtxSend);
	return rc;
}

//-----------------------------------------------------------------------
void endSession(int session, char* szPeer, char* userCall, time_t starttime)
{
   char szLog[MAX],infomsg[MAX];


      pthread_mutex_lock(pmtxSend);  //Be sure not to close during a send() operation
      DeleteSession(session);
      shutdown(session,2);
      close(session);                 //Close socket
      pthread_mutex_unlock(pmtxSend);
  
      {
      char* cp = new char[128];
      ostrstream msg(cp,128);
      msg << szPeer << " " << userCall
          << " has disconnected\n"
          <<	ends;
      conQueue.write(cp,0);
      }

      strncpy(szLog,szPeer,MAX-1);
      strcat(szLog," ");
      strcat(szLog,userCall);
      strcat(szLog," disconnected ");
      time_t endtime = time(NULL);
      double  dConnecttime = difftime(endtime , starttime);
      int iMinute = (int)(dConnecttime / 60);
      iMinute = iMinute % 60;
      int iHour = (int)dConnecttime / 3600;
      int iSecond = (int)dConnecttime % 60;
      char timeStr[32];
      sprintf(timeStr,"%3d:%02d:%02d",iHour,iMinute,iSecond);
      strcat(szLog,timeStr)	;
  
      WriteLog(szLog,MAINLOG);
      {
      ostrstream msg(infomsg,MAX-1);
  
      
     msg   << szServerCall
           << szJAVAMSG
           << MyLocation << " "
           <<	szServerID
           <<	szPeer
           << " " << userCall
           << " disconnected. "
           << ConnectedClients
           << " users online.\r\n"
           << ends;
      }
  
     BroadcastString(infomsg);			  //Say IP address of disconected client
  
     if(strlen(userCall) > 0)
        { ostrstream msg(infomsg,MAX-1);
  
         msg  << szServerCall
              <<  szUSERLISTMSG
              <<  MyLocation
              <<  ": Disconnected from "                              
              <<  userCall                              
              <<  "."                                                  
              <<    "\r\n"                                              
              <<    ends;
         BroadcastString(infomsg);		//Say call sign of disconnected client
        }
                                 
  
      
       
      pthread_exit(0);







}
//-----------------------------------------------------------------------

//An instance of this thread is created for each user who connects.


void *TCPSessionThread(void *p)
{
	char lastch=0;
   char buf[SBUFSIZE];
	int BufRemaining;

   SessionParams *psp = (SessionParams*)p;
  	int session = psp->Socket;
   int EchoMask = psp->EchoMask;
   int serverport = psp->ServerPort;
   
   delete psp;

	int BytesSent,BytesRead,dsize, i;
	struct sockaddr_in peer_adr;
	char szPeer[MAX], szError[MAX], szLog[MAX], infomsg[MAX], logmsg[MAX];
	char *szUserMsg;
   const char *szUserStatus;
	unsigned char c;
	unsigned adr_size = sizeof(peer_adr);
	int n, rc,data,verified=FALSE, loggedon=FALSE;
	ULONG State = BASE;
   char userCall[10];
   char tc, checkdeny;
   const char *szRestriction;
   int dummy;
   

   //These deal with Telnet protocol option negotiation suppression
   int iac,sbEsc;
   const int IAC = 255, SB = 250, SE = 240;
   
	
	char szUser[16], szPass[16];
   char* szServerPort[10];

   
   

	if (session < 0) return NULL;



	TotalConnects++;

	time_t  starttime = time(NULL);

	szPeer[0] = '\0';
   userCall[0] = '\0';

	if (getpeername(session, (struct sockaddr *)&peer_adr, &adr_size) == 0)
		{
			strncpy(szPeer,inet_ntoa(peer_adr.sin_addr),32);
		}


   
   
   {ostrstream msg(szError,MAX-1); //Build an error message in szError

		msg   << szServerCall
            << szJAVAMSG
            << "Limit of "
            << MaxClients
            << " users exceeded.  Try again later. Disconnecting...\r\n"
				<<	ends;

   }
   


   { char* cp = new char[256];
     ostrstream msg(cp,256);
  	  msg << szPeer << " has connected to port " << serverport << endl << ends;
     conQueue.write(cp,0);  //queue reader deletes cp
   }

   {ostrstream msg(szLog,MAX-1);
      msg   << szPeer
            << " connected on "
            << serverport
            << ends;
      WriteLog(szLog,MAINLOG);
   }


	data = 1;  //Set socket for non-blocking
	ioctl(session,FIONBIO,(char*)&data,sizeof(int));

   rc = SendSessionStr(session,SIGNON);
   if(rc < 0) endSession(session,szPeer,userCall,starttime); 
   if(NetBeacon) rc = SendSessionStr(session,NetBeacon);
   if(rc < 0) endSession(session,szPeer,userCall,starttime);

   if (EchoMask & sendHISTORY)
      {
          n = SendHistory(session,(EchoMask & ~(srcSYSTEM | srcSTATS)));   //Send out the previous N minutes of APRS activity
	                                                           //except messages generated by this system.
          if (n < 0)
		      {  ostrstream msg(szLog,MAX-1);
                  msg   << szPeer
                        << " aborted during history dump on "
                        << serverport
                        << ends;
               WriteLog(szLog,MAINLOG);
               endSession(session,szPeer,userCall,starttime);
		      }

           {   char* cp = new char[256];
               ostrstream msg(cp,256);
  	            msg << "Sent " << n << " history items to " << szPeer << endl << ends;
               conQueue.write(cp,0);  //queue reader deletes cp
            }

       }

   

	char *pWelcome = new char[strlen(CONFPATH) + strlen(WELCOME) + 1];
	strcpy(pWelcome,CONFPATH);
	strcat(pWelcome,WELCOME);
  	rc = SendFiletoClient(session,pWelcome);		 //Read Welcome message from file
   if(rc < 0){
            ostrstream msg(szLog,MAX-1);
             msg   << szPeer
                   << " aborted welcome msg on "
                   << serverport
                   << ends;
            WriteLog(szLog,MAINLOG);
            delete pWelcome;
            endSession(session,szPeer,userCall,starttime);
       }

   delete pWelcome;


	if (!AddSession(session,EchoMask))
		{ 
		  rc = SendSessionStr(session,szError);
		  if (rc == -1) perror("AddSession");
		  WriteLog("Error, too many users",MAINLOG);
		  endSession(session,szPeer,userCall,starttime);
        char* cp = new char[256];
        ostrstream msg(cp,256);
        msg <<  "Can't add client to session list, too many users - closing connection.\n"
            << ends;
        conQueue.write(cp,0);
		}

  
   
	{ostrstream msg(infomsg,MAX-1);

		msg   << szServerCall
            << szJAVAMSG
            <<	MyLocation << " "
				<<	szServerID
				<<	szPeer
				<< " connected "
            << ConnectedClients
				<< " users online.\r\n"
				<<	ends;

	
	  	BroadcastString(infomsg);

		
	}

	if (ConnectedClients > MaxConnects) MaxConnects= ConnectedClients;
   iac = 0;
   sbEsc = FALSE;
	do
		{											 /*The logic here is that 1 char is fetched
														on each loop and tested for a CR or LF	before
														the buffer is processed.  If "i" is -1 then
														any socket error other that "would block" causes
														BytesRead and i to be set to zero which causes
														the socket to be closed and the thread terminated.
														If "i" is greater than 0 the character in "c" is
														put in the buffer.
													 */
											
			
		   BytesRead = 0;						 //initialize byte counter
 			
			do
			  {	if((charQueue.ready()) && (State == REMOTE) )
                   { tc = (char) charQueue.read(&dummy);
                     send(session,&tc,1,0);   //send a tnc character to sysop
                     //printhex(&tc,1);
                   }

               do{
               c = 0x7f;
					i = recv(session,&c,1,0);		//get 1 byte into c
               }while(c == 0x00);
               //if (State == REMOTE) printhex(&c,1); debug
					if (i == -1)
						{
							
						 	if (errno != EWOULDBLOCK)
										{	BytesRead = 0;	  //exit on errors other than EWOULDBLOCK
											i = 0;

										}

                     //cerr << "i=" << i << "  chTimer=" << chTimer << "   c=" << c << endl;
							if(State != REMOTE) sleep(1);  // Don't hog cpu while in loop awaiting data
							
						}

                if (ShutDownServer)
						 {	
							pthread_exit(0);
						 }
					 
					 
					 if (i != -1)   //Got a real character from the net
						 {	
						   if(loggedon == FALSE){						   
                        if ((c == IAC) && (sbEsc == FALSE)) iac = 3;  //This is a Telnet IAC byte.  Ignore the next 2 chars
                        if ((c == SB) && (iac == 2)) sbEsc = TRUE;    //SB is suboption begin.
                      }                                  //Ignore everything until SE, suboption end.
                     

                    // printhex(&c,1);  //debug

						   if ( !((lastch == 0x0d) && (c == 0x0a)) && (c != 0) && (iac == 0) )	  //reject LF after CR and NULL
								{	//c = c & 0x7f;
									if (BytesRead < SBUFSIZE-3) buf[BytesRead] = c;
									BytesRead += i;
                            
							//Only enable control char interpreter if user is NOT logged on in client mode		
                     if(loggedon == FALSE){  
							   switch (c)
								   {
									case 0x04:	i = 0;   		//Control-D = disconnect
													BytesRead = 0;
													break;
																		 
									case 0x1b:	  if((State == BASE) && tncPresent)   //ESC = TNC control
                                          {  DeleteSession(session);
                                             rc = SendSessionStr(session,"\r\n220 Remote TNC control mode. <ESC> to quit.\r\n503 User:");
													      if(rc < 0) endSession(session,szPeer,userCall,starttime);
                                             State = USER;    //   Set for remote control of TNC
                                             BytesRead = 0;
													      break;
                                          }
													  
																		  //<ESC> = exit TNC control
									           	if ((State != BASE) && tncPresent)
														{	
															
                                             
                                             if (State == REMOTE){
                                                ostrstream log(szLog,MAX);
                                                log   << szPeer << " "
                                                      << szUser << " "
                                                      << " Exited TNC remote sysop mode."
                                                      << endl
                                                      << ends;
                                                WriteLog(szLog,MAINLOG);
                                             }

                                             tncMute = FALSE;
                                             TncSysopMode = FALSE;
                                             State = BASE;  // <ESC>Turn off remote
															rc = SendSessionStr(session,"\r\n200 Exit remote mode successfull\r\n");
                                             if(rc < 0) endSession(session,szPeer,userCall,starttime);
                                             AddSession(session,EchoMask);
                                             BytesRead = 0;  //Reset buffer
														}
                                       
													//i = 0;
													break;

                              };
                           }//end if (loggedon==FALSE)

                           if ((State == REMOTE) && (c != 0x1b) && (c != 0x0) && (c != 0x0a)) 
                                 {  
                                     char chbuf[2];
                                     chbuf[0] = c;
                                     chbuf[1] = '\0';
                                     WriteCom(chbuf); //Send chars to TNC in real time if REMOTE
                                    
                                 }
                        }

     						 }else c = 0;

             if(loggedon == FALSE){
				   if (c == SE) { sbEsc = FALSE; iac = 0;}  //End of telnet suboption string  					
               if (sbEsc == FALSE) if(iac-- <= 0) iac = 0;  //Count the bytes in the Telnet command string
             }

			  }while ((i != 0) && (c != 0x0d) && (c != 0x0a)  );
			
				
			if ((BytesRead > 0) && ( i > 0) && (c != 0x0a) )		 // 1 or more bytes needed
				{																 // and discard line feeds

				 		i = BytesRead - 1;
						buf[i++] = 0x0d;
						buf[i++] = 0x0a;
						buf[i++] = 0;    
						
                  TotalUserChars += i;

						//cout << szPeer << ": " << buf << flush;		 //Debug code
					  //printhex(buf,i);
					 
                  //cout << endl;
                  //printhex(buf,strlen(buf));
               
               if (State == REMOTE)
						{	buf[i-2] = '\0';							//No line feed required for TNC
							//printhex(buf,strlen(buf));			//debug code
                  }

					if (State == BASE)           //Internet to RF messaging handler
                  {
                     BOOL sentOnRF=FALSE;
                     
                     aprsString atemp(buf,session,srcUSER);


                     if(atemp.aprsType == APRSQUERY){ /* non-directed query ? */
                        queryResp(session,&atemp);   /* yes, send our response */
                     }
                     
                     //cout << atemp << endl;
                     //cout << atemp.stsmDest << "|" << szServerCall << "|" << atemp.aprsType << endl;
                     
                     if((atemp.aprsType == APRSMSG)
                       && (atemp.msgType == APRSMSGQUERY)){    /* is this a directed query message? */

                       if((stricmp(szServerCall, atemp.stsmDest.c_str()) == 0)
                           || (stricmp("aprsd",atemp.stsmDest.c_str()) == 0)
                           || (stricmp("igate",atemp.stsmDest.c_str()) == 0)){   /* Is query for us? */
                           queryResp(session,&atemp);   /*Yes, respond. */
                        }
                       
                     }
                     
                     string vd;
                     int idxInvalid=0;
                     if(atemp.aprsType == APRSLOGON) 
                           { loggedon = TRUE;

                             verified = FALSE;

                             vd = atemp.user + atemp.pass ;
                             
                             /* 2.0.7b Security bug fix - don't allow ;!@#$%~^&*():="\<>[]  in user or pass! */
                             
                             if(((idxInvalid = vd.find_first_of(";!@#$%~^&*():=\"\\<>[]",0,20)) == string::npos)
                                  && (atemp.user.length() <= 15)   /*Limit length to 15 or less*/
                                  && (atemp.pass.length() <= 15)){

                             
                               if(validate(atemp.user.c_str(), atemp.pass.c_str(),APRSGROUP) == 0)
                                        verified = TRUE;

                             } 
                               else {if(idxInvalid != string::npos){ 
                                      char *cp = new char[256];
                                       ostrstream msg(cp,256);
         
	                                    msg   << szPeer
                                             << " Invalid character \"" 
                                             << vd[idxInvalid]
                                             << "\" in APRS logon"
                                             <<	endl
			                                    <<	ends ;
                                       conQueue.write(cp,0);    //cp deleted by queue reader
                                       WriteLog(cp,MAINLOG);
                                       }
                                    }
                                                            
                               checkdeny = toupper(checkUserDeny(atemp.user));  // returns + , L or R 
                                                                                // + = no restriction
                                                                                // L = No login
                                                                                // R = No RF
                               if (verified) szUserStatus = szVERIFIED ; 
                                 else szUserStatus = szUNVERIFIED;

                               switch(checkdeny){
                               case 'L':  szUserStatus = szACCESSDENIED; //Read only access
                                          szRestriction = szRM2;
                                          verified = false;
                                          break;

                               case 'R':  szRestriction = szRM3;  //No send on RF access
                                          break;

                               default:   szRestriction = szRM1;    
                               
                               }


                               

                               if(checkdeny != 'L'){
                                 ostrstream msg(infomsg,MAX-1);
                                 msg	<< szServerCall
                                       <<  szUSERLISTMSG
                                       <<  MyLocation << ": "
                                       <<  szUserStatus << "user "
                                       <<  atemp.user
                                       <<  " logged on using "
                                       <<  atemp.pgmName << " "
                                       <<  atemp.pgmVers
                                       <<  "."
                                       <<  "\r\n"  /* Don't want acks from this! */
                                       <<    ends;
                               
                                 BroadcastString(infomsg);         //send users logon status to everyone
                               }


                                 
                                {
                                 ostrstream msg(logmsg,MAX-1);
                                 msg   << szPeer
                                       << " " << atemp.user
                                       << " " << atemp.pgmName
                                       << " " << atemp.pgmVers
                                       << " " << szUserStatus
                                       << endl 
                                       << ends;
                                }

                                 WriteLog(logmsg,MAINLOG);
                                 strncpy(userCall,atemp.user.c_str(),9);  //save users call sign
                                 
                                if((!verified) || (checkdeny == 'R'))
                                 {  char call_pad[] = "         "; //9 spaces
                                    int len = strlen(atemp.user.c_str());
                                    if(len > 9) len = 9;
                                    memmove(call_pad,atemp.user.c_str(),len);
                                    
                                    {
                                    ostrstream msg(infomsg,MAX-1);  //Message to user...
                                    msg   << szServerCall
                                          << szAPRSDPATH
                                          << ':' 
                                          << call_pad
                                          << ":" << szRestriction
                                          << ends ;
                                    }

                                    rc = SendSessionStr(session,infomsg);
                                    if(rc < 0) endSession(session,szPeer,userCall,starttime);

                                    if(checkdeny == '+'){
                                       ostrstream msg(infomsg,MAX-1); //messsage to user
                                       msg   << szServerCall
                                             << szAPRSDPATH
                                             << ':' 
                                             << call_pad
                                             << ":Contact program author for registration number.\r\n"
                                             << ends ;
                                    rc = SendSessionStr(session,infomsg);
                                    if(rc < 0) endSession(session,szPeer,userCall,starttime);
                                    }
                                 }
                                 
                                 
                                 if(verified && (atemp.pgmName.compare("monitor") == 0))
                                  { DeleteSession(session);
                                    AddSession(session,srcSTATS);
                                    char prompt[] = "#Entered Monitor mode\n\r";
                                    aprsString *amsg = new aprsString(prompt,SRC_USER,srcSTATS);
                                    sendQueue.write(amsg);
                                  }
                                    
                           }


                     /*One of the stations in the gate2rf list?*/
                     BOOL RFalways = find_rfcall(atemp.ax25Source,rfcall);
                     

                     if( verified  && (!RFalways) && (atemp.aprsType == APRSMSG) && (checkdeny == '+'))
                        {   sentOnRF = FALSE;
                            atemp.changePath("TCPIP*","TCPIP");
                            
                            sentOnRF = sendOnRF(atemp,szPeer,userCall,srcUSERVALID);   // Send on RF if dest local
                              
                            
                            if(sentOnRF)  //Now find the posit for this guy in the history list
                                           // and send it too.
                               { aprsString* posit = getPosit(atemp.ax25Source,srcIGATE | srcUSERVALID);
                                 if(posit != NULL)
                                    { time_t Time = time(NULL);           //get current time
                                      if((Time - posit->timeRF) >= 60*10) //every 10 minutes only
                                          {  
                                             timestamp(posit->ID,Time); //Time stamp the original in hist. list
                                             posit->stsmReformat(MyCall);  // Reformat it for RF delivery
                                             tncQueue.write(posit);    //posit will be deleted elsewhere
                                             
                                          }else delete posit;

                                    } /*else cout << "Can't find matching posit for "
                                                << atemp.ax25Source
                                                << endl
                                                << flush;        //Debug only
                                                */
                               }
                     
                        }


                     if ( verified && (atemp.aprsType != APRSLOGON) )
                        {
                            aprsString* inetpacket = new aprsString(buf,session,srcUSERVALID,szPeer,userCall);
                            inetpacket->changePath("TCPIP*","TCPIP") ;

                            if(inetpacket->aprsType == APRSMIC_E)   //Reformat Mic-E packets
                               { reformatAndSendMicE(inetpacket,sendQueue);
                                
                               }else
                                  sendQueue.write(inetpacket); //note: inetpacket is deleted in DeQueue
                                    

                        }



                      if (!verified && (atemp.aprsType != APRSLOGON) && (checkdeny != 'L') )
                        { 
                            aprsString* inetpacket = new aprsString(buf,session,srcUSER,szPeer,userCall);
                             
                            if(inetpacket->ax25Source.compare(userCall) != 0)
                                          inetpacket->EchoMask = 0;  //No tcpip echo if not from user

                            inetpacket->changePath("TCPIP*","TCPIP") ;
                            if(inetpacket->changePath("TCPXX*","TCPIP*") == FALSE) 
                                          inetpacket->EchoMask = 0;  //No tcpip echo if no TCPXX* in path;
                            //inetpacket->print(cout);  //debug
                            sendQueue.write(inetpacket); //note: inetpacket is deleted in DeQueue
                        }



                      if((atemp.aprsType == APRSMSG) && (RFalways == FALSE) )
                         {
                           aprsString* posit = getPosit(atemp.ax25Source,srcIGATE | srcUSERVALID | srcTNC);
                           if(posit != NULL)
                              { 
                                 posit->EchoMask = src3RDPARTY;
                                 sendQueue.write(posit);  //send matching posit only to msg port
                                 
                              } 

                          }




               //Here's where the priviledged get their posits sent to RF full time.
               if(configComplete
                    && verified
                    && RFalways 
                    && (StationLocal(atemp.ax25Source.c_str(),srcTNC) == FALSE)
                    && (atemp.tcpxx == FALSE)
                    && (checkdeny == '+'))
                  {
                     aprsString* RFpacket = new aprsString(buf,session,srcUSER,szPeer,userCall); 
                     RFpacket->changePath("TCPIP*","TCPIP");
                     
                     if(RFpacket->aprsType == APRSMIC_E)   //Reformat Mic-E packets
                           {  
                               aprsString* posit = NULL;                                                                              
                               aprsString* telemetry = NULL;                                            
                               RFpacket->mic_e_Reformat(&posit,&telemetry);                           
                               if(posit){
                                  posit->stsmReformat(MyCall);  // Reformat it for RF delivery
                                  tncQueue.write(posit);       //Send reformatted packet on RF
                               }
                               if(telemetry) {
                                  telemetry->stsmReformat(MyCall); // Reformat it for RF delivery
                                  tncQueue.write(telemetry);      //Send packet on RF
                               }
                               
                                  
                               delete RFpacket;                                                       

                             
                           }else{
                                 RFpacket->stsmReformat(MyCall);
                                 tncQueue.write(RFpacket);  // send data to RF  
                                }                           //Note: RFpacket is deleted elsewhere
                    }





                  }


					int j = i-3;

               if ((State == PASS) && (BytesRead > 1))
						{	strncpy(szPass,buf,15);
							if(j<16) szPass[j] = '\0'; else szPass[15] = '\0';
                     
                     BOOL verified_tnc = FALSE;
                     int idxInvalid=0;
                     int pid , childStatus = -1;
                     int valid = -1;
                     
                     string vd = string(szUser) + string(szPass) ;
                                                                        
                                                                                                                          
                     // 2.0.7b Security bug fix - don't allow ;!@#$%~^&*():="\<>[]  in szUser or szPass!
                      //Probably not needed in 2.0.9 because validate is not an external pgm anymore!
                                                                                                                                          
                     if(((idxInvalid = vd.find_first_of(";!@#$%~^&*():=\"\\<>[]",0,20)) == string::npos)                                  
                          && (strlen(szUser) <= 16)   //Limit length to 16 or less
                          && (strlen(szPass) <= 16)){                                                
                      
                       valid = validate(szUser,szPass,TNCGROUP);  //Check user/password


                     } 
                      else {  if(idxInvalid != string::npos){
                              char *cp = new char[256];
                              ostrstream msg(cp,256);
         
	                            msg   << szPeer
                                     << " Invalid character \"" 
                                     << vd[idxInvalid]
                                     << "\" in TNC logon"
                                     <<	endl
			                            <<	ends ;
                               conQueue.write(cp,0);    //cp deleted by queue reader
                               WriteLog(cp,MAINLOG);
                              }
                            }

                        
                     if(valid == 0) verified_tnc = TRUE;                                                                             
                                                                                                                          
                                                   
                      


							if (verified_tnc)
								{ if(TncSysopMode == FALSE)
                           { TncSysopMode = TRUE;
                             
                              State = REMOTE;
                              tncMute = TRUE;
		                        rc = SendSessionStr(session,"\r\n230 Login successful. <ESC> to exit remote mode.\r\n");
								      if(rc < 0) endSession(session,szPeer,userCall,starttime);
                              ostrstream log(szLog,MAX);
                              log << szPeer << " "
                                  << szUser 
                                  << " Entered TNC remote sysop mode." 
                                  << endl 
                                  << ends;
                              WriteLog(szLog,MAINLOG);
                           }else
                              { rc = SendSessionStr(session,"\r\n550 Login failed, TNC is busy\r\n");
                                if(rc < 0) endSession(session,szPeer,userCall,starttime);
                                 ostrstream log(szLog,MAX);
                                 log << szPeer << " "
                                    << szUser 
                                    << " Login failed: TNC busy." 
                                    << endl 
                                    << ends;
                                 WriteLog(szLog,MAINLOG);
                                
										  State == BASE;
                                AddSession(session,EchoMask);
 
                              }

							   } else
									{	rc = SendSessionStr(session,"\r\n550 Login failed, invalid user or password\r\n");
                              if(rc < 0) endSession(session,szPeer,userCall,starttime);
                              ostrstream log(szLog,MAX);
                              log << szPeer << " "
                                 << szUser  << " "
                                 << szPass
                                 << " Login failed: Invalid user or password." 
                                 << endl 
                                 << ends;
                              WriteLog(szLog,MAINLOG);
                              State = BASE;
                              AddSession(session,EchoMask);
									}

                     
						}

					if ((State == USER) && (BytesRead > 1))
						{ 	strncpy(szUser,buf,15);
							if(j < 16) szUser[j] = '\0'; else szUser[15]='\0';
							State = PASS;
						   rc = SendSessionStr(session,"\r\n331 Pass:");
                     if(rc < 0) endSession(session,szPeer,userCall,starttime);
							
						}

					
					
				 }


      } while (BytesRead != 0);		  // Loop back and get another line from remote user.

    if(State == REMOTE) tncMute == FALSE; TncSysopMode = FALSE;

    endSession(session,szPeer,userCall,starttime);

      
}


//------------------------------------------------------------------------

//One instance of this thread is created for each port definition in aprsd.conf.
//Each instance listens on the a user defined port number for clients
//wanting to connect.  Each connect request is assigned a
//new socket and a new instance of TCPSessionThread() is created.

void *TCPServerThread(void *p)
{

	int	s,rc;
   unsigned i;
   SessionParams* session;
   pthread_t SessionThread;
   pid_t pid;
	struct sockaddr_in server,client;
	char buf[SBUFSIZE];
	int optval;
   ServerParams *sp = (ServerParams*)p;

	ShutDownServer = FALSE;

   
   sp->pid = getpid();

	s = socket(PF_INET,SOCK_STREAM,0);
	
   sp->ServerSocket = s;

	optval = 1;								  //Allow address reuse
	setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char*)&optval,sizeof(int));
   setsockopt(s,SOL_SOCKET,SO_KEEPALIVE,(char*)&optval,sizeof(int));

	if (s == 0)
		{  perror("TCPServerThread socket error");
			ShutDownServer = TRUE;
			return NULL;
		}

	memset(&server,0,sizeof(server));
	memset(&client,0,sizeof(client));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(sp->ServerPort);
	if (bind(s,(struct sockaddr *)&server, sizeof(server)) <  0)
		{
		   perror("TCPServerThread bind error");
			ShutDownServer = TRUE;
			return NULL;
		}

   cout << "TCP Server listening on port " << sp->ServerPort << endl << flush;
   

   while(!configComplete) usleep(100000);  //Wait till everything else is running.

	listen(s,2);	

	

	for(;;)
		{
		   i = sizeof(client);
         session = new SessionParams;
			session->Socket = accept(s,(struct sockaddr *)&client,&i);
         session->EchoMask = sp->EchoMask;
         session->ServerPort = sp->ServerPort;
			if (ShutDownServer)
				{	close(s);
			      if (session->Socket >= 0) close(session->Socket);
					cerr << "Ending TCP server thread\n" << flush;
               delete session;
					pthread_exit(0);
				}

			if (session->Socket < 0)
				{ perror( "Error in accepting a connection");
              delete session;
				} else
				
              
					if(	pthread_create(&SessionThread,NULL,TCPSessionThread,session) != 0)
					  {
	 					  cerr << "Error creating new client thread.\n" << flush;
						  shutdown(session->Socket,2);
						  rc = close(session->Socket);      // Close it if thread didn't start
                    delete session;
						  if (rc < 0) perror("Session Thread close()");
					  }else   //session will be deleted in TCPSession Thread
							pthread_detach(SessionThread);	 //run session thread DETACHED!
                              
					memset(&client,0,sizeof(client));
		}

   return 0;

}
//----------------------------------------------------------------------
//This thread listens to a UDP port and sends all packets heard to all
//logged on clients unless the destination call is "TNC" it sends it 
//out to RF.

void *UDPServerThread(void *p)
{

#define UDPSIZE 256
	int sockint,s,i,rc, namelen;
   unsigned client_address_size;
	struct sockaddr_in client, server;
	char buf[UDPSIZE+3],szLog[UDPSIZE+50];
   UdpParams* upp = (UdpParams*)p;
	int UDP_Port = upp->ServerPort;   //UDP port set in aprsd.conf
   char *CRLF = "\r\n";

  
   upp->pid = getpid();

   /*
    * Create a datagram socket in the internet domain and use the
    * default protocol (UDP).
    */
   if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   {
       perror("Failed to create datagram socket");
       ShutDownServer = TRUE;
		 return NULL;
   }

   /*
    *
    * Bind my name to this socket so that clients on the network can
    * send me messages. (This allows the operating system to demultiplex
    * messages and get them to the correct server)
    *
    * Set up the server name. The internet address is specified as the
    * wildcard INADDR_ANY so that the server can get messages from any
    * of the physical internet connections on this host. (Otherwise we
    * would limit the server to messages from only one network interface)
    */
   server.sin_family      = AF_INET;   	/* Server is in Internet Domain */
   server.sin_port        = htons(UDP_Port) ;/* 0 = Use any available port       */
   server.sin_addr.s_addr = INADDR_ANY;	/* Server's Internet Address    */

   if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
   {
       perror("Datagram socket bind error");
       ShutDownServer = TRUE;
		 return NULL;

   }

  cout << "UDP Server listening on port " << UDP_Port << endl << flush;


  for(;;)		  //Loop forever
  {
   
   client_address_size = sizeof(client);

   i = recvfrom(s, buf, UDPSIZE, 0, (struct sockaddr *) &client, &client_address_size) ; //Get client udp data

   
   BOOL sourceOK = FALSE;
   int n=0;
      do                   //look for clients IP address in list of trusted addresses.
        {   long maskedTrusted = Trusted[n].sin_addr.s_addr & Trusted[n].sin_mask.s_addr;
            long maskedClient = client.sin_addr.s_addr & Trusted[n].sin_mask.s_addr;
            if(maskedClient == maskedTrusted) sourceOK = TRUE;
            n++;
      }while((n < nTrusted) && (sourceOK == FALSE)) ;

   if(sourceOK && configComplete && (i > 0)){

      if(buf[i-1] != '\n') strcat(buf,CRLF);  //Add a CR/LF if not present

      ostrstream log(szLog,UDPSIZE+50);
      log   <<	inet_ntoa(client.sin_addr)
			   <<	": " << buf
			   <<	ends;
           
      WriteLog(szLog,UDPLOG);



      aprsString* abuff = new aprsString(buf,SRC_UDP,srcUDP,inet_ntoa(client.sin_addr),"UDP");

      
     //printhex(abuff->c_str(),strlen(abuff->c_str())); //debug

      if  (abuff->ax25Dest.compare("TNC") == 0  )   //See if it's  data for the TNC
      {
            
            tncQueue.write(abuff,SRC_UDP);       //Send remaining data from ourself to the TNC
                                                 
      }else
         {
            
            sendQueue.write(abuff,0);  //else send data to all users. 
         }                             //Note that abuff is deleted in History list expire func.
	
   }


    

    if (ShutDownServer)
				{	close(s);
			      pthread_exit(0);
				}

	}

  return NULL;

 }

//----------------------------------------------------------------------

/* Receive a line of ASCII from "sock" . Nulls and line feeds are
removed.  End of line is determined by CR .  A LF is appended before
the string is returned.  If no data is received in "timeoutMax" seconds
it returns int Zero else it returns the number of characters in the received
string.
*/

int recvline(int sock, char *buf, int n, int *err,int timeoutMax)
{

   int c,lastch;
   int i,BytesRead ,timeout;

   *err = 0;
   BytesRead = 0;

   timeout = timeoutMax;

do
	{	c = -1;

		i = recv(sock,&c,1,0);		//get 1 byte into c
		if (i == -1)
				{
					*err = errno;		
					if (*err != EWOULDBLOCK)
							{	BytesRead = 0;	  //exit on errors other than EWOULDBLOCK
								i = 0;
                        
							}

                sleep(1);  // Wait 1 sec. Don't hog cpu while in loop awaiting data
					 if(timeout-- <= 0) i = 0;  //Force exit if timeout

                //cout << timeout << " Waiting...\n";  //debug code
				}

       if(i == 1) {  c &= 0x7f ;
                     if ((BytesRead < n) && (c != 0x0a) && (c != '\0'))  //reject LF and NULL chars
                        { buf[BytesRead] = (char)c;
                          BytesRead++;
                          timeout = timeoutMax;
                        }
                  }

       

  }while ((i != 0) && (c != 0x0d)   );
           
              
  if ((BytesRead > 0) && ( i > 0) )		// 1 or more bytes needed
        {                                           
            i = BytesRead ;
            buf[i++] = (char)0x0a;   //add a line feed
            buf[i++] = 0;           //Stick a NULL on the end.
            return i-1;
                                         
         }
  return i;
               
 
 
}
  
  //---------------------------------------------------------------------------------------------

/* This thread connects to another aprsd, IGATE or APRServe machine  */

 void *TCPConnectThread(void *p)
{

	int	s,i,rc,length,state;
   int clientSocket;
   int data;
   SessionParams session;
   pthread_t SessionThread;
   struct hostent *hostinfo = NULL;
	struct sockaddr_in host;
	char buf[SBUFSIZE];
   char logonBuf[255];
   char szLog[MAX];
	int retryTimer, optval;
   ConnectParams *pcp = (ConnectParams*)p;
   int err;
    

   pcp->pid = getpid();

   char szRemSock[10];
   strstream ss(szRemSock,10);
   ss << pcp->RemoteSocket << ends;            //ascii version of remote socket number


   retryTimer = 60;      //time between connection attemps in seconds

   

   do{   state = 0;

      
      hostinfo = gethostbyname2(pcp->RemoteName,AF_INET);
      
      if(!hostinfo){
         cerr << "Can't resolve host name: " << pcp->RemoteName << endl;
         perror("gethostbyname error: ");
         
      }else state = 1;
      
      if(state == 1)
         {  clientSocket = socket(AF_INET,SOCK_STREAM, 0);

            host.sin_family = AF_INET;
            host.sin_port = htons(pcp->RemoteSocket);
            host.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;
            length = sizeof(host);

            
                       
            rc = connect(clientSocket,(struct sockaddr *)&host, length);
            if(rc == -1){
               ostrstream os(szLog,MAX);
               os << "Connection attempt failed " << pcp->RemoteName
                  << " " << pcp->RemoteSocket << ends;
               
               WriteLog(szLog,MAINLOG);

               {
                  char* cp = new char[256];
                  ostrstream msg(cp,256);
                  msg  <<  szLog << endl << ends;
                  conQueue.write(cp,0);
               }


              


               state = 0;
            } else 
               {  state++;
                  ostrstream os(szLog,MAX);
                  os << "Connected to " << pcp->RemoteName
                     << " " << pcp->RemoteSocket << ends;
                  
                  WriteLog(szLog,MAINLOG);
                  
                  char* cp = new char[256];
                  ostrstream msg(cp,256);
                  msg  <<  szLog << endl << ends;
                  conQueue.write(cp,0);           //cp deleted in queue reader
                 
               }
         }

      if (state == 2){

         data = 1;  //Set socket for non-blocking
	      ioctl(clientSocket,FIONBIO,(char*)&data,sizeof(int));

         int optval = 1;         //Enable keepalive option
         setsockopt(clientSocket,SOL_SOCKET,SO_KEEPALIVE,(char*)&optval,sizeof(int));


         /* If user and password are supplied we will send our Internet user and TNC
            data to the IGATE, otherwise we just read data from the IGATE.
          */
         if (pcp->user)
            {  //cout << pcp->user << " " << pcp->pass << endl << flush;
               ostrstream logon(logonBuf,254);   //Build logon string
               logon << "user "
                     << pcp->user
                     << " pass "
                     << pcp->pass
                     << " vers "
                     << VERS
                     << "\r\n"
                     << ends;

               pthread_mutex_lock(pmtxSend);
               rc = send(clientSocket,logonBuf,strlen(logonBuf),0);//Send logon string to IGATE or Hub

               if (!AddSession(clientSocket,pcp->EchoMask))   //Add this to list of sockets to send on
                  {  cerr << "Can't add IGATE to session list .";
                     WriteLog("Failed to add IGATE to session list",MAINLOG);
                  }
               pthread_mutex_unlock(pmtxSend);
            }

        
         do{
         retryTimer = 60;                 //Retry connection in 60 seconds if it breaks
        
         rc = recvline(clientSocket,buf,SBUFSIZE,&err, 300);  //300 sec (5 min) timeout value

         if (rc > 0)
            {
               BOOL sentOnRF=FALSE;
               aprsString atemp(buf,clientSocket,srcIGATE);

               if(atemp.aprsType == APRSQUERY){ /* non-directed query ? */
                       queryResp(SRC_IGATE,&atemp);   /* yes, send our response */
                    }
                    
                    //cout << atemp << endl;
                    //cout << atemp.stsmDest << "|" << szServerCall << "|" << atemp.aprsType << endl;
                    
                    if((atemp.aprsType == APRSMSG)
                      && (atemp.msgType == APRSMSGQUERY)){    /* is this a directed query message? */

                      if((stricmp(szServerCall, atemp.stsmDest.c_str()) == 0)
                           || (stricmp("aprsd",atemp.stsmDest.c_str()) == 0)
                           || (stricmp("igate",atemp.stsmDest.c_str()) == 0)){ /* Is query for us? */
                          queryResp(SRC_IGATE,&atemp);   /*Yes, respond. */
                       }
                      
                    }




               //One of the stations in the gate2rf list?
               BOOL RFalways = find_rfcall(atemp.ax25Source,rfcall);
                  
               TotalIgateChars += rc;

               /* Send it on RF if it's 3rd party msg AND TCPXX is not in the path.
                  The sendOnRF() function determines if the "to station" is local
                  and the "from station" is not.  It also reformats the packet in
                  station to station 3rd party format before sending.
               */
               
               if((atemp.aprsType == APRSMSG) 
                     && (atemp.tcpxx == FALSE) 
                     && configComplete 
                     && (!RFalways))
                   {
                         
                        sentOnRF = sendOnRF(atemp,pcp->RemoteName,"IGATE",srcIGATE);   // Try to send on RF
                        
                        if(sentOnRF)  //Now find the posit for this guy in the history list
                                          // and send it too.
                              { aprsString* posit = getPosit(atemp.ax25Source,srcIGATE | srcUSERVALID);
                                if(posit != NULL)
                                   { 
                                     time_t Time = time(NULL);           //get current time
                                     if((Time - posit->timeRF) >= 60*10) //posit every 10 minutes only
                                         { 
                                            timestamp(posit->ID,Time);  //Time stamp the original in hist. list

                                            posit->stsmReformat(MyCall);  // Reformat it for RF delivery
                                            tncQueue.write(posit);      //posit will be deleted elsewhere
                                            
                                         }else delete posit;

                                   } /*else  cout << "Can't find matching posit for "
                                               << atemp.ax25Source
                                               << endl
                                               << flush;     //Debug only
                                               */
                              }
                    
                    
                   }

               /* Send it on TCPIP if it's NOT a 3rd party msg 
                    OR TCPXX is in path .
               */
               /*if(configComplete && (( atemp.aprsType != APRSMSG) || (atemp.tcpxx == TRUE)))*/

               if(configComplete)  /* Send everything back out to tcpip users */
                  {  
                     
                     aprsString* inetpacket = new aprsString(buf,clientSocket,srcIGATE,pcp->RemoteName,"IGATE"); 
                     inetpacket->changePath("TCPIP*","TCPIP");


                     if(inetpacket->aprsType == APRSMIC_E)   //Reformat Mic-E packets
                               {  reformatAndSendMicE(inetpacket,sendQueue);
                                                       
                               }else
                                  sendQueue.write(inetpacket,0);  // send data to users.  
                                                                  //Note: inetpacket is deleted in DeQueue
                                 

                  } 



               if(configComplete && (atemp.aprsType == APRSMSG)) //find matching posit for 3rd party msg
                  {  
                     aprsString* posit = getPosit(atemp.ax25Source,srcIGATE | srcUSERVALID | srcTNC);
                     if(posit != NULL)
                         { 
                           posit->EchoMask = src3RDPARTY;
                           
                           sendQueue.write(posit);  //send matching posit only to msg port
                                 
                         }

                  }


               //Here's where the priviledged get their posits sent to RF full time.
               if(configComplete 
                    && RFalways 
                    && (StationLocal(atemp.ax25Source.c_str(),srcTNC) == FALSE)
                    && (atemp.tcpxx == FALSE))
                  {
                     aprsString* RFpacket = new aprsString(buf,clientSocket,srcIGATE,pcp->RemoteName,"IGATE"); 
                     RFpacket->changePath("TCPIP*","TCPIP");

                     
                     if(RFpacket->aprsType == APRSMIC_E)   //Reformat Mic-E packets
                           {  
                               aprsString* posit = NULL;                                                                              
                               aprsString* telemetry = NULL;                                            
                               RFpacket->mic_e_Reformat(&posit,&telemetry);                           
                               if(posit){
                                  posit->stsmReformat(MyCall);  // Reformat it for RF delivery
                                  tncQueue.write(posit);       //Send reformatted packet on RF
                               }
                               if(telemetry) {
                                  telemetry->stsmReformat(MyCall); // Reformat it for RF delivery
                                  tncQueue.write(telemetry);      //Send packet on RF
                               }
                               
                                  
                               delete RFpacket;                                                       

                             
                           }else{
                                 RFpacket->stsmReformat(MyCall);
                                 tncQueue.write(RFpacket);  // send data to RF  
                                }                           //Note: RFpacket is deleted elsewhere
                    }
                      
             }
        

        }while(rc > 0);

         pthread_mutex_lock(pmtxSend);
         DeleteSession(clientSocket);  //Only deletes if it was in the list else does nothing
         close(clientSocket);
         pthread_mutex_unlock(pmtxSend);

         ostrstream os(szLog,MAX);
                 os << "Disconnected " << pcp->RemoteName
                    << " " << pcp->RemoteSocket 
                    << ends;
                 
                 WriteLog(szLog,MAINLOG);

                 {
                  char* cp = new char[256];
                  ostrstream msg(cp,256);
                  msg  <<  szLog << endl << ends;
                  conQueue.write(cp,0);
                 }

                 

      }

      
      sleep(retryTimer);
      retryTimer *= 2;               //Double retry time delay if next try is unsuccessful
      if (retryTimer >= (16 * 60)) retryTimer = 16 * 60;     //Limit max to 16 minutes

   }while(ShutDownServer == FALSE);
   pthread_exit(0);

}

//----------------------------------------------------------------------

BOOL sendOnRF(aprsString& atemp,  char* szPeer, char* userCall, const int src)
{
  BOOL sentOnRF = FALSE;

  
  BOOL stsmRFalways =  find_rfcall(atemp.stsmDest,stsmDest_rfcall); //Always send on RF ?

  if((atemp.tcpxx == FALSE) && (atemp.aprsType == APRSMSG))                                                         
               
    {  if(checkUserDeny(atemp.ax25Source) != '+') return FALSE; //Reject if no RF or login permitted
       
          //Destination station active on VHFand source not?                                                                                                                         
       if (((StationLocal(atemp.stsmDest.c_str(),srcTNC) == TRUE) || stsmRFalways)                                                              
           && (StationLocal(atemp.ax25Source.c_str(),srcTNC) == FALSE))                   
         {                                                                                                                          
                                                                                                                 
             aprsString* rfpacket = new aprsString(atemp.getChar(),atemp.sourceSock,src,szPeer,userCall);                                                     
             //ofstream debug("rfdump.txt");                                                                                                                       
             //debug << rfpacket->getChar << endl ;  //Debug 
             //debug.close();
             rfpacket->stsmReformat(MyCall);  // Reformat it for RF delivery
             tncQueue.write(rfpacket);        // queue read deletes rfpacket                                                       
             sentOnRF = TRUE;  


                                                                                                                                    
          }                                                                                                                         
                                                                                                                                    
    } 

  
  return sentOnRF;
                                                                                                                        
}                                                                                                                       

//----------------------------------------------------------------------
int SendFiletoClient(int session, char *szName)
{	char Line[256];
	APIRET rc;
	int n,retrys;
	char *cp;
   int throttle;


	pthread_mutex_lock(pmtxSendFile);

	ifstream file(szName);
	if (!file)
		{	cerr << "Can't open " << szName << endl << flush;
			return -1  ;
		}

	do
		{  file.getline(Line,256);	  //Read each line in file and send to client session
			if (!file.good()) break;
			if (strlen(Line) > 0)
					{	strncat(Line,"\r\n",256);
						n = strlen(Line);
                  pthread_mutex_lock(pmtxSend);
                  retrys = -0;
                  do{
						   rc = send(session,Line,n,0);
                     throttle = n * 150;
                     usleep(throttle);  // Limit max rate to about 50kbaud
                     if(rc < 0) { usleep(100000); retrys++;  } //0.1 sec between retrys
                  }while((rc < 0) && (errno == EAGAIN) && (retrys <= MAXRETRYS));
                  
						if (rc == -1)
							{  
								perror("SendFileToClient()");
                        shutdown(session,2);
								close(session);				//close the socket if error happened
                     }
						
					   pthread_mutex_unlock(pmtxSend); 

					}

		}while (file.good() && (rc >= 0));


   file.close();

	pthread_mutex_unlock(pmtxSendFile);


	return rc;

}
//----------------------------------------------------------------------


char* getStats()
{
   time_t time_now;
   int aprsStreamRate, serverLoad,tncStreamRate;
   static time_t last_time = 0;
   static ULONG last_chars = 0;
   static ULONG last_tnc_chars=0;
   double upTime;

   char *cbuf = new char[1024];

   time(&time_now);
   upTime = (double) (time_now - serverStartTime) / 3600;

   aprsStreamRate =  (TotalTncChars + TotalIgateChars 
                         + TotalUserChars - last_chars) / (time_now - last_time);

   tncStreamRate = (TotalTncChars - last_tnc_chars) / (time_now - last_time);

   serverLoad =  bytesSent / (time_now - last_time);
         
   
   ostrstream os(cbuf,1023);
	os		<< setiosflags(ios::showpoint | ios::fixed)
         << setprecision(1)
         << "#\r\n"
         << "Server Up Time    = " << upTime << " hours" << "\r\n"
			<< "Total TNC packets = " << TotalLines << "\r\n"
         << "TNC stream rate   = " << tncStreamRate << " bytes/sec" << "\r\n"
         << "Msgs gated to RF  = " << msg_cnt << "\r\n"
			<< "Connect count     = " << TotalConnects << "\r\n"
         << "Users             = " << ConnectedClients << "\r\n"
			<< "Peak Users        = " << MaxConnects << "\r\n"
			<< "APRS Stream rate  = " << aprsStreamRate << " bytes/sec" << "\r\n"
         << "Server load       = " << serverLoad << " bytes/sec" << "\r\n"
         << "History Items     = " << ItemCount << "\r\n"
         << "aprsString Objs   = " << nObjects << "\r\n"
         << "Items in InetQ    = " << sendQueue.getItemsQueued() << "\r\n"
         << "InetQ overflows   = " << sendQueue.overrun << "\r\n"
         << "TncQ overflows    = " << tncQueue.overrun << "\r\n"
         << "conQ overflows    = " << conQueue.overrun << "\r\n"
         << "charQ overflow    = " << charQueue.overrun << "\r\n"
         << "Hist. dump aborts = " << dumpAborts << "\r\n"
			<< ends;

   last_time = time_now;
   last_chars = TotalTncChars + TotalIgateChars + TotalUserChars;
   last_tnc_chars = TotalTncChars;

   bytesSent = 0;  //Reset this 
   
  return cbuf;
}
//----------------------------------------------------------------------
void resetCounters()
{
   dumpAborts = 0;
   sendQueue.overrun = 0 ;
   tncQueue.overrun = 0 ;
   conQueue.overrun = 0 ;
   charQueue.overrun = 0;
   TotalLines = 0;
   msg_cnt = 0;
   TotalConnects = 0;
   MaxConnects = ConnectedClients;
}



//----------------------------------------------------------------------

void serverQuit(termios* initial_settings)      /* Invoked by console 'q' quit */
{
   cout << endl << "Beginning shutdown...\n";  
   WriteLog("Server Shutdown",MAINLOG);
   tcsetattr(fileno(stdin),TCSANOW,initial_settings); //restore terminal mode

	char *pSaveHistory = new char[strlen(CONFPATH) + strlen(SAVE_HISTORY)+1];
	strcpy(pSaveHistory,CONFPATH);
	strcat(pSaveHistory,SAVE_HISTORY);
	int n = SaveHistory(pSaveHistory);
   
	cout << "Saved " 
        << n 
        << " history items in " 
        << pSaveHistory 
        << endl << flush ;
   
   delete pSaveHistory;

   char *ShutDown = new char[255];
   strcpy(ShutDown,szServerCall);
	strcpy(ShutDown,szJAVAMSG);
   strcat(ShutDown,MyLocation);
   strcat(ShutDown," ");
   strcat(ShutDown,szServerID);
   strcat(ShutDown," shutting down.  Bye.\r\n");
   aprsString* abuff = new aprsString(ShutDown,SRC_INTERNAL,srcTNC);
   //cout << abuff->c_str() << endl;
   sendQueue.write(abuff,0);
   delete ShutDown;
   sleep(1);
  
   if(tncPresent){
   char *pRestore = new char[strlen(CONFPATH) + strlen(TNC_RESTORE) + 1];
	strcpy(pRestore,CONFPATH);
	strcat(pRestore,TNC_RESTORE);
   
   SendFiletoTNC(pRestore);
   delete pRestore;
	AsyncClose() ;
   }

   ShutDownServer = TRUE;
   
   sleep(2);
   exit(0);

	
}

//---------------------------------------------------------------------
int serverConfig(char* cf)
{
  const int maxToken=32;
  int nTokens ;
  char Line[256];
  string  cmd;
  APIRET rc;
  int n,m=0;
  string RXwhite = " \t\r\n";

  rfcall_idx = 0;
  posit_rfcall_idx = 0;
  stsmDest_rfcall_idx = 0;
  for(int i=0; i < MAXRFCALL; i++) 
      {   rfcall[i] = NULL;         //clear the rfcall arrays
          posit_rfcall[i] = NULL;
          stsmDest_rfcall[i] = NULL;
      }

  cout << "Reading " << cf << endl << flush;
  
  ifstream file(cf);
  if (!file)
     {	cerr << "Can't open " << cf << endl << flush;
        return -1  ;
     }

  do
     {  file.getline(Line,256);	  //Read each line in file 
        if (!file.good()) break;
        n = 0;
        if (strlen(Line) > 0)
             { if(Line[0] != '#')  //Ignore comments
               {  string sLine(Line);
                  string token[maxToken];
                  nTokens = split(sLine, token, maxToken, RXwhite);  //Parse into tokens

                  for(int i = 0 ;i<nTokens;i++) cout << token[i] << " " ;
                  cout << endl << flush;
                  upcase(token[0]);
                  cmd = token[0];

                  
                  if((cmd.compare("TNCPORT") == 0) && (nTokens >= 2))
                     {  szComPort = strdup(token[1].c_str());
                        n = 1;
                     }

                  if((cmd.compare("UDPPORT") == 0) && (nTokens >= 2))
                     {  upUdpServer.ServerPort = atoi(token[1].c_str());
                        n = 1;
                     }

                  if((cmd.compare("TRUST") == 0) && (nTokens >= 2) && (nTrusted < maxTRUSTED))
                     {  int rc = inet_aton(token[1].c_str(),&Trusted[nTrusted].sin_addr);
                        if(nTokens >= 3) 
                           inet_aton(token[2].c_str(),&Trusted[nTrusted].sin_mask);
                                    else Trusted[nTrusted].sin_mask.s_addr = 0xffffffff;
                        if (rc ) nTrusted++; else Trusted[nTrusted].sin_addr.s_addr = 0;
                        n = 1;
                     }
                        

                  if(cmd.compare("IGATE") == 0)
                  {  if (nTokens >= 3)
                        {  cpIGATE[m].RemoteName = strdup(token[1].c_str());
                           cpIGATE[m].RemoteSocket = atoi(token[2].c_str());
                        }
                      if(nTokens >= 5)
                           {  cpIGATE[m].user = strdup(token[3].c_str());        
                              cpIGATE[m].pass = strdup(token[4].c_str());    
                              cpIGATE[m].EchoMask =  srcUSERVALID                 
                                                     + srcUSER                    
                                                     + srcTNC                     
                                                     + srcUDP                     
                                                     + srcSYSTEM;                 
                            }else{                                                
                                 cpIGATE[m].user = NULL;                          
                                 cpIGATE[m].pass = NULL;                          
                                 cpIGATE[m].EchoMask = 0;  //Echo nothing if no logon info                       
                              }                                                   
                      m++;
                      nIGATES = m;
                      n = 1;
                                    
                  }

                  if(cmd.compare("LOCALPORT") == 0)    //provides local TNC data only
                         {  
                           spLocalServer.ServerPort = atoi(token[1].c_str());  //set server port number
                           spLocalServer.EchoMask =   srcUDP
                                                      + srcTNC   /*Set data sources to echo*/
                                                      + srcSYSTEM  
                                                      + sendHISTORY;    
                     
                           
                            n = 1;
                         }

                  if(cmd.compare("RAWTNCPORT") == 0)    //provides local TNC data only
                        {  
                          spRawTNCServer.ServerPort = atoi(token[1].c_str());  //set server port number
                          spRawTNCServer.EchoMask =   srcTNC   /*Set data sources to echo*/
                                                      + sendDUPS  //No dup filtering
                                                      + wantRAW;  //RAW data
                                                         
                    
                          
                           n = 1;
                        }



                  if(cmd.compare("MAINPORT") == 0)       //provides all data
                         {  
                           spMainServer.ServerPort = atoi(token[1].c_str());  //set server port number
                           spMainServer.EchoMask = srcUSERVALID
                                                      + srcUSER
                                                      + srcIGATE
                                                      + srcUDP 
                                                      + srcTNC 
                                                      + srcSYSTEM 
                                                      + sendHISTORY;    
                     
                           
                            n = 1;
                         }


                  if(cmd.compare("MAINPORT-NH") == 0)       //provides all data but no history dump
                         {  
                           spMainServer_NH.ServerPort = atoi(token[1].c_str());  //set server port number
                           spMainServer_NH.EchoMask = srcUSERVALID
                                                      + srcUSER
                                                      + srcIGATE
                                                      + srcUDP 
                                                      + srcTNC 
                                                      + srcSYSTEM; 
                                                      
                           
                            n = 1;
                         }


                  if(cmd.compare("LINKPORT") == 0)       //provides local TNC data + logged on users
                        {
                           spLinkServer.ServerPort = atoi(token[1].c_str()); 
                           spLinkServer.EchoMask = srcUSERVALID
                                                   + srcUSER
                                                   + srcTNC 
                                                   + srcUDP 
                                                   + srcSYSTEM; 
                           n = 1;

                        }

                  if(cmd.compare("MSGPORT") == 0)
                        {  spMsgServer.ServerPort  = atoi(token[1].c_str()); 
                           spMsgServer.EchoMask = src3RDPARTY;                                                   + srcSYSTEM; 
                           n = 1;

                        }


                  if (cmd.compare("MYCALL") == 0)
                        {
                           MyCall = strdup(token[1].c_str());
                           if(strlen(MyCall) > 9) MyCall[9] = '\0';  //Truncate to 9 chars.
                           n = 1;
                        }

                  if (cmd.compare("MYEMAIL") == 0)
                        {
                           MyEmail = strdup(token[1].c_str());
                           n = 1;
                        }
                  
                  if (cmd.compare("SERVERCALL") == 0)
                        {
                           
                           szServerCall = strdup(token[1].c_str());
                           if(strlen(szServerCall) > 9) szServerCall[9] = '\0';  //Truncate to 9 chars.
                           n = 1;
                        }


                  if(cmd.compare("MYLOCATION") == 0)
                        {  MyLocation = strdup(token[1].c_str());
                           n = 1;
                        }


                  if (cmd.compare("MAXUSERS") == 0)   //Set max users of server.
                     {  int mc = atoi(token[1].c_str());
                        if (mc > 0) MaxClients = mc;
                        n = 1;
                     }

                  if (cmd.compare("EXPIRE") == 0)     //Set time to live for history items (minutes)
                     {  int ttl = atoi(token[1].c_str());
                        if (ttl > 0) ttlDefault = ttl;
                        n = 1;
                     }

                  if (cmd.compare("RF-ALLOW") == 0) //Allow internet to RF message passing.
                     {  upcase(token[1]);
                        if(token[1].compare("YES") == 0 ) RF_ALLOW = TRUE; else RF_ALLOW = FALSE;
                        n = 1;
                     }

                  if (cmd.compare("IGATEMYCALL") == 0) //Allow igating packets from "MyCall"
                    {  upcase(token[1]);
                       if(token[1].compare("YES") == 0 ) igateMyCall = TRUE; else igateMyCall = FALSE;
                       n = 1;
                    }
                   

                  if (cmd.compare("GATE2RF") == 0)   //Call signs of users always gated to RF
                     {   for(int i=1; i < nTokens;i++)
                           {  
                              string* s = new string(token[i]);
                              if(rfcall_idx < MAXRFCALL) rfcall[rfcall_idx++] = s; //add it to the list
                           }
                         n = 1;
                     }

                  if (cmd.compare("POSIT2RF") == 0)   //Call sign posits gated to RF every 16 minutes
                     {   for(int i=1; i < nTokens;i++)
                           {  
                              string* s = new string(token[i]);
                              if(posit_rfcall_idx < MAXRFCALL) posit_rfcall[posit_rfcall_idx++] = s; //add it to the list
                           }
                         n = 1;
                     }

                  if (cmd.compare("MSGDEST2RF") == 0)   //Destination call signs
                                                         // of station to station messages
                     {   for(int i=1; i < nTokens;i++)   //always gated to RF
                           {  
                              string* s = new string(token[i]);
                              if(stsmDest_rfcall_idx < MAXRFCALL) 
                                   stsmDest_rfcall[stsmDest_rfcall_idx++] = s; //add it to the list
                           }
                         n = 1;
                     }


                  if (cmd.compare("ACKREPEATS") == 0)   //How many extra ACKs to send to tnc
                   {    int mc = atoi(token[1].c_str());
                        if (mc < 0) { mc = 0; cout << "ACKREPEATS set to ZERO\n";}
                        if (mc > 9) { mc = 9; cout << "ACKREPEATS set to 9\n";}
                        ackRepeats = mc;
                        n = 1;
                   }

                  if (cmd.compare("ACKREPEATTIME") == 0)   //Time in secs between extra ACKs
                  {     int mc = atoi(token[1].c_str());
                        if (mc < 1) { mc = 1; cout << "ACKREPEATTIME set to 1 sec.\n";}
                        if (mc > 30) { mc = 30; cout << "ACKREPEATTIME set to 30 sec.\n";}
                        ackRepeatTime = mc;
                        n = 1;
                  }

                  if (cmd.compare("NETBEACON") == 0)   //Internet Beacon text
                  {     
                     if(nTokens > 1){
                        NetBeaconInterval = atoi(token[1].c_str());//Set Beacon Interval in minutes
                        if(nTokens > 2){
                           string s = token[2]; 
                           for(int i = 3 ;i<nTokens;i++) s = s + " " + token[i] ;
                           NetBeacon = strdup(s.c_str());
                        }
                     }
                     n = 1;
                  }

                  if (cmd.compare("TNCBEACON") == 0)   //TNC Beacon text
                 {     
                    if(nTokens > 1){
                       TncBeaconInterval = atoi(token[1].c_str()); //Set Beacon Interval in minutes  
                       if(nTokens > 2){
                          string s = token[2]; 
                          for(int i = 3 ;i<nTokens;i++) s = s + " " + token[i] ;
                          
                          TncBeacon = strdup(s.c_str());
                       }
                    }
                    n = 1;
                 }

                  if (cmd.compare("TNCPKTSPACING") == 0)   //Set tnc packet spacing in ms
                 {     
                    if(nTokens > 1){
                       tncPktSpacing = 1000 * atoi(token[1].c_str());// ms to microsecond conversion
                       
                    }
                    
                    n = 1;
                 }






                  if (n == 0) cout << "Unknown command: " << Line << endl << flush;   
               }
             }
        }while(file.good());

  file.close();

  return 0;
  }


//---------------------------------------------------------------------

/* FOR DEBUGGING ONLY */
/*
void segvHandler(int signum)  //For debugging seg. faults
{
   pid_t pid,ppid;
   char* err;


   pid = getpid();

   if(pid == pidlist.main) err = "aprsd main";
   if(pid == spMainServer.pid) err = "spMainServer";
   if(pid == spMainServer_NH.pid) err = "spMainServer_NH";
   if(pid == spLocalServer.pid) err = "spLocalServer";
   if(pid == spLinkServer.pid) err = "spLinkServer" ;
   if(pid == spMsgServer.pid) err = "spMsgServer";
   if(pid == upUdpServer.pid) err = "upUdpServer";
   if(pid == pidlist.SerialInp) err = "Serial input thread";
   if(pid == pidlist.TncQueue) err = "TNC Dequeue";
   if(pid == pidlist.InetQueue) err = "Internet Dequeue";
   
   char buf[256];
   ostrstream sout(buf,256);
   sout  << "A segment violation has occured in process id " 
         << pid 
         << " Thread: "
         << err
         << endl ;

   char buf2[256];
   ostrstream sout2(buf2,256);

   sout2 << "Died in "                                     
         << DBstring                             
         << " Last packet:  " << lastPacket->c_str()                                               
         << " Packet length = " << lastPacket->length()   
         << endl
         << ends;


   cout << buf << buf2;
   WriteLog(buf,"segfault.log");
   WriteLog(buf2,"segfault.log");
    
   exit(-1);
}
*/ 

//----------------------------------------------------------------------
// Send the posits of selected users (in posit_rfcall array) to
// RF every 14.9 minutes.  Call this every second.
// Stations are defined in the aprsd.conf file with the posit2rf command.
// Posits are read from the history list. 
void schedule_posit2RF(time_t t)
{
   static int ptr=0;
   static time_t last_t = 0;
   aprsString* abuff;

   if(difftime(t,last_t) < 14) return;  //return if not time yet (14 seconds)
   last_t = t;
  
   if(posit_rfcall[ptr] != NULL)
      {  
         abuff = getPosit(*posit_rfcall[ptr] , srcIGATE | srcUSERVALID | srcUSER);
         if(abuff){
            abuff->stsmReformat(MyCall);  //Convert to 3rd party format
            tncQueue.write(abuff);        //Send to TNC
         }
      }

   ptr++;                         //point to next call sign 
   if(ptr >= (MAXRFCALL-1)) ptr = 0;  //wrap around if past end of array



}

//----------------------------------------------------------------------

int daemonInit(void)
{
   pid_t pid;
   if((pid = fork()) < 0)         
      return -1 ;
   else if(pid != 0) exit(0);  /*Parent goes away*/

   /* child continues */

  
   int nulfd = open("/dev/null", O_RDWR);
   dup2(nulfd,1);     //Redirect all console I/O to /dev/null
   dup2(nulfd,2);
   dup2(nulfd,0);
   
   setsid();         //Become session leader
   chdir(HOMEDIR);   //change to the aprsd2 directory
   umask(0);         //Clear our file mode selection mask
      
   return 0;


}



//----------------------------------------------------------------------
int main(int argc, char *argv[])
{
	
	int i,rc;
	char debug;
	char buf[1024];
   char *pSaveHistory, *szConfFile;
	time_t lastSec,tNow,tLast,tLastDel, tPstats;
   void *library;

   time_t LastNetBeacon , LastTncBeacon;

   size_t stack_size;
   pthread_attr_t thread_attr;
   pid_t pid = -1;

	time_t Time = time(NULL);
   serverStartTime = Time;

   //Find and store our servers IP address in szHostIP for future reference
   union ulb{
      long l;
      char b[4];
   };
   union ulb lb;
   lb.l = gethostid();   //get out own IP address
   char c = lb.b[0] ;
   lb.b[0] = lb.b[2];  //Unpermute it!
   lb.b[2] = c;
   c = lb.b[1];
   lb.b[1] = lb.b[3];
   lb.b[3] = c;
   struct in_addr ia;
   ia.s_addr =  lb.l;    //Put it in structure that inet_ntoa() likes.
   char* cp = inet_ntoa(ia); //convert it it ascii
   szHostIP = new char[strlen(cp)+1];
   strcpy(szHostIP,cp);     //Save it.

   

	TotalConnects = TotalTncChars = TotalLines = MaxConnects = TotalIgateChars = 0;
   TotalUserChars = 0;
   bytesSent = 0;
   TotalTNCtxChars = 0;
   msg_cnt = 0;
   posit_cnt = 0;
   MyCall = "N0CLU";
   MyLocation = "NoWhere";
   MyEmail = "nobody@NoWhere.net";
   TncBeacon = NULL;
   NetBeacon = NULL;
   TncBeaconInterval = 0;
   NetBeaconInterval = 0;
   tncPktSpacing = 1500000;  //1.5 second default
   LastNetBeacon = 0;
   LastTncBeacon = 0;
   igateMyCall = TRUE;        //To be compatible with previous versions set it TRUE
   tncPresent = FALSE;
   tncMute = FALSE;
   MaxClients = MAXCLIENTS;  //Set default. aprsd.conf file will override this
   struct sigaction sa;
   ackRepeats = 2;            //Default extra acks to TNC
   ackRepeatTime = 5;         //Default time between extra acks to TNC in seconds.
   msgsn = 0;                 //Clear message serial number

   
   spLinkServer.ServerPort = 0;      //Ports are set in aprsd.conf file        
   spMainServer.ServerPort = 0;
   spMainServer_NH.ServerPort = 0;
   spLocalServer.ServerPort = 0;
   spRawTNCServer.ServerPort = 0;
   spMsgServer.ServerPort = 0;
   upUdpServer.ServerPort = 0;

   
   if(argc > 1) if(strcmp("-d",argv[1]) == 0) daemonInit();  //option -d means run as daemon

   signal(SIGPIPE,SIG_IGN);
   

   pidlist.main = getpid();
   
   /*
   memset(&sa,0,sizeof(sa));
   sa.sa_handler = segvHandler;
   if(sigaction(SIGSEGV,&sa,NULL)) perror("sigaction");
    */
	
   configComplete = FALSE;
	szComPort = NULL;				       //null string for TNC com port

   szServerCall = "aprsd";    //default server FROM CALL used in system generated pkts.

   
   szConfFile = new char[strlen(CONFPATH) + strlen(CONFFILE) + 1];
   strcpy(szConfFile,CONFPATH);
   strcat(szConfFile,CONFFILE);     //default server configuration file

   CreateHistoryList();            //Make the history linked list structure
   
	cout << SIGNON << endl << flush;

  
   /*   //DEBUG & TEST CODE
   //aprsString mic_e("K4HG-9>RU4U9T,WIDE,WIDE,WIDE:`l'q#R>/\r\n");
   aprsString mic_e("K4HG-9>RU4W5S,WIDE,WIDE,WIDE:`l(XnUU>/Steve's RX-7/Mic-E\r\n");
   aprsString* posit = NULL;
   aprsString* telemetry = NULL;
   if(mic_e.aprsType == APRSMIC_E) mic_e.mic_e_Reformat(&posit,&telemetry);
   if(posit){ posit->print(cout); delete posit;}
   if(telemetry) { telemetry->print(cout); delete telemetry;}
   
   sleep(5);
   */

   pSaveHistory = new char[strlen(CONFPATH) + strlen(SAVE_HISTORY)+1];
	strcpy(pSaveHistory,CONFPATH);
	strcat(pSaveHistory,SAVE_HISTORY);

   
   int nHist = ReadHistory(pSaveHistory);
   

  
  
  
	if (argc > 1)
	   {  
         
			if(strcmp("-d",argv[argc-1]) != 0){
            szConfFile = new char[sizeof(argv[argc-1]+1)]; 
            szConfFile = argv[argc-1];	//get optional 1st or 2nd arg which is configuration file name
         }
         
      }

   if(serverConfig(szConfFile) != 0) exit(-1);     //Read configuration file (aprsd.conf)

   //Now add a ax25 path to the Internet beacon text string

   if(NetBeacon){
      char* netbc = new char[256];
      ostrstream osnetbc(netbc,255);
      osnetbc  << szServerCall
               << szAPRSDPATH
               << NetBeacon
               << "\r\n"
               << ends;

      delete NetBeacon;
      NetBeacon = netbc;   //Internet beacon complete with ax25 path
   }

   if(TncBeacon){
      char* tncbc = new char[256];
      ostrstream ostncbc(tncbc,255);
      ostncbc  << TncBeacon
               << "\r\n"
               << ends;

      delete TncBeacon;
      TncBeacon = tncbc;  //TNC beacon (no ax25 path)
   }

  
   
	//Make the semaphores
	pmtxSendFile 	= new pthread_mutex_t;
   pmtxSend			= new pthread_mutex_t;
	pmtxAddDelSess = new pthread_mutex_t;

	
   rc = pthread_mutex_init(pmtxSendFile,NULL);
   rc = pthread_mutex_init(pmtxSend,NULL);
	rc = pthread_mutex_init(pmtxAddDelSess,NULL);


	sessions = new SessionParams[MaxClients];

	if (sessions == NULL) { cerr << "Can't create sessions pointer\n" ; return -1;}

	for(i=0;i<MaxClients;i++) { sessions[i].Socket = -1;  sessions[i].EchoMask = 0;}
	ConnectedClients = 0;

   if(spMainServer.ServerPort > 0)
      {
	      //Create Main Server thread. (Provides Local data, Logged on users and IGATE data)   
         rc = pthread_create(&spMainServer.tid, NULL,TCPServerThread,&spMainServer);
	      if (rc != 0)
		      {  cerr << "Error: Main TCPServerThread failed to start\n";
			      exit(-1);
		      }
      }


   if(spMainServer_NH.ServerPort > 0)
      {
	      //Create another Main Server thread . 
         // (Provides Local data, Logged on users and IGATE data but doesn't dump the 30 min. history)   
         rc = pthread_create(&spMainServer_NH.tid, NULL,TCPServerThread,&spMainServer_NH);
	      if (rc != 0)
		      {  cerr << "Error: Main-NH TCPServerThread failed to start\n";
			      exit(-1);
		      }
      }


   if (spLinkServer.ServerPort > 0)
      {
         //Create Link Server thread.  (Provides local TNC data plus logged on users data)
         rc = pthread_create(&spLinkServer.tid, NULL,TCPServerThread,&spLinkServer);
	      if (rc != 0)
		      {  cerr << "Error: Link TCPServerThread failed to start\n";
			      exit(-1);
		      }
      }



	if(spLocalServer.ServerPort > 0)
      {
         //Create Local Server thread  (Provides only local TNC data).
         rc = pthread_create(&spLocalServer.tid, NULL,TCPServerThread,&spLocalServer);
	      if (rc != 0)
		   {  cerr << "Error: Local TCPServerThread failed to start\n";
			   exit(-1);
		   }
      }

   if(spRawTNCServer.ServerPort > 0)
      {
         //Create Local Server thread  (Provides only local TNC data).
         rc = pthread_create(&spRawTNCServer.tid, NULL,TCPServerThread,&spRawTNCServer);
	      if (rc != 0)
		   {  cerr << "Error: RAW TNC TCPServerThread failed to start\n";
			   exit(-1);
		   }
      }



   if(spMsgServer.ServerPort > 0)
      {
         //Create message Server thread  (Provides only 3rd party message data).
         rc = pthread_create(&spMsgServer.tid, NULL,TCPServerThread,&spMsgServer);
	      if (rc != 0)
		   {  cerr << "Error: 3rd party message TCPServerThread failed to start\n";
			   exit(-1);
		   }
      }



   if(upUdpServer.ServerPort > 0)
    {   rc = pthread_create(&upUdpServer.tid, NULL,UDPServerThread,&upUdpServer);
	      if (rc != 0)
		      {  cerr << "Error: UDP Server thread failed to start\n";
			      exit(-1);
		      }
   }



  	ShutDownServer = FALSE;
	
   
  

  pthread_t  tidDeQueuethread;
  rc = pthread_create(&tidDeQueuethread, NULL,DeQueue,NULL);
  if (rc != 0)
		{  cerr << "Error: DeQueue thread failed to start\n";
			exit(-1);
		}
 /*
 pthread_t  tidTNC_DeQueuethread;
 rc = pthread_create(&tidTNC_DeQueuethread, NULL,TNC_DeQueue,NULL);
 if (rc != 0)
     {  cerr << "Error: TNC_DeQueue thread failed to start\n";
        exit(-1);
     }

  */



   if(szComPort != NULL)      //Initialize TNC Com port if specified in config file
      {
	      cout  << "Opening serial port device "
	            << szComPort                    
	            << endl
               << flush;                        
                                               
	      if ((rc = AsyncOpen(szComPort)) != 0) 
	         {  sleep(2);                       
	            return -1;                      
	         }                                  

         cout << "Setting up TNC\n" << flush;

         char *pInitTNC = new char[strlen(CONFPATH) + strlen(TNC_INIT) +1];
	      strcpy(pInitTNC,CONFPATH);
	      strcat(pInitTNC,TNC_INIT);


         SendFiletoTNC(pInitTNC);	//Setup TNC from initialization file
         tncPresent = TRUE;

      }else cout << "TNC com port not defined.\n" << flush;


   if (RF_ALLOW) cout << "Internet to RF data flow is ENABLED\n" ; 
           else cout << "Internet to RF data flow is DISABLED\n";

   
   cout << "Server IP Address = " << szHostIP << endl << flush;
   WriteLog("Server Start",MAINLOG);
	cout << "Server Started\n" << flush;

   

   if (nIGATES > 0) cout << "Connecting to IGATEs and Hubs now..." << endl << flush;   
   for(i=0;i<nIGATES;i++)
      {  rc = pthread_create(&cpIGATE[i].tid, NULL,TCPConnectThread,&cpIGATE[i]);
         if(rc == 0) pthread_detach(cpIGATE[i].tid);
         sleep(2);  //Spread out the connect requests in 2 second intervals
      }

   
	   

	Time = time(NULL);
	tNow = Time;
	tLast = Time;
   tLastDel = Time ;
	tPstats = Time;

   configComplete = TRUE;

   if(TncBeacon)
      cout << "TncBeacon every " << TncBeaconInterval << " minutes : " << TncBeacon << endl;
   if(NetBeacon)
       cout << "NetBeacon every " << NetBeaconInterval << " minutes : " << NetBeacon << endl;

   

   struct termios initial_settings, new_settings;
   tcgetattr(fileno(stdin),&initial_settings);
   new_settings = initial_settings;
   new_settings.c_lflag &= ~ICANON;
   new_settings.c_lflag &= ~ISIG;
   new_settings.c_cc[VMIN] = 0;
   new_settings.c_cc[VTIME] = 5;		 //.5 second timeout for input chars

   tcsetattr(fileno(stdin),TCSANOW,&new_settings);

   

  do
	{
	  
    usleep(100000);			  //do this loop 10 times each second

    if(msgsn > 9999) msgsn = 0;

    while(conQueue.ready())         //Data for Console?
       { char *cp = (char*)conQueue.read(NULL);    //Yes, read and print it.
         if(cp){  printf("%s",cp);
                  strcat(cp,"\r");
                  aprsString* monStats = new aprsString(cp,SRC_INTERNAL,srcSTATS);
                  sendQueue.write(monStats);
                  
                  delete cp;
               }
       }

    char ch = fgetc(stdin);

    switch(ch)
       {
         case  'r': resetCounters();
                     break;
         case 0x03: 
         case 'q' :
                     serverQuit(&initial_settings);
      }
   
	 lastSec = Time;
    Time = time(NULL);
    if(difftime(Time,lastSec) > 0) schedule_posit2RF(Time);  //Once per second


    if(difftime(Time,LastNetBeacon) >= NetBeaconInterval * 60){  //Send Internet Beacon text
       LastNetBeacon = Time;
       if((NetBeacon) && (NetBeaconInterval > 0)){
         aprsString* netbc = new aprsString(NetBeacon,SRC_INTERNAL,srcSYSTEM);
         sendQueue.write(netbc);
       }
    }

    if(difftime(Time,LastTncBeacon) >= TncBeaconInterval * 60){  //Send TNC Beacon text
       LastTncBeacon = Time;
       if((TncBeacon) && (TncBeaconInterval > 0)){
        aprsString* tncbc = new aprsString(TncBeacon,SRC_INTERNAL,srcSYSTEM);
        tncQueue.write(tncbc);
      }

    }


 /*debug*/
   /*
	 if (Time != lastSec)	  //send the test message for debugging
		 {
		      char *test = "W4ZZ>APRS,TCPIP:!BOGUS PACKET ";
            char testbuf[256];
            ostrstream os(testbuf,255);
            os << test << Time << "\r\n" << ends;
            aprsString* inetpacket = new aprsString(testbuf,0,srcTNC);
            inetpacket->changePath("TCPIP*","TCPIP") ;     
            tncQueue.write(inetpacket); //note: inetpacket is deleted in DeQueue

		 }
  	 */
    
	if ((Time - tPstats) > 60)			// 1 minute
		{	if (WatchDog < 2)
					{	cerr 	<< "** No data from TNC during previous 2 minutes **\n"
								<< flush;

					  								
					}

          aprsString* tickle = new aprsString("# Tickle\r\n",SRC_INTERNAL,srcSYSTEM);
          sendQueue.write(tickle);
          
		    tPstats = Time;
			 WatchDog = 0;				//Serial port reader increments this with each rx line.
          char *stats = getStats();
          cout << stats << flush;
          //getProcStats();
          aprsString* monStats = new aprsString(stats,SRC_INTERNAL,srcSTATS);
          sendQueue.write(monStats);
          delete stats;

          /*
          for(int i=0;i<MaxClients;i++)
		         cout << setw(4) << sessions[i].Socket  ;
          
          cout << endl;
          */
		}

	
	if ((Time - tLastDel) > 300 )		//do this every 5 minutes.
			{														//Remove old entrys from history list
				int di = DeleteOldItems(5);
            if (di > 0) cout << " Deleted " << di << " expired items from history list" << endl << flush;
			   tLastDel = Time;
         }

	if ((Time - tLast) > 900)		//Save history list every 15 minutes
		{
		  SaveHistory(pSaveHistory);
		  tLast = Time;
		}



  	}while (1==1);		// ctrl-C to quit

    return 0;


}
