/* utils.cpp */

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

#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#define _REENTRANT 1
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fstream.h>
#include <iostream.h>
#include <strstream.h>
#include <iomanip.h>
#include <pwd.h>
#include <grp.h>
#include <vector.h>

#include "constant.h"
#include "utils.h"
#include "cpqueue.h"


using namespace std;


int ttlDefault = 30;	  			//Default time to live of a history item (30 minutes)
int CountDefault = 7;		   //Max of 7 instances of one call sign in history list
extern cpQueue conQueue;


//----------------------------------------------------------------------
int WriteLog(const char *pch, char *LogFile)
{	FILE *f;
	time_t ltime;
	char szTime[40];
	char *p;
	int rc;
   static pthread_mutex_t*   pmtxLog; //Mutual exclusion semi for WriteLog function
   static BOOL logInit = FALSE;

   char *cp = strdup(pch);   //Make local copy of input string.

   if(!logInit)
      {  

         pmtxLog = new pthread_mutex_t; 
         pthread_mutex_init(pmtxLog,NULL);
         logInit = TRUE;
         cout << "logger initialized\n";

      }
	pthread_mutex_lock(pmtxLog);

   
   char *pLogFile = new char[strlen(CONFPATH) + strlen(LogFile) +1];
	strcpy(pLogFile,CONFPATH);
	strcat(pLogFile,LogFile);


	f = fopen(pLogFile,"a");

	if (f == NULL)
		{	f = fopen(pLogFile,"w");
		}
	
	if (f == NULL)
		{	cerr << "failed to open " << pLogFile << endl;
         rc = -1;
		}else

			{ 	char *eol = strpbrk(cp,"\n\r");
            if(eol) *eol = '\0';              //remove crlf
            time(&ltime) ;                    //Time Stamp
				strncpy(szTime,ctime(&ltime),39);
				p = strchr(szTime,(int)'\n');
				if (p) *p = ' ';			         //convert new line to a space
				fprintf(f,"%s %s\n",szTime,cp);	 //Write log entry with time stamp
            fflush(f);
				fclose(f);
				rc = 0;
			}
   delete cp;
   delete pLogFile;
	pthread_mutex_unlock(pmtxLog);

  	return rc;
}


//------------------------------------------------------------------------
//Convert all lower case characters in a string to upper case.
// Assumes ASCII chars.

char* strupr(char *cp)
{  int i;
   int l = strlen(cp);
   for(i=0;i<l;i++)
      {
         if ((cp[i] >= 'a') && (cp[i] <= 'z')) cp[i] = cp[i] - 32;
      }
   return cp;
}

//-----------------------------------------------------------------------
void printhex(char *cp, int n)
{
   
	for (int i=0;i<n;i++) printf("%02X ",cp[i]);
	printf("\n");

}

//---------------------------------------------------------------------
//return TRUE if destination of packet matches "ref"
//This is for filtering out unwanted packets
BOOL CmpDest(const char *line, const char *ref)
{	BOOL rv = FALSE;
	char *cp = new char[strlen(ref)+3];
	strcpy(cp,">");
	strcat(cp,ref);
	strcat(cp,",");
	if (strstr(line,cp) !=	NULL) rv = TRUE;
	delete cp;
	return rv;
}
//----------------------------------------------------------------------

//Return TRUE if any string in the digi path matches "ref".

BOOL CmpPath(const char *line, const char *ref)
{	BOOL rv = FALSE;
   char *cp = new char[strlen(line)+1];
	strcpy(cp,line);
   char *path_end = strchr(cp,':');          //find colon
   if (path_end != NULL)
      {
         *path_end = '\0';             //replace colon with a null
         if (strstr(cp,ref) != NULL) rv = TRUE;
	   }

   delete cp;
   return rv;
}

//---------------------------------------------------------------------
//Returns true if "call" matches first string in "s"
//"call" must be less than 15 chars long.
BOOL callsign(char *s, const char *call)
{
	char cp[17];
	if (strlen(call) > 14) return FALSE;
	strncpy(cp,call,16);
	strncat(cp,">",16);
	char *ss = strstr(s,cp);
	if (ss != NULL) return TRUE ;else return FALSE;

}
//---------------------------------------------------------------------
//Compares two packets and returns TRUE if the source call signs are equal
BOOL CompareSourceCalls(char *s1, char *s2)
{
	char call[12];
	strncpy(call,s2,10);
	char *eos = strchr(call,'>');
	if (eos != NULL) eos[0] = '\0'; else return FALSE;
	return callsign(s1,call);
}

//---------------------------------------------------------------------
// This sets the time-to-live and max count values for each packet received.

void GetMaxAgeAndCount( int *MaxAge, int *MaxCount)
{

	*MaxAge = ttlDefault;
	*MaxCount = CountDefault;

  				
}
//------------------------------------------------------------------------
/* Checks for a valid user/password combination from the 
   Linux etc/passwd file .  Returns ZERO if user/pass is valid.
   This doesn't work with shadow passwords.
*/
int  checkpass(const char *szUser, const char *szGroup, const char *szPass)
{
	passwd *ppw;
	group *pgrp;
	char *member ;
	int i;
	char salt[3];
	int usrfound = 0 ;
	int rc = BADGROUP;

   //cout << szUser << " " << szPass << " " << szGroup << endl;  //debug

	pgrp = getgrnam(szGroup);		  /* Does group name szGroup exist? */
	if (pgrp == NULL) return rc;	  /* return BADGROUP if not */

   ppw = getpwnam(szUser);			  /* get the users password information */
	if (ppw == NULL) return BADUSER ; /* return BADUSER if no such user */
   
	i = 0;
		
	/* find out if user is a member of szGroup */
	while(((member = pgrp->gr_mem[i++]) != NULL) && (usrfound == 0 ))
		{	//cerr << member << endl;	 //debug code
			if(strcmp(member,szUser) == 0)  usrfound = 1	;
		}
	
	if(usrfound == 0) return BADGROUP;	 /* return BADGROUP if user not in group */

   /* check the password */
	  
	strncpy(salt,ppw->pw_passwd,2);
	salt[2] = '\0';
   if (strcmp(crypt(szPass,salt), ppw->pw_passwd) == 0 ) 
		rc = 0; 
		else 
			rc = BADPASSWD;
	

	return rc;



}
//--------------------------------------------------------------------
//Removes all control codes ( < 20h ) from a string and set the 8th bit to zero
void RemoveCtlCodes(char *cp)
{
   int i,j;
   int len = strlen(cp);
   unsigned char *ucp = (unsigned char*)cp;
   unsigned char *temp = new unsigned char[len+1];
   
   for(i=0, j=0; i<len; i++)
      {
         ucp[i] &= 0x7f;      //Clear 8th bit
         if (ucp[i]  >= 0x20) //Check for printable
            {
               temp[j++] = ucp[i] ;   //copy to temp if printable
            }
         
      }
   
   temp[j] = ucp[i];  //copy terminating NULL
   strcpy(cp,(char*)temp);  //copy result back to original
   delete temp;
   
}

//---------------------------------------------------------------------  
char* StripPath(char* cp)
{
   char *p = strchr(cp,':');       //Find the colon and strip off path info
   if (p != NULL){
   p++;                          //move pointer beyond colon
   strcpy(cp,p);                //move data back to start of cp
   }
   return cp;

}
//---------------------------------------------------------------------
//Return the destination callsign for the Internet message if present
//Returns zero length *data and FALSE if callsign not present

BOOL getMsgDestCall(char *data, char* cp, int n)
{
    BOOL rc = FALSE;
    char* p;
    char* temp = strdup(cp);
    
    p = strtok(temp,":");            //p-> path
    p = strtok(NULL,":");            //p-> destination call or null
    if (p != NULL)
       { if (strlen(p) == 9)         //dest call should be exactly 9 chars (space padded)
          { p = strtok(p," :");
            strncpy(data,p,n);       //copy dest call to *data
            rc = TRUE;               //set return code to TRUE
          }
       }
    
    delete temp;
    return rc;
}

//---------------------------------------------------------------------
BOOL getMsgSourceCall(char* data, char* cp, int n)
{
    BOOL rc = FALSE;
    char* p;
    char* temp = strdup(cp);
    
    p = strtok(temp,">");            //p-> path
    if (p != NULL)
    {
         strncpy(data,p,n);
         rc = TRUE;
    }
    
    delete temp;
    return rc;
}

//---------------------------------------------------------------------
 
 //-----------------------------
/*returns the number if instances of char "c" in string "s". */
int freq( string& s, char c)
{
   int count=0;
   int len = s.length();
   for(int i=0;i<len;i++) if(s[i] == c) count++;
   
   return count;
}

//----------------------------------------------------------------------------


int split( string& s, string sa[],  int saSize,  string& delim)
{  
   int wordcount,start,end;
   
   start = s.find_first_not_of(delim);  //find first token
   end = 0;
   wordcount = 0;
   while((start != string::npos) && (wordcount < saSize))
      {  end = s.find_first_of(delim,start+1);
         if(end == string::npos) end = s.length();

         sa[wordcount++] = s.substr(start,end-start);

         start = s.find_first_not_of(delim,end+1);
      }

   
   return wordcount;
}

//----------------------------------------------------------------------------

void upcase(string& s)
{
  for(int i=0; i< s.length(); i++)
     s[i] = toupper(s[i]);
}


  
//---------------------------------------------------------------------
void reformatAndSendMicE(aprsString* inetpacket, cpQueue& sendQueue)
{
   //WriteLog(inetpacket->getChar(),"mic_e.log");
   aprsString* posit = NULL;                                                                              
   aprsString* telemetry = NULL;                                                                          
   inetpacket->mic_e_Reformat(&posit,&telemetry);                                                       
   if(posit) sendQueue.write(posit);          //Send reformatted packets                                
   if(telemetry) sendQueue.write(telemetry);                                                            
   delete inetpacket;  
}

//--------------------------------------------------------------------
/* Return TRUE if string s is in the list cl. */
BOOL find_rfcall(const string& s, string **cl)
{
   BOOL rc = FALSE;
   int i = 0;

   while((cl[i] != NULL) && (rc == FALSE))
      {  if(s.compare(*cl[i]) == 0) rc = TRUE;
         i++;
      }
   return rc;

}

//------------------------------------------------------------------

//Case insensitive c char string compare function
int stricmp(const char* szX, const char* szY)
{
   int i;
   int len = strlen(szX);                        
   char* a = new char[len+1];                                                          
   for(i = 0;i<len;i++) a[i] = tolower(szX[i]);                               
   a[i]='\0';                                                                          
   len = strlen(szY);                                                      
   char* b = new char[len+1];                                                          
   for(i = 0;i<len;i++) b[i] = tolower(szY[i]);                     
   b[i]='\0';                                                                          
                                                         
   int rc = strcmp(a,b);                                                               
   delete a;                                                                           
   delete b;                                     
   return rc;                                    
}
                                            
//--------------------------------------------------------------------
/* Returns the deny code if user found or "+" if user not in list 
   Deny codes:  L = no login   R = No RF access
   Note: ssid suffix on call sign is ignored */ 
 

char checkUserDeny(string& user)
{ const int maxl = 80;
  const int maxToken=32;
  int nTokens ;
  string RXwhite = " \t\r\n";
  char Line[maxl];
  string baduser;
  char rc = '+';
  

  ifstream file(USER_DENY);
  if (!file) return rc;

  string User = string(user);
  int i = user.find("-");
  if(i != string::npos) User = user.substr(0,i);  //remove ssid

     
  do
     {  file.getline(Line,maxl);	  //Read each line in file 
        if (!file.good()) break;
        
        if (strlen(Line) > 0){
            if(Line[0] != '#'){  //Ignore comments
               string sLine(Line);
               string token[maxToken];
               nTokens = split(sLine, token, maxToken, RXwhite);  //Parse into tokens
               upcase(token[0]);
               baduser = token[0];
               if((stricmp(baduser.c_str(),User.c_str()) == 0) && (nTokens >= 2)){
                   rc = token[1][0];
                   break;
               }
              
            }
        }

  }while(file.good());

  file.close();
  return rc;

}
   
   
   


