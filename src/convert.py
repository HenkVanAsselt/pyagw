f = open('hosts2.ini', 'r')
lines = f.readlines()
f.close

for line in lines:
    line = line.strip()
    if not line:
        continue
    print line
    s = line.split('   ')
    print len(s), s

    #print '%s,%s,  #%s' % (s[1], s[2], s[0])

