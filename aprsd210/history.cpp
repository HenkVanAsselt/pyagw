/* history.cpp */


// July 1998.  Completely revised this code to deal with aprsSting objects
//instead of History structures.

/*This code maintains a linked list of aprsString objects called the
  "History List".  It is used to detect and remove duplicate packets
  from the APRS stream, provide a 30 minute history of activity to
  new users when they log on and determine if the destination of a
  3rd party station to station message is "local".
  */

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

#include <assert.h>


#include "constant.h"
#include "utils.h"
#include "history.h"

struct histRec                   //This is for the History Array
         {  int count;
            time_t timestamp;
            int EchoMask;
            int type;
            int reformatted;
            char *data;
         } ;



aprsString *pHead, *pTail;

int ItemCount;
pthread_mutex_t*	pmtxHistory;
pthread_mutex_t*  pmtxDupCheck;


extern const int srcTNC;
extern int ttlDefault;

int dumpAborts = 0;

//---------------------------------------------------------------------
void CreateHistoryList()
{
    pmtxHistory 	= new pthread_mutex_t;
    pthread_mutex_init(pmtxHistory,NULL);
    pmtxDupCheck = new pthread_mutex_t;
    pthread_mutex_init(pmtxDupCheck,NULL);
    pHead = NULL;
	 pTail = NULL;
	 ItemCount = 0;
}

//---------------------------------------------------------------------
//Adds aprs packet to history list if no duplicate is there already.
//Returns TRUE if packet was already in the history list or
//we don't want it sent for other reasons.
BOOL AddHistoryItem(aprsString *hp)
{

   
   if (hp == NULL) return TRUE;
   
   
   pthread_mutex_lock(pmtxHistory);
   

  	BOOL dupPresent = FALSE;
  
	if (pTail == NULL)		 //Starting from empty list
		{
			pTail = hp;
			pHead = hp;
			hp->next = NULL;
			hp->last = NULL;
			ItemCount++;
		}
		else                       //List has at least one item in it.

		{  
         dupPresent = DupCheck(hp, 30);  //Check for identical data in last 30 seconds
                                         //caused by multiple digipeats.
         
         if(!dupPresent)     //If it's not a duplicate then add it to the list.
                              // In theory only the "most local" path will be in the list.

            {  int n = DeleteItem(hp);  //Delete any previously stored items with same call and type
              
            //Debug
            /*
               if (n > 0) cout   << "Overwriting history item " 
                                 << hp->ax25Source 
                                 << " " << hp->aprsType << endl
                                 << hp->getChar() << endl;
             */ 
            
               if(ItemCount == 0)  //List is empty, put in first item
                  {  pTail = hp;
			            pHead = hp;
			            hp->next = NULL;
			            hp->last = NULL;
                  } else               //list not empty...
                     {  
                        pTail->next = hp;		//link the last item to us				    
                        hp->last = pTail;    //link us to last item
			               hp->next = NULL;     //link us to next item which is NULL
                        pTail = hp;          //link pTail to us
                     }
                     
                     
               ItemCount++;
               
            }else {  //cout << "Duplicate found \n\r" << hp->getChar() << endl;   //Debug
                     
                  }
  		 }

	pthread_mutex_unlock(pmtxHistory);
	return dupPresent;
}
//---------------------------------------------------------------------
//Don't call this from anywhere except DeleteOldItems().
void DeleteHistoryItem(aprsString *hp)
{

   if(hp == NULL) return;
  	aprsString *li = hp->last;
	aprsString *ni = hp->next;
	
	if (hp != NULL)
		{  if (li != NULL) li->next = ni;
			if (ni != NULL) ni->last = li;
			if (hp == pHead) pHead = ni;
			if (hp == pTail)  pTail  = li;

         delete hp;					//Delete the aprsString object
			ItemCount--;
	   }
	
   return;

}

//-----------------------------------------------------------------------
//Call this once per 5 minutes
//This reduces the ttl field by x then deletes items when the ttl field is zero or less.
int DeleteOldItems(int x)	
{	int delCount = 0;


	if ((pHead == NULL) || (pHead == pTail)) return 0;  //empty list

	pthread_mutex_lock(pmtxHistory);

	aprsString *hp = pHead;
	aprsString *pNext;

	while(hp)
		{	
		   hp->ttl = hp->ttl - x; //update time to live
			pNext = hp->next;
			if (hp->ttl <= 0 )
					{   DeleteHistoryItem(hp);
						 delCount++;
					}
			
			hp = pNext;
		}
	
 	pthread_mutex_unlock(pmtxHistory);
	return delCount;
}
	
//-------------------------------------------------------------------------
//Deletes an aprsString object matching the ax25Source call sign and packet type
// of the "ref" aprsString.  The destination (destTNC or destINET) must also match.
int DeleteItem(aprsString* ref)	
{	int delCount = 0;


	if ((pHead == NULL) || (pHead == pTail) || (ref == NULL)) return 0;

  
   
	aprsString *hp = pHead;
	aprsString *pNext;

	while(hp != NULL)
		{	
		   
			pNext = hp->next;
			if ((hp->aprsType == ref->aprsType )
               && (hp->dest == ref->dest))
            {  if(hp->ax25Source.compare(ref->ax25Source) == 0)
              
					{   //cerr << "deleteing " << hp->getChar() << flush;
                   DeleteHistoryItem(hp);
						 delCount++ ;
					}
            }
		  
			hp = pNext;
		}
	
  	return delCount;
}

//-------------------------------------------------------------------------
/* Check history list for a packet whose data matches that of "*ref"
   that was transmitted less or equal to "t" seconds ago.  Returns TRUE
   if duplicate exists.  Scanning stops when a packet older than "t" seconds
   is found.
*/
BOOL DupCheck(aprsString* ref, time_t t)
{	int x = -1;
	BOOL z;
   time_t tNow,age;

   if (ref->allowdup) return FALSE;
	
	if ((pTail == NULL) || (pHead == NULL)) return FALSE;  //Empty list
   
   
   pthread_mutex_lock(pmtxDupCheck);        //Grab semaphore

   time(&tNow);                             //get current time

	aprsString *hp = pTail;       //Point hp to last item in list
	age = tNow - hp->timestamp;

   
   int ii = 0;
   while((hp != NULL) && (x != 0) && (age <= t))
		{ 
        
        age = tNow - hp->timestamp;
        if((ref->ax25Source.compare(hp->ax25Source) == 0)
             && (ref->dest == hp->dest))  //check for matching call signs  and destination (TNC or Internet)
            {   
              if(ref->data.compare(hp->data) == 0) x = 0;  //and matching data
            }
        
			hp = hp->last;									//Go back in time through list
                                              //Usually less than 10 entrys need checking for 30 sec.
		}
  
  if (x == 0) z = TRUE; else z = FALSE;
  pthread_mutex_unlock(pmtxDupCheck);
  return z;
 }

//--------------------------------------------------------------------------

//Scan history list for source callsign which exactly matches *cp .
//If callsign is present and GATE* is not in the path and hops are 3 or less then return TRUE
//The code to scan for "GATE* and hops have been moved to aprsString.queryLocal() .

BOOL StationLocal(const char *cp, int em)
{
   int x;
	BOOL z = FALSE;
   int cpLen = strlen(cp);
	
   if ((pTail == NULL) || (pHead == NULL)) return FALSE;  //Empty list

   pthread_mutex_lock(pmtxHistory);	 // grab mutex semaphore
    //cout << cp << " " << em << endl; //debug
	if (ItemCount == 0)                        //if no data then...
      {  pthread_mutex_unlock(pmtxHistory);   // ...release mutex semaphore
         return FALSE;                        // ...return FALSE
   }

   
	aprsString *hp = pTail;                      //point to end of history list

   while ((z == FALSE) && (hp != NULL))    //Loop while no match and not at beginning of list
      {  
      
       if(hp->EchoMask & em)
          {
            if(hp->ax25Source.compare(cp) == 0)      //Find the source call sign
                              z = hp->queryLocal(); //then see if it's local
          }

          hp = hp->last;

       }

   pthread_mutex_unlock(pmtxHistory);   //release mutex semaphore
   return z;                            //return TRUE or FALSE

}

//------------------------------------------------------------------------
//Returns a new aprsString of the posit packet whose ax25 source call
//matches the "call" arg and echomask bit matches a bit in "em".
//Memory allocated for the returned aprsString  must be deleted by the caller.
aprsString* getPosit(const string& call, int em)
{
   
   if ((pTail == NULL) || (pHead == NULL)) return NULL ;  //Empty list



   aprsString* posit = NULL;

   pthread_mutex_lock(pmtxHistory);	 // grab mutex semaphore
    
    if (ItemCount == 0)                        //if no data then...
       {  pthread_mutex_unlock(pmtxHistory);   // ...release mutex semaphore
          return NULL;                        // ...return NULL
    }
 
    
    aprsString *hp = pTail;                      //point to end of history list

    while ((posit == NULL) && (hp != NULL))    //Loop while no match and not at beginning of list
       {  
       
        if(hp->EchoMask & em)
            { 
             if((hp->ax25Source.compare(call) == 0)    //Find the source call sign
                  && ((hp->aprsType == APRSPOS) || hp->aprsType == NMEA))   //of a posit packet   
                   { 
                     posit = new aprsString(*hp);    //make a copy of the packet
                     
                   }
            }
 
           hp = hp->last;
 
        }
 
    pthread_mutex_unlock(pmtxHistory);   //release mutex semaphore
    return posit;

      
}
//-------------------------------------------------------------------------

/* timestamp the timeRF variable in an aprsString object in the history list
   given the objects serial number.  Return TRUE if object exists. */
BOOL timestamp(long sn, time_t t)
{
   BOOL x = FALSE;

   if ((pTail == NULL) || (pHead == NULL)) return FALSE ;  //Empty list



  aprsString* posit = NULL;

  pthread_mutex_lock(pmtxHistory);	 // grab mutex semaphore
   
   if (ItemCount == 0)                        //if no data then...
      {  pthread_mutex_unlock(pmtxHistory);   // ...release mutex semaphore
         return FALSE;                        // ...return FALSE
   }

   
   aprsString *hp = pTail;                      //point to end of history list

   while ((x == FALSE) && (hp != NULL))    //Loop while no match and not at beginning of list
      {  
      
       if(hp->ID == sn)
           { 
              hp->timeRF = t;       //time stamp it.
              x = TRUE;             //indicate we did it.

           }

          hp = hp->last;

       }

   pthread_mutex_unlock(pmtxHistory);   //release mutex semaphore
   
    return x;
}



//-------------------------------------------------------------------------
/* This reads the history list into an array to facilitate sending
the data to logged on clients without locking the History linked list
for long periods of time.

Note: Must be called with pmtxHistory locked !
*/

histRec* createHistoryArray(aprsString* hp)
{  int i;
   
   if(hp == NULL) return NULL;

   histRec* hr = new histRec[ItemCount];    //allocate memory for array

   for(i=0;i<ItemCount;i++)
      {  hr[i].count = ItemCount - i;
         hr[i].timestamp = hp->timestamp;
         hr[i].EchoMask = hp->EchoMask;
         hr[i].type = hp->aprsType;
         hr[i].reformatted = hp->reformatted;
         hr[i].data = strdup(hp->getChar());    //Allocate memory and copy data
         if (hp->next == NULL) break;
			hp = hp->next;					
      }


   
   return hr;
}
//--------------------------------------------------------------------------
/* Deletes the history array and it's data created above */

void deleteHistoryArray(histRec* hr)
{
   int i;

   if(hr){
      int arraySize = hr[0].count;

      for(i = 0;i<arraySize;i++)
      { if(hr[i].data != NULL) delete hr[i].data;
      
      }
      delete hr;
   }

}

//--------------------------------------------------------------------------
//Send the history items to the user when he connects
//returns number of history items sent or -1 if an error in sending occured
int SendHistory(int session, int em)
{
	
	int 	rc,count, i=0;
   int retrys;


	if (pHead == NULL) return 0;			//Nothing to send

   pthread_mutex_lock(pmtxHistory);	 // grab mutex semaphore

	aprsString *hp = pHead;					  //Start at the beginning of list
   histRec* hr = createHistoryArray(hp); //copy it to an array
	pthread_mutex_unlock(pmtxHistory);    //release semaphore
   if (hr == NULL) return 0;
   int n = hr[0].count;                // n has total number of items
	count = 0;
   float throttle = 30;    //Initial rate  about 250kbaud
   for(i=0;i < n; i++)
		{	
         if ((hr[i].EchoMask & em) 
               && (hr[i].reformatted == FALSE))     //filter data intended for this users port
         {  count++;                //Count how many items we actually send
	     	   size_t dlen  = strlen(hr[i].data);
            retrys = 0;

            do {
             
		       rc = send(session,(const void*)hr[i].data,dlen,0); //Send history list item to client
             
             usleep((int)throttle * dlen);   //pace ourself

             if(rc < 0) {
                sleep(1 );     //Pause output 1 second if resource unavailable
                if(retrys == 0) {throttle = throttle * 2;} //cut our speed in half
                if(throttle > 3333) throttle = 3333;        //Don't go slower than 2400 baud
                retrys++;
             }else
                if(throttle > 30) throttle = throttle * 0.98; //Speed up 2% 

             
            }while((errno == EAGAIN) && (rc < 0) && ( retrys <= 180));  //Keep trying for 3 minutes
			   if(rc < 0)
	          {	cerr <<  "send() error in SendHistory() errno= " << errno << " retrys= " << retrys
							<< " \n[" << strerror(errno) <<  "]" << endl;
					deleteHistoryArray(hr);
               dumpAborts++;
               return -1;
				 }

         }
		  			
		}

      deleteHistoryArray(hr);
 	  	return count;
}
//---------------------------------------------------------------------
//Save history list to disk
int SaveHistory(char *name)
{
	
	int icount = 0;
	if (pHead == NULL) return 0;
   pthread_mutex_lock(pmtxHistory);

   ofstream hf(name);			  // Open the output file

	if (hf.good())						  //if no open errors go ahead and write data
		{
			aprsString *hp = pHead;
	
			for(;;)
				{
             if(hp->EchoMask){           //Save only items which have a bit set in EchoMask
					hf << hp->ttl  << " "
                  << hp->timestamp << " "
                  << hp->EchoMask << " "
                  << hp->aprsType << " "
                  << hp->getChar() ;  //write to file
					if (!hf.good())
						{	cerr << "Error writing " << SAVE_HISTORY << endl;
							break;
						}
					icount++;
             }
					if (hp->next == NULL) break;
					hp = hp->next;											//go to next item in list
				}
         hf.close();
		}

	pthread_mutex_unlock(pmtxHistory);
   return icount;
}
//---------------------------------------------------------------------

int ReadHistory(char *name)
{
	int icount = 0;
   int expiredCount = 0;
	int ttl,EchoMask,aprsType,count;
   char dummy;
	char *data = new char[256];
   time_t now,timestamp;

   ifstream hf(name);			  	// Create ifstream instance "hf" and open the input file

   if (hf.good())						  		//if no open errors go ahead and read data
		{
         now = time(NULL);
			while(hf.good() )
				{
					hf >> ttl  >> timestamp >> EchoMask >> aprsType;  
              	hf.get();							// skip 1 space
				   hf.get(data,252,'\r');			// read the rest of the line as a char string
               int i = strlen(data);
               
               data[i++] = '\r';
               data[i++] = '\n';
               data[i++] = '\0';
               
					
               aprsString *hist = new aprsString(data);
               
               hist->EchoMask = EchoMask;
               hist->timestamp = timestamp;
               hist->ttl = ttl;
               hist->aprsType = aprsType;

               //Now add to list only items which have NOT expired
               if(difftime(now,timestamp) <= (ttlDefault * 60))
                  { AddHistoryItem(hist);  
                    icount++;
                  }else
                     {  delete hist;
                        expiredCount++;
                     }
               
				}
         hf.close();
		}


   delete data;
   cout << "Read " << icount << " items from " 
         << name  << endl;

   if(expiredCount)
      cout  << "Ignored " << expiredCount
            << " expired items in "
            << name << endl;

   return icount;
  }
 
//-------------------------------------------------------------------------------------
