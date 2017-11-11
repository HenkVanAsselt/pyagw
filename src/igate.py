
# global imports
import os
import logging
import datetime
import sys, traceback
import telnetlib

# local imports
import ping
import stationdb

# ----------------------------------------------------------------------------
#
# ----------------------------------------------------------------------------

class Host:
    """ A class containing host information
    """

    # --------------------------------------------------------------------------

    def __init__(self, name=None, port=23):
        """ Intialize this class
        @param name: name or ip address of the host
        @param port: portnumber for the host connection.
        """

        if not name:
            logging.error('No hostname given')
            return

        self.name_or_ip = name          #: Name or IP address of the host
        self.port = port                #: The portnumber to use
        self.socket = None

        self.comment = None             #: Comments about the host

        self.version = None             #: Software version of this host
        self.pinged = None              #: Set datetime.datetime.now() on success
        self.connected = None           #: Set datetime.datetime.now() on success

    # --------------------------------------------------------------------------

    def comment(self, s = None):
        """ Set comment string of this host
        param s: version to set
        """
        if not s:
            return self.comment
        else:
            self.comment = s

    # --------------------------------------------------------------------------

    def ping(self):
        """ ping this host
        @returns: True on success, False on failure
        """

        logging.info('pinging %s' % self.name_or_ip)
        try:
            response = ping.ping(self.name_or_ip)
            if not response:
                logging.info("no response from %s" % (self.name_or_ip))
                return False
            else:
                logging.info("%s is alive" % (self.name_or_ip))
                return True
        except:
            logging.error('ping error')
            return False

    # --------------------------------------------------------------------------

    def connect(self):
        """ Make a telnet connection to the this host
        If a succesfull connection is made, the .connected datestamp will be set
        @returns: True on success, False in case of a failure
        """

        try:
            logging.info('Connecting to %s:%s' % (self.name_or_ip, self.port))
            self.socket = telnetlib.Telnet(self.name_or_ip, self.port)
            if self.socket:
                logging.info('Connected')
                self.connected = datetime.datetime.now()
                return True
        except:
            logging.error('Error in connecting to %s' % self.name_or_ip)
            print '-'*60
            traceback.print_exc(file=sys.stdout)
            print '-'*60
            return False

    # -----------------------------------------------

    def disconnect(self):
        """ Disconnect the telnet connection to the this host
        @returns: Nothing
        """

        logging.debug('Disconnecting from host')
        self.socket.close()

    # --------------------------------------------------------------------------

    def read_until(self, timeout=5):
        """ Read available data for the given number of seconds
        @param timeout: The number of seconds
        @returns: The data which was received
        """

        s = self.socket.read_until( "xyxyxy" , timeout)
        return s

    # --------------------------------------------------------------------------

    def expect(self, tags=[], timeout=5):
        """ Read available data till one of the given tags (regular expressions) is found
        @param timeout: The number of seconds
        @returns: A tuple of the tagindex and the data received
        """

        if not tags:
            tags = ['\n']

        index, matchobj, text = self.socket.expect(tags, timeout)
        if matchobj:
            #logging.debug('host.expect found match for tags %s\nindex=%d matchobj=%s text=\n%s' % (tags, index, matchobj, text))
            return index, text
        else:
            logging.error('host.expect timout. No match found for tags %s\nindex=%d matchobj=%s text=\n%s' % (tags, index, matchobj, text))
            #for s in tags:
            #    if '?' in s or '*' in s:
            #        logging.info('Maybe you forgot to escape the meta characters "?" or "*" in the string "%s"\n' % s)
            return None, None

    # --------------------------------------------------------------------------

    def login(self, username='', password = ''):
        """ Login on the host with the given username and password
        @returns: True on success, False in case of an error
        """

        #logging.debug('Waiting for login prompt')
        tags = ['login:', 'Login:', 'Callsign:', 'callsign:', 'call:']
        index, s = self.expect(tags)
        logging.info('index:%s string=%s' % (index,s))
        if s:
            self.send(username + '\r\n')
        else:
            logging.error('No login prompt recognized')
            return False

        #logging.debug('Waiting for password prompt')
        tags = ['password:', 'Password:', 'Password.*:']
        index, s = self.expect(tags)
        logging.info('index:%s string=%s' % (index,s))
        if s:
            self.send(password + '\r\n')
        else:
            logging.info('No password prompt recognized')
            return False

        return True

    # --------------------------------------------------------------------------

    def send(self, s):
        """ Send a string to the connected host
        """

        logging.debug('sending "%s"' % `s`)
        self.socket.write(s)

    # --------------------------------------------------------------------------

    def interact(self):
        """ Interact with the connected server
        """

        logging.debug('Starting interactive mode')
        self.socket.interact()


# ----------------------------------------------------------------------------
#
# ----------------------------------------------------------------------------
def get_hosts(filename = 'hosts.ini'):
    """ Process an ini file and return a dictionary with host information
    @param filename: Name of file to process
    @returns: Dictionary of hosts

    Format of the hosts.ini file is a comma seperated text file, containg
    hostname, port, comments, version

    Example:
    third.aprs.net,10152,Full APRS-IS Feed (Ohio)
    """

    hosts = {}      #: Dictionary of hosts

    # If the filename was not found, return an empty list
    if not os.path.isfile(filename):
        logging.error('file %s was not found' % filename)
        return {}

    # Open the file and read the contents
    f = open(filename)
    lines = f.readlines()
    f.close()

    # Process all the lines
    for line in lines:
        line = line.strip()
        if not line:
            continue
        if line.startswith('#') or line.startswith(';'):
            continue
        s = line.split(',')

        name = ''
        port = 23
        comment = ''

        if len(s) > 0:
            name = s[0].strip()
        if len(s) > 1:
            port = int(s[1])

        host = Host(name, port)
        hosts[name] = host      # Add this host to the dictionary

    # Return the generated dictionary
    return hosts

# ----------------------------------------------------------------------------
#
# ----------------------------------------------------------------------------

def write_ini(hosts, filename='heard.ini'):
    """ Write all host information to an INI file
    @param hostlist: List of hosts show
    @param filename: Name of file to write
    @returns: Nothing
    """

    f = open(filename, 'w')
    for h in hosts:
        host = hosts.get(h)
        if not host.connected:
            continue
        s = ("%s,%d,%s,%s" % (host.name_or_ip, host.port, host.comment, host.version))
        f.write(s)
    f.close()


# ----------------------------------------------------------------------------
#
# ----------------------------------------------------------------------------

def show_all(hosts):
    """ Show all stations in the given host list
    @param hostlist: List of hosts show
    @returns: Nothing
    """

    for h in hosts:
        host = hosts.get(h)
        print ("%s:%d - %s / %s / %s" % (hosts.name_or_ip, hosts.port, hosts.comment, hosts.version, hosts.connected))

# ----------------------------------------------------------------------------
#
# ----------------------------------------------------------------------------

def handle_aprsd(host):
    """ Handle aprsd host
    @param host: host object
    @returns: Nothing
    """

    return

# ----------------------------------------------------------------------------
#
# ----------------------------------------------------------------------------

def handle_ka9q(host):
    """ Handle KA9Q host
    @param host: host object
    @returns: Nothing
    """

    host.login('PA3BTL', '')

    host.expect(['=>'])
    host.send('P\n')        # Ask for Ports
    host.expect(['=>'])
    host.send('N\n')        # Ask for Nodes
    host.expect(['=>'])
    host.send('J\n')        # Ask for Jheard (will list AX.25 systems it heard)
    host.expect(['=>'])
    host.send('B\n')

    #host.interact()

    return

# ----------------------------------------------------------------------------
#
# ----------------------------------------------------------------------------

def handle_javaAPRS(host):
    """ Handle javaAPRS host
    @param host: host object
    @returns: Nothing
    """

    host.send('user PA3BTL pass -1\n')
    host.expect(['Hello HenkiePenkie'], timeout=2)

    #host.interact()

    return

# ----------------------------------------------------------------------------
#
# ----------------------------------------------------------------------------

def handle_dxspider(host):
    """ Handle DX Spider host
    @param host: host object
    @returns: Nothing
    """

    host.login('PA3BTL', '')

    return
# ----------------------------------------------------------------------------
#
# ----------------------------------------------------------------------------

def handle_xrouter(host):
    """ Handle Xrouter host
    @param host: host object
    @returns: Nothing
    """

    host.login('PA3BTL', 'guest')

    host.expect(['Type \? for list of commands.'])
    host.send('P\n')        # Ask for Ports
    host.expect(['\r\n\r\n'])
    host.send('N\n')        # Ask for Nodes
    host.expect([' nodes found'])
    host.send('B\n')

    #host.interact()

    return

# ----------------------------------------------------------------------------
#
# ----------------------------------------------------------------------------
def connect_all(hostlist, db):
    """ Connect to all stations in the given host dictionary
    @param hostlist: Dictionary of hosts to connect to
    @returns: Updated hosts dictionary.

    During the connection process, the flag 'success' will be set (or not)
    """

    tags = ['.*javAPRS.*','.*aprsd.*', '.*KA9Q.*', '.*Xrouter.*', '.*AR-Cluster.*', '.*DX-Spider.*', '.*DXSpider.*', '.*clx.*' '.*CC Cluster.*']

    for h in hostlist:
        logging.info('-------------------------------------------------')
        logging.info(h)
        #try:
        host = hostlist.get(h)
        #success = host.ping()
        db.add(host)
        now = datetime.datetime.now()
        host.pinged = now
        db.update(host, pinged = now)
        host.connect()
        if host.connected:
            db.update(host, connected = now)
            index, s = host.expect(tags)
            logging.info("\n%s" % s)
            if index == 0:
                handle_javaAPRS(host)
            elif index == 2:
                handle_ka9q(host)
            elif index == 3:
                handle_xrouter(host)
            elif index == 4 or index == 5:
                handle_dxspider(host)
            else:
                # Try to login anyway
                host.login('PA3BTL', '')

            host.disconnect()

        #except:
        #    logging.error('Problem in connection to %s' % h)

# ----------------------------------------------------------------------------
#
# ----------------------------------------------------------------------------
def ping_all(hostlist):
    """ Connect to all stations in the given host dictionary
    @param hostlist: Dictionary of hosts to connect to
    @returns: Updated hosts dictionary.

    During the connection process, the flag 'success' will be set (or not)
    """

    for h in hostlist:
        host = hostlist.get(h)
        success = host.ping()


# ----------------------------------------------------------------------------
#
# ----------------------------------------------------------------------------
def main():
    '''

    '''

    #ini = dict4ini.DictIni('hosts.ini')
    #print ini

    db = stationdb.StationDatabase('stations.db')

    hosts = get_hosts('hosts.ini')
    connect_all(hosts, db)
    write_ini(hosts, 'test.ini')

    #hosts = get_hosts('hosts.ini')
    #ping_all(hosts)

# ----------------------------------------------------------------------------
#
# ----------------------------------------------------------------------------

if __name__ == '__main__':
    """
    """

    #-----------------------------------
    # Logging setup
    #-----------------------------------
    logging.basicConfig(level=logging.DEBUG,
                    format='%(levelname)-8s %(message)s',
                    filename = 'igate.log',
                    filemode = 'w')

    # define a Handler which writes ERROR messages or higher to the sys.stderr
    console = logging.StreamHandler()
    console.setLevel(logging.DEBUG)
    logging.getLogger('').addHandler(console)   # add the handler to the root logger

    main()




"""
KA9Q NOS (k2mf.ampr.org)

You have reached the TCP/IP Mailbox Server at k2mf.ampr.org located in
Wayne, NJ.  Please log in with your amateur radio call sign.  Thank you!

login: Password:

[MFNOS-1.31-HM$]

Welcome pa3btl, to the k2mf.ampr.org [44.64.20.100]
NOS TCP/IP Mailbox (K2MF v1.31.3 80386 build 071210.145847)

MFNOS is now a lean and very mean cross-interface digipeating machine!

WANTED: A PBBS pmail and bulletin suck bucket for the northeast region of
        the U.S.  Must be FlexNet-ready and not require any intervention
        by humans.  Contact devnull@LinWink2K.biz.  No serious offer will
        not be considered.  LinWink2K is a proud distributor of equal
        opportunity and ampr.org-filtered bloatware.

DigiPeeps, DxPeeps, FaxPeeps, FlexPeeps, NodePeeps, PbbsPeeps and you...
Flexing the Net together for over 20 years!

There is currently 1 user
The Escape character is: CTRL-T

IPMF:K2MF-8} Area: pa3btl Current message # 0
?,B,C,CONV,DXC,E,F,H,I,IP,J,K,L,N,NS,P,Q,R,RW,S,T,U,USC,WHO,WX,X
=> BBSGAA:N4GAA-4    BBSURO:N1URO-4    BJGNET:K2BJG-5    CEFLA:K2BJG-11
DXBJG:K2BJG-6     DXNURO:N1URO-2    FBBBJG:K2BJG-13   FXWHVN:N4GAA-7
IPBJG:K2BJG-10    IPCNJ:WB2ONA-1    IPGE:K2GE-8       LINGAA:N4GAA-3
LNXBJG:K2BJG-12   MACTFX:N1URO-9    NS105:VE1PPR-7    PPRDXC:VE1PPR-2
QSONNJ:K2MF-11    QSOSCT:N4GAA-11   RBRA:K2GE-2       SPGMA:N1URO-11
TROY:N2TY-5       UROHUB:N1URO-3    USRCT:N1URO-6     WHAVEN:N4GAA-2
WHVNX:N4GAA-15    WMASS:N1URO-14

IPMF:K2MF-8} Area: pa3btl Current message # 0
?,B,C,CONV,DXC,E,F,H,I,IP,J,K,L,N,NS,P,Q,R,RW,S,T,U,USC,WHO,WX,X
=>
k2mf.ampr.org [44.64.20.100]
KA9Q NOS version 911229 (K2MF v1.31.3 80386 build 071210.145847)

Welcome to K2MF's TCP/IP system!

My system is configured as follows:

Host ID:          k2mf.ampr.org
IP Address:       [44.64.20.100]
Net/Rom Node:     IPMF:K2MF-8
TCP/UDP Servers:  SYSTAT   Port 11    Systat Server
                  FTP      Port 21    File Transfer Protocol Server
                  TELNET   Port 23    Terminal/Mailbox Server
                  SMTP     Port 25    Simple Mail Transfer Protocol Server
                  FINGER   Port 79    Finger Server
                  PORTMAP  Port 111   Portmap Server
                  NODE     Port 3694  Node/Mailbox Server

You may obtain more information about the computer configuration at
this machine by fingering:

sysdiag  - or -  system

Please NOTE:  There are no email services available at this machine
for mailbox users, however, you may leave a message for me using the
'sp sysop' command and feel free to otherwise fully explore the system.

Have fun!

IPMF:K2MF-8} Area: pa3btl Current message # 0
?,B,C,CONV,DXC,E,F,H,I,IP,J,K,L,N,NS,P,Q,R,RW,S,T,U,USC,WHO,WX,X
=>

"""


"""

Xrouter v187f

Callsign: pa3btl

Password (or "guest"): guest
ZL2AQY-1 AX25/IP Router.
Type ? for list of commands.
?

ZL2AQY-1:AK} Basic commands:

<C>onnect <B>ye <I>nfo <M>heard <N>odes <P>orts <R>outes <U>sers <V>ersion
DXC       BBS       ZL2BAU    ZL2TZE    NASA      CHAT      VK1       VK2
VK4       VK5       VK6       VK7

Use "? *" for full list
Use "? <cmd>" for specific help on <cmd>
? *

ZL2AQY-1:AK} Full command list:

?         arp       amsg      bye       connect   chat      ctrl
dx        echo      finger    gping     help      host      info
ip        iproutes  j         links     mheard    man       nodes
nping     nrr       ports     ping      pms       quit      routes
stats     send      telnet    tracert   ttylink   users     version
wx        yell      xlink
DXC       BBS       ZL2BAU    ZL2TZE    NASA      CHAT      VK1       VK2
VK4       VK5       VK6       VK7

Use "? <cmd>" for specific help on <cmd>
m

ZL2AQY-1:AK} Syntax: M[heard] <portnum>
n

ZL2AQY-1:AK} Nodes:
MINAS:CX2SA-5     RESint:DB0RES-10  WUENET:DK0WUE     SHANDX:EI7SDX
SDXCHT:EI7SDX-8   SDXIP:EI7SDX-9    BWT28:F4BWT-5     :F6GGY-4
WOOL:G0CGL-1      WOOLPM:G0CGL-2    WOCHAT:G0CGL-8    BLOX:G0CNG-8
MAXCHT:G0CNG-9    ELMDX:G0MDV       SOTON:G1GXB-3     ESX:G1NNB
ESXCHT:G1NNB-8    SWAR:G3LDI        NOR1:G4VLS        NORWI:G4VLS-6
HJPCHT:G6HJP-1    FIELD:G6HJP-3     DIRPMS:G7DIR-2    PLMCHT:G7DIR-8
KIDDER:G8PZT      KDRCHT:G8PZT-8    KDRPTR:GB3KD      BEDTEL:GB7BED-8
WOOLDX:GB7CGL     SYRCHT:GB7CP-8    DXC2:GB7DXC-2     DXECLU:GB7DXE
DXE:GB7DXE-1      ESXBBS:GB7ESX     PLYMTH:GB7GZ      LDIBBS:GB7LDI
LGCCHT:GB7LGS-7   CHELT:GB7LGS-9    MAXBBS:GB7MAX     NEWPRT:GB7NT
BBSPFD:GB7PFD     KDRBBS:GB7PZT     DX:GB7RDX-1       TUTBBS:GB7TUT
ENFLD:GB7TUT-1    YORKS:GB7YB       YBCHAT:GB7YB-8    DXYORK:GB7YDX-8
YRK:GB7YK         YKSNTS:GB7YKS     YKS:GB7YKS-1      PXLNET:HG8PXL
WNYDX:K2CAN-2     FGRLKS:K2CAN-3    BRGH2O:KA0MOS-7   MONIP:KB5WBH-11
MONCHT:KB5WBH-8   GVCITY:KG6BAJ-2   PLYDX:N0AT        COUBBS:N0LBA
COU:N0LBA-1       COUCHT:N0LBA-8    XUAN:N1UAN-3      TRFDX:N5IN-3
MSYGAT:N5OMG-4    WIRAC:N9PMO-2     ILDIA:N9ZZK-5     MINAS:NOCALL
SJCCTY:OK2PEN-5   DAZapp:PI1DAZ-10  NORA2:SP7MGD-11   X-LAM:SV1CMG-11
CMGCHT:SV1CMG-13  CMGPMS:SV1CMG-2   X-CMG:SV1CMG-8    STGEOR:VA2PGN-4
TRGATE:VA2TRG-5   XHAR:VE2HAR-6     HARGW:VE2HAR-9    JOLCHT:VE2HOM-13
XRJOL:VE2HOM-4    pktbbs:VE2PKT     STECAT:VE2PKT-4   CHTpkt:VE2PKT-8
PETERB:VE3KPG-2   SFALLS:VE3UIL-7   BPOINT:VK2AAL     AFYNOD:VK2AFY
DOTNOS:VK2DOT     DOTNOD:VK2DOT-1   DOTFBB:VK2DOT-2   DOTCHT:VK2DOT-3
DOTNOS:VK2DOT-5   KARION:VK2KJ      X-ATM:VK3ATM-5    X-AYM:VK3AYM-5
HGRBBS:VK6HGR-14  HGRNOD:VK6HGR-15  HDMCHT:VK7HDM-14  HDMDXC:VK7HDM-2
HDMPMS:VK7HDM-5   XTAS:VK7HDM-7     HDMBBS:VK7HDM-9   AXBBS:VK7NW
MAPES:YO6PLB-5    NSHORE:ZL1AB-1    NSHPMS:ZL1AB-2    NSHBBS:ZL1AB-5
NSHCHT:ZL1AB-8    OTO:ZL1IS-1       HAMNOD:ZL1PK-1    PKPMS:ZL1PK-2
HAMCHT:ZL1PK-8    AKLDXC:ZL2AQY-10  AQYPMS:ZL2AQY-2   AKLCHT:ZL2AQY-8
KAPNOD:ZL2ARN-1   ZLDXC:ZL2ARN-10   ZLTNN:ZL2ARN-3    TNNZL:ZL2ARN-5
KAPCHT:ZL2ARN-7   ARNPMS:ZL2ARN-9   BAUBBS:ZL2BAU     BAUPMS:ZL2BAU-2
BAUNOD:ZL2BAU-3   BAUCHT:ZL2BAU-8   ZLBBS:ZL2TZE      MLBNOD:ZL2TZE-1
TZEBBS:ZL2TZE-2   TZENOD:ZL2TZE-3   MLBCHT:ZL2TZE-7   TZECHT:ZL2TZE-8
4GQBBS:ZL4GQ      INVNOD:ZL4OKY-1   INVCHT:ZL4OKY-8

135 nodes found
p

ZL2AQY-1:AK} Ports:
  1 Auckland Data 145.625
  2 Auckland Data 438.350
  3 BBS 144.600
  4 AXUDP link to VE2PKT - Quebec, Canada
  5 AXUDP link to G8PZT -Kidderminster, England
  6 AXUDP Link to ZL2ARN-1 - Waikanae, N.Z.
  7 AXUDP Link to ZL2BAU - Waimate, NZ.
  8 AXUDP link to ZL2UFW - Waitara, NZ.
  9 AXUDP link to ZL1AMC-1 - Hokitika, NZ
 10 AXUDP Link to ZL1PK-1 - Hamilton, NZ.
 11 AXUDP link to TU5EX-7 - Ivory Coast, Africa.
 12 AXUDP link to ZL2TZE - Blenheim, NZ.
 13 AXUDP Link to VK6HGR - Perth, Western Australia
 14 AXUDP Link to VK3RPA-3 - Ashburton, Victoria Australia
 15 AXUDP link to VK7HDM-7 - Gagebrook, Tasmania
 16 AXUDP Link to VK4TTT-7 - Goldcoast Australia
 17 AXUDP Link to KB7RSI-3 - Las Vegas USA
 18 AXUDP link to VK2DOT-1 - Niagara Park, Australia.
 19 AXIP Link to VK5ASF - Shepparton, South Australia
 20 Ethernet LAN
 21 AXUDP link to ZL2AL-1 - Hastings NZ
 22 AXUDP link to ZL4OKY-1 - Invercargill, NZ

r

ZL2AQY-1:AK} Routes:
Port Callsign  Qty Nod
>  6 ZL2ARN-1  150  75
> 13 VK6HGR-15  50   2
> 22 ZL4OKY-1  150  14
>  3 ZL1AB-1   100   4
x 10 ZL1PK-1     0   4
> 18 VK2DOT-1   10  23
>  4 VE2PKT-4   10  45
>  5 G8PZT      10  31
> 15 VK7HDM-7   10   9
> 12 ZL2TZE-3  150  16
>  7 ZL2BAU-3  150  42
(End of list)
b

ZL2AQY-1:AK} Goodbye


Connection to host lost.

"""