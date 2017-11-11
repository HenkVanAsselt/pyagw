##
## @file REPORT.PY
## @brief PYSqlite Station database reporting tool
## @AUTH Henk van Asselt, PA3BTL
##

# Global imports
import logging
import os.path
import datetime

# Local imports
import stationdb

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
                    #filename = 'igate.log',
                    filemode = 'w')

    # define a Handler which writes ERROR messages or higher to the sys.stderr
    console = logging.StreamHandler()
    console.setLevel(logging.DEBUG)
    logging.getLogger('').addHandler(console)   # add the handler to the root logger

    db = stationdb.StationDatabase('stations.db')
    stations = db.getall()
    if len(stations) == 0:
        print 'no stations reported'
    else:
        print len(stations)
        for station in stations:
            dummy_date = datetime.datetime(2000, 01, 01, 00, 00, 00, 000000)
            if station[2] > dummy_date:
                last_pinged = station[2]
            else:
                last_pinged = 'never'
            if station[3] > dummy_date:
                last_connected = station[3]
            else:
                last_connected = 'never'
            print ('%s:%s  pinged:%s connected:%s' % (station[0], station[1], last_pinged, last_connected))