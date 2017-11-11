/* /* serialp.cpp  */

	
/* 
 Copyright 1997 by Dale A. Heatherington, WA4DSY


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
*/

#define _REENTRANT 1

#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <termios.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <fstream.h>
#include <iostream.h>
#include <strstream.h>
#include <iomanip.h>


#include "constant.h"
#include "serialp.h"
#include "utils.h"
#include "cpqueue.h"
#include "history.h"
#include "queryResp.h"

int WriteLog(const char *cp, char *LogFile);


struct pidList{
   pid_t main;
   pid_t SerialInp;
   pid_t TncQueue;
   pid_t InetQueue;
};




extern ULONG WatchDog, tickcount, TotalConnects, TotalTncChars, TotalLines, MaxConnects ;
extern BOOL ShutDownServer;
extern cpQueue sendQueue;
extern cpQueue charQueue;
extern const int srcTNC;
extern BOOL configComplete;
extern BOOL igateMyCall;
extern pidList pidlist;


extern const int srcTNC;  
extern const int srcUSER; 
extern const int srcUSERVALID ;
extern const int srcIGATE; 
extern const int srcSYSTEM;
extern const int srcUDP; 
extern const int srcHISTORY; 
extern const int src3RDPARTY; 
extern const int sendHISTORY;

extern char* szServerCall;


int ttySread ;
int ttySwrite;

pthread_t  tidReadCom;
extern char* MyCall;

termios newSettings, originalSettings;
speed_t newSpeed, originalOSpeed, originalISpeed;
pthread_mutex_t *pmtxWriteTNC;

char txbuffer[260];
int txrdy;

int CloseAsync, threadAck;
BOOL  TncSysopMode;         /*Set true when sysop wants direct TNC access */

//cpQueue sendTNC(256);		/* create a queue for sending data to TNC */

//---------------------------------------------------------------------

/* Sets various parameters on a COM port for use with TNC*/

int SetupComPort( int fIn, int fOut)
 {

   
   tcgetattr(fIn,&originalSettings);
   newSettings = originalSettings;

   //newSettings.c_lflag &= ~ICANON;
   //newSettings.c_lflag &= ~ECHO;

   //newSettings.c_cflag &= ~ISIG;

   newSettings.c_lflag = 0;
   newSettings.c_cc[VMIN] = 0;
   newSettings.c_cc[VTIME] = 1;		 //.1 second timeout for input chars
   newSettings.c_cflag = CLOCAL | CREAD | CS8 ;
   newSettings.c_oflag = 0;
   newSettings.c_iflag = IGNBRK|IGNPAR ;

	//newSettings.c_cflag |= CLOCAL;
   //newSettings.c_cflag &= ~(CS5 | CS6 | CS7) ;
   //newSettings.c_cflag |=  CS8;
   //newSettings.c_cflag &=  ~PARENB ;

	//newSettings.c_iflag &= ~IXOFF	;
  // newSettings.c_iflag &= ~IXON	;
	//newSettings.c_iflag &= ~IGNCR	;
   //newSettings.c_iflag &= ~ICRNL	;

   //newSettings.c_oflag &= 	~OPOST;
   //newSettings.c_oflag &= 	~ONLCR;
   //newSettings.c_oflag &= 	~OCRNL;

	cfsetispeed(&newSettings, B9600);
   cfsetospeed(&newSettings, B9600);


   if(tcsetattr(fIn, TCSANOW, &newSettings) != 0)
   {   cerr << " Error: Could not set input serial port attrbutes\n";
	    return -1;
   }

   if(tcsetattr(fOut, TCSANOW, &newSettings) != 0)
   {  cerr << " Error: Could not set output serial port attrbutes\n";
		return -1;
   }

	return 0;

 }


//--------------------------------------------------------------------
int AsyncOpen(char *szPort)
{
  APIRET	rc;
  ULONG		ulAction;
  char		ch;

  TncSysopMode = FALSE;
  txrdy = 0;

 ttySwrite = open(szPort,O_WRONLY | O_NOCTTY);
 if(ttySwrite == -1)
		{
			cerr 	<< "Error: Could not open serial port in WRITE mode: "
					<< szPort << " [" << sys_errlist[errno] << "]" << endl;
		  	return -1;
      }
 
 ttySread = open( szPort, O_RDONLY | O_NOCTTY);

  if (ttySread == -1)
      {
         cerr 	<< "Error: Could not open serial port in READ mode: "
					<< szPort << " [" << sys_errlist[errno] << "]" << endl;
		  	return -1;
      }



 if ((rc = SetupComPort(ttySread, ttySwrite)) != 0)
	       {  cerr << "Error in setting up COM port rc=" << rc << endl << flush;
				return -2;
	       }

 pmtxWriteTNC = new pthread_mutex_t;
 pthread_mutex_init(pmtxWriteTNC,NULL);

 CloseAsync = FALSE;
 threadAck = FALSE;

 /* Now start the serial port reader thread */

 rc = pthread_create(&tidReadCom, NULL, ReadCom, NULL);
 if (rc)
	{  cerr << "Error: ReadCom thread creation failed. Error code = " << rc << endl;
      CloseAsync = TRUE;
  	}

 return 0;

 }

//--------------------------------------------------------------------
 int AsyncClose(void)
 {

    if(tcsetattr(ttySread, TCSANOW, &originalSettings) != 0)
   {   cerr << " Error: Could not reset input serial port attrbutes\n";
	    
   }

   if(tcsetattr(ttySwrite, TCSANOW, &originalSettings) != 0)
   {  cerr << " Error: Could not reset input serial port attrbutes\n";
		
   }
   CloseAsync = TRUE;                      //Tell the read thread to quit
   while(threadAck = FALSE) usleep(1000);   //wait till it does
	close(ttySwrite);
	delete pmtxWriteTNC;
   return close(ttySread);     //close COM port


}

//-------------------------------------------------------------------
/*Write NULL terminated string to serial port */
int WriteCom(char *cp)
{
   int rc;

	pthread_mutex_lock(pmtxWriteTNC);
   //size_t len = strlen(cp);
  // printhex(cp,len);       //debug only
   //rc = fwrite(cp,1,len,ttySwrite);
	//fflush(ttySwrite);
   strncpy(txbuffer,cp,256);
   //int txlen = strlen(txbuffer);
   //if(txbuffer[txlen-1] == '\n') txbuffer[txlen-1] = '\0'; //remove line feed 
   txrdy = 1;
   while(txrdy) usleep(10000);   //The ReadCom thread will clear txrdy when it takes data

	pthread_mutex_unlock(pmtxWriteTNC);

	return rc;

}

//-------------------------------------------------------------------
 /*
//Write a single character to serial port 
int WriteCom(char c)
{
   int rc;
   if (c == 0x0a) return 1;  //TNCs don't want line feeds!
	pthread_mutex_lock(pmtxWriteTNC);
   rc = fwrite(&c,1,1,ttySwrite);
   
	fflush(ttySwrite);

   //printhex(&c,1);       //debug only

	pthread_mutex_unlock(pmtxWriteTNC);

	return rc;

}

 */
//-------------------------------------------------------------------
//--------------------------------------------------------------------
// This is the COM port read thread.
// It also handles buffered writes to the COM port.

void *ReadCom(void* vp)
{
  USHORT i,j,n;
  APIRET  rc;
  unsigned char buf[BUFSIZE];
  unsigned char c;
  FILE *rxc = (FILE*)vp;
  size_t	BytesRead;
  int count = 0;
  BOOL lineTimeout;
  aprsString* abuff;


  i = 0;
  c = 0;

  pidlist.SerialInp = getpid();

while (!ShutDownServer)
	{
		 do                        //Sit here till end of line
							  				//or timeout (defined in SetupComPort() )
         {							   //and read chars from TNC into buffer.

           if(txrdy)                                  //Check for data to Transmit
              {   size_t len = strlen(txbuffer);
                  write(ttySwrite,txbuffer,len);      //Send TX data to TNC
                  txrdy = 0;                          //Indicate we sent it.
                  
              }
  
							  				
			  lineTimeout = FALSE;
           
           BytesRead = read(ttySread,&c,1);       //Non-blocking read.  100ms timeout.
                  
			             
			  if (BytesRead == 0)  //Serial input timeout
				  {
				      if(i > 0) lineTimeout = TRUE;	 // We have some data 
																//  but none 
																//  has arrived lately.

	 					if (CloseAsync)
							{	cerr << "Exiting Async com thread\n" << endl << flush;
						      threadAck = TRUE;
							   pthread_exit(0);
							}
					}

           TotalTncChars += BytesRead ;

           if (BytesRead > 0)		  //Actual serial data from TNC has arrived
				  {   //printhex(&c,1);   //debug

						if (i < (BUFSIZE-4)) buf[i++] = c; else i = 0;
                  if (TncSysopMode) 
                     {  i=1; 
                        charQueue.write((char*)c,(int)c); //single char mode...
                        //printhex(&c,1);   //debug
                     }

				  }else c = 0;


			}while (( c != 0x0a) && (c != 0x0d) && (lineTimeout == FALSE));

         WatchDog++;
         tickcount = 0;

	  		if ((i > 0) && ((buf[0] != 0x0d) && (buf[0] != 0x0a)) )
			{	
			  if(lineTimeout == FALSE)
					{	TotalLines++;
						buf[i-1] = 0x0d;
						buf[i++] = 0x0a;
                }

			  buf[i++] = '\0'; 
			  count = i;       

			   //printhex((char*)buf,i);		  //debug
				//sleep(1);

				if ((count > 0) && (!TncSysopMode) && (configComplete))
					{ 
                  abuff = new aprsString((char*)buf,SRC_TNC,srcTNC);
                  //abuff->print(cout);  //debug

                if(abuff != NULL){ //don't let a null pointer get past here!
                        

                   if(abuff->aprsType == APRSQUERY){ /* non-directed query ? */
                       queryResp(SRC_TNC,abuff);   /* yes, send our response */
                    }
                    
                    //cout << atemp << endl;
                    //cout << atemp.stsmDest << "|" << szServerCall << "|" << atemp.aprsType << endl;
                    
                    if((abuff->aprsType == APRSMSG)
                      && (abuff->msgType == APRSMSGQUERY)){    /* is this a directed query message? */

                      if((stricmp(szServerCall, abuff->stsmDest.c_str()) == 0)
                           || (stricmp("aprsd",abuff->stsmDest.c_str()) == 0)
                           || (stricmp("igate",abuff->stsmDest.c_str()) == 0)){ /* Is query for us? */
                          queryResp(SRC_TNC,abuff);   /*Yes, respond. */
                       }
                      
                    }
                   


                   if(abuff->ax25Source.compare(MyCall) == 0)
                        WriteLog(abuff->c_str(),RFLOG); //Log our own packets that were digipeated

                   if((abuff->reformatted) || ((abuff->ax25Source.compare(MyCall) == 0) && (igateMyCall == FALSE)))
                    {  
                        delete abuff;      //don't igate packets which have been igated to RF...
                        abuff = NULL;      // ... and/or originated from our own TNC
                                          
                     }else   //Not reformatted and not from our own TNC
						      {  
                           if(abuff->aprsType == APRSMSG) //find matching posit for 3rd party msg
                              {
                                 aprsString* posit = getPosit(abuff->ax25Source,srcIGATE | srcUSERVALID | srcTNC);
                                 if(posit != NULL)
                                    { 
                                       posit->EchoMask = src3RDPARTY;
                                       sendQueue.write(posit);  //send matching posit only to msg port
                               
                                    } 
                              }

                         
                           if(abuff->aprsType == APRSMIC_E)   //Reformat if it's a Mic-E packet
                                   { 
                                     reformatAndSendMicE(abuff,sendQueue);
                                   }else
                                      sendQueue.write(abuff,0);// Now put it in the Internet send queue.            
                                                               // *** abuff must be freed by Queue reader ***.  

                       
                       
                     }
                }
					}
           } 

      i = 0;		//Reset buffer pointer
		c = 0;

	} // Loop until server is shut down

return NULL;

}
		

//---------------------------------------------------------------------


int SendFiletoTNC(char *szName)
{	char Line[256];
   char *cp;


   

	ifstream file(szName);
	if (file.is_open() == FALSE)
		{	cerr << "Can't open " << szName << endl << flush;
			return -1 ;
		}


 	Line[0] = 0x03;			//Control C to get TNC into command mode
	Line[1] = '\0';
	WriteCom(Line);
	sleep(1);


	while(file.good())
		{
			file.getline(Line,256);	  //Read each line in file and send to TNC
			strncat(Line,"\r",256);

			if ((Line[0] != '*') && (Line[0] != '#') && (strlen(Line) >= 2)  ) //Reject comments
					{	cerr << Line << endl;
					   //WriteCom(Line);	
                  int len = strlen(Line);					 //Send line to TNC
                  write(ttySwrite,Line,len);
						usleep(500000);						 //sleep for 500ms between lines
                  
               }
		}

   file.close();

	return 0;

}
//----------------------------------------------------------------------

