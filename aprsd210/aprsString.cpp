/* aprsString class implimentation*/

/*
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
*/



#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#define _REENTRANT 1

#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <fstream.h>
#include <iostream.h>
#include <strstream.h>
#include <iomanip.h>
#include <string.h>
#include <time.h>
#include <string>
#include <stdexcept>

#include "aprsString.h"
#include "utils.h"
#include "mic_e.h"


int aprsString::NN = 0;
long aprsString::objCount = 0;
string pathDelm(">,:");
extern const int srcTNC, src3RDPARTY;
extern int ttlDefault;

string RXwhite = " \t\n\r";




aprsString::aprsString(const char* cp, int s, int e, const char* szPeer, const char* userCall) : string(cp) 
{
   constructorSetUp(cp,s,e);
   peer = szPeer;
   user = userCall;
}
 
aprsString::aprsString(const char* cp, int s, int e) : string(cp) 
{
   peer = "";
   user = "";
   constructorSetUp(cp,s,e);
   
}

aprsString::aprsString(const char* cp) : string(cp)
{
   peer = "";
   user = "";
   constructorSetUp(cp,0,0);
}


aprsString::aprsString(string& cp) : string(cp)
{
   peer = "";
   user = "";
   constructorSetUp(cp.c_str(),0,0);
}


//Copy constructor

aprsString::aprsString(aprsString& as)
{
   *this = as;
   ax25Source = as.ax25Source;
   ax25Dest = as.ax25Dest;
   stsmDest = as.stsmDest;
   query = as.query;
   acknum = as.acknum;
   dest = as.dest;
   path = as.path;
   data = as.data;
   user = as.user;
   pass = as.pass;
   pgmName = as.pgmName;
   pgmVers = as.pgmVers;
   peer = as.peer;
   raw = as.raw;
   for(int i=0;i<MAXWORDS;i++) ax25Path[i] = as.ax25Path[i];
   
   timestamp = as.timestamp;
   ttl = as.ttl;
   next = NULL;
   last = NULL;
   aprsType = as.aprsType;
   msgType = as.msgType;
   allowdup = as.allowdup;
   sourceSock = as.sourceSock;
   pathSize = as.pathSize;
   EchoMask = as.EchoMask;
   NN++;
   objCount++;
   ID = as.ID ;  //also copy serial number.  This one is NOT unique!!
   instances = 0;
}





aprsString::~aprsString(void)
{
   NN--;
   
   
       
}




void aprsString::constructorSetUp(const char* cp, int s, int e)
{
   
try{
   aprsType = APRSUNKNOWN;
   dest = 0;
   instances = 0;
   reformatted = FALSE;
   allowdup = FALSE;
   msgType = 0;
   sourceSock = s;
   EchoMask = e;
   last = NULL;
   next = NULL;
   ttl = ttlDefault;
   timeRF = 0;
   AEA = FALSE;
   acknum = "";
   query = "";

   objCount++;
   ID = objCount;         //set unique ID number for this new object

   timestamp = time(NULL);

   
   NN++;

   for(int i = 0;i<MAXPATH;i++) ax25Path[i] = "";
   for(int i = 0; i< MAXWORDS;i++) words[i] = "";
   ax25Source = "";
   ax25Dest = "";
   stsmDest = "";
   raw = string(cp);
   
   pathSize = 0;

   if (length() <= 0) {cerr << "Zero or neg string Length\n"; return;}

   if (cp[0] == '#')
      {  aprsType = COMMENT;
        // print();
         return;
      }

   if((find("user ") == 0 || find("USER ") == 0))    //Must be a logon string
      {
        
        int n = split(*this,words,MAXWORDS,RXwhite);
        if (n > 1) user = words[1]; else user = "Telnet";
        if (n > 3) pass = words[3]; else pass = "-1";
        if (n > 5) pgmName = words[5]; else pgmName = "Telnet";
        if (n > 6) pgmVers = words[6]; 
        EchoMask = 0;      //Don't echo any logon strings
        aprsType = APRSLOGON;
        
        return;
        
      }                 //else it's a aprs posit packet or something



         
           
    if(find(":") > 0)
               {     
                     size_type pIdx = find(":");   //Find location of first ":"
                     path = substr(0,pIdx);  //extract all chars up to the ":" into path 
                     dataIdx = pIdx+1;       //remember where data starts
                     
                     size_type gtIdx = path.find(">");  //find first ">" in path
                     size_type gt2Idx = path.find_last_of(">"); // find last ">" in path

                     if ((gtIdx != gt2Idx)&&(gtIdx != npos))  //This is in AEA TNC format because it has more than 1 ">"           
                       {                                                            
                            //cout << "AEA " << *this << endl << flush;
                            size_type savepIdx = pIdx;
                            string rs;
                            AEAtoTAPR(*this,rs);
                                   //Replace AEA path with TAPR path
                            pIdx = rs.find(":");
                            string rsPath = rs.substr(0,pIdx);
                            replace(0,savepIdx,rsPath);
                            path = rsPath;                  
                                                                                    
                            AEA = TRUE;                                             
                                                                      
                                                                                    
                       }                                                                               


                                          
                     if ((pIdx+1) < length()) data = substr(pIdx+1,MAXPKT); 
                                          
                      
                     pathSize = split(path ,ax25Path,MAXPATH,pathDelm);
                     if (pathSize >= 2) ax25Dest = ax25Path[1];
                     if (pathSize >= 1) ax25Source = ax25Path[0];
                     
                     

                     if (data.length() == 0) return;

                     
                     size_type idx,qidx;

                     switch(data[0])                                                                             
                                                                                                                 
                     {                                                                                           
                                                                                                                 
                     case ':' :    /* station to station message, new 1998 format*/  
                                    /* example of string in "data" :N0CLU    :ack1   */                            
                                         
                                          stsmDest = data.substr(1,MAXPKT); 
                                          idx = stsmDest.find(":");
                                          if (idx == npos) break;
                                          stsmDest = stsmDest.substr(0,idx);
                                          if(stsmDest.length() != 9) break;
                                          idx = stsmDest.find_first_of(RXwhite);
                                          if(idx != npos)
                                             stsmDest = stsmDest.substr(0,idx);                       
                                          aprsType = APRSMSG; 
                                          EchoMask |= src3RDPARTY;
                                          msgType = APRSMSGTEXT;
                                          if(data.length() >= 15)
                                            if(data.substr(10,4) == string(":ack")) msgType = APRSMSGACK ;
                                            if(data.substr(10,2) == string(":?")){
                                                qidx = data.find('?',12);
                                                if(qidx != string::npos) {
                                                   query = data.substr(12,qidx-12);
                                                   msgType = APRSMSGQUERY;
                                                   qidx = data.find('{',qidx);
                                                   if(qidx != string::npos) {
                                                      acknum = data.substr(qidx);
                                                   }
                                                   //cout << "Query=" << query << " qidx=" << qidx << endl;
                                                }
                                            }
                                          break;                                                                 
                                                                                                                 
                     case '_' :           aprsType = APRSWX ; break;/* weather */                                                          
                     case '@' :                    /* APRS mobile station */                                              
                     case '=' :                    /* APRS fixed station */                                               
                     case '!' :                    /* APRS not runing, fixed short format */                              
                     case '/' :           aprsType = APRSPOS;  /* APRS not running, fade to gray in 2 hrs */                          
                                          break; 

                     case '>' :           aprsType = APRSSTATUS;
                                          break;

                     case '?' :           qidx = data.find('?',1);
                                          if(qidx != string::npos){
                                          query = data.substr(1,qidx-1);
                                          aprsType = APRSQUERY;
                                          }
                                          break;

                     case ';' :           aprsType = APRSOBJECT;
                                          break;

                     case 0x60:          //These indicate it's a Mic-E packet
                     case 0x27:
                     case 0x1c:
                     case 0x1d:           
                                          aprsType = APRSMIC_E;
                                          break;

                                                                                                                 
                     case '}' :           {  reformatted = TRUE;
                                             string temp = data.substr(1,MAXPKT);
                                             aprsString reparse(temp);
                                             aprsType = reparse.aprsType;
                                             //print(cout);
                                             break;
                                          }

                     case '$' :           aprsType = NMEA;
                                          break;

                     
                                          /* check for messages in the old format */                             
                                                                                                                 
                     default:             if(data.length() >= 10)                                                
                                          {  if((data.at(9) == ':') && isalnum(data.at(0)))      
                                                                                                                 
                                             {  
                                                idx = data.find(":");
                                                stsmDest = data.substr(0,idx);  //Old format                       
                                                aprsType = APRSMSG; 
                                                EchoMask |= src3RDPARTY;
                                                idx = stsmDest.find_first_of(RXwhite);
                                                if(idx != npos)
                                                   stsmDest = stsmDest.substr(0,idx);

                                                
                                                if(data.substr(9,4) == string(":ack")) msgType = APRSMSGACK ; 
                                                if(data.substr(9,2) == string(":?")){
                                                qidx = data.find('?',11);
                                                if(qidx != string::npos) {
                                                   query = data.substr(11,qidx-11);
                                                   msgType = APRSMSGQUERY;
                                                   qidx = data.find('{',qidx);
                                                   if(qidx != string::npos) {
                                                      acknum = data.substr(qidx);
                                                   }

                                                   //cout << "Query=" << query << " qidx=" << qidx << endl;
                                                }
                                            }

                                             }                                                                   
                                            if((data.at(9) == '*') && isalnum(data.at(0)))
                                                { aprsType = APRSOBJECT;
                                                }
                                                                                                           
                                          }  
                                          break;
                                                                                                                 
                     }; /* end switch */                                                                         
                                                                                                                 
                                                                                                                 
                                                                                                                 
                                                                                                                 
                                                                                                                
                                                                                                                 
                     if(path.find("TCPXX") != npos) tcpxx = TRUE; else tcpxx = FALSE;                                
            }
    
    return;  

} //end try

catch(exception& rEx)
{
   cerr << "Caught exception in aprsString: " 
         << rEx.what()
         << endl ;


   print(cerr);
   exit(1);
         
}

}


//-------------------------------------------------------------
void aprsString::AEAtoTAPR(string& s, string& rs)
{
   string pathElem[MAXPATH+2];
   string pathPart, dataPart;
   size_type pIdx = find(":");
   dataPart = s.substr(pIdx+1,MAXPKT);
   pathPart = s.substr(0,pIdx+1);
   int n = split(pathPart,pathElem,MAXPATH+2,pathDelm);
   rs  = pathElem[0] + '>' + pathElem[n-1];
   for(int i=n-2;i>0;i--) rs = rs + ',' + pathElem[i];
   rs = rs + ':' + dataPart;
  
   

}

//--------------------------------------------------------------

/* This is for debugging only.
   It dumps important variables to the console */

void aprsString::print(ostream& os)
{
   os << *this << endl
        << "Serial No. = " << ID << endl
        << "TCPIP source socket = " << sourceSock << endl
        << "Packet type = " << aprsType << endl
        << "EchoMask = " << EchoMask << endl;

   if (aprsType == APRSLOGON)
      {  os  << "User: " << user << endl
               << "Pass: " << pass << endl
               << "Program: " << pgmName << endl
               << "Vers: " << pgmVers << endl;
      }else
         {

            os  << "Source =" << getAX25Source() << endl
                  << "Destination = " << getAX25Dest() << endl;

            for(int i = 0; i< pathSize; i++)
               {  os << "Path " << i << " " << ax25Path[i] << endl;
               }

         }
   
}

/*returns the string as a char* 
*/

const char* aprsString::getChar(void)
{
   return c_str();
}

int aprsString::getEchoMask(void)
{
   return EchoMask;

}

void aprsString::setEchoMask(int m)
{
   EchoMask = m;
}

string aprsString::getAX25Source(void)
{
  return ax25Path[0];
}

string aprsString::getAX25Dest(void)
{
   return ax25Path[1];


}



//---------------------------------------------------------------------------
BOOL aprsString::queryLocal(void)
{
   BOOL localSource = FALSE;

   if((EchoMask & srcTNC) 
      && (path.find("GATE*") == npos )
      && (freq(path,'*') < 3))
         localSource = TRUE;                                                        
     
  return localSource;                                                          
}

//-------------------------------------------------------------------------------

BOOL aprsString::queryPath(char* cp)   //Search ax25path for match with *cp
{
   BOOL rc = FALSE;

  for(int i=0;i<pathSize;i++)
     {
        if(ax25Path[i].compare(cp) == 0)
            {   rc = TRUE;
                break;
            }


      }

 return rc;
}



void aprsString::stsmReformat(char *mycall)
{ char *co;
  char out[256];
  ostrstream os(out,255);
  co = ":";

   if((aprsType == APRSMSG) && (data.at(0) != ':')) co = "::"; //convert to new msg format if old

      
   os <<  "}" << ax25Source                   
      <<  ">" << ax25Dest                     
      << ",TCPIP," <<  mycall                   
      << "*" << co << data << ends;

   data = out;
   reformatted = TRUE;
   
}

 


//--------------------------------------------------------
//Converts mic-e packets to 1 or 2 normal APRS packets
//Returns pointers to newly allocated aprsStrings

void aprsString::mic_e_Reformat(aprsString** posit, aprsString** telemetry)
{

   unsigned char mic1[512], mic2[512];
   int l1, l2;
   time_t now = time(NULL);

   //printhex(ax25Dest.c_str(),strlen(ax25Dest.c_str()));
   //printhex(data.c_str(),strlen(data.c_str()));
   //cout << "data len=" << data.find("\r") << endl;

   int i = fmt_mic_e((unsigned char*)ax25Dest.c_str(),
                     (unsigned char*)data.c_str(),
                     data.find("\r"),
                     mic1,&l1,
                     mic2,&l2,
                     now);

   if(i)
      {  
         mic1[l1] = mic2[l2] = '\0';
         if(l1){                 
                 char *buf1 = new char[512];
                 ostrstream pbuf(buf1,512);
                 pbuf <<  path << ':' << mic1 << "\r\n" << ends;
                 aprsString* Posit = new aprsString(buf1,sourceSock,EchoMask,peer.c_str(),user.c_str());
                 Posit->raw = string(raw);  // Save a copy of the raw mic_e packet
                 Posit->changePath("APRS",ax25Dest.c_str());
                 delete buf1;
                 *posit = Posit;
                 
               }
                  
         
         if(l2){  
               char *buf2 = new char[512];
               ostrstream tbuf(buf2,512);
               tbuf <<  path << ':' << mic2 << "\r\n" << ends;
               aprsString* Telemetry = new aprsString(buf2,sourceSock,EchoMask,peer.c_str(),user.c_str());
               Telemetry->raw = string(raw);  // Save a copy of the raw mic_e packet
               Telemetry->changePath("APRS",ax25Dest.c_str());
               delete buf2;
               *telemetry = Telemetry;
               
               
               }
      }
  
}

//---------------------------------------------------------

//Find path element oldPath and replace with newPath
//Returns TRUE if success.

BOOL aprsString::changePath(const char* newPath, const char* oldPath)
{
   BOOL rc = FALSE;
   int i;
   
   
   for(i = 0;i<pathSize;i++)
      if(ax25Path[i].compare(oldPath) == 0 )
            {   
               ax25Path[i] = newPath;
               size_type idx = find(oldPath);
               replace(idx,strlen(oldPath),newPath);
               rc = TRUE;
               break;
            }

     
   return rc;
}

void aprsString::printN(void)
{
   cout << "N=" << NN << endl;
}


void aprsString::addInstance(void)
{
   instances++;
}

void aprsString::removeInstance(void)
{
   instances--;
}


//----------------

