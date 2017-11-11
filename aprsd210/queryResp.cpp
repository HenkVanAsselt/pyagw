/* queryResp.cpp */

#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#define _REENTRANT 1

#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <pthread.h>

#include <fstream.h>
#include <iostream.h>
#include <strstream.h>
#include <iomanip.h>
#include <errno.h>


#include "constant.h"
#include "utils.h"
#include "aprsString.h"

void BroadcastString(char *cp);

extern char* szServerCall ;
extern char* MyLocation ;
extern char* MyCall;
extern char* MyEmail;
extern char* szHostIP;
extern int msgsn;
extern cpQueue tncQueue;

char szAPRSDPATH[] = ">APRS,TCPIP*" ;


void queryResp(int session, const aprsString* pkt)
{  int rc,hlen;
   aprsString *rfpacket, *ackRFpacket;
   char* hostname = new char[60];
   char* cp = new char[256];
   char* cpAck = new char[256];
   ostrstream reply(cp,256);
   ostrstream ack(cpAck,256);
   BOOL wantAck = FALSE;

   rc = gethostname(hostname,60);
   if(rc != 0) strcpy(hostname,"Host_Unknown");

   //cout << pkt << endl;

   //Send ack only if there was a line number on the end of the query.
   if(pkt->acknum.length() == 0) wantAck = FALSE; else wantAck = TRUE;

   char sourceCall[] = "         ";  //9 blanks
   int i = 0;
   while((i <9) && (pkt->ax25Source.c_str()[i] != '\0')) {
      sourceCall[i] = pkt->ax25Source.c_str()[i];
      i++;
   }

   
   if(wantAck){  /* construct an ack packet */
      ack   << pkt->stsmDest << szAPRSDPATH << "::"
            << sourceCall << ":ack"
            << pkt->acknum << "\r\n"  /*use senders sequence number */
            << ends;
   }

   /* Now build the query specfic packet(s) */

   if(pkt->query.compare("IGATE") == 0){
      reply << szServerCall << szAPRSDPATH << "::"
            << sourceCall << ":" 
            << MyCall << " "
            << MyLocation << " "
            << hostname << " " 
            << szHostIP << " "
            << MyEmail << " "
            << VERS
            << "\r\n"   /* Don't send a sequence number! */
            << ends;

     
    switch(session){

    case SRC_IGATE: if(wantAck) BroadcastString(cpAck);  // To the whole aprs network
                    BroadcastString(cp);
                    break;

    case SRC_TNC:    if(wantAck){
                        ackRFpacket = new aprsString(cpAck);  //Ack reply
                        ackRFpacket->stsmReformat(MyCall);
                        tncQueue.write(ackRFpacket);
                     }

                     rfpacket = new aprsString(cp);   // Query reply                                                 
                     rfpacket->stsmReformat(MyCall);  // Reformat it for RF delivery
                     //cout << rfpacket << endl;
                     tncQueue.write(rfpacket);        // queue read deletes rfpacket

                     break;

    
    default:   if(session >= 0) {
                  if(wantAck) rc = send(session,(const void*)cpAck,strlen(cpAck),0);  //Only to one user
                  rc = send(session,(const void*)cp,strlen(cp),0);   
               }
    }
    
   }

   delete cp;
   delete cpAck;
   delete hostname;

}


//---------------------------------------

                   
