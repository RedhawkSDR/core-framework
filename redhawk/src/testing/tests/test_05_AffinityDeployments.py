#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK core.
#
# REDHAWK core is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import unittest, os
from _unitTestHelpers import scatest
from ossie.cf import CF
from ossie.utils import redhawk
from omniORB import any
import time
import copy

maxcpus=32
maxnodes=2
all_cpus='0-'+str(maxcpus-1)
all_cpus_sans0='1-'+str(maxcpus-1)

# expected results from affinity deployments
numa_match={ "all" : "0-31",
             "sock0": "0-7,16-23",
             "sock1": "8-15,24-31", 
             "sock0sans0": "1-7,16-23", 
             "sock1sans0": "8-15,24-31", 
             "5" : "5",
             "8-10" : "8-10" }

# default layout for devel 
numa_layout=[ "0-7,16-23", "8-15,24-31" ]

# matching affinity results for devices
dev_affinity_ctx={}

# affinity settings for waveform and node definitions
affinity_test_src={ "all" : "0-31",
                 "sock0": "0",
                 "sock1": "1",
                 "sock0sans0": "0", 
                 "5" : "5",
                 "8-10" : "8,9,10",
                 "eface" : "em1" }


def get_match( key="all", match_ctx=None ):
    if not match_ctx : match_ctx=numa_match
    if key and  key in match_ctx:
        return match_ctx[key]
    return match_ctx["all"]

def  setXmlSource( src, dest=None ):
    import re
    if dest == None: dest = src.replace(".GOLD","")
    dest_f = open(dest,'w')
    lines = [line.rstrip() for line in open(src)]
    for l in lines:
        newline=l
        for k in affinity_test_src.keys():
            newline = re.sub(r'XXX'+k+'XXX', r''+affinity_test_src[k], newline )
        dest_f.write(newline+'\n')

def get_process_affinity( pname, use_pidof=True):
    try:
        if use_pidof == True:
            o1=os.popen('pidof -x '+pname )
        else:
            o1=os.popen("pgrep -f testing/.*/"+pname )
        pid=o1.read().split('\n')[0]
        o2=os.popen('cat /proc/'+pid+'/status | grep Cpus_allowed_list')
        cpus_allowed=o2.read().split()
    except:
        cpus_allowed=[]

    return cpus_allowed   


class TestNodeAffinity(scatest.CorbaTestCase):

    def get_node_info(self):
        pass


    def check_affinity(self, pname, affinity_match="0-31", use_pidof=True):
        cpus_allowed = get_process_affinity(pname,use_pidof )
        #print pname, cpus_allowed
        self.assertEqual(cpus_allowed[1],affinity_match)
        return
        
    @scatest.requireJava
    def test_NodeDeployment(self):
        nodebooter, domMgr = self.launchDomainManager()
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_affinity_node_unlocked/DeviceManager.dcd.xml")

        # Ensure the expected device is available
        self.assertNotEqual(devMgr, None)
        # wait for all devices to register
        count=0
        for count in range(10):
            #print "count " , count, " dev", len( devMgr._get_registeredDevices() )
            if len( devMgr._get_registeredDevices() ) < 2 :
                time.sleep(1)
            else:
                break

        # check java device to be 8,9,10
        self.check_affinity( 'JavaTestDevice', get_match("8-10", dev_affinity_ctx ), False)

        self.check_affinity( 'S1', get_match("sock1", dev_affinity_ctx), False)

        self.check_affinity( 'GPP', get_match("sock0", dev_affinity_ctx ) , False )

        devMgr.shutdown()

        self.assertEqual(len(domMgr._get_deviceManagers()), 0)


class TestWaveformAffinity(scatest.CorbaTestCase):

    def get_node_info(self):
        pass


    def check_affinity(self, pname, affinity_match="0-31", use_pidof=True):
        cpus_allowed = get_process_affinity(pname, use_pidof )
        #print pname, cpus_allowed, affinity_match
        self.assertEqual(cpus_allowed[1],affinity_match)
        return

    def test_WaveformAffinityPolicy(self):
        nodebooter, domMgr = self.launchDomainManager()
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_affinity_node_unlocked/DeviceManager.dcd.xml")

        # Ensure the expected device is available
        self.assertNotEqual(devMgr, None)
        # wait for all devices to register
        count=0
        for count in range(10):
            if len( devMgr._get_registeredDevices() ) < 2 :
                time.sleep(1)
            else:
                break

        rhdom= redhawk.attach(scatest.getTestDomainName())
        try:
            app=rhdom.createApplication('affinity_test1')

            if app: 
                self.check_affinity( 'C2', get_match("5") , False )
                app.releaseObject()
                if ( maxcpus < 2):  time.sleep(2)
        except CF.ApplicationFactory.CreateApplicationError as e:
            if ( "busy" in e.msg ) == False:
              raise
        try:
           app=rhdom.createApplication('affinity_test2')

           if app != None:
               self.check_affinity( 'C2', get_match("sock1sans0") , False )
               if app: app.releaseObject()
        except CF.ApplicationFactory.CreateApplicationError as e:
            if ( "busy" in e.msg ) == False:
              raise
        
        devMgr.shutdown()

        self.assertEqual(len(domMgr._get_deviceManagers()), 0)


    def test_DeployOnSocketPolicy(self):
        nodebooter, domMgr = self.launchDomainManager()
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_affinity_node_socket/DeviceManager.dcd.xml")

        # Ensure the expected device is available
        self.assertNotEqual(devMgr, None)
        # wait for all devices to register
        count=0
        for count in range(10):
            if len( devMgr._get_registeredDevices() ) < 2 :
                time.sleep(1)
            else:
                break

        rhdom= redhawk.attach(scatest.getTestDomainName())
        
        try:
            app=rhdom.createApplication('affinity_test1')

            if app != None:
                self.check_affinity( 'C2',  get_match("sock0sans0") , False )
                app.releaseObject()
                if ( maxcpus < 2) : time.sleep(2)
        except CF.ApplicationFactory.CreateApplicationError as e:
            if ( "busy" in e.msg ) == False:
              raise

        try:
            app=rhdom.createApplication('affinity_test2')

            if app != None:
                self.check_affinity( 'C2',  get_match("sock0sans0") , False )
                app.releaseObject()
        except CF.ApplicationFactory.CreateApplicationError as e:
            if ( "busy" in e.msg ) == False:
                raise

        devMgr.shutdown()

        self.assertEqual(len(domMgr._get_deviceManagers()), 0)


    def test_NicDeploymentPolicySocketLocked(self):
        nodebooter, domMgr = self.launchDomainManager()
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_affinity_node_socket/DeviceManager.dcd.xml")

        # Ensure the expected device is available
        self.assertNotEqual(devMgr, None)
        # wait for all devices to register
        count=0
        for count in range(10):
            if len( devMgr._get_registeredDevices() ) < 2 :
                time.sleep(1)
            else:
                break

        rhdom= redhawk.attach(scatest.getTestDomainName())

        try:
            app=rhdom.createApplication('affinity_test3')
            if app != None:
                self.check_affinity( 'C1', get_match("sock0sans0"), False )
                self.check_affinity( 'C2', get_match("sock0sans0"), False )
                app.releaseObject()
                if ( maxcpus < 2) : time.sleep(2)
        except CF.ApplicationFactory.CreateApplicationError as e:
            if ( "busy" in e.msg ) == False:
              raise

        # run second application to make sure we are still working
        try:
            app=rhdom.createApplication('affinity_test1')

            if app != None:
                self.check_affinity( 'C2',  get_match("sock0sans0") , False )
                app.releaseObject()
        except CF.ApplicationFactory.CreateApplicationError as e:
            if ( "busy" in e.msg ) == False:
              raise

        devMgr.shutdown()

        self.assertEqual(len(domMgr._get_deviceManagers()), 0)



    def test_NicDeploymentPolicySocketUnlocked(self):
        nodebooter, domMgr = self.launchDomainManager()
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_affinity_node_unlocked/DeviceManager.dcd.xml")

        # Ensure the expected device is available
        self.assertNotEqual(devMgr, None)
        # wait for all devices to register
        count=0
        for count in range(10):
            if len( devMgr._get_registeredDevices() ) < 2 :
                time.sleep(1)
            else:
                break

        rhdom= redhawk.attach(scatest.getTestDomainName())
        
        try:
            app=rhdom.createApplication('affinity_test3')

            if app != None:
                self.check_affinity( 'C1',  get_match("sock0sans0"), False )
                self.check_affinity( 'C2',  get_match("sock0sans0"), False )
                app.releaseObject()
                if ( maxcpus < 2):  time.sleep(2)
        except CF.ApplicationFactory.CreateApplicationError as e:
            if ( "busy" in e.msg ) == False:
              raise

        # run second application to make sure we are still working
        #rhdom= redhawk.attach(scatest.getTestDomainName())
        try:
            app=rhdom.createApplication('affinity_test1')

            if app: 
                self.check_affinity( 'C2', get_match("5") , False )
                app.releaseObject()
                if ( maxcpus < 2):  time.sleep(2)
        except CF.ApplicationFactory.CreateApplicationError as e:
            if ( "busy" in e.msg ) == False:
              raise

        try:
            app=rhdom.createApplication('affinity_test2')

            if app != None:
                self.check_affinity( 'C2', get_match("sock1sans0"), False )
                if app: app.releaseObject()
        except CF.ApplicationFactory.CreateApplicationError as e:
            if ( "busy" in e.msg ) == False:
              raise

        
        devMgr.shutdown()

        self.assertEqual(len(domMgr._get_deviceManagers()), 0)



import sys,os

def get_numa_spec():
    maxnode=0
    maxcpu=1
    numasupport=False
    try:
        lines = [line.rstrip() for line in os.popen('numactl --show')]
        if len(lines) > 0 : numasupport=True
        for l in lines:
            if l.startswith('nodebind'):
                maxnode=int(l.split()[-1])
            if l.startswith('physcpubind'):
                maxcpu=int(l.split()[-1])
            if "NO NUMA" in l.upper():
                numasupport=False
            
    except:
        numasupport=False
        
    return numasupport,maxnode,maxcpu

def get_nonnuma_affinity_ctx( affinity_ctx ):
    # test should run but affinity will be ignored
    import multiprocessing
    maxcpus=multiprocessing.cpu_count()
    maxnodes=1
    all_cpus='0-'+str(maxcpus-1)
    all_cpus_sans0='0-'+str(maxcpus-1)
    if maxcpus == 2:
        all_cpus_sans0='0-1'
    elif maxcpus == 1 :
        all_cpus='0'
        all_cpus_sans0=''

    numa_layout=[ all_cpus ]
    affinity_match={ "all" :  all_cpus,
             "sock0":  all_cpus,
             "sock1": all_cpus,
             "sock0sans0":  all_cpus_sans0,
             "sock1sans0":  all_cpus_sans0,
             "5" : all_cpus,
             "8-10" : all_cpus }

    affinity_ctx['maxcpus']=maxcpus
    affinity_ctx['maxnodes']=maxnodes
    affinity_ctx['all_cpus']=all_cpus
    affinity_ctx['all_cpus_sans0']=all_cpus_sans0
    affinity_ctx['numa_layout']=numa_layout
    affinity_ctx['affinity_match']=affinity_match
    affinity_ctx['affinity_dev_match']=copy.copy(affinity_match)


def get_numa_affinity_ctx( affinity_ctx ):
    maxnode=0
    maxcpu=1
    # test numactl --show .. look for cpu bind of 0,1 and cpu id atleast 31
    numasupport, maxnode, maxcpu =  get_numa_spec()
    maxcpus=maxcpu+1
    maxnodes=maxnode+1
    numa_layout=[]
    try:
        for i in range(maxnodes):
            xx = [line.rstrip() for line in open('/sys/devices/system/node/node'+str(i)+'/cpulist')]
            numa_layout.append(xx[0])
    except:
        pass

    all_cpus='0-'+str(maxcpus-1)
    all_cpus_sans0='1-'+str(maxcpus-1)
    if maxcpus == 2:
        all_cpus_sans0='1'
    elif maxcpus == 1 :
        all_cpus="0"
        all_cpus_sans0=''

    affinity_match={}
    affinity_dev_match={}

    def fill_match( match_5, match_8 ):
        am = { "all":all_cpus,
                       "sock0":  all_cpus,
                       "sock1": all_cpus,
                       "sock0sans0":  all_cpus_sans0,
                       "sock1sans0":  all_cpus_sans0,
                       "5" : match_5,
                       "8-10" : match_8 }

        if len(numa_layout) > 0:
           am["sock0"]=numa_layout[0]
           aa=numa_layout[0]
           if maxcpus > 2:
               am["sock0sans0"] = str(int(aa[0])+1)+aa[1:]

        if len(numa_layout) > 1:
            am["sock1"]=numa_layout[1]
            am["sock1sans0"]=numa_layout[1]

        if maxcpus > 5:
            am["5"]="5"

        if maxcpus > 11:
            am["8-10"]="8-10"
        return am

    affinity_match=fill_match(all_cpus_sans0, all_cpus_sans0 )
    affinity_dev_match=fill_match( all_cpus, all_cpus)

    affinity_ctx['maxcpus']=maxcpus
    affinity_ctx['maxnodes']=maxnodes
    affinity_ctx['all_cpus']=all_cpus
    affinity_ctx['all_cpus_sans0']=all_cpus_sans0
    affinity_ctx['numa_layout']=numa_layout
    affinity_ctx['affinity_match']=affinity_match
    affinity_ctx['affinity_dev_match']=affinity_dev_match

def hasNumaSupport( exec_path):
    # figure out if exec has numa library dependency
    have_numa=False
    lines = [ line.rstrip() for line in os.popen( 'ldd ' + exec_path ) ]
    for l in lines:
        if "libnuma" in l:
            have_numa=True

    return have_numa


## Set default to minimal system
all_cpus="0"
maxcpus=1
maxnodes=1
eface="eth0"

#
# Figure out ethernet interface to use
#
lines = [line.rstrip() for line in os.popen('cat /proc/net/dev')]
import re
for l in lines[2:]:
    t1=l.split(':')[0].lstrip()
    if re.match('e.*', t1 ) :
        eface=t1
        break
affinity_test_src['eface']=eface

nonnuma_affinity_ctx={}
get_nonnuma_affinity_ctx(nonnuma_affinity_ctx)
numa_affinity_ctx={}
get_numa_affinity_ctx(numa_affinity_ctx)
numasupport, mn, mc = get_numa_spec()

#
# Figure out GPP numa context
# figure out if exec has numa library dependency
#
n1=hasNumaSupport( scatest.getSdrPath()+'/dev/devices/GPP/cpp/GPP')
n2=hasNumaSupport( scatest.getSdrPath()+'/dev/devices/GPP/cpp/.libs/GPP')
if numasupport and (n1 or n2):
   maxcpus = numa_affinity_ctx['maxcpus']
   maxnodes = numa_affinity_ctx['maxnodes']
   all_cpus = numa_affinity_ctx['all_cpus']
   all_cpus_sans0 = numa_affinity_ctx['all_cpus_sans0']
   numa_layout=numa_affinity_ctx['numa_layout']
   numa_match=numa_affinity_ctx['affinity_match']
else:
   print "NonNumaSupport ", nonnuma_affinity_ctx
   maxcpus = nonnuma_affinity_ctx['maxcpus']
   maxnodes = nonnuma_affinity_ctx['maxnodes']
   all_cpus = nonnuma_affinity_ctx['all_cpus']
   all_cpus_sans0 = nonnuma_affinity_ctx['all_cpus_sans0']
   numa_layout=nonnuma_affinity_ctx['numa_layout']
   numa_match=nonnuma_affinity_ctx['affinity_match']

if numasupport and hasNumaSupport( scatest.getSdrPath()+'/dev/mgr/DeviceManager') :
    dev_affinity_ctx=numa_affinity_ctx['affinity_dev_match']
else:
    dev_affinity_ctx=nonnuma_affinity_ctx['affinity_dev_match']

if maxnodes < 2 :
    affinity_test_src["sock1"] = "0"

if maxcpus == 2:
    affinity_test_src["8-10"] = all_cpus_sans0
    affinity_test_src["5"] = all_cpus_sans0
else:
    if maxcpus < 9 or maxcpus < 11 :
        affinity_test_src["8-10"] = all_cpus
        affinity_test_src["5"] = all_cpus

print "numa_layout:", numa_layout
print "maxcpus:", maxcpus
print "maxnodes:", maxnodes
print "affinity_test_src:", affinity_test_src
print "numa_match (wf) :", numa_match
print "numa_match (dev) :", dev_affinity_ctx

## GPP blacklist=0, affinity=socket,0  deploy_per_socket=true
setXmlSource( "./sdr/dev/nodes/test_affinity_node_socket/DeviceManager.dcd.xml.GOLD")

## GPP blacklist=0, affinity=socket,0  deploy_per_socket=false
setXmlSource( "./sdr/dev/nodes/test_affinity_node_unlocked/DeviceManager.dcd.xml.GOLD")
setXmlSource( "./sdr/dom/components/C1/C1.spd.xml.GOLD")
setXmlSource( "./sdr/dom/waveforms/affinity_test1/affinity_test1.sad.xml.GOLD")
setXmlSource( "./sdr/dom/waveforms/affinity_test2/affinity_test2.sad.xml.GOLD")
setXmlSource( "./sdr/dom/waveforms/affinity_test3/affinity_test3.sad.xml.GOLD")

