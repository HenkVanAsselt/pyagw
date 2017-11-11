##
## @file FRAME.PY
## @brief The contents of one frame and a list of frames
## @AUTH Henk van Asselt, PA3BTL
##

# Global imports
import logging
import re

# Local imports
import aprs
import station

class Frame:
    """! @brief Frame class, contains all data of a frame (splitted)
    """

    def __init__( self, raw = '', format='APRSD' ):
        self.raw = raw              # Will contain the whole frame contents
        self.format = format        # An format identifier (like APRSD or AGWPE)
        self.callsign = None        # The callsign of the originator
        self.to =   None            # The destination callsign
        self.path = None            # The path (via)
        self.data = None            # The datapart of the frame
        self.position = None        # Position part of the message
        self.timestamp = None       # Timestamp part of the message
        self.status = None          # Status message
        self.message = None         # Message message

        if len( self.raw ) > 0:
            self.decode()

    def decode( self ):
        '''@ brief Decode the given frame.

            This can only be done if the format is known.
        '''
        if self.format == 'APRSD':
            #logging.info(70 * '*')
            #logging.debug('Decode %s' % self.raw)

            if self.raw[0] == '#':      # Comment frame
                return
            if len(self.raw) == 0:      # Empty line
                return

            # Split the frame in the header part and the data part
            t = self.raw.split(':',1)
            self.header = t[0]
            self.data = t[1]
            #logging.info('header: %s' % self.header)
            #logging.info('data:   %s' % self.data)


            # Split the header in the from, to and path components
            t = re.split('[>,]',self.header)
            self.callsign = t[0]
            self.to = t[1]
            self.path = t[2:]
#            logging.info('from: %s', self.callsign)
#            logging.info('to:   %s', self.to)
#            logging.info('path: %s', self.path)
#            logging.info('type: %s', aprs.type_description(self.data))
#            logging.info('data: %s', self.data)

            if aprs.is_position(self.data):
                self.position = aprs.position(self.data)
#            if self.position:
#                logging.info('position: %s', self.position)

            if aprs.is_timestamp(self.data):
                self.timestamp = aprs.timestamp(self.data)
#            if self.timestamp:
#                logging.info('timestamp: %s', self.timestamp)

            if aprs.is_status(self.data):
                self.status = aprs.status(self.data)
#            if self.status:
#                logging.info('status: %s', self.status)

            if aprs.is_message(self.data):
                self.message = aprs.message(self.data)
#            if self.message:
#                logging.info('message: %s', self.message)

    def variables(self):
            return self.__dict__

def load(filename):
    """! @brief load a trace file from disk
         @param filename Name of file to load
         @returns List of frame strings
    """
    f = open(filename,'r')
    s = f.read()
    f.close()
    lines = s.splitlines()
    return lines

def get_callsigns(frames):
    """! @brief Get callsigns out of a list of frames
         @param Name of list of frames
         @returns Sorted list of unique callsigns
    """

    # Create list of callsigns
    callsigns = []
    for frame in frames:
        if frame.callsign:
            callsigns.append(frame.callsign)
    
    # Remove duplicates
    callsigns = uniq(callsigns)
    
    # print sorted list
    callsigns.sort()
    return callsigns


def uniq(alist):    # Fastest order preserving
    set = {}
    return [set.setdefault(e,e) for e in alist if e not in set]

if __name__ == '__main__':
    """! @brief main for APRS.PY
         @returns Nothing
    """

    logging.basicConfig( level=logging.DEBUG, format='%(levelname)s %(message)s' )
    logging.info( 'testing frame.py' )

    format = "APRSD"
    #s = 'IR3BH>APNU19,IQ3MF*,WIDE,qAo,IR3CO-5:!4551.72N/01330.23E# / APRS DIGIPEATER sez.ARI MONFALCONE (GO)'

    lines = load('..\\log\\log_20060214_1.txt')

    frames = []
    for s in lines:
        frame = Frame(s, format )
        frames.append(frame)
        
    callsigns = get_callsigns(frames)
    print callsigns
    
    stationlist= {}        # List of stations
    for call in callsigns:
        s = station.Station(call)
        stationlist[call] = s 
    print stationlist
        
    for f in frames:
        if f.callsign:
            s = stationlist.get(f.callsign,None)
            if s:
                if frame.path:
                    s.path = frame.path
                if frame.position:
                    s.position = frame.position
                if frame.timestamp:
                    s.timestamp = frame.timestamp
                print 'frame: ',f.__dict__
                print 'station: ',s.__dict__
    

    




