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

import unittest
from _unitTestHelpers import scatest
from omniORB import URI, any
from ossie.cf import CF
from ossie.properties import *
import threading
import time
from _unitTestHelpers import runtestHelpers

java_support = runtestHelpers.haveJavaSupport('../Makefile')

class EventChannelManager(scatest.CorbaTestCase):
    def setUp(self):
        self._domBooter, self._domMgr = self.launchDomainManager()

    def tearDown(self):
        try:
            self._app.stop()
            self._app.releaseObject()
        except AttributeError:
            pass

        try:
            self._devMgr.shutdown()
        except AttributeError:
            pass

        try:
            self.terminateChild(self._devBooter)
        except AttributeError:
            pass

        try:
            self.terminateChild(self._domBooter)
        except AttributeError:
            pass

        # Do all application and node booter shutdown before calling the base
        # class tearDown, or failures will occur.
        scatest.CorbaTestCase.tearDown(self)

    def test_ECM_RegId(self):
        self.localEvent = threading.Event()
        self.eventFlag = False
        reg = CF.EventChannelManager.EventRegistration( channel_name = 'prop_Channel', reg_id = 'my_reg_id')
        ret_reg = self._domMgr._get_eventChannelMgr().registerResource( reg )
        self.assertEquals(reg.reg_id, ret_reg.reg.reg_id)
        reg_2 = CF.EventChannelManager.EventRegistration( channel_name = 'prop_Channel', reg_id = '')
        ret_reg_2 = self._domMgr._get_eventChannelMgr().registerResource( reg_2 )
        self.assertNotEquals(reg_2.reg_id, ret_reg_2.reg.reg_id)


    def test_ECM_CppComponent(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/ECM1/ECM1.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        app.start()
        time.sleep(5)
        components = app._get_registeredComponents()
        for component in components:
            #print component.componentObject._get_identifier()
            if 'ECM' in component.componentObject._get_identifier():
                stuff = component.componentObject.query([])
        
        if stuff:
            pdict = props_to_dict(stuff)
            mlimit = pdict['msg_limit']
            mxmit = pdict['msg_xmit']
            mrecv = pdict['msg_recv']
            self.assertEquals(mlimit, mxmit )
            self.assertEquals(mlimit, mrecv )



        app.releaseObject()


    def test_ECM_PythonComponent(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/ECM2/ECM2.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        app.start()
        time.sleep(5)
        components = app._get_registeredComponents()
        for component in components:
            #print component.componentObject._get_identifier()
            if 'ECM' in component.componentObject._get_identifier():
                stuff = component.componentObject.query([])
        
        if stuff:
            pdict = props_to_dict(stuff)
            mlimit = pdict['msg_limit']
            mxmit = pdict['msg_xmit']
            mrecv = pdict['msg_recv']
            self.assertEquals(mlimit, mxmit )
            self.assertEquals(mlimit, mrecv )



        app.releaseObject()


    def test_ECM_JavaComponent(self):
        if not java_support:
            return
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/ECM3/ECM3.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        app.start()
        time.sleep(5)
        components = app._get_registeredComponents()
        for component in components:
            #print component.componentObject._get_identifier()
            if 'ECM' in component.componentObject._get_identifier():
                stuff = component.componentObject.query([])
        
        if stuff:
            pdict = props_to_dict(stuff)
            mlimit = pdict['msg_limit']
            mxmit = pdict['msg_xmit']
            mrecv = pdict['msg_recv']
            self.assertEquals(mlimit, mxmit )
            self.assertEquals(mlimit, mrecv )



        app.releaseObject()

class EventChannelManagerRedhawkUtils(scatest.CorbaTestCase):
    def setUp(self):
        self._domBooter, self._domMgr = self.launchDomainManager()
        from ossie.utils import redhawk
        d=None
        d = redhawk.Domain(self._domMgr._get_name())
        self.assertNotEqual(d, None )

        # Get Event Channel Manager
        self.ecm=None
        self.ecm = d.getEventChannelMgr()

        self.assertNotEqual(self.ecm, None )
        self.assertNotEqual(self.ecm.ref, None )

    def tearDown(self):
        try:
            self._app.stop()
            self._app.releaseObject()
        except AttributeError:
            pass

        try:
            self._devMgr.shutdown()
        except AttributeError:
            pass

        try:
            self.terminateChild(self._devBooter)
        except AttributeError:
            pass

        try:
            self.terminateChild(self._domBooter)
        except AttributeError:
            pass

        # Do all application and node booter shutdown before calling the base
        # class tearDown, or failures will occur.
        scatest.CorbaTestCase.tearDown(self)



    def test_ECM_redhawkUtilsAccess(self):

        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)

        # get list of channels (should be ODM/IDM
        clist,citer = self.ecm.listChannels(2)
        self.assertEqual(citer, None )
        self.assertEqual(len(clist), 2 )

        # access list via iterator
        clist,citer = self.ecm.listChannels()
        self.assertEqual(clist, [] )
        
        ch=citer.next_one()
        self.assertEqual(ch.channel_name, 'IDM_Channel')
        ch=citer.next_one()
        self.assertEqual(ch.channel_name, 'ODM_Channel')
        ch=citer.next_one()
        self.assertEqual(ch, None)

        # test registration listeners
        clist,citer = self.ecm.listRegistrants('IDM_Channel')
        self.assertEqual(clist, [] )
        ch=citer.next_one()
        self.assertNotEqual(ch.channel_name,None)
        ch=citer.next_one()
        self.assertNotEqual(ch.channel_name,None)
        ch=citer.next_one()
        self.assertEqual(ch, None)

        clist,citer = self.ecm.listRegistrants('IDM_Channel', 2)
        self.assertEqual(citer, None )
        self.assertEqual(len(clist), 2 )

        # test event channel creation
        ch = self.ecm.create('ecm_test')
        self.assertNotEqual(ch, None)

        self.assertRaises( CF.EventChannelManager.ChannelAlreadyExists, self.ecm.create, 'ecm_test') 

        self.ecm.markForRegistrations( 'ecm_test' ) 
        self.assertRaises( CF.EventChannelManager.ChannelDoesNotExist, self.ecm.markForRegistrations, 'ecm_doesnotexists') 

        ch = self.ecm.createForRegistrations('ecm_test2')
        self.assertNotEqual(ch, None)

        self.assertRaises( CF.EventChannelManager.ChannelAlreadyExists, self.ecm.createForRegistrations, 'ecm_test2') 

        reg = self.ecm.registerResource('ecm_test2')
        self.assertNotEqual(reg, None)

        self.assertRaises( CF.EventChannelManager.InvalidChannelName, self.ecm.registerResource, 'ecm_doesnotexists:bad_name') 

        # test unregister method

        # test exceptions during unregister
        self.assertRaises( CF.EventChannelManager.RegistrationDoesNotExist, self.ecm.unregister, 'ecm_test2', '' )

        self.ecm.unregister('ecm_test2', reg.reg.reg_id)

        reg = self.ecm.registerResource('ecm_test2')
        self.assertNotEqual(reg, None)

        self.assertRaises( CF.EventChannelManager.ChannelDoesNotExist, self.ecm.unregister, 'ecm_test3', reg.reg.reg_id )

        # test exceptions during release
        self.assertRaises( CF.EventChannelManager.RegistrationsExists, self.ecm.release, 'ecm_test2' )

        self.ecm.unregister('ecm_test2', reg.reg.reg_id)

        self.assertRaises( CF.EventChannelManager.ChannelDoesNotExist, self.ecm.release, 'ecm_test2' )

        self.ecm.release( 'ecm_test' )

        self.assertRaises( CF.EventChannelManager.ChannelDoesNotExist, self.ecm.release, 'ecm_test')


