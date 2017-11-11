##
## @file STATIONDB.PY
## @brief PYSqlite Station database handler
## @AUTH Henk van Asselt, PA3BTL
##

# Global imports
import logging
import os.path
import sqlite3
import datetime

# ----------------------------------------------------------------------------
#
# ----------------------------------------------------------------------------

class StationDatabase:
    """ Class containing database information
    """

    #--------------------

    def __init__(self, filename='station.db'):
        """ Intialize this class
        """

        self.filename = filename
        self.con = None
        self.cur = None     # Cursor to connection

        if not os.path.isfile(filename):
            logging.info('Creating station database %s' % filename)

            self.con = sqlite3.connect(filename, detect_types=sqlite3.PARSE_DECLTYPES|sqlite3.PARSE_COLNAMES, isolation_level=None)
            self.cur = self.con.cursor()

            # Create table

            self.cur.execute('''create table stations
                (name text primary key,
                port int,
                pinged timestamp,
                connected timestamp,
                description text)
            ''')

            self.con.commit()

    #--------------------

    def add(self, host):
        """ Add station to the database
        """

        now = datetime.datetime.now()

        self.con = sqlite3.connect(self.filename, detect_types=sqlite3.PARSE_DECLTYPES|sqlite3.PARSE_COLNAMES, isolation_level=None)
        self.cur = self.con.cursor()
        try:
            dummy_date = datetime.datetime(1900, 01, 01, 00, 00, 00, 000000)
            self.cur.execute("insert into stations(name, port, pinged, connected, description) values (?, ?, ?, ?, ?)", (host.name_or_ip, host.port, dummy_date, dummy_date, ''))
            self.con.commit()
            logging.info('Station %s is added' % host.name_or_ip)
        except sqlite3.IntegrityError:
            logging.info('Station %s was already in database' % host.name_or_ip)

    #--------------------

    def update(self, host, pinged = 0L, connected = 0L):
        """ Update host entry with the given information
        """

        if pinged:
            self.con = sqlite3.connect(self.filename, detect_types=sqlite3.PARSE_DECLTYPES|sqlite3.PARSE_COLNAMES, isolation_level=None)
            self.cur = self.con.cursor()
            self.cur.execute('update stations set pinged = "%s" where name = "%s"' % (pinged, host.name_or_ip))
            self.con.commit()
            #execute("update test set name='bar'")

        if connected:
            self.con = sqlite3.connect(self.filename, detect_types=sqlite3.PARSE_DECLTYPES|sqlite3.PARSE_COLNAMES, isolation_level=None)
            self.cur = self.con.cursor()
            self.cur.execute('update stations set connected = "%s" where name = "%s"' % (connected, host.name_or_ip))
            self.con.commit()


    #--------------------

    def getall(self):
        """ Generate a report of station activity
            @returns: list of tuples of stations in the database
        """

        self.con = sqlite3.connect(self.filename, detect_types=sqlite3.PARSE_DECLTYPES|sqlite3.PARSE_COLNAMES, isolation_level=None)
        self.cur = self.con.cursor()

        self.cur.execute("select * from stations")
        # Retrieve all rows as a sequence and print that sequence:
        data = self.cur.fetchall()
        return data


if __name__ == '__main__':
    """
    """

    db = StationDatabase('stations.db')
