##@file TX.PY
##@brief SUBJ: Main Interface to the AGWPE Engine
##@AUTH Henk van Asselt, PA3BTL

"""AGWPE Interface
AUTH: Henk van Asselt, PA3BTL
HIST: 20030411
SUBJ: AGWPE transmit frames handler
"""

#-------------------------------------

def unregister(callsign):
    """! @brief Unregister the given callsign 
         @brief callsign Callsign to unregister
         @returns frame
    """
    frame = set_frame(0,'x',0,callsign,'',0)
    return(frame)

#-------------------------------------

def unproto(port, callfrom, callto, datalen):
    """! @brief Create unproto frame
         @param port -
         @param callfrom -
         @param callto -
         @param datalent -
         @returns frame
    """
    frame = set_frame(port, 'M', 0xF0, callfrom, callto, datalen)
    return(frame)

#-------------------------------------

