/* cpqueue.h */

#ifndef CPQUEUE
#define CPQUEUE

#include "aprsString.h"


class  queueData
{	public:
	void* qcp;      //Pointer to dynamically allocated aprsString or char string.
	int	qcmd;     //Optional interger data command or info
   BOOL rdy;
};


class cpQueue
{


 private:
   queueData *base_p;
	int  write_p;
	int  read_p;
	int size, lock, inRead, inWrite;
   pthread_mutex_t* Q_mutex;
   BOOL dyn;
   

  public:
   int overrun;
   int itemsQueued;
	cpQueue(int n, BOOL d);			  // fifo Queue constructor
	~cpQueue(void);			  // destructor

	int write(aprsString* cs, int cmd);
   int write(aprsString* cs);
   int write(char* cs, int cmd);
   int write(unsigned char* cs, int cmd);
	
   void* read(int *cmd);
	int ready(void);				//return non-zero when queue data is available

   int getWritePtr(void);   //For debugging
   int getReadPtr(void);
   int getItemsQueued(void);


};

#endif

