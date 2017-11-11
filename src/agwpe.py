##
## @file AGWPE.PY
## @brief SUBJ: Main Interface to the AGWPE Engine
## @AUTH Henk van Asselt, PA3BTL
##
## HIST: 20030411
##


# Global imports
import asyncore, socket
import struct

# Local imports
import rx

# Variables

# Global variables, which are valid for the AGW Packet Engine itself,
# Regardless of the client.
# (Maybe I should put the client itself in another module....)
version = None      # AGWPE version number
ports = None        # Number of ports
port = []           # Port information


#----------------------------------------------------------------------------#
#       CLASS: FRAME
#----------------------------------------------------------------------------#
class Frame:
    """! @brief Class Frame handles the frames to and from AGWPE
    """

    def __init__(self):
        """! @brief Class initalization
             @param -
             @returns Nothing
        """
        # variables
        self.port     = 0
        self.datakind = ''
        self.pid      = 0
        self.callform = ''
        self.callto   = ''
        self.datalen  = 0
        self.header   = ''
        self.data     = ''
        welf.callfrom = ''

    def disect_header(self):
        """! @brief Disect the AGWPE header
             @param self -
             @returns nothing

             The given input should be the 36 bytes header of an AGWPE frame.
        """

        # Get header out of buffer
        #header = self.in_buffer[0:36]

        if len(self.header) == 36:
            pass
        else:
            print 'header lenght is not 36'
        x=struct.unpack('B3sc3B10s10sii',self.header)

        self.port = x[0] + 1
        #reserved1 = x[1]
        self.datakind = x[2]
        #reserved2 = x[3]
        self.pid = x[4]
        #reserved3 = x[5]

        pos = x[6].find('\x00')
        self.callfrom = x[6][:pos]

        pos = x[7].find('\x00')
        self.callto = x[7][:pos]

        self.datalen = x[8]
        #user = x[9]

        # debug
        # print 'port %d  datakind %c  pid %d   from %s - to %s  len %d\n' % (self.port,self.datakind,self.pid,self.callfrom,self.callto,self.datalen)

    def make_header(self, port, datakind, pid, callfrom, callto, datalen):
        """! @brief Make a frame out of the required components
             @param self -
             @param port -
             @param datakind -
             @param pid -
             @param callfrom -
             @param callto -
             @param datalen -
             @returns Nothing
        """
        self.header = struct.pack('B3scBBB10s10sii', port, '', datakind, 0, pid, 0, callfrom, callto, datalen, 0)



#----------------------------------------------------------------------------#
#       CLASS: AGWPE_CLIENT
#----------------------------------------------------------------------------#
class Client(asyncore.dispatcher):
    """! @brief AGWPE Client
         @params asyncore.dispatcher base class
         @returns Nothing
    """

    def __init__(self, callsign='NOCALL', host='127.0.0.1', port=8000):
        """! @brief Intialize the class
             @param self -
             @param callsign Callsign to register
             @param host IP address of host to connect to
             @param port Port number of host to connect to
             @returns Nothing
        """

        # Save given parameters
        self.callsign = callsign
        self.host = host
        self.port = port

        # Set some variables to initial values
        self.debuglevel = 0
        self.in_buffer = ''
        self.out_buffer = ''
        self.monitor_state = 0      # 0 is off, 1 is on

        # Create a socket and connect to AGWPE
        asyncore.dispatcher.__init__(self)
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connect( (host, port) )    # Connect to the host and port, were AGWPE is running on.

        # Get information from the AGWPE engine
        self.get_version()          # Get AGWPE version number
        self.get_portinfo()         # Get AGWPE port information
        self.get_portcapabilities() # Get AGWPE port capabilities
        
        # Register given callsign in AGWPE
        self.register(callsign)     

#-------------------------------------

    def handle_connect(self):
        """! @brief Hanler for the intial connect.
             @param self
             @returns Nothing

             The initial connect is already done in during the initializations.
        """
        print "Connected to AGWPE"

#-------------------------------------

    def handle_read(self):
        """! @brief Handler for a read request
             @param self -
             @returns Nothing. The data read is stored in self.in_buffer
        """

        data = self.recv(8192)

        l = len(data)
        if self.debuglevel > 0:
            print 'received: %d bytes : %s' % (l, `data`)
        self.in_buffer = self.in_buffer + data

        bufferlen = len(self.in_buffer)
        if self.debuglevel > 0:
            print 'buffer: ', bufferlen, ' ', self.in_buffer

        while 1:
            bufferlen = len(self.in_buffer)
            if bufferlen < 36:
                # We did not yet receive at least the header.
                break
            else:
                # process the header
                frame = Frame()
                frame.header = self.in_buffer[0:36]
                frame.disect_header()
                if bufferlen < frame.datalen + 36:
                    break
                else:
                    # Handle the contents
                    rx.function[frame.datakind](self.in_buffer[36:36+frame.datalen])
                    # remove header (36 bytes fixed) and data (variable lenght) from buffer
                    self.in_buffer = self.in_buffer[36:]
                    self.in_buffer = self.in_buffer[frame.datalen:]

#-------------------------------------

    def writable(self):
        """! @brief ???
             @param self -
             @returns The length of the input buffer
        """
        return (len(self.in_buffer) > 0)

#-------------------------------------

    def handle_write(self):
        """! @brief ???
             @param self -
             @returns Nothing
        """
        print 'handle_write'
        sent = self.send(self.out_buffer)
        self.out_buffer = self.out_buffer[sent:]

#-------------------------------------

    def set_debuglevel(self, debuglevel):
        """! @brief Set debug level to a given level
             @param self -
             @param debuglevel The new debug level
             @returns
        """

        self.debuglevel = debuglevel

#---------------------------------------------------------------------

    def get_version(self):
        """! @brief Get the AGWPE version number
             @param self -
             @returns Nothing. The version number will be printed
        """

        frame = Frame()
        frame.make_header(0, 'R', 0, '', '', 0)
        while not version:
            self.send(frame.header)
            try:
                asyncore.poll(1)
            except KeyboardInterrupt:
                print 'version() kbd interrupt'
                return
        print 'AGWPE version %s' % version

    def get_portinfo(self):
        """! @brief Get the AGWPE port information
             @param self -
             @returns Nothing. The information will be printed
        """

        while not ports:
            frame = Frame()
            frame.make_header(0, 'G', 0, '', '', 0)
            self.send(frame.header)
            try:
                asyncore.poll(1)
            except KeyboardInterrupt:
                print 'version() kbd interrupt'
                return

    def get_portcapabilities(self):
        """! @brief Get the AGWPE port capabilities
             @param self -
             @returns Nothing. The information will be printed
        """

        for n in range(ports):
            frame = Frame()
            frame.make_header(n, 'g', 0, '', '', 0)
            self.send(frame.header)
            try:
                asyncore.poll(1)
            except KeyboardInterrupt:
                print 'version() kbd interrupt'
                return

    def version(self):
        """! @brief Return the AGWPE version number
             @param self -
             @returns The version number
        """

        return version

    def register(self, callsign):
        """Register the given callsign
         @param self -
         @param callsign Callsign to register
         @returns Nothing
        """
        print 'registering %s' % callsign
        frame = Frame()
        frame.make_header(0, 'X', 0, callsign, '', 0)
        self.send(frame.header)
        try:
            asyncore.poll(1)
        except KeyboardInterrupt:
            print 'register() kbd interrupt'
            return

    def monitor(self, on_off=2):
        """Toggle the monitor
         @param self -
         @param on_off Toggle parameter
         @returns Nothing

         Send an 'm' frame to turn monitoring on and off.
         It works like a toggle switch
        """

        frame = Frame()
        frame.make_header(0, 'm', 0, '', '', 0)
                
        if on_off == 1 or on_off == 0:
            # Only send new 'm' frame if new status is not equal the current status
            if self.monitor_state == 0 and on_off == 1:
                self.send(frame.header)
                self.monitor_state = 1
            if self.monitor_state == 1 and on_off == 0:
                self.send(frame.header)
                self.monitor_state = 0

        # If no value is given, toggle the current monitor_state
        else:
            self.send(frame.header)
            if self.monitor_state == 0:
                self.monitor_state = 1
            else:
                self.monitor_state = 0

        if self.monitor_state == 0:
            print 'monitor off'
        else:
            print 'monitor on'

    def heard(self, port):
        """! @brief Ask AGEPE for the stations heard on portnumber port
         @param self -
         @param port The portnumber to get the heard info from
         @returns Nothing.
        """

        print 'Ask for heard stations on port %d' % port

        frame = Frame()
        frame.make_header(port, 'H', 0, self.callsign, '', 0)
        self.send(frame.header)
        try:
            asyncore.poll(1)
        except KeyboardInterrupt:
            print 'register() kbd interrupt'
            return




