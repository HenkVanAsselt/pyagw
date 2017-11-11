/* serialp.h */

#ifndef SERIAL_P
#define SERIAL_P




/*--------------------------------------------------------------*/

void *ReadCom(void* vp) ;	  //Com port read thread
int SendFiletoTNC(char *szName);
int SetupComPort(FILE* f);
int AsyncOpen(char *szPort);
int AsyncClose(void);
int WriteCom(char *cp);
int WriteCom(char c) ;


#endif




