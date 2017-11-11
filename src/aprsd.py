##
## @file APRSD.PY
## @brief APRSD Decoding routines
## @AUTH Henk van Asselt, PA3BTL
##

# Global imports
import logging

# Local imports
import aprs
import station

#===============================================================================
#
#===============================================================================
def info(message):
    """! @brief Split an APRSD message in the path part, and the information part
         @returns The information part, None in case of an error

         >>> info('IR3BH>APNU19,IQ3MF*,WIDE,qAo,IR3CO-5:!4551.72N/01330.23E# / APRS DIGIPEATER sez.ARI MONFALCONE (GO)')
         '!4551.72N/01330.23E# / APRS DIGIPEATER sez.ARI MONFALCONE (GO)'
    """

    s = message.split( ':', 1 )
    if len(s) < 2:
        logging.error('Error in info(%s)' % message)
        return None
    return s[1]

#===============================================================================
#
#===============================================================================
def path( s ):
    """! @brief Split an APRSD message in the path part, and the remainder
         @returns The path, None in case of an error

         >>> path('DW8004>APRS,TCPXX*,qAX,CWOP-2:@302044z4252.23N/07252.92W_251/001g006t080r000p069P001h53b10137.DsVP')
         'DW8004>APRS,TCPXX*,qAX,CWOP-2'
    """


    l = s.split( ':', 1 )
    if len( l ) < 2:
        logging.error('split_aprsd_path() has < 2 arguments (%s)' % s)
        return None
    return l[0]

#===============================================================================
#
#===============================================================================
def fromtovia(message):
    """! Given an APRSD string, return the from, to and via parts
        @param message The APRS string
        @returns tuple with (from,to,via), (None,None,None) in case of failures.

        >>> fromtovia('IR3BH>APNU19,IQ3MF*,WIDE,qAo,IR3CO-5:!4551.72N/01330.23E# / APRS DIGIPEATER sez.ARI MONFALCONE (GO)')
        ('IR3BH', 'APNU19', 'IQ3MF*,WIDE,qAo,IR3CO-5')
    """


    l = message.split( ':', 1 )
    if len( l ) < 2:
        print('Error in splitting path()')
        return None,None,None

    x = l[0].split('>')

    src = x[0]
    logging.debug('from: %s' % src)

    x = x[1].split(',',1)
    dest = x[0]
    logging.debug('to: %s' % dest)

    via = x[1]
    logging.debug('via: %s' % via)

    return src, dest, via

#===============================================================================
#
#===============================================================================
def src(message):
    """! Given an APRSD string, return the call of the originator

         @returns callsign of the originator, or None in case of failures.
         >>> src('IR3BH>APNU19,IQ3MF*,WIDE,qAo,IR3CO-5:!4551.72N/01330.23E# / APRS DIGIPEATER sez.ARI MONFALCONE (GO)')
         'IR3BH'
    """

    s, t, v = fromtovia(message)
    return s

#===============================================================================
#
#===============================================================================
def dest(message):
    """! @brief Given an APRS message, return the destination call
        @param message The APRS message to decode
        @returns callsign of destination, or None in case of error

        >>> dest('IR3BH>APNU19,IQ3MF*,WIDE,qAo,IR3CO-5:!4551.72N/01330.23E# / APRS DIGIPEATER sez.ARI MONFALCONE (GO)')
        'APNU19'
    """

    s, t, v = fromtovia(message)
    return t

#===============================================================================
#
#===============================================================================
def via(message):
    """! @brief Given an APRS message, return the via path
        @param message The APRS message to decode
        @returns via path.

        >>> via('IR3BH>APNU19,IQ3MF*,WIDE,qAo,IR3CO-5:!4551.72N/01330.23E# / APRS DIGIPEATER sez.ARI MONFALCONE (GO)')
        'IQ3MF*,WIDE,qAo,IR3CO-5'
    """

    s, t, v = fromtovia(message)
    return v

#===============================================================================
#
#===============================================================================
def receivedfrom(message):
    """! Given an APRSD string, return the callsign of which we received this message
         @returns string with the callsign

         Example:
         IR3BH>APNU19,IQ3MF*,WIDE,qAo,IR3CO-5:!4551.72N/01330.23E# / APRS DIGIPEATER sez.ARI MONFALCONE (GO)
         returns 'IQ3MF'

    """

    s, t, v = fromtovia(message)

    # Now having 'IR3BH>APNU19,IQ3MF*,WIDE,qAo,IR3CO-5'
    # Split the string on the comma's and look which callsign ends with '*'
    # Return this callsing (without the '*')
    x = v.split(',')
    for c in x:
        if c.find('*') > 0:
            return c[:-1]

    # If we bubble down to here, we did not find an '*'
    logging.error('No * found in %s' % message)
    return ''

#===============================================================================
#
#===============================================================================
def decode( message ):
    """! @brief Decode an string, given the APRSD format
         @returns Nothing

         >>> decode('IR3BH>APNU19,IQ3MF*,WIDE,qAo,IR3CO-5:!4551.72N/01330.23E# / APRS DIGIPEATER sez.ARI MONFALCONE (GO)')

    """

    p = path( message )
    f,t,v = fromtovia(message)
    i = info( message )

#===============================================================================
#
#===============================================================================

def test():
    """! test
         @returns Nothing
    """

    logging.basicConfig(level=logging.DEBUG,format='%(levelname)s %(message)s')

#    f = open('../log_20060214_1.txt.txt')
#    s = f.read()
#    f.close()
#
#    lines = s.split('\n')

    format = "APRSD"

    f = open('testdata.txt')
    lines = f.read()
    f.close()
    lines = lines.split('\n')

    for s in lines:
        logging.debug(s)
        if len(s) < 3:
            continue
        if s[0] == '#':
            continue
        data = info( s)
        print(aprs.type_description(data))
        print(fromcall(s))
        print(tocall(s))

        stat = station.Station()
        stat.set_callsign(fromcall(s))

        station.stationlist.append(stat)

    print('%d stations heared' % len(station.stationlist))
    for stat in station.stationlist:
        print(stat.callsign)

#===============================================================================
#
#===============================================================================
if __name__ == '__main__':

    import doctest
    doctest.testmod()