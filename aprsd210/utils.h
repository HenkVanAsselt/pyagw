#ifndef UTILS_1
#define UTILS_1



#include "constant.h"
#include "aprsString.h"
#include "cpqueue.h"
#include <vector.h>


#define BADUSER -1
#define BADGROUP -2
#define BADPASSWD -3





int WriteLog(const char *cp, char *LogFile);
char* strupr(char *cp);
void printhex(char *cp, int n);
BOOL CmpDest(const char *line, const char *ref);
BOOL CmpPath(const char *line, const char *ref);
BOOL callsign(char *s, const char *call);
BOOL CompareSourceCalls(char *s1, char *s2);
void GetMaxAgeAndCount( int *MaxAge, int *MaxCount);
int  checkpass(const char *szUser, const char *szGroup, const char *szPass);
void RemoveCtlCodes(char *cp);
char* StripPath(char* cp);
BOOL getMsgDestCall(char *data, char* cp, int n);
BOOL getMsgSourceCall(char* data, char* cp, int n);
char checkUserDeny(string& user);

int stricmp(const char* szX, const char* szY);


int split(  string& s, string sa[],   int saSize,  string& delim);
int freq(string& s, char c);
void upcase(string& s);
void reformatAndSendMicE(aprsString* inetpacket, cpQueue& sendQueue);
BOOL find_rfcall(const string& s, string* rfcall[]);
BOOL find_rfcall(const string& s, vector<string*>& rfcall);




#endif


