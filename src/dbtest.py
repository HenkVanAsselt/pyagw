import sqlite3 as sqlite
import datetime

cols = 'name text primary key, port int, pinged timestamp, connected float, description text'

con = sqlite.connect(":memory:")
cur = con.cursor()
cur.execute('''create table stations (%s) ''' % cols)
cur.execute("insert into stations(name) values ('henka')")
cur.execute("insert into stations(description) values ('desc')")

now = datetime.datetime.now()
print now
cur.execute("update stations set connected=%s" % now)

cur.execute("select * from stations")
res = cur.fetchall()
print res