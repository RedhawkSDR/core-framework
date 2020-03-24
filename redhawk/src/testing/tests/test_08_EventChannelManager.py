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
from omniORB import CORBA, URI, any
from ossie.cf import CF
from ossie.properties import *
from ossie.events import Manager
from ossie.events.Publisher import Receiver as DisconnectReceiver
import threading
import time
import CosEventComm,CosEventComm__POA
import CosEventChannelAdmin
import traceback

class SimpleConsumer(CosEventComm__POA.PushConsumer):
    def __init__(self):
        self.disconnect=False
        pass

    def push(self, data_obj):
        self.data = data_object

    def disconnect_push_consumer(self):
        self.disconnect=True

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


    def _process_results( self, app, comp_name='ECM', delay=1, wait_int=1.5, enablecb=False):
        components = app._get_registeredComponents()
        comp = None
        for component in components:
            if comp_name in component.componentObject._get_identifier():
                comp = component.componentObject
                if enablecb :
                    component.componentObject.configure([CF.DataType( id='enablecb', value=any.to_any(True)) ])
                break

        app.start()
        time.sleep(delay)
        mlimit=None
        mxmit = None
        mrecv=None
        self.assertNotEqual(comp,None)
        stuff = comp.query([])
        self.assertNotEqual(stuff,None)
        pdict = props_to_dict(stuff)
        mlimit = pdict['msg_limit']

        gotmrecv=False
        pcnt=0
        while gotmrecv == False :
            stuff = comp.query([] )
            stuff = comp.query([ CF.DataType( id='msg_xmit', value=any.to_any(None)) ] )
            self.assertNotEqual(stuff,None)
            pdict = props_to_dict(stuff)
            mxmit = pdict['msg_xmit']


            stuff = comp.query([ CF.DataType(id='msg_recv', value=any.to_any(None)) ] )
            self.assertNotEqual(stuff,None)
            pdict = props_to_dict(stuff)
            mrecv = pdict['msg_recv']
            if ( mlimit == mrecv and mlimit == mxmit) or pcnt == 5:
                gotmrecv=True
                continue

            pcnt+=1
            time.sleep(wait_int)

        return (mlimit, mxmit, mrecv)

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
        self.app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(self.app, None)
        mlimit, mxmit, mrecv = self._process_results( self.app )
        self.assertNotEquals(mlimit, None )
        self.assertNotEquals(mxmit, None )
        self.assertNotEquals(mrecv, None )
        self.assertEquals(mlimit, mxmit )
        self.assertEquals(mlimit, mrecv )

    def test_ECM_CppComponent_Callbacks(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/ECM1/ECM1.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        self.app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(self.app, None)
        mlimit, mxmit, mrecv = self._process_results( self.app , enablecb=True)
        self.assertNotEquals(mlimit, None )
        self.assertNotEquals(mxmit, None )
        self.assertNotEquals(mrecv, None )
        self.assertEquals(mlimit, mxmit )
        self.assertEquals(mlimit, mrecv )


    def test_ECM_PythonComponent(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/ECM2/ECM2.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        self.app = appFact.create(appFact._get_name(), [], [])
        mlimit, mxmit, mrecv = self._process_results( self.app )
        self.assertNotEquals(mlimit, None )
        self.assertNotEquals(mxmit, None )
        self.assertNotEquals(mrecv, None )
        self.assertEquals(mlimit, mxmit )
        self.assertEquals(mlimit, mrecv )

    def test_ECM_PythonComponent_Callbacks(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/ECM2/ECM2.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        self.app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(self.app, None)
        mlimit, mxmit, mrecv = self._process_results( self.app , enablecb=True)
        self.assertNotEquals(mlimit, None )
        self.assertNotEquals(mxmit, None )
        self.assertNotEquals(mrecv, None )
        self.assertEquals(mlimit, mxmit )
        self.assertEquals(mlimit, mrecv )

    @scatest.requireJava
    def test_ECM_JavaComponent(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/ECM3/ECM3.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        self.app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(self.app, None)
        mlimit, mxmit, mrecv = self._process_results( self.app )
        self.assertNotEquals(mlimit, None )
        self.assertNotEquals(mxmit, None )
        self.assertNotEquals(mrecv, None )
        self.assertEquals(mlimit, mxmit )
        self.assertEquals(mlimit, mrecv )


    @scatest.requireJava
    def test_ECM_JavaComponent_Callbacks(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/ECM3/ECM3.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        self.app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(self.app, None)
        mlimit, mxmit, mrecv = self._process_results( self.app, enablecb=True )
        self.assertNotEquals(mlimit, None )
        self.assertNotEquals(mxmit, None )
        self.assertNotEquals(mrecv, None )
        self.assertEquals(mlimit, mxmit )
        self.assertEquals(mlimit, mrecv )

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
        clist,citer = self.ecm.listRegistrants('doesnotexist')
        self.assertEqual(clist, [] )
        self.assertEqual(citer, None)
        self.assertEqual(self._domBooter.poll(), None)


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

        # need to wait for channel to be deleted and cleaned up
        time.sleep(1)
        reg = self.ecm.registerResource('ecm_test2')
        self.assertNotEqual(reg, None)

        self.assertRaises( CF.EventChannelManager.ChannelDoesNotExist, self.ecm.unregister, 'ecm_test3', reg.reg.reg_id )

        # test exceptions during release
        self.assertRaises( CF.EventChannelManager.RegistrationsExists, self.ecm.release, 'ecm_test2' )

        self.ecm.unregister('ecm_test2', reg.reg.reg_id)

        self.assertRaises( CF.EventChannelManager.ChannelDoesNotExist, self.ecm.release, 'ecm_test2' )

        self.ecm.release( 'ecm_test' )

        self.assertRaises( CF.EventChannelManager.ChannelDoesNotExist, self.ecm.release, 'ecm_test')

    def _test_ECM_InvalidChannelName(self, invalid_name ):

        reg=CF.EventChannelManager.EventRegistration("", "test-blank-channel")

        # test channel name blank
        self.assertRaises( CF.EventChannelManager.OperationFailed, self.ecm.ref.create, invalid_name)

        self.assertRaises( CF.EventChannelManager.OperationFailed, self.ecm.ref.get, invalid_name)

        self.assertRaises( CF.EventChannelManager.OperationFailed, self.ecm.ref.createForRegistrations, invalid_name)

        self.assertRaises( CF.EventChannelManager.InvalidChannelName, self.ecm.ref.registerResource, reg )

        c=SimpleConsumer()
        self.assertRaises( CF.EventChannelManager.InvalidChannelName, self.ecm.ref.registerConsumer,c._this(), reg )

        d=DisconnectReceiver()
        self.assertRaises( CF.EventChannelManager.InvalidChannelName, self.ecm.ref.registerPublisher, reg, d._this() )

        self.assertRaises( CF.EventChannelManager.ChannelDoesNotExist, self.ecm.ref.release, invalid_name)
        self.assertRaises( CF.EventChannelManager.ChannelDoesNotExist, self.ecm.ref.forceRelease, invalid_name)
        self.assertRaises( CF.EventChannelManager.ChannelDoesNotExist, self.ecm.ref.markForRegistrations, invalid_name)
        self.assertRaises( CF.EventChannelManager.ChannelDoesNotExist, self.ecm.ref.unregister, reg)


    def test_ECM_InvalidChannelNames(self):
        
        self._test_ECM_InvalidChannelName('')
        self._test_ECM_InvalidChannelName('\"\"')
        self._test_ECM_InvalidChannelName('\'\'')
        self._test_ECM_InvalidChannelName('\"          \"')
        self._test_ECM_InvalidChannelName('              ')
        self._test_ECM_InvalidChannelName('\t\n\r\f\v ')
        self._test_ECM_InvalidChannelName('\"          \"')
        self._test_ECM_InvalidChannelName('\"\t\n\r\f\v\t\"')
        self._test_ECM_InvalidChannelName('.badname')
        self._test_ECM_InvalidChannelName('badname.')
        self._test_ECM_InvalidChannelName('bad.name')
        self._test_ECM_InvalidChannelName('.badname.')
        self._test_ECM_InvalidChannelName(':badname')
        self._test_ECM_InvalidChannelName('badname:')
        self._test_ECM_InvalidChannelName('bad:name')
        self._test_ECM_InvalidChannelName(':bad:name:')
        
    def test_EM_selfUnregister(self):
        
        class domContainer:
            def __init__(self, dommgr):
                self.dom = dommgr
            def getRef(self):
                return self.dom

        class resourceContainer:
            def __init__(self, dommgr):
                self.dom = domContainer(dommgr)
            def getDomainManager(self):
                return self.dom

        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        ecm = self._domMgr._get_eventChannelMgr()

        # get list of channels (should be ODM/IDM
        clist,citer = self.ecm.listChannels(2)
        self.assertEqual(citer, None )
        self.assertEqual(len(clist), 2 )

        clist,citer = self.ecm.listRegistrants('IDM_Channel', 3)
        self.assertEqual(citer, None )
        self.assertEqual(len(clist), 2 )

        evt_reg = CF.EventChannelManager.EventRegistration( channel_name = 'IDM_Channel', reg_id = 'my_reg_id')
        reg = ecm.registerResource(evt_reg)
        res = resourceContainer(self._domMgr)
        mgr = Manager.GetManager(res)
        em_pub = mgr.Publisher('IDM_Channel', 'foo')
        em_sub = mgr.Subscriber('IDM_Channel', 'hello')

        # push some data and make sure it arrives
        em_pub.push(any.to_any(['hello']))
        time.sleep(1)
        self.assertEquals(em_sub.getData()._v, ['hello'])

        # release the subscriber and push some data and make sure it does not arrive
        em_sub.terminate()
        em_pub.push(any.to_any(['hello']))
        time.sleep(1)
        self.assertEquals(em_sub.getData(), None)

        # create a new subscriber and push some data and make sure the publisher is still ok
        em_sub_2 = mgr.Subscriber('IDM_Channel', 'hello_2')
        em_pub.push(any.to_any(['hello']))
        time.sleep(1)
        self.assertEquals(em_sub_2.getData()._v, ['hello'])

        # release the publisher and push some data and make sure it does not arrive
        em_pub_2 = mgr.Publisher('IDM_Channel', 'foo_2')
        em_pub.terminate()
        em_pub.push(any.to_any(['hello']))
        time.sleep(1)
        self.assertEquals(em_sub_2.getData(), None)

        # create a new publisher and push some data and make sure the subcriber is still ok
        em_pub_2.push(any.to_any(['hello']))
        time.sleep(1)
        self.assertEquals(em_sub_2.getData()._v, ['hello'])
        
    def test_ECM_RegisterConsumerNonExistentChannel(self):

        # test setup starts up domain and connects to event channel manager 

        # event channel context 
        orb=CORBA.ORB_init()
        cname='testchannel'
        curi="corbaname::#"+self._domMgr._get_name()+'/'+cname
        
        # check channel does not exist
        try:
            c=orb.string_to_object(curi)
            self.ecm.forceRelease(cname)            
        except:
            pass

        # create EventRegistration and call registerConsumer
        try:
            _consumer = ossie.events.Receiver()
            evt_reg = CF.EventChannelManager.EventRegistration(channel_name = cname, reg_id = 'my_reg_id')
            reg = self.ecm.registerConsumer(_consumer, evt_reg)
        except:
            self.fail("EventChannelManager RegisterConsumer failed for non-existent channel, " + cname)

        # verify returned channel is valid
        self.assertNotEqual(reg.channel, None, "registerConsumer returned empty Event Channel.")
        tch = reg.channel._narrow(CosEventChannelAdmin.EventChannel)
        self.assertNotEqual(tch, None, "Event Channel was not created")

        # verify channel is in event service
        try:
            c=orb.string_to_object(curi)            
        except:
            self.fail("Event channel (testchannel) does not exist")

        # cleanup
        try:
            self.ecm.forceRelease(cname)
        except:
            pass

    def test_ECM_RegisterPublisherNonExistentChannel(self):
        
        # test setup starts up domain and connects to event channel manager

        # event channel context 
        orb=CORBA.ORB_init()
        cname='testchannel'
        curi="corbaname::#"+self._domMgr._get_name()+'/'+cname
        
        # check channel does not exist
        try:
            c=orb.string_to_object(curi)
            self.ecm.forceRelease(cname)                        
        except:
            pass

        # create EventRegistration and call registerPublisher        
        try:
            _supplier = DisconnectReceiver()
            evt_reg = CF.EventChannelManager.EventRegistration(channel_name = cname, reg_id = 'my_reg_id')
            reg = self.ecm.registerPublisher(evt_reg, _supplier)
        except:
            self.fail("EventChannelManager RegisterPublisher failed for non-existent channel, " + cname)

        # verify returned channel is valid            
        self.assertNotEqual(reg.channel, None, "registerPublisher returned empty Event Channel.")
        tch = reg.channel._narrow(CosEventChannelAdmin.EventChannel)
        self.assertNotEqual(tch, None, "Event Channel was not created")

        # verify channel is in event service
        try:
            c=orb.string_to_object(curi)            
        except:
            self.fail("Event channel (testchannel) does not exist")

            # cleanup
        try:
            self.ecm.forceRelease(cname)
        except:
            pass

        
