#!/usr/bin/env python

import time, sys, socket, asyncore
from math import *

_PORT = 38975

class PredictConnection(object):
    def __init__(self, host, port):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.connect((host, port))
        self.sock = sock

    def get_list(self):
        sock = self.sock
        sock.send('GET_LIST\0')
        data = sock.recv(65536)
        return data.split()

    def get_sat(self, sat):
        sock = self.sock
        sock.send('GET_SAT %s\0' % sat)
        data = sock.recv(65536)
        name, lon, lat, az, el, aos, footprint, range, alt = data.split()[:9]
        lon = float(lon)
        lat = float(lat)
        footprint = float(footprint)
        alt = float(alt)
        return name, lon, lat, footprint, alt

    def get_all(self):
        sats = self.get_list()
        r = []
        for sat in sats:
            r.append(self.get_sat(sat))

        return r


class APRSServer(asyncore.dispatcher):
    def __init__(self, conns):
        asyncore.dispatcher.__init__(self)
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.set_reuse_addr()
        self.bind(('127.0.0.1', _PORT))
        self.listen(1)
        self.conns = conns

    def handle_accept(self):
        conn, address = self.accept()
        self.conns.append(APRSConnection(conn))


class APRSConnection(asyncore.dispatcher_with_send):
    def readable(self):
        return 0


def mkmin(coord):
    s = '%.2f' % (modf(coord)[0] * 60.0)
    s = '0' * (5-len(s)) + s
    return s


def coord2aprs(lat, lon):
    if lon <= 180:
        ew = 'W'
    elif lon > 180:
        lon = 360 - lon
        ew = 'E'

    if lat < 0:
        ns = 'S'
        lat = -lat
    else:
        ns = 'N'

    return '%02d%s%s\\%03d%s%sS' % (lat, mkmin(lat), ns, lon, mkmin(lon), ew)


def base91(n, digits=4):
    r = ''
    for i in range(digits):
        r = chr(33 + (n % 91)) + r
        n /= 91

    return r


def coord2compr(lat, lon, foot):
    lat = base91(int(round(380926 * (90 - lat))))

    if lon > 180:
        lon = lon - 360
    lon = -lon

    lon = base91(int(round(190463 * (180 + lon))))

    #alt = base91(int(round(log(alt) / log(1.002))), 2)
	    #foot = base91(int(round(log(foot/2.0) / log(1.08))), 1)
	    return '\%s%sS   ' % (lat, lon)




	def pad(s):
	    s = s[:9]
	    s = s + ' ' * (9 - len(s))
	    return s


	def timestamp():
	    t = time.gmtime(time.time())
	    return '%02d%02d%02dz' % (t[3], t[4], t[5])


	def update(predict_conn, aprs_conns, sat_list = None):
	    if sat_list is None:
	        sats = predict_conn.get_all()
	    else:
	        sats = [predict_conn.get_sat(sat) for sat in sat_list]

	    for name, lon, lat, footprint, alt in sats:
	        data = 'KG6CVV>APU25N:;%s*%s%s' % (pad(name), timestamp(), coord2aprs(lat, lon))
	        footprint = int(round(footprint * 1.0609344))
	        if footprint <= 9999:
	            data = data + 'RNG%04d' % footprint

	        data = data + '\n'

	        dead = []
	        for c in aprs_conns:
	            try:
	                c.send(data)
	            except:
	                print >> sys.stderr, 'Closing %r' % c
	                dead.append(c)

	        for c in dead:
	            aprs_conns.remove(c)



	def main():
	    c = PredictConnection('localhost', 1210)
	    conns = []
	    s = APRSServer(conns)

	    while True:
	        if conns:
	            update(c, conns)
	        t = time.time() + 10.0
	        while time.time() < t:
	            asyncore.poll(t - time.time())


	if __name__ == '__main__':
	    main()