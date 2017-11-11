/* cpqueue.cpp */

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
#include <pthread.h>
#include <stdio.h>
#include <fstream.h>
#include <iostream.h>
#include <strstream.h>
#include <iomanip.h>

#include "cpqueue.h"
#include "aprsString.h"



/*-------------------------------------------------------*/

//Build a Queue object

cpQueue::cpQueue(int n, BOOL d)
{  overrun = 0;
	size = n;
   dyn = d;
	base_p = new queueData[size];
	write_p = 0;
	read_p = 0;
   itemsQueued = 0;
	Q_mutex = new pthread_mutex_t;
   pthread_mutex_init(Q_mutex,NULL);
	lock = 0;
	inRead = 0;
	inWrite = 0;

   for(int i=0; i < size; i++)
      {   base_p[i].qcp = NULL;   //NULL all the pointers
          base_p[i].qcmd = 0;
          base_p[i].rdy = FALSE;  //Clear the ready flags
      }

}

//Destroy Queue object

cpQueue::~cpQueue(void)
{
	lock = 1;
	while(inRead);
	while(inWrite);
   if(dyn)
      {
	      while(read_p != write_p)
			   if (base_p[read_p].qcp != NULL) delete base_p[read_p++].qcp ;
      }
	delete Q_mutex;
	delete base_p;
}


//Place a pointer and an Integer in the queue
//returns 0 on success;
int cpQueue::write(char *cp, int n)
{  int rc=0;
   if (lock) return -2;  //Lock is only set true in the destructor
	inWrite = 1;
	pthread_mutex_lock(Q_mutex);
   int idx = write_p;
   if(base_p[idx].rdy == FALSE)       //Be sure not to overwrite old stuff
      {
	      base_p[idx].qcp = (void*)cp; //put char* on queue
	      base_p[idx].qcmd = n;		  //put int (cmd) on queue
         base_p[idx].rdy = TRUE;      //Set the ready flag
	      idx++;
         itemsQueued++;
	      if (idx >= size) idx = 0;
         write_p = idx;
      }else
         {  overrun++ ;
            if (dyn) delete cp;
            rc = -1;
            
         }

	pthread_mutex_unlock(Q_mutex);
	inWrite = 0;
   return rc;
}

int cpQueue::write(unsigned char *cp, int n)
{
	return write((char*)cp, n);
}


int cpQueue::write(aprsString* cs, int n)
{  int rc = 0;
if (lock) return -2;
inWrite = 1;
pthread_mutex_lock(Q_mutex);
int idx = write_p;
if(base_p[idx].rdy == FALSE)         //Be sure not to overwrite old stuff
   {  base_p[idx].qcp = (void*)cs;	//put String on queue
      base_p[idx].qcmd = n;		   //put int (cmd) on queue
      base_p[idx].rdy = TRUE;       //Set the ready flag
      itemsQueued++;
      idx++;
      if (idx >= size) idx = 0;
      write_p = idx;
   }else { overrun++ ;  if (dyn) delete cs; rc = -1;} //Delete the object that couldn't be put in the queue

pthread_mutex_unlock(Q_mutex);
inWrite = 0;
return rc;
}
   

int cpQueue::write(aprsString* cs)
{
   return write(cs,cs->sourceSock);
}

//Read a pointer and Integer from the Queue

void* cpQueue::read(int *ip)
{
   inRead = 1;
  
   pthread_mutex_lock(Q_mutex);
	void* cp = base_p[read_p].qcp ;     //Read the aprsString*
   if(ip) *ip = base_p[read_p].qcmd ;  //read the optional integer command
   base_p[read_p].qcp = NULL;          //Set the data pointer to NULL
   base_p[read_p].rdy = FALSE;         //Clear ready flag
   read_p++;
   itemsQueued--;
	if(read_p >= size) read_p = 0;
	inRead = 0;
   
   pthread_mutex_unlock(Q_mutex);
	return cp;
}


int cpQueue::ready(void)			//returns TRUE if data available
{  int rc=FALSE;
   pthread_mutex_lock(Q_mutex);
	//if ((read_p != write_p) || wrap) rc = TRUE ;
   if(base_p[read_p].rdy) rc = TRUE;
   pthread_mutex_unlock(Q_mutex);
   return rc;

}

int cpQueue::getWritePtr(void)
{
   return write_p;
}

int cpQueue::getReadPtr(void)
{
   return read_p;
}

int cpQueue::getItemsQueued(void)
{
   return itemsQueued;
}


/*---------------------------------------------------------------------*/

