## @file APRS.PY
## @brief AGWPE APRS Decoding routines
## @AUTH Henk van Asselt, PA3BTL\
##

# Global imports
import logging

# Variables
do_aprs_decode = 1


#===============================================================================
#
#===============================================================================
aprs_data_type_identifier = {
    '\x1c' : 'Current Mic-E Data (Rev 0 beta)',
    '<'    : 'Station Capabilities',
    '\x1d' : 'Old Mic-E Data (Rev 0 beta)',
    '='    : 'Position without timestamp (with APRS messaging)',
    '!'    : 'Position without timestamp (no APRS messaging), or Ultimeter 2000 WX Station',
    '>'    : 'Status',
    '"'    : '[Unused]',
    '?'    : 'Query',
    '#'    : 'Peet Bros U-II Weather Station',
    '@'    : 'Position with timestamp (with APRS messaging)',
    '$'    : 'Raw GPS data or Ultimeter 2000',
    '%'    : 'Agrelo DFJr / MicroFinder',
    'T'    : 'Telemetry data',
    '&'    : '[Reserved - Map Feature]',
    "'"    : 'Old Mic-E Data (but Current data for TM-D700)',
    '['    : 'Maidenhead grid locator beacon (obsolete)',
    '('    : '[Unused]',
    '\\'   : '[Unused]',
    ')'    : 'Item',
    ']'    : '[Unused]',
    '*'    : 'Peet Bros U-II Weather Station',
    '^'    : '[Unused]',
    '+'    : '[Reserved - Shelter data with time]',
    '_'    : 'Weather Report (without position)',
    ','    : 'Invalid data or test data',
    '`'    : 'Current Mic-E Data',
    '-'    : '[Unused]',
    '.'    : '[Reserved - Space weather]',
    '{'    : 'User-Defined APRS packet format',
    '/'    : 'Position with timestamp (no APRS messaging)',
    '}'    : 'Third-party traffic',
    ':'    : 'Message',
    ';'    : 'Object'
}

#===============================================================================
#
#===============================================================================
# Generic beacon-style destination addresses
# See APRS Protocol Reference V1.0 Chapter 4

beacon_address = [
    'AIR',  'ALL',   'AP',    'BEACON'  'CQ',     'GPS',   'DF',
    'DGPS', 'DRILL', 'DX',    'ID',     'JAVA',   'MAIL',  'MICE',
    'QST',  'QTH',   'RTCM',  'SKY',    'SPACE',  'SPC',   'SYM',
    'TEL',  'TEST',  'TLM',   'WX',     'ZIP'
]

def is_beacon(call):
    """! @brief Check if the given callsign is an APRS beacon address
         @param call Callsign to check
         @return 1 if it is a beacon, and 0 if it is not
    """

    if call.upper() in beacon_address:
        return 1
    else:
        return 0


#===============================================================================
#
#===============================================================================
digipeater_address = [
    'RELAY', 'ECHO', 'WIDE', 'TRACE', 'GATE'
]

def is_digipeater(call):
    """! @brief Check if the given callsign is an APRS digipeater
         @param call Callsign to check
         @return 1 if it is a digipeater address, 0 if it is not
    """

    if call in digipeater_address:
        return 1
    else:
        return 0

#===============================================================================
#
#===============================================================================
def do_decoding(on_off=2):
    """! @brief Toggle APRS decoding on or off
         @param on_off 0 = off / 1 == on / None == toggle
         @return Nothing
    """

    global do_aprs_decode

    if on_off == 1 or on_off == 0:
        # Toggle if the new status is not equal the old status
        if do_aprs_decode == 0 and on_off == 1:
            do_aprs_decode = 1
        if do_aprs_decode == 1 and on_off == 0:
            do_aprs_decode = 0

    # If no value is given, toggle the current monitor_state
    else:
        if do_aprs_decode == 0:
            do_aprs_decode = 1
        else:
            do_aprs_decode = 0

    if do_aprs_decode == 0:
        print 'aprs decoding off'
    else:
        print 'aprs decoding on'


#===============================================================================
#
#===============================================================================
def is_position(s):
    """! @ brief Check if given message is a position message
         @ param s Message to check
         @ returns 1 if it is a position message, 0 if not
    """

    if len(s) == 0:
        return 0

    # Check if this is a postion message
    if s[0] not in ['=','!','@']:
        return 0
    else:
        return 1

#===============================================================================
#
#===============================================================================
def position(s):
    """! Given the string, return the position information.
         @param s The string to examine
         @return Position information, None if no position

         The path must be stripped already from the given string

         >>> position("!4551.72N/01330.23E# / APRS DIGIPEATER sez.ARI MONFALCONE (GO)")
         '4551.72N/01330.23E'
         >>> position('!4145.83N/01237.15E#PHG3260/DIGIPEATER APRS IR0CH - Castelli Romani')
         '4145.83N/01237.15E'

         The latitude and longitude are expressed in degrees, minutes and decimal fractions of minutes, not degrees-minutes-seconds.
         The character after the latitude (emphasized in red above) determines if the symbol to be displayed is from the primary set or secondary set below.
         The character after the longitude (emphasized in green above) determines which symbol out of the selected set will be displayed.

         Primary Symbol Table - Selected by placing " / " between Lat and Long
         Secondary Symbol Table  - Selected by placing  " \" between Lat and Long
    """

    # Check if this is a postion message
    if not is_position(s):
        logging.warning("'%s' is not the start of a postion message" % s[0])
        return ''

    # React on the given postion message type
    if s[0] == '=':
        if len(s) > 19:
            # Postion without timestamp
            return s[1:19]
        else:
            logging.error('Invalid length for postion message' % s)
            return None

    elif s[0] == '!':
        if len(s) > 19:
            # Postion without timestamp
            return s[1:19]
        else:
            logging.error('Invalid length for postion message' % s)
            return None

    elif s[0] == '@':
        # Postion with timestamp
        if len(s) > 26:
            return s[8:26]
        else:
            logging.error('Invalid length for postion message' % s)
            return None

    logging.warning('Problem in decoding position message %s' % s)
    return None

#===============================================================================
#
#===============================================================================
def is_timestamp(s):
    """! @ brief Check if given message is contains a timestamp
         @ param s Message to check
         @ returns 1 if it contains a timestamp, 0 if not
    """

    if len(s) == 0:
        return 0

    # Check if this is a timestamp message
    if s[0] not in ['@','>']:
        return 0
    else:
        return 1

def timestamp(s):
    """! @brief Given the info string, return the timestamp
         @param info The string to examine
         @return The timestamp, None if it does not contain a timestamp
    """

    if not is_timestamp(s):
        return None

    if s[0] == '@':         # Postion with timestamp
        return s[1:8]

    elif s[0] == '>':       # Status message with timestamp
        return s[1:8]

    return None             # If we come here, return None

#===============================================================================
#
#===============================================================================
def is_status(s):
    """! @ brief Check if given message is a status
         @ param s Message to check
         @ returns 1 if it contains a status, 0 if not
    """

    if len(s) == 0:
        return 0

    # Check if this is a status message
    if s[0] not in ['>']:
        return 0
    else:
        return 1

def status(s):
    """! @brief Given the info string, return the status
         @param info The string to examine
         @return The status, None if it does not contain a status
    """

    if not is_status(s):
        return None
    elif s[0] == '>':       # Status message, preceded by timestamp
        return s[8:]
    return None             # If we come here, return None




#===============================================================================
#
#===============================================================================
def is_message(s):
    """! @ brief Check if given message is a message
         @ param s Message to check
         @ returns 1 if it contains a message, 0 if not
    """

    if len(s) == 0:
        return 0

    # Check if this is a message message
    if s[0] not in [':']:
        return 0
    else:
        return 1

def message(s):
    """! @brief Given the info string, return the message
         @param info The string to examine
         @return The message, None if it does not contain a message
    """

    if not is_message(s):
        return None

    if s[0] == ':':         # message message, preceded by timestamp
        return s[1:]

    return None             # If we come here, return None

#===============================================================================
#
#===============================================================================
def type_description(data):
    """! @brief
         @param
         @return

        Check if first character is an APRS datatype identifier.
        if it is, then show the type
    """

    if len(data) == 0:
        logging.error('Empty line')
        return ''

    id = data[0]
    description = aprs_data_type_identifier.get(data[0],'Unknown')
    return '%s = %s' % (id,description)

#===============================================================================
#
#===============================================================================
if __name__ == '__main__':

    import doctest
    doctest.testmod()