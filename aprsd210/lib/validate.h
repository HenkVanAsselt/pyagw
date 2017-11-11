/* validate.h */

#define BADUSER -1
#define BADGROUP -2
#define BADPASSWD -3
#define MUSTRUNROOT -4
#define TRUE 1
#define FALSE 0
#define BOOL int


int validate(const char* user,const char* pass, const char* group);
short doHash(const char *theCall);




