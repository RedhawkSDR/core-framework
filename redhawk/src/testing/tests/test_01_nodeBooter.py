#!/usr/bin/env python
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

import unittest, os, signal, time, sys
from subprocess import Popen
from _unitTestHelpers import scatest
from xml.dom import minidom
from omniORB import URI, any
from ossie.cf import CF, CF__POA
import CosNaming

class NodeBooterTest(scatest.CorbaTestCase):
    """The nodebooter test case is the only place where we directly use the nodebooter
    arguments.  Other testcases should use spawnNodeBooter instead."""

    def waitDomainManager(self, domUri):
        def _resolve():
            try:
                return self._root.resolve(domUri)._narrow(CF.DomainManager)
            except:
                return None
        self.waitPredicate(lambda: _resolve() != None)
        return _resolve()

    def test_nodeBooterDomainNameOverride(self):
        """Test that we can allow the user to override the domainname with the --domainname argument."""
        domainName = scatest.getTestDomainName()
        domainMgrURI = URI.stringToName("%s/%s" % (domainName, domainName))
        # Test that we don't already have a bound domain
        try:
            domMgr = self._root.resolve(domainMgrURI)
            self.assertEqual(domMgr, None)
        except CosNaming.NamingContext.NotFound:
            pass # This exception is expected
        args = ["../../control/framework/nodeBooter","-D", "--domainname", domainName, "-debug", "9","--nopersist" ]
        nb = Popen(args, cwd=scatest.getSdrPath() )
        domMgr = self.waitDomainManager(domainMgrURI)
        self.assertNotEqual(domMgr, None)

        # Kill the nodebooter
        os.kill(nb.pid, signal.SIGINT)
        self.assertPredicateWithWait(lambda: nb.poll() == 0)

        # Test that we cleaned up the name; this should be automatic because
        # the naming context should be empty.
        try:
            domMgr = self._root.resolve(domainMgrURI)
            self.assertEqual(domMgr, None)
        except CosNaming.NamingContext.NotFound:
            pass # This exception is expected

    def test_nodeBooterDomainNameFromDMD(self):
        """Test that we read the correct domainname from the DMD file, the test domain
        should have been created by the test runner"""
        domainName = scatest.getTestDomainName()
        # Test that we don't already have a bound domain
        try:
            domMgr = self._root.resolve(scatest.getDomainMgrURI())
            self.assertEqual(domMgr, None)
        except CosNaming.NamingContext.NotFound:
            pass # This exception is expected

        args = ["../../control/framework/nodeBooter","-D","-debug", "9","--nopersist" ]
        nb = Popen( args, cwd=scatest.getSdrPath())
        domMgr = self.waitDomainManager(scatest.getDomainMgrURI())
        self.assertNotEqual(domMgr, None)

        # Kill the nodebooter
        os.kill(nb.pid, signal.SIGINT)
        self.assertPredicateWithWait(lambda: nb.poll() == 0)

        # Test that we cleaned up the name; this should be automatic because
        # the naming context should be empty.
        try:
            domMgr = self._root.resolve(scatest.getDomainMgrURI())
            self.assertEqual(domMgr, None)
        except CosNaming.NamingContext.NotFound:
            pass # This exception is expected

    def test_nodeBooterShutdown(self):
        """Test that nodeBooter correctly cleans up.
        In OSSIE 0.7.4, and possibly before, killing a nodebooter that was running
        a device manager would prevent you from restarting the devicemanager without
        first restarting the domainmanager.  Test that this condition is fixed"""
        #  It is important that these core pieces somewhat work for all the other tests to succeed
        args = ["../../control/framework/nodeBooter","-D","-debug", "9","--nopersist" ]
        nb1= Popen( args, cwd=scatest.getSdrPath())
        domainName = scatest.getTestDomainName()

        domMgr = self.waitDomainManager(scatest.getDomainMgrURI())
        self.assertNotEqual(domMgr, None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 0)

        args = ["../../control/framework/nodeBooter","-d","/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml",
                "--domainname", domainName ]
        nb2 = Popen( args, cwd=scatest.getSdrPath())
        self.assertPredicateWithWait(lambda: len(domMgr._get_deviceManagers()) == 1)

        # Wait until the DeviceManager has finished launching its devices
        devMgr = domMgr._get_deviceManagers()[0]
        self.assertPredicateWithWait(lambda: len(devMgr._get_registeredDevices()) == 1)

        os.kill(nb2.pid, signal.SIGINT)
        self.assertPredicateWithWait(lambda: nb2.poll() != None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 0)

        # Restart the device manager to prove that the shutdown was graceful.
        # In OSSIE 0.7.4 this would fail.
        args = ["../../control/framework/nodeBooter","-d","/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml",
                "--domainname", domainName ]
        nb3 = Popen(args,  cwd=scatest.getSdrPath())
        self.assertPredicateWithWait(lambda: len(domMgr._get_deviceManagers()) == 1)

        # Wait until the DeviceManager has finished launching its devices
        devMgr = domMgr._get_deviceManagers()[0]
        self.assertPredicateWithWait(lambda: len(devMgr._get_registeredDevices()) == 1)

        os.kill(nb3.pid, signal.SIGINT)
        self.assertPredicateWithWait(lambda: nb3.poll() != None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 0)

        os.kill(nb1.pid, signal.SIGINT)
        self.assertPredicateWithWait(lambda: nb1.poll() != None)

        # Test that we cleaned up the name; this should be automatic because
        # the naming context should be empty.
        try:
            domMgr = self._root.resolve(scatest.getDomainMgrURI())
            self.assertEqual(domMgr, None)
        except CosNaming.NamingContext.NotFound:
            pass # This exception is expected

    def test_nodeBooterShutdownSIGQUIT(self):
        """Test that nodeBooter correctly cleans up on SIGQUIT.
        In OSSIE 0.7.4, and possibly before, killing a nodebooter that was running
        a device manager would prevent you from restarting the devicemanager without
        first restarting the domainmanager.  Test that this condition is fixed"""
        #  It is important that these core pieces somewhat work for all the other tests to succeed
        args = ["../../control/framework/nodeBooter","-D","--nopersist" ]
        nb1= Popen( args, cwd=scatest.getSdrPath())
        domainName = scatest.getTestDomainName()
        domMgr = self.waitDomainManager(scatest.getDomainMgrURI())
        self.assertNotEqual(domMgr, None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 0)

        args = ["../../control/framework/nodeBooter","-d","/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml",
                "--domainname", domainName ]
        nb2 = Popen(args,  cwd=scatest.getSdrPath() )
        self.assertPredicateWithWait(lambda: len(domMgr._get_deviceManagers()) == 1)

        # Wait until the DeviceManager has finished launching its devices
        devMgr = domMgr._get_deviceManagers()[0]
        self.assertPredicateWithWait(lambda: len(devMgr._get_registeredDevices()) == 1)

        os.kill(nb2.pid, signal.SIGQUIT)
        self.assertPredicateWithWait(lambda: nb2.poll() != None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 0)

        # Restart the device manager to prove that the shutdown was graceful.
        # In OSSIE 0.7.4 this would fail.
        args = ["../../control/framework/nodeBooter","-d","/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml",
                "--domainname", domainName ]
        nb3 = Popen( args,  cwd=scatest.getSdrPath() )
        self.assertPredicateWithWait(lambda: len(domMgr._get_deviceManagers()) == 1)

        # Wait until the DeviceManager has finished launching its devices
        devMgr = domMgr._get_deviceManagers()[0]
        self.assertPredicateWithWait(lambda: len(devMgr._get_registeredDevices()) == 1)

        os.kill(nb3.pid, signal.SIGQUIT)
        self.assertPredicateWithWait(lambda: nb3.poll() != None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 0)

        os.kill(nb1.pid, signal.SIGQUIT)
        self.assertPredicateWithWait(lambda: nb1.poll() != None)

        # Test that we cleaned up the name; this should be automatic because
        # the naming context should be empty.
        try:
            domMgr = self._root.resolve(scatest.getDomainMgrURI())
            self.assertEqual(domMgr, None)
        except CosNaming.NamingContext.NotFound:
            pass # This exception is expected

