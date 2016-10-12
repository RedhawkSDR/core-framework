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

import unittest, os, signal, time
from _unitTestHelpers import scatest
from omniORB import URI, any, CORBA
from ossie.cf import CF, CF__POA
import CosNaming
import threading
import commands
import ossie.properties as properties
import CosEventComm,CosEventComm__POA
import CosEventChannelAdmin, CosEventChannelAdmin__POA
from ossie.cf import StandardEvent
from ossie.events import ChannelManager
from ossie.utils import redhawk
from ossie.events import Subscriber

java_support = scatest.hasJavaSupport()
execDeviceNode = "/nodes/test_GPP_node/DeviceManager.dcd.xml"

class Consumer_i(CosEventComm__POA.PushConsumer):
    def __init__(self, parent):
        self.parent = parent
        self.count = 0

    def push(self, data):
        if data:
            self.parent.eventFlag = True
            self.parent.localEvent.set()
            self.count = self.count +1

    def disconnect_push_consumer (self):
        pass


class PropertyChangeListener_Receiver(CF__POA.PropertyChangeListener):
    def __init__(self):
        self.count = 0

    def propertyChange( self, pce ) :
        self.count = self.count +1


class PropertyChangeListenerTest(scatest.CorbaTestCase):
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

    def test_PropertyChangeListener_CPP(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager(execDeviceNode, self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        if java_support:
            self._domMgr.installApplication("/waveforms/PropertyChangeListener/PropertyChangeListener.sad.xml")
        else:
            self._domMgr.installApplication("/waveforms/PropertyChangeListenerNoJava/PropertyChangeListenerNoJava.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        self._app = app

        ps=None
        c=None
        d=redhawk.attach(scatest.getTestDomainName())
        a=d.apps[0]
        c=filter( lambda c : c.name == 'PropertyChange_C1', a.comps )[0]
        self.assertNotEqual(c,None)
        ps = c.ref._narrow(CF.PropertySet)
        self.assertNotEqual(ps,None)
        
        # create listener interface
        myl = PropertyChangeListener_Receiver()
        t=float(0.5)
        regid=ps.registerPropertyListener( myl._this(), ['prop1'],t)
        app.start()
        time.sleep(1)


        # assign 3 changed values
        c.prop1 = 100.0
        time.sleep(.6)   # wait for listener to receive notice
        c.prop1 = 200.0
        time.sleep(.6)   # wait for listener to receive notice
        c.prop1 = 300.0
        time.sleep(.6)   # wait for listener to receive notice
        
        # now check results
        self.assertEquals(myl.count,4)

        # change unmonitored property
        c.prop2 = 100
        time.sleep(.6)   # wait for listener to receive notice

        # now check results
        self.assertEquals(myl.count,4)

        # unregister
        ps.unregisterPropertyListener( regid )

        c.prop1 = 100.0
        time.sleep(.6)   # wait for listener to receive notice
        
        # now check results, should be same... 
        self.assertEquals(myl.count,4)

        self.assertRaises( CF.InvalidIdentifier,
            ps.unregisterPropertyListener, regid )
                           

        app.releaseObject()
        self._app=None


    def test_PropertyChangeListener_PYTHON(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager(execDeviceNode, self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        if java_support:
            self._domMgr.installApplication("/waveforms/PropertyChangeListener/PropertyChangeListener.sad.xml")
        else:
            self._domMgr.installApplication("/waveforms/PropertyChangeListenerNoJava/PropertyChangeListenerNoJava.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        self._app=app

        ps=None
        c=None
        d=redhawk.attach(scatest.getTestDomainName())
        a=d.apps[0]
        c=filter( lambda c : c.name == 'PropertyChange_P1', a.comps )[0]
        self.assertNotEqual(c,None)
        ps = c.ref._narrow(CF.PropertySet)
        self.assertNotEqual(ps,None)
        
        # create listener interface
        myl = PropertyChangeListener_Receiver()
        t=float(0.5)
        regid=ps.registerPropertyListener( myl._this(), ['prop1'],t)
        app.start()
        time.sleep(1)

        # assign 3 changed values
        c.prop1 = 100.0
        time.sleep(.6)   # wait for listener to receive notice
        c.prop1 = 200.0
        time.sleep(.6)   # wait for listener to receive notice
        c.prop1 = 300.0
        time.sleep(.6)   # wait for listener to receive notice
        
        # now check results
        self.assertEquals(myl.count,4)

        # change unmonitored property
        c.prop2 = 100
        time.sleep(.6)   # wait for listener to receive notice

        # now check results
        self.assertEquals(myl.count,4)

        # unregister
        ps.unregisterPropertyListener( regid )

        c.prop1 = 100.0
        time.sleep(.6)   # wait for listener to receive notice
        
        # now check results, should be same... 
        self.assertEquals(myl.count,4)

        self.assertRaises( CF.InvalidIdentifier,
            ps.unregisterPropertyListener, regid )
                           

        app.releaseObject()
        self._app=None


    def test_PropertyChangeListener_JAVA(self):
        if not java_support:
            return
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager(execDeviceNode, self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/PropertyChangeListener/PropertyChangeListener.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        self._app=app

        ps=None
        c=None
        d=redhawk.attach(scatest.getTestDomainName())
        a=d.apps[0]
        c=filter( lambda c : c.name == 'PropertyChange_J1', a.comps )[0]
        self.assertNotEqual(c,None)
        ps = c.ref._narrow(CF.PropertySet)
        self.assertNotEqual(ps,None)
        
        # create listener interface
        myl = PropertyChangeListener_Receiver()
        t=float(0.5)
        regid=ps.registerPropertyListener( myl._this(), ['prop1'],t)

        app.start()
        time.sleep(1)

        # assign 3 changed values
        c.prop1 = 100.0
        time.sleep(.6)   # wait for listener to receive notice
        c.prop1 = 200.0
        time.sleep(.6)   # wait for listener to receive notice
        c.prop1 = 300.0
        time.sleep(.6)   # wait for listener to receive notice
        
        # now check results
        self.assertEquals(myl.count,4)

        # change unmonitored property
        c.prop2 = 100
        time.sleep(.6)   # wait for listener to receive notice

        # now check results
        self.assertEquals(myl.count,4)

        # unregister
        ps.unregisterPropertyListener( regid )

        c.prop1 = 100.0
        time.sleep(.6)   # wait for listener to receive notice
        
        # now check results, should be same... 
        self.assertEquals(myl.count,4)

        self.assertRaises( CF.InvalidIdentifier,
            ps.unregisterPropertyListener, regid )
                           

        app.releaseObject()
        self._app=None


    def test_PropertyChangeListener_APP(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager(execDeviceNode, self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/PropertyChangeListener/PropertyChangeListener.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        self._app=app

        ps=None
        c=None
        d=redhawk.attach(scatest.getTestDomainName())
        a=d.apps[0]
        # component with external property
        c=filter( lambda c : c.name == 'PropertyChange_C1', a.comps )[0]
        # assembly controller
        c2=filter( lambda c : c.name == 'PropertyChange_P1', a.comps )[0]
        self.assertNotEqual(a,None)
        self.assertNotEqual(c,None)
        self.assertNotEqual(c2,None)
        ps = a.ref._narrow(CF.PropertySet)
        self.assertNotEqual(ps,None)
        
        # create listener interface
        myl = PropertyChangeListener_Receiver()
        t=float(0.5)
        regid=ps.registerPropertyListener( myl._this(), ['prop1', 'app_prop1'],t)
        app.start()
        time.sleep(1)


        # assign 3 changed values
        c.prop1 = 100.0
        c2.prop1 = 100.0
        time.sleep(.6)   # wait for listener to receive notice
        c.prop1 = 200.0
        c2.prop1 = 100.0
        time.sleep(.6)   # wait for listener to receive notice
        c.prop1 = 300.0
        c2.prop1 = 100.0
        time.sleep(.6)   # wait for listener to receive notice
        
        # now check results
        self.assertEquals(myl.count,8)

        # change unmonitored property
        c.prop2 = 100
        c2.prop2 = 100
        time.sleep(.6)   # wait for listener to receive notice

        # now check results
        self.assertEquals(myl.count,8)

        # unregister
        ps.unregisterPropertyListener( regid )

        c.prop1 = 100.0
        c2.prop1 = 100.0
        time.sleep(.6)   # wait for listener to receive notice
        
        # now check results, should be same... 
        self.assertEquals(myl.count,8)

        self.assertRaises( CF.InvalidIdentifier,
            ps.unregisterPropertyListener, regid )
                           
        app.releaseObject()
        self._app=None


class PropertyChangeListenerEventTest(scatest.CorbaTestCase):
    def setUp(self):
        self._domBooter, self._domMgr = self.launchDomainManager()
        # create listener interface
        orb = CORBA.ORB_init()
        self.chanMgr = ChannelManager(orb)
        self._app=None
        # Force creation
        self.channel1 = self.chanMgr.createEventChannel("TestChan", force=True)


    def tearDown(self):
        try:
            if self.channel1:
                self.chanMgr.destroyEventChannel("TestChan")                           
        except:
            pass

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


    def test_PropertyChangeListener_EC_CPP(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager(execDeviceNode, self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        if java_support:
            self._domMgr.installApplication("/waveforms/PropertyChangeListener/PropertyChangeListener.sad.xml")
        else:
            self._domMgr.installApplication("/waveforms/PropertyChangeListenerNoJava/PropertyChangeListenerNoJava.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        self._app = app

        ps=None
        c=None
        d=redhawk.attach(scatest.getTestDomainName())
        a=d.apps[0]
        c=filter( lambda c : c.name == 'PropertyChange_C1', a.comps )[0]
        self.assertNotEqual(c,None)
        ps = c.ref._narrow(CF.PropertySet)
        self.assertNotEqual(ps,None)

        # check if channel is valid
        self.assertNotEqual(self.channel1, None)
        self.assertNotEqual(self.channel1._narrow(CosEventChannelAdmin.EventChannel), None)

        sub = Subscriber( self.channel1 )

        t=float(0.5)
        regid=ps.registerPropertyListener( self.channel1, ['prop1'],t)
        app.start()
        time.sleep(1)

        # assign 3 changed values
        c.prop1 = 100.0
        time.sleep(.6)   # wait for listener to receive notice
        c.prop1 = 200.0
        time.sleep(.6)   # wait for listener to receive notice
        c.prop1 = 300.0
        time.sleep(.6)   # wait for listener to receive notice

        for n in range(4):
            xx=sub.getData()
            self.assertNotEqual(xx, None)

        # unregister
        ps.unregisterPropertyListener( regid )

        self.assertRaises( CF.InvalidIdentifier,
            ps.unregisterPropertyListener, regid )

        app.releaseObject()
        self._app=None

    def test_PropertyChangeListener_EC_PYTHON(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager(execDeviceNode, self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        if java_support:
            self._domMgr.installApplication("/waveforms/PropertyChangeListener/PropertyChangeListener.sad.xml")
        else:
            self._domMgr.installApplication("/waveforms/PropertyChangeListenerNoJava/PropertyChangeListenerNoJava.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        self._app = app

        ps=None
        c=None
        d=redhawk.attach(scatest.getTestDomainName())
        a=d.apps[0]
        c=filter( lambda c : c.name == 'PropertyChange_P1', a.comps )[0]
        self.assertNotEqual(c,None)
        ps = c.ref._narrow(CF.PropertySet)
        self.assertNotEqual(ps,None)

        # check if channel is valid
        self.assertNotEqual(self.channel1, None)
        self.assertNotEqual(self.channel1._narrow(CosEventChannelAdmin.EventChannel), None)

        sub = Subscriber( self.channel1 )

        t=float(0.5)
        regid=ps.registerPropertyListener( self.channel1, ['prop1'],t)
        app.start()
        time.sleep(1)

        # assign 3 changed values
        c.prop1 = 100.0
        time.sleep(.6)   # wait for listener to receive notice
        c.prop1 = 200.0
        time.sleep(.6)   # wait for listener to receive notice
        c.prop1 = 300.0
        time.sleep(.6)   # wait for listener to receive notice

        for n in range(4):
            xx=sub.getData()
            self.assertNotEqual(xx, None)

        # unregister
        ps.unregisterPropertyListener( regid )

        self.assertRaises( CF.InvalidIdentifier,
            ps.unregisterPropertyListener, regid )

        app.releaseObject()
        self._app=None

    def test_PropertyChangeListener_EC_JAVA(self):
        if not java_support:
            return
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager(execDeviceNode, self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/PropertyChangeListener/PropertyChangeListener.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        self._app = app

        ps=None
        c=None
        d=redhawk.attach(scatest.getTestDomainName())
        a=d.apps[0]
        c=filter( lambda c : c.name == 'PropertyChange_J1', a.comps )[0]
        self.assertNotEqual(c,None)
        ps = c.ref._narrow(CF.PropertySet)
        self.assertNotEqual(ps,None)
        
        # check if channel is valid
        self.assertNotEqual(self.channel1, None)
        self.assertNotEqual(self.channel1._narrow(CosEventChannelAdmin.EventChannel), None)

        sub = Subscriber( self.channel1 )

        t=float(0.5)
        regid=ps.registerPropertyListener( self.channel1, ['prop1'],t)
        app.start()
        time.sleep(1)

        # assign 3 changed values
        c.prop1 = 100.0
        time.sleep(.6)   # wait for listener to receive notice
        c.prop1 = 200.0
        time.sleep(.6)   # wait for listener to receive notice
        c.prop1 = 300.0
        time.sleep(.6)   # wait for listener to receive notice

        for n in range(4):
            xx=sub.getData()
            self.assertNotEqual(xx, None)

        # unregister
        ps.unregisterPropertyListener( regid )

        self.assertRaises( CF.InvalidIdentifier,
            ps.unregisterPropertyListener, regid )

        app.releaseObject()
        self._app=None

    def test_PropertyChangeListener_EC_APP(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager(execDeviceNode, self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/PropertyChangeListener/PropertyChangeListener.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        self._app = app

        ps=None
        c=None
        d=redhawk.attach(scatest.getTestDomainName())
        a=d.apps[0]
        # component with external property
        c=filter( lambda c : c.name == 'PropertyChange_C1', a.comps )[0]
        # assembly controller
        c2=filter( lambda c : c.name == 'PropertyChange_P1', a.comps )[0]
        self.assertNotEqual(a,None)
        self.assertNotEqual(c,None)
        self.assertNotEqual(c2,None)
        ps = a.ref._narrow(CF.PropertySet)
        self.assertNotEqual(ps,None)

        # check if channel is valid
        self.assertNotEqual(self.channel1, None)
        self.assertNotEqual(self.channel1._narrow(CosEventChannelAdmin.EventChannel), None)

        sub = Subscriber( self.channel1 )

        t=float(0.5)
        regid=ps.registerPropertyListener( self.channel1, ['prop1', 'app_prop1'],t)
        app.start()
        time.sleep(1)

        # assign 3 changed values
        c.prop1 = 100.0
        time.sleep(.6)   # wait for listener to receive notice
        c.prop1 = 200.0
        time.sleep(.6)   # wait for listener to receive notice
        c.prop1 = 300.0
        time.sleep(.6)   # wait for listener to receive notice

        for n in range(4):
            xx=sub.getData()
            self.assertNotEqual(xx, None)

        # unregister
        ps.unregisterPropertyListener( regid )

        self.assertRaises( CF.InvalidIdentifier,
            ps.unregisterPropertyListener, regid )

        app.releaseObject()
        self._app=None
