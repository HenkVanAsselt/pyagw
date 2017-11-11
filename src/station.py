##@file STATION.PY
##@brief Station handler
##@AUTH Henk van Asselt, PA3BTL

#
# HIST: 20030411
# 

import logging

stationlist = []



#----------------------------------------------------------------------------#
#       CLASS: Station
#----------------------------------------------------------------------------#
class Station:
    """ Class Station handles the information about received stations
    """

    # Class Initialization
    def __init__(self,callsign=None):
        """! @brief Class initialization
             @param self -
             @param call Callsign
             @returns Nothing
        """
        
        self.callsign = None
        self.info = None
        self.path = None         # Path through which station is heard.
        self.postition = None
        self.timestamp = None    # Last heard timestamp from this station
        
        if callsign:
            self.callsign = callsign

    def set_callsign(self,callsign=None):
        """! @brief Set Station callsign
        """
        self.callsign = callsign
        
    def set_info(self,info=None):
        """! @brief Set Station information string
        """
        self.info = info
        
    def set_path(self,path=None):
        """! @brief Set Station information string
        """
        self.path = path
        
        
def get_stationlist():
    """! @brief get station list
         @returns list of stations
    """
    return stationlist
        
if __name__ == '__main__':
    """! @brief main for APRS.PY
         @returns Nothing
    """
    
    logging.basicConfig( level=logging.INFO, format='%(levelname)s %(message)s' )
    logging.info( 'testing connect.py' )
          
