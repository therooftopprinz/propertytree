#!/usr/bin/python
import sys
import re
import getopt
def getTimeOut(lines):
    print "looking for TIMEOUT"
    timeouts = []
    for line in lines:
        m = re.search("^([0-9]+)us.*TIMEOUT.*TID: ([0-9]+)", line);
        if m is not None and len(m.groups()) >= 1:
            # print m.group(0), " @", m.group(1), ", ", m.group(2)
            timeouts.append((int(m.group(1)), int(m.group(2))))
    return timeouts

def getAddTcv(lines):
    print "looking for add TCV"
    addTcvs = []
    for line in lines:
        m = re.search("^([0-9]+)us.*addTransactionCV: adding: ([0-9]+)", line);
        if m is not None and len(m.groups()) >= 1:
            # print m.group(0), " @", m.group(1), ", ", m.group(2)
            addTcvs.append((int(m.group(1)), int(m.group(2))))
    return addTcvs

def getUnlockTcv(lines):
    print "looking for unlock TCV"
    unlocks = []
    for line in lines:
        m = re.search("^([0-9]+)us.*notifyTransactionCV for tid=([0-9]+)", line);
        if m is not None and len(m.groups()) >= 1:
            # print m.group(0), " @", m.group(1), ", ", m.group(2)
            unlocks.append((int(m.group(1)), int(m.group(2))))
    return unlocks

#=======================================================

print str(sys.argv[1])
with open(str(sys.argv[1])) as f:
    print "Reading lines..."
    content = f.readlines()
    timeouts = getTimeOut(content)
    adds = getAddTcv(content)
    unlocks = getUnlockTcv(content)

# [transactionId] = (add,timeout,unlock)
cvATU = dict()
for i in timeouts:
    if (cvATU.has_key(i[1])):
        print "ERROR: Timeout transaction Id has occurred again:", i
    else:
        cvATU[i[1]] = [i[0],None,None]
        # print "added timeout ", i
for i in adds:
    if (cvATU.has_key(i[1])):
        if (cvATU[i[1]][1] is not None):
            print "ERROR: Add transaction Id has occurred again:", i
            continue
        # print "added adds ",i
        cvATU[i[1]][1] = i[0]
for i in unlocks:
    if (cvATU.has_key(i[1])):
        if (cvATU[i[1]][2] is not None):
            print "ERROR: Unlock transaction Id has occurred again:", i
            continue
        # print "added timeouts ",i
        cvATU[i[1]][1] = i[0]

print "Anomaly:"
for i in cvATU:
    if cvATU[i][2] is not None:
        print "Transaction id=", i, " Timeout=", cvATU[i][0], " Added=", cvATU[i][1], " Unlock=", cvATU[i][2]