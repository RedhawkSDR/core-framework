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
from _unitTestHelpers import buildconfig
from ossie.cf import CF
from ossie.utils import redhawk
from omniORB import any
import time
from _unitTestHelpers import runtestHelpers

java_support = runtestHelpers.haveJavaSupport('../Makefile')


maxcpus=32
maxnodes=2
numa_match={ "all" : "0-31",
             "sock0": "0-7,16-23",
             "sock1": "8-15,24-31", 
             "sock0sans0": "1-7,16-23", 
             "5" : "5",
             "8-10" : "8-10" }
numa_layout=[ "0-7,16-23", "8-15,24-31" ]

numa_xml_src={ "all" : "0-31",
                 "sock0": "0",
                 "sock1": "1", 
                 "sock0sans0": "0", 
                 "5" : "5",
                 "8-10" : "8,9,10",
                 "eface" : "em1" }

def get_match( key="all" ):
    if key and  key in numa_match:
        return numa_match[key]
    return numa_match["all"]



def  setXmlSource( src, dest=None ):
    import re
    if dest == None: dest = src.replace(".GOLD","")
    dest_f = open(dest,'w')
    lines = [line.rstrip() for line in open(src)]
    for l in lines:
        newline=l
        for k in numa_xml_src.keys():
            newline = re.sub(r'XXX'+k+'XXX', r''+numa_xml_src[k], newline )
        dest_f.write(newline+'\n')
    


def check_affinity( pname, affinity_match="0-31", use_pidof=True):
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
        cpus_allowed = check_affinity(pname, affinity_match, use_pidof )
        #print pname, cpus_allowed
        self.assertEqual(cpus_allowed[1],affinity_match)
        return
        

    def test_NodeDeployment(self):
        if not java_support:
            return
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
        self.check_affinity( 'JavaTestDevice', get_match("8-10"), False)

        self.check_affinity( 'S1', get_match("sock1"), False)

        self.check_affinity( 'GPP', get_match("sock0") , False )

        devMgr.shutdown()

        self.assertEqual(len(domMgr._get_deviceManagers()), 0)


class TestWaveformAffinity(scatest.CorbaTestCase):

    def get_node_info(self):
        pass


    def check_affinity(self, pname, affinity_match="0-31", use_pidof=True):
        cpus_allowed = check_affinity(pname, affinity_match, use_pidof )
        #print pname, cpus_allowed
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
               self.check_affinity( 'C2', get_match("sock1") , False )
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
                self.check_affinity( 'C2', get_match("sock1"), False )
                if app: app.releaseObject()
        except CF.ApplicationFactory.CreateApplicationError as e:
            if ( "busy" in e.msg ) == False:
              raise

        
        devMgr.shutdown()

        self.assertEqual(len(domMgr._get_deviceManagers()), 0)



import sys,os
print buildconfig.DEFS.find("HAVE_LIBNUMA")
all_cpus="0"
maxcpus=1
maxnodes=1
eface="em1"

#
# Figure out ethernet interface to use
#
eface="em1"
lines = [line.rstrip() for line in os.popen('cat /proc/net/dev')]
import re
for l in lines[2:]:
    t1=l.split(':')[0].lstrip()
    if re.match('e.*', t1 ) :
        eface=t1
        break

if buildconfig.DEFS.find("HAVE_LIBNUMA") == -1:
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
    numa_match={ "all" :  all_cpus,
             "sock0":  all_cpus,
             "sock1": all_cpus,
             "sock0sans0":  all_cpus_sans0,
             "5" : all_cpus,
             "8-10" : all_cpus }

#    if maxcpus < 10:
#        del TestNodeAffinity
#        del TestWaveformAffinity.test_WaveformAffinityPolicy
#        del TestWaveformAffinity.test_NicDeploymentPolicySocketUnlocked
#        del TestWaveformAffinity.test_NicDeploymentPolicySocketLocked

else:
    # test numaclt --show .. look for cpu bind of 0,1 and cpu id atleast 31
    import os
    maxnode=0
    maxcpu=1
    try:
        lines = [line.rstrip() for line in os.popen('numactl --show')]
        for l in lines:
            if l.startswith('nodebind'):
                maxnode=int(l.split()[-1])
            if l.startswith('physcpubind'):
                maxcpu=int(l.split()[-1])

        maxcpus=maxcpu+1
        maxnodes=maxnode+1
        numa_layout=[]
        for i in range(maxnodes):
            xx = [line.rstrip() for line in open('/sys/devices/system/node/node'+str(i)+'/cpulist')]
            numa_layout.append(xx[0])

        all_cpus='0-'+str(maxcpus-1)
        all_cpus_sans0='1-'+str(maxcpus-1)
        if maxcpus == 2:
            all_cpus_sans0='1'
        elif maxcpus == 1 :
            all_cpus="0"
            all_cpus_sans0=''

        numa_match = { "all":all_cpus,
                       "sock0":  all_cpus,
                       "sock1": all_cpus,
                       "sock0sans0":  all_cpus_sans0,
                       "5" : all_cpus,
                       "8-10" : all_cpus }

        if len(numa_layout) > 0:
            numa_match["sock0"]=numa_layout[0]
            aa=numa_layout[0]
            if maxcpus > 2:
               numa_match["sock0sans0"] = str(int(aa[0])+1)+aa[1:]

        if len(numa_layout) > 1:
            numa_match["sock1"]=numa_layout[1]

        if maxcpus > 5:
            numa_match["5"]="5"

        if maxcpus > 11:
            numa_match["8-10"]="8-10"

        if maxcpus == 2:
            numa_match["5"] = all_cpus_sans0
            numa_match["8-10"]= all_cpus_sans0

    except:
        pass
        #del TestNodeAffinity
        #del TestWaveformAffinity.test_WaveformAffinityPolicy
        #del TestWaveformAffinity.test_NicDeploymentPolicySocketUnlocked



numa_xml_src={ "all" :  all_cpus,
               "sock0": "0",
               "sock1": "1",
               "sock0sans0": "0",
               "5" : "5",
               "8-10" : "8,9,10",
               "eface" : eface }
if maxnodes < 2 :
    numa_xml_src["sock1"] = "0"

if maxcpus < 9 or maxcpus < 11 :
    numa_xml_src["8-10"] = all_cpus
    if maxcpus == 2:
       numa_xml_src["8-10"] = all_cpus_sans0

if maxcpus < 9 or maxcpus < 11 :
    numa_xml_src["5"] = all_cpus
    if maxcpus == 2:
       numa_xml_src["5"] = all_cpus_sans0

#print numa_layout
#print maxcpus
#print numa_xml_src
#print numa_match

setXmlSource( "./sdr/dev/nodes/test_affinity_node_socket/DeviceManager.dcd.xml.GOLD")
setXmlSource( "./sdr/dev/nodes/test_affinity_node_unlocked/DeviceManager.dcd.xml.GOLD")
setXmlSource( "./sdr/dom/components/C1/C1.spd.xml.GOLD")
setXmlSource( "./sdr/dom/waveforms/affinity_test1/affinity_test1.sad.xml.GOLD")
setXmlSource( "./sdr/dom/waveforms/affinity_test2/affinity_test2.sad.xml.GOLD")
setXmlSource( "./sdr/dom/waveforms/affinity_test3/affinity_test3.sad.xml.GOLD")

