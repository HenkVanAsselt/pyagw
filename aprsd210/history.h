/* history.h */

#ifndef HISTORY_1
#define HISTORY_1


void CreateHistoryList();
BOOL AddHistoryItem(aprsString *hp);
void DeleteHistoryItem(aprsString *hp);
int DeleteOldItems(int x);
int DeleteItem(aprsString* ref);
BOOL DupCheck(aprsString* ref, time_t t);
int SendHistory(int session,int em);
int SaveHistory(char *name);
int ReadHistory(char *name);
BOOL StationLocal(const char *cp, int em);
aprsString* getPosit(const string& call, int em);
BOOL timestamp(long sn, time_t t);


#endif

