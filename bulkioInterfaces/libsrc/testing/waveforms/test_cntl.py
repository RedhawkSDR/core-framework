#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import time
import sys
from ossie.utils import uuid
import random
from omniORB import any
from ossie.utils import redhawk
import getopt

dom=None
apps=[]
frs=[]

def attach(dname='REDHAWK_DEV', debug=False):
    global dom
    try :
        olist, a= getopt.gnu_getopt(sys.argv,"d:")
        d=filter(lambda x:  x[0] == '-d', olist )[0]
        dname=d[1]
        sys.argv=a
    except:
        pass

    print "Attach:"+str(dname)
    dom=redhawk.attach(dname )
    if debug:
        redhawk.setDebug(true)


def start_waveforms( wname, nchan=1, startup=True, looping=True, debug_fr=False):
    for i in range(nchan):
        print "Starting up WAVEFORM INSTANCE: " + str(i+1)
        app = dom.createApplication("/waveforms/"+wname+"/"+wname+".sad.xml")
        for c in app.comps:
            if c.name == 'FileReader':
                c.advanced_properties.looping = looping
                c.debug_output = debug_fr
                frs.append(c)

        if startup == True:
            app.start()
        apps.append(app)

def start(delay=3.0):
    i=0
    for fr in frs:
        fr.playback_state = 'PLAY'
        x.random.random()*delay
        print "Starting FileReader:" + str(i+1) + " wait:"+str(x)
        time.sleep(x)
        i=i+1

def stop():
    i=0
    for fr in frs:
        fr.playback_state = 'STOP'
        print "Stopping FileReader:" + str(i+1)
        i=i+1    

def get_port(app, comp, pname ) :
    p=None
    for c in app.comps:
        if c.name == comp:
            p=c.getPort(pname)
            if p:
                return p

    return p


def connect( a1, c1, p1, a2, c2, p2 ):
    src = get_port( a1, c1, p1 )
    if src : 
        dest = get_port( a2, c2, p2 )
        if dest :
            print "Connect:" + str(a1.name) + ":" + str(c1) + ":" + str(p1) + " TO " + str(a2.name) + ":" + str(c2) + ":" + str(p2)
            src.connectPort( dest, str(uuid.uuid4()))


def get_stats( app, comp, pname ) :
    p = get_port(app, comp, pname )
    if p:
        s=p._get_statistics()
        flushTime=None
        for kw in s.keywords:
            if kw.id == 'timeSinceFlush':
                flushTime=any.from_any( kw.value )

        if flushTime:
            print str(comp) + '/' + str(pname) + " Elems/BitsPs " + str(s.elementsPerSecond) + '/' + str(s.bitsPerSecond) + " LastFlush:" + str(flushTime)
        else:
            print str(comp) + '/' + str(pname) + " Elems/BitsPs " + str(s.elementsPerSecond) + '/' + str(s.bitsPerSecond) 
                

def get_apps_stats( comp, pname ):
    for a in apps:
        print " APP:" + str(a.name)
        get_stats( a, comp, pname )

def track_stats( comp, pname, cnt=-1, interval=1 ):
    if cnt > 0:
        for i in range(cnt):
            get_stats( apps[0], comp, pname )
            time.sleep( interval )
    
    else:
        while True:
            get_stats( apps[0], comp, pname )
            time.sleep( interval )


def track_all( comp, pname, cnt=-1, interval=1 ):
    if cnt > 0:
        for i in range(cnt):
            for a in apps:
                print "App:" + str(a.name)
                get_stats( a, comp, pname )
                time.sleep( interval )
    
    else:
        while True:
            for a in apps:
                print "App:" + str(a.name)
                get_stats( a, comp, pname )
                time.sleep( interval )

