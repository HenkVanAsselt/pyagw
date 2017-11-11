## @file RX.PY
## @brief SUBJ: Main Interface to the AGWPE Engine
## @AUTH Henk van Asselt, PA3BTL


import struct
import string
import agwpe

import station
import aprs

#----------------------------------------------------------------------------#
#       CLASS: U_FRAME
#----------------------------------------------------------------------------#
class UI_Frame:
    """! @brief
         @returns
    """
    """ Class UI_Frame handles unnnumberd information frames (as used in APRS)
        An example is:

        2:Fm PE1FVU To APU25C Via TCPIP*,PI1FLD,WIDE2-2,TRACE5-5 <UI pid=F0 Len=61 >[22:38:01]
        =5230.01N/00457.30E-Wim, Purmerend, pe1fvu@amsat.org {UIV32}

    """

    # Class Initialization
    def __init__(self, header, data):
        """! @brief
             @returns
        """
        # variables
        self.port     = 0
        self.datakind = ''
        self.pid      = 0
        self.callform = ''
        self.callto   = ''
        self.via      = None
        self.datalen  = 0
        self.time     = ''
        self.header   = header
        self.data     = data

        s = self.header.split()
        #print 'ui_split: ',s

        # head:   2:Fm PE1NAT-2 To APZ023 Via PI1FLD,PE1FVU-4*,WIDE3-3 <UI pid=F0 Len=49 >
        # [20:43:45]
        # data:  =5242.43N/00450.83E#PHG5260/DIGI_NED voor Linux.
        # ui_split:  ['2:Fm', 'PE1NAT-2', 'To', 'APZ023', 'Via', 'PI1FLD,PE1FVU-4*,WIDE3-3
        # ', '<UI', 'pid=F0', 'Len=49', '>[20:43:45]']

        # Process Port, From and To. After that, delete first 4 entries of the list
        self.port = int(s[0][0:1])
        self.callfrom = s[1]
        self.callto   = s[3]
        s = s[4:]
        #print 'after del: ',s

        # If there is a route indicated, process the route, and delete the first 2 entries of the list
        if s[0] == 'Via':
            self.via = string.split(s[1],',')
            # Now we have a list of Via calls
            print 'VIA ',self.via
            s = s[2:]
            #print 'after del via: ',s
        #print '--> ',self.port,self.callfrom,self.callto,self.via

        self.pid = s[1][4:]
        self.datalen = s[2][4:]
        self.time = s[3][2:10]
        #print '--> ',self.pid,self.datalen,self.time
        #print
        #print 'Decoded: ',self.port,self.callfrom,self.callto,self.via,self.pid,self.datalen,self.time
        print '> ',header
        print '> ',data

        x = station.Station(self.callfrom)
        #aprs.show_id(self.data)
        pos = aprs.position(self.data)
        if pos:
            x.position(pos)




#===============================================================================
#
#===============================================================================
def rx_R_frame(s):
    """! @brief
         @returns
    """
    """ Received R frame
        Contains AGWPE version number
        sets global string 'version' in module agwpe
    """
    major = struct.unpack('H',s[0:2])
    minor = struct.unpack('H',s[4:6])
    agwpe.version = '%i.%i' % (major[0],minor[0])

#===============================================================================
#
#===============================================================================
def rx_G_frame(s):
    """! @brief
         @returns
    """
    """ Received information about the available AGW ports.
        about these ports.
        Set the global variable ports in the agwpe module,
        and build a list of port information
    """
    p = string.split(s,';')
    agwpe.ports = int(p[0])
    print 'number of ports: ',agwpe.ports
    for n in range(0,agwpe.ports):
        agwpe.port.append(p[n+1])
        print agwpe.port[n]

#===============================================================================
#
#===============================================================================
def rx_X_frame(s):
    """! @brief
         @returns
    """
    """ Received callsign registration result.
        Returns 1 if successfull, 0 if not
    """
    if s[0] == '\x01':
        print 'Registration successfull'
        return 1
    else:
        print 'Registration failed'
        return 0

#===============================================================================
#
#===============================================================================
def rx_g_frame(s):
    """! @brief
         @returns
    """
    """ Received frame 'g' with capabilities of a port
        At this moment, just print it. It will be used later
    """
    x = struct.unpack('8Bi',s)
    print x

#===============================================================================
#
#===============================================================================
def rx_U_frame(s):
    """! @brief
         @returns
    """
    """
    U-frame: sent by AGWPE when an AX.25 UI frame is detected directed to any registered callsign+SSID.
    """
    pos = string.find(s,'\x0d')
    head = s[:pos]
    data = s[pos+1:]
    #print 'head: ', head

    # Extract datalen
    pos = string.find(head,'Len=')
    if pos > 0:
        lenstr = ''
        i = 4
        c = head[pos+i]
        while c in string.digits:
            lenstr = lenstr + head[pos+i]
            i = i+1
            c = head[pos+i]
        #if len(lenstr) > 0:
        #    datalen = int(lenstr)

    else:
        print 'Unknown error: header does not contain lenght information'
        print head
        return

    #print 'data: ',data[:datalen]

    # Process the header of the UI frame
    UI_Frame(head,data)

    print

#===============================================================================
#
#===============================================================================
def rx_I_frame(s):
    """! @brief
         @returns
    """
    #--- Monitored connection Information
    #--- This frame is sent by AGWPE when an AX.25 UI frame is detected directed to any registered callsign+SSID.
    
    #print 'Monitored information on port', self.port
    print `s`

#===============================================================================
#
#===============================================================================
def rx_M_frame(s):
    """! @brief
         @returns
    """
    """
    M-frame: Monitored Supervisory Information
    """
    print 'Monitored Supervisory Information ',
    print `s`

#===============================================================================
#
#===============================================================================
def rx_T_frame(s):
    """! @brief
         @returns
    """
    """
    T-frame: Monitored Own Information
    """
    print 'Monitored own information ',
    print `s`

#===============================================================================
#
#===============================================================================
def rx_K_frame(s):
    """! @brief K-frame: Monitored information in RAW formate
         @param s -
         @returns Nothing. Just print an 's'
    """
    print 'Raw format: ',
    print `s`

#===============================================================================
#
#===============================================================================
def rx_H_frame(s):
    """! @brief Received frame with heard stations on a port
         @param s -
         @returns Nothing
    """

    pos = string.find(s,'\x00')
    while pos:
        heard_string = s[:pos]
        s = s[pos:]
        if string.find(heard_string,"00:00:00") > 0:
            pass
        else:
            print heard_string
        pos = string.find(s,'\x00')


#===============================================================================
#
#===============================================================================
function = {
    'R': rx_R_frame,
    'X': rx_X_frame,
    'G': rx_G_frame,
    'g': rx_g_frame,
    'y': None,    #frames_outstanding_on_a_port,
    'Y': None,    #frames_outstanding_on_a_connection,
    'H': rx_H_frame,
    'C': None,    #AX25_connection_received,
    'D': None,    #connected_AX25_data,
    'I': rx_I_frame,
    'M': rx_M_frame,
    'U': rx_U_frame,
    'T': rx_T_frame,
    'K': rx_K_frame
}
