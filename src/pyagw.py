##
## @file PYAGW.PY
## @brief SUBJ: Test Interface to the AGWPE Engine
## @AUTH Henk van Asselt, PA3BTL
##

"""AGWPE Interface
AUTH: Henk van Asselt, PA3BTL
HIST: 20030411
SUBJ: Terminal program for AGWPE.
"""

# Imports

import asyncore, sys
import msvcrt       # for keyboard handling

# Import AGWPE specific modules
import agwpe
import aprs


#-----------------------------------------------

def keyboard():
    """! @brief
       ! @returns
    """
    """ Handle keyboard input
    """

    x = msvcrt.getch()

    if x == 'h' or x == 'H':
        # Ask for heard station list
        print
        print 'Stations heard by AGWPE'
        print '======================='
        for n in range(agwpe.ports):
            c.heard(n)
        print

    if x == 'm' or x == 'M':
        # Toggel monitor state
        c.monitor(3)

    if x == 'q' or x == 'Q':
        sys.exit(0)

#===============================================================================
#    if x == 's' or x == 'S':
#        # Show list of heard stations and their information
#        print
#        print 'Station info'
#        print '============'
#        for call in station.stationlist:
#            pos = station.pos.get(call,'')
#            info = station.info.get(call,'')
#            print '%10s : %18.18s %s' % (call,pos,info)
#        print
#===============================================================================

    if x == 'a' or x == 'A':
        # Toggle APRS decoding flag
        aprs.do_decoding()


#-----------------------------------------------

if __name__ == '__main__':
    """! @brief
       ! @returns
    """

    c = agwpe.Client('PA3BTL-5')
    c.monitor(1)    # Turn monitoring on

    #for n in range(agwpe.ports):
    #    c.heard(n)
        
    while(1):
        try:
            asyncore.poll(1)
        except KeyboardInterrupt:
            print 'kbd interrupt'
            sys.exit(0)
        if msvcrt.kbhit():
            keyboard()

    #atexit.register(tx.unregister("PA3BTL-1"))




