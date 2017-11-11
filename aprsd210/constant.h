
#define SIGNON "# aprsd 2.1.0 June 22 1999 ( Copyright 1998-1999 Dale Heatherington ) \r\n"
#define VERS "aprsd 2.1.0"

#define TRUE 1
#define FALSE 0
#define BOOL int
#define ULONG unsigned long
#define LONG long
#define USHORT short int
#define APIRET int

#define LINK_SERVER_PORT 1313

#define CMD_END_THREAD -1L
#define SRC_TNC	-2L
#define SRC_USER	-3L
#define SRC_INTERNAL -4L
#define SRC_UDP -5L
#define SRC_IGATE -6L

#define MAXCLIENTS 30
#define BUFSIZE 1024
#define SBUFSIZE 1024
#define RUNTSIZE 0
#define MAXRETRYS 7

#define destINET 1
#define destTNC  2

#define HOMEDIR "/home/aprsd2"
#define CONFPATH ""
#define CONFFILE "aprsd.conf"
#define MAINLOG "aprsd.log"
#define STSMLOG "stsm.log"
#define RFLOG "rf.log"
#define UDPLOG "udp.log"
#define WELCOME "welcome.txt"
#define TNC_INIT  "INIT.TNC"
#define TNC_RESTORE "RESTORE.TNC"
#define APRSD_INIT "INIT.APRSD"
#define SAVE_HISTORY "history.txt"
#define USER_DENY "user.deny"



/* These are for Linux user/pass logons.  They define the group used by
   /etc/group .   You must have these groups and users defined in /etc/group.
   
   The "tnc" group defines which users are allowed direct access to control the TNC.
   The "aprs"  group defines users which log on with a Linux user/password
   instead of the Mac/WinAPRS automatic password. These users are allowed to
   insert data into the data stream.

   To add a group logon as root and edit the /etc/group file by adding
	a line such as: tnc::102:root,wa4dsy,bozo   */

#define APRSGROUP "aprs"
#define TNCGROUP "tnc"


/* this is not used in production code*/
#define TEST "WA4DSY>APRS,WIDE:!3405.31N/08422.46WyWA4DSY APRS Internet Server running on Linux.\r\n"




