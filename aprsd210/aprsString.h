/* aprsString.h */

#ifndef APRSSTRING
#define APRSSTRING

#define MAXPATH 12
#define MAXPKT 256

using namespace std;

#include <string>
#include "constant.h"

 static const int APRSMSGACK = 31;  //These three are for the msgType variable
 static const int APRSMSGTEXT = 32; // and indicate what kind of message it is.
 static const int APRSMSGQUERY = 33; 

 static const int APRSMIC_E = 20;
 static const int APRSOBJECT = 9;
 static const int COMMENT = 10;
 static const int NMEA = 100;
 static const int APRSREFORMATTED = 8;
 static const int APRSQUERY = 7;
 static const int APRSSTATUS = 6;
 static const int APRSTCPXX = 5;
 static const int APRSLOGON = 4;
 static const int APRSMSG = 3;
 static const int APRSWX = 2;
 static const int APRSPOS = 1;
 static const int APRSUNKNOWN = 0;
       
 static const int MAXWORDS = 8;

 

class aprsString: public string
{  
 
      
  
public:                        
   static int NN;             //Tells how many objects exist
   static long objCount;      //Total of objects created (used as ID)
   long ID;                   //Unique ID number for this object
   int instances ;             //Number of pointers to this object that exist in user queues
   int tcpxx;                 //TRUE = TCPXX or TCPXX* was in the path
   int reformatted;           //TRUE = packet has been reformatted into 3rd party format
   time_t timestamp;          // time it was created
   int ttl;                   // time to live (minutes)
   time_t timeRF;             // time this was sent on RF (0 = never)
   aprsString* next;          //Only used if this is part of a linked list
   aprsString* last;          // ditto..
   int localSource;           //TRUE = source station is local
   int aprsType;
   int sourceSock;            //tcpip socket this came from
   int pathSize;
   int EchoMask;              //Set a bit to indicate where this object came from
   int dest;                  //Where it's going, destTNC or destINET
   size_type dataIdx;         //index of data part of this packet
   
  
   int allowdup;              //TRUE or FALSE, object is a duplicate (for redundent ACKs)
   int msgType;               //Indicates if message is an ACK or just text
                


   string ax25Source;         //ax25 source
   string ax25Dest ;          //ax25 destination
   string stsmDest;           //Station to Station Message destination
   string path;               //Raw ax25 path  (up to but not including the colon)
   string data;               //Raw ax25 data (all following first colon)
   string ax25Path[MAXPATH];
   string words[MAXWORDS];
   string user;
   string pass;
   string pgmName;
   string pgmVers;
   string peer;
   string query;              //APRS Query string
   string acknum;             //The "{nnn" at the end of messages
   string raw;                //copy of complete raw packet preserved here
   BOOL AEA;

   
  
   
   aprsString(const char *cp, int s,int e);
   aprsString(const char *cp, int s,int e, const char* szPeer, const char *userCall);
   aprsString(const char *cp);
   aprsString(string& as);
   aprsString(aprsString& as);
   ~aprsString(void);

   void print(ostream& os);
   string getAX25Source(void);
   string getAX25Dest(void);
   const char* getChar(void);
   int getEchoMask(void);
   void setEchoMask(int m);
   BOOL queryPath(char* cp);        //Tells if char string cp is in the ax25 path
   BOOL changePath(const char* newPath, const char* oldPath); //Change one path element
   BOOL queryLocal(void);        //returns true if source call is local
   void stsmReformat(char *call);
   void aprsString::mic_e_Reformat(aprsString** posit, aprsString** telemetry);
   void printN(void);
   void addInstance(void);
   void removeInstance(void);
private:
   void constructorSetUp(const char *cp, int s, int e);
   void AEAtoTAPR(string& s, string& rs);


   


} ;

#endif
