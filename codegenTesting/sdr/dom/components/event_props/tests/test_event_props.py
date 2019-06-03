#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK codegenTesting.
#
# REDHAWK codegenTesting is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK codegenTesting is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import unittest
import ossie.utils.testing
import os, copy
from omniORB import any, CORBA, tcInternal
from ossie import properties, resource
from ossie.cf import CF, PortTypes
import copy
import threading
import struct
import time
try:
    import CosEventComm,CosEventComm__POA
    import CosEventChannelAdmin, CosEventChannelAdmin__POA
    from ossie.cf import StandardEvent
    from ossie.events import ChannelManager
    hasEvents = True
except:
    hasEvents = False

class MessageConsumerPort(CosEventChannelAdmin__POA.EventChannel):
    class Consumer_i(CosEventChannelAdmin__POA.ProxyPushConsumer):
        def __init__(self, parent, instance_id):
            self.supplier = None
            self.parent = parent
            self.instance_id = instance_id
            self.existence_lock = threading.Lock()
            
        def push(self, data):
            self.parent.queueLock.acquire()
            self.parent.eventQueue.append(data)
            self.parent.queueLock.release()
        
        def connect_push_supplier(self, supplier):
            self.supplier = supplier
            
        def disconnect_push_consumer(self):
            self.existence_lock.acquire()
            try:
                self.supplier.disconnect_push_supplier()
            except:
                pass
            self.parent.actionQueue.put(('destroy',self.instance_id))
            self.existence_lock.release()
    
    class SupplierAdmin_i(CosEventChannelAdmin__POA.SupplierAdmin):
        def __init__(self, parent):
            self.parent = parent
            self.instance_counter = 0
    
        def obtain_push_consumer(self):
            self.consumer = self.parent.Consumer_i(self.parent,self.instance_counter)
            return self.consumer._this()

    def __init__(self):
        self.supplier_admin = self.SupplierAdmin_i(self)
        self.eventQueue = []
        self.queueLock = threading.Lock()
    
    # CosEventChannelAdmin.EventChannel
    def for_suppliers(self):
        return self.supplier_admin._this()
    
    def getEvent(self):
        self.queueLock.acquire()
        try:
            x = self.eventQueue[0]
            self.eventQueue.remove(x)
        except:
            x = None
        self.queueLock.release()
        return x

class ComponentTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all component implementations in event_props"""
    
    def testSimples(self):
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()

        consumerPort = MessageConsumerPort()
        self.eventPort = self.comp_obj.getPort('propEvent')
        self.eventPort.connectPort(consumerPort._this(), 'some_id')

        numericProps = {"eventShortSimple":CORBA.TC_short,
                        "eventUlongSimple":CORBA.TC_ulong,
                        "eventFloatSimple":CORBA.TC_float,
                        "eventUshortSimple":CORBA.TC_ushort,
                        "eventDoubleSimple":CORBA.TC_double,
                        "eventLongSimple":CORBA.TC_long,
                        "eventLonglongSimple":CORBA.TC_longlong,
                        "eventUlonglongSimple":CORBA.TC_ulonglong,
                        "eventOctetSimple":CORBA.TC_octet
                        }
        
        for propID in numericProps:
            #test that an event is created for the first configure
            val = 5
            self.comp_obj.configure([ossie.cf.CF.DataType(id=propID, value=CORBA.Any(numericProps[propID], val))])
            event = consumerPort.getEvent()
            if not event:
                self.fail("No event was generated for " + str(propID) + " for value " + str(val))
            ret = self.comp_obj.query([ossie.cf.CF.DataType(id=propID,value=any.to_any(None))])[0] 
    
            #test that an event is not created if the value is not changed
            self.comp_obj.configure([ossie.cf.CF.DataType(id=propID, value=CORBA.Any(numericProps[propID], val))])
            event = consumerPort.getEvent()
            if event:
                self.fail("An event was generated for " + str(propID) + " when the value was not changed")
            
            #test that an event is created when the value is changed
            val = 6
            self.comp_obj.configure([ossie.cf.CF.DataType(id=propID, value=CORBA.Any(numericProps[propID], val))])
            event = consumerPort.getEvent()
            if not event:
                self.fail("No event was generated for " + str(propID) + " for value " + str(val))
    
            #test that the configure worked properly
            ret = self.comp_obj.query([ossie.cf.CF.DataType(id=propID,value=any.to_any(None))])[0] 
            self.assertEquals(ret.value.value(), val, msg='configure failed for ' + str(propID) + ', ' + str(ret.value.value()) + ' != ' + str(val))
        

        ###NOW WE TEST THE REMAINING TYPES (eventBoolSimple, eventStringSimple, eventCharSimple)
        #test that an event is created for the first configure
        propID = 'eventBoolSimple'
        val = True
        now_val_prop = self.comp_obj.query([ossie.cf.CF.DataType(id=propID, value=any.to_any(None))])
        now_val = now_val_prop[0].value._v
        if now_val == True:
            self.comp_obj.configure([ossie.cf.CF.DataType(id=propID, value=CORBA.Any(CORBA.TC_boolean, False))])
            event = consumerPort.getEvent()
        self.comp_obj.configure([ossie.cf.CF.DataType(id=propID, value=CORBA.Any(CORBA.TC_boolean, val))])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(val))
        ret = self.comp_obj.query([ossie.cf.CF.DataType(id=propID,value=any.to_any(None))])[0] 

        #test that an event is not created if the value is not changed
        self.comp_obj.configure([ossie.cf.CF.DataType(id=propID, value=CORBA.Any(CORBA.TC_boolean, val))])
        event = consumerPort.getEvent()
        if event:
            self.fail("An event was generated for " + str(propID) + " when the value was not changed")
        
        #test that an event is created when the value is changed
        val = False
        self.comp_obj.configure([ossie.cf.CF.DataType(id=propID, value=CORBA.Any(CORBA.TC_boolean, val))])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(val))

        #test that the configure worked properly
        ret = self.comp_obj.query([ossie.cf.CF.DataType(id=propID,value=any.to_any(None))])[0] 
        self.assertEquals(ret.value.value(), val, msg='configure failed for ' + str(propID) + ', ' + str(ret.value.value()) + ' != ' + str(val))
        
        #test that an event is created for the first configure
        propID = 'eventStringSimple'
        val = 'hello'
        self.comp_obj.configure([ossie.cf.CF.DataType(id=propID, value=CORBA.Any(CORBA.TC_string, val))])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(val))
        ret = self.comp_obj.query([ossie.cf.CF.DataType(id=propID,value=any.to_any(None))])[0] 

        #test that an event is not created if the value is not changed
        self.comp_obj.configure([ossie.cf.CF.DataType(id=propID, value=CORBA.Any(CORBA.TC_string, val))])
        event = consumerPort.getEvent()
        if event:
            self.fail("An event was generated for " + str(propID) + " when the value was not changed")
        
        #test that an event is created when the value is changed
        val = 'goodbye'
        self.comp_obj.configure([ossie.cf.CF.DataType(id=propID, value=CORBA.Any(CORBA.TC_string, val))])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(val))

        #test that the configure worked properly
        ret = self.comp_obj.query([ossie.cf.CF.DataType(id=propID,value=any.to_any(None))])[0] 
        self.assertEquals(ret.value.value(), val, msg='configure failed for ' + str(propID) + ', ' + str(ret.value.value()) + ' != ' + str(val))
        
        #test that an event is created for the first configure
        propID = 'eventCharSimple'
        val = 'a'
        data = struct.pack('1c', val)
        self.comp_obj.configure([ossie.cf.CF.DataType(id=propID, value=CORBA.Any(CORBA.TC_char, data))])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(val))
        ret = self.comp_obj.query([ossie.cf.CF.DataType(id=propID,value=any.to_any(None))])[0] 

        #test that an event is not created if the value is not changed
        self.comp_obj.configure([ossie.cf.CF.DataType(id=propID, value=CORBA.Any(CORBA.TC_char, data))])
        event = consumerPort.getEvent()
        if event:
            self.fail("An event was generated for " + str(propID) + " when the value was not changed")
        
        #test that an event is created when the value is changed
        val = 'b'
        data = struct.pack('1c', val)
        self.comp_obj.configure([ossie.cf.CF.DataType(id=propID, value=CORBA.Any(CORBA.TC_char, data))])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(val))

        #test that the configure worked properly
        ret = self.comp_obj.query([ossie.cf.CF.DataType(id=propID,value=any.to_any(None))])[0] 
        self.assertEquals(ret.value.value(), val, msg='configure failed for ' + str(propID) + ', ' + str(ret.value.value()) + ' != ' + str(val))
        

    def testSeqs(self):
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        
        consumerPort = MessageConsumerPort()
        self.eventPort = self.comp_obj.getPort('propEvent')
        self.eventPort.connectPort(consumerPort._this(), 'some_id')
        
        numericProps = {"eventShortSeq":CORBA.ShortSeq,
                        "eventUlongSeq":CORBA.ULongSeq,
                        "eventFloatSeq":CORBA.FloatSeq,
                        "eventUshortSeq":CORBA.UShortSeq,
                        "eventDoubleSeq":CORBA.DoubleSeq,
                        "eventLongSeq":CORBA.LongSeq,
                        "eventLonglongSeq":PortTypes.LongLongSequence,
                        "eventUlonglongSeq":PortTypes.UlongLongSequence
                        }
        
        for propID in numericProps:
            #test that an event is created for the first configure
            values = [1,2,3,4,5]
            valueSet = ossie.cf.CF.DataType(id=propID, value=any.to_any(values))
            valueSet.value._t = tcInternal.typeCodeFromClassOrRepoId(numericProps[propID])
            self.comp_obj.configure([valueSet])
            event = consumerPort.getEvent()
            if not event:
                self.fail("No event was generated for " + str(propID) + " for value " + str(values))
            
            #test that an event is not created if the value is not changed
            self.comp_obj.configure([valueSet])
            event = consumerPort.getEvent()
            if event:
                self.fail("An event was generated for " + str(propID) + " when the value was not changed")
            
            #test that an event is created when the value is changed
            values = [1,2,3,4,6]
            valueSet = ossie.cf.CF.DataType(id=propID, value=any.to_any(values))
            valueSet.value._t = tcInternal.typeCodeFromClassOrRepoId(numericProps[propID])
            self.comp_obj.configure([valueSet])
            event = consumerPort.getEvent()
            if not event:
                self.fail("No event was generated for " + str(propID) + " for value " + str(values))
    
            #test that the configure worked properly
            ret = self.comp_obj.query([ossie.cf.CF.DataType(id=propID,value=any.to_any(None))])[0]
            self.assertEquals(ret.value.value(), values, msg='configure failed for ' + str(propID) + ', ' + str(ret.value.value()) + ' != ' + str(values))
    
    
        ###NOW WE TEST THE REMAINING TYPES (eventBoolSeq, eventStringSeq, eventCharSeq, eventOctetSeq)
        #test that an event is created for the first configure
        propID = 'eventBoolSeq'
        values = [True, False, True, False]
        valueSet = ossie.cf.CF.DataType(id=propID, value=any.to_any(values))
        valueSet.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.BooleanSeq)
        self.comp_obj.configure([valueSet])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(values))
        
        #test that an event is not created if the value is not changed
        self.comp_obj.configure([valueSet])
        event = consumerPort.getEvent()
        if event:
            self.fail("An event was generated for " + str(propID) + " when the value was not changed")
        
        #test that an event is created when the value is changed
        values = [False, True, False, True]
        valueSet = ossie.cf.CF.DataType(id=propID, value=any.to_any(values))
        valueSet.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.BooleanSeq)
        self.comp_obj.configure([valueSet])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(values))

        #test that the configure worked properly
        ret = self.comp_obj.query([ossie.cf.CF.DataType(id=propID,value=any.to_any(None))])[0]
        self.assertEquals(ret.value.value(), values, msg='configure failed for ' + str(propID) + ', ' + str(ret.value.value()) + ' != ' + str(values))
        
        #test that an event is created for the first configure
        propID = 'eventStringSeq'
        values = ['one','two','three','four','five']
        valueSet = ossie.cf.CF.DataType(id=propID, value=any.to_any(values))
        valueSet.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.StringSeq)
        self.comp_obj.configure([valueSet])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(values))
        
        #test that an event is not created if the value is not changed
        self.comp_obj.configure([valueSet])
        event = consumerPort.getEvent()
        if event:
            self.fail("An event was generated for " + str(propID) + " when the value was not changed")
        
        #test that an event is created when the value is changed
        values = ['one','two','three','four','six']
        valueSet = ossie.cf.CF.DataType(id=propID, value=any.to_any(values))
        valueSet.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.StringSeq)
        self.comp_obj.configure([valueSet])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(values))

        #test that the configure worked properly
        ret = self.comp_obj.query([ossie.cf.CF.DataType(id=propID,value=any.to_any(None))])[0]
        self.assertEquals(ret.value.value(), values, msg='configure failed for ' + str(propID) + ', ' + str(ret.value.value()) + ' != ' + str(values))
        
        #test that an event is created for the first configure
        propID = 'eventCharSeq'
        values = 'hello'
        valueSet = ossie.cf.CF.DataType(id=propID, value=any.to_any(values))
        valueSet.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.CharSeq)
        self.comp_obj.configure([valueSet])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(values))
        
        #test that an event is not created if the value is not changed
        self.comp_obj.configure([valueSet])
        event = consumerPort.getEvent()
        if event:
            self.fail("An event was generated for " + str(propID) + " when the value was not changed")
        
        #test that an event is created when the value is changed
        values = 'goodbye'
        valueSet = ossie.cf.CF.DataType(id=propID, value=any.to_any(values))
        valueSet.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.CharSeq)
        self.comp_obj.configure([valueSet])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(values))

        #test that the configure worked properly
        ret = self.comp_obj.query([ossie.cf.CF.DataType(id=propID,value=any.to_any(None))])[0]
        self.assertEquals(str(ret.value.value()), values, msg='configure failed for ' + str(propID) + ', ' + str(ret.value.value()) + ' != ' + str(values))
        
        #test that an event is created for the first configure
        propID = 'eventOctetSeq'
        values = [1,2,3,4,5]
        data = struct.pack('5b', *[x for x in values]) #octet data must be packed bytes
        valueSet = ossie.cf.CF.DataType(id=propID, value=any.to_any(data))
        valueSet.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.OctetSeq)
        self.comp_obj.configure([valueSet])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(values))
        
        #test that an event is not created if the value is not changed
        self.comp_obj.configure([valueSet])
        event = consumerPort.getEvent()
        if event:
            self.fail("An event was generated for " + str(propID) + " when the value was not changed")
        
        #test that an event is created when the value is changed
        values = [1,2,3,4,6]
        data = struct.pack('5b', *[x for x in values]) #octet data must be packed bytes
        valueSet = ossie.cf.CF.DataType(id=propID, value=any.to_any(data))
        valueSet.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.OctetSeq)
        self.comp_obj.configure([valueSet])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(values))

        #test that the configure worked properly
        ret = self.comp_obj.query([ossie.cf.CF.DataType(id=propID,value=any.to_any(None))])[0]
        self.assertEquals([x for x in struct.unpack('5b', ret.value.value())], values, msg='configure failed for ' + str(propID) + ', ' + str(ret.value.value()) + ' != ' + str(values))
    
    
    def testStructs(self):
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        
        consumerPort = MessageConsumerPort()
        self.eventPort = self.comp_obj.getPort('propEvent')
        self.eventPort.connectPort(consumerPort._this(), 'some_id')

        #test that an event is created for the first configure
        propID = 'eventStruct'
        val = 5
        phrase = 'hello'
        flag = True
        char = 'a'
        chardata = struct.pack('1c', char)
        configValue = [ossie.cf.CF.DataType(id='shortSimple', value=CORBA.Any(CORBA.TC_short, val)), 
                       ossie.cf.CF.DataType(id='stringSimple', value=CORBA.Any(CORBA.TC_string, phrase)),
                       ossie.cf.CF.DataType(id='boolSimple', value=CORBA.Any(CORBA.TC_boolean, flag)),
                       ossie.cf.CF.DataType(id='ulongSimple', value=CORBA.Any(CORBA.TC_ulong, val)),
                       ossie.cf.CF.DataType(id='floatSimple', value=CORBA.Any(CORBA.TC_float, val)),
                       ossie.cf.CF.DataType(id='octetSimple', value=CORBA.Any(CORBA.TC_octet, val)),
                       ossie.cf.CF.DataType(id='ushortSimple', value=CORBA.Any(CORBA.TC_ushort, val)),
                       ossie.cf.CF.DataType(id='doubleSimple', value=CORBA.Any(CORBA.TC_double, val)),
                       ossie.cf.CF.DataType(id='longSimple', value=CORBA.Any(CORBA.TC_long, val)),
                       ossie.cf.CF.DataType(id='longlongSimple', value=CORBA.Any(CORBA.TC_longlong, val)),
                       ossie.cf.CF.DataType(id='ulonglongSimple', value=CORBA.Any(CORBA.TC_ulonglong, val)),
                       ossie.cf.CF.DataType(id='charSimple', value=CORBA.Any(CORBA.TC_char, chardata))]
        self.comp_obj.configure([ossie.cf.CF.DataType(id=propID, value=CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), configValue))])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(configValue))
          
        #test that an event is not created if the value is not changed
        self.comp_obj.configure([ossie.cf.CF.DataType(id=propID, value=CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), configValue))])
        event = consumerPort.getEvent()
        if event:
            self.fail("An event was generated for " + str(propID) + " when the value was not changed")
        
        #test that an event is created when the value is changed
        val = 6
        phrase = 'goodbye'
        flag = False
        char = 'b'
        chardata = struct.pack('1c', char)
        configValue = [ossie.cf.CF.DataType(id='shortSimple', value=CORBA.Any(CORBA.TC_short, val)), 
                       ossie.cf.CF.DataType(id='stringSimple', value=CORBA.Any(CORBA.TC_string, phrase)),
                       ossie.cf.CF.DataType(id='boolSimple', value=CORBA.Any(CORBA.TC_boolean, flag)),
                       ossie.cf.CF.DataType(id='ulongSimple', value=CORBA.Any(CORBA.TC_ulong, val)),
                       ossie.cf.CF.DataType(id='floatSimple', value=CORBA.Any(CORBA.TC_float, val)),
                       ossie.cf.CF.DataType(id='octetSimple', value=CORBA.Any(CORBA.TC_octet, val)),
                       ossie.cf.CF.DataType(id='ushortSimple', value=CORBA.Any(CORBA.TC_ushort, val)),
                       ossie.cf.CF.DataType(id='doubleSimple', value=CORBA.Any(CORBA.TC_double, val)),
                       ossie.cf.CF.DataType(id='longSimple', value=CORBA.Any(CORBA.TC_long, val)),
                       ossie.cf.CF.DataType(id='longlongSimple', value=CORBA.Any(CORBA.TC_longlong, val)),
                       ossie.cf.CF.DataType(id='ulonglongSimple', value=CORBA.Any(CORBA.TC_ulonglong, val)),
                       ossie.cf.CF.DataType(id='charSimple', value=CORBA.Any(CORBA.TC_char, chardata))]
        self.comp_obj.configure([ossie.cf.CF.DataType(id=propID, value=CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), configValue))])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(configValue))
        
        #test that the configure worked properly
        ret = self.comp_obj.query([ossie.cf.CF.DataType(id='eventStruct', value=CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), 
                                        [ossie.cf.CF.DataType(id='shortSimple', value=any.to_any( None)), 
                                         ossie.cf.CF.DataType(id='stringSimple', value=any.to_any( None))]))])[0]
        propDict = properties.prop_to_dict(ret)[propID]
        self.assertEquals(propDict['shortSimple'], val, 
                          msg='configure failed for ' + str(propID) + ', ' + str(propDict['shortSimple']) + ' != ' + str(val))
        self.assertEquals(propDict['stringSimple'], phrase, 
                          msg='configure failed for ' + str(propID) + ', ' + str(propDict['stringSimple']) + ' != ' + str(phrase))


    def testStructSeqs(self):
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        
        consumerPort = MessageConsumerPort()
        self.eventPort = self.comp_obj.getPort('propEvent')
        self.eventPort.connectPort(consumerPort._this(), 'some_id')
        
        #test that an event is created for the first configure
        propID = 'eventStructSeq'
        val = 5
        flag = True
        phrase = 'hello'
        seq = ossie.cf.CF.DataType(id=propID, value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/AnySeq:1.0"), 
                                [CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), 
                                   [ossie.cf.CF.DataType(id='ss_stringSimple', value=CORBA.Any(CORBA.TC_string, phrase)), 
                                    ossie.cf.CF.DataType(id='ss_boolSimple', value=CORBA.Any(CORBA.TC_boolean, flag)),
                                    ossie.cf.CF.DataType(id='ss_shortSimple', value=CORBA.Any(CORBA.TC_short, val)),
                                    ossie.cf.CF.DataType(id='ss_floatSimple', value=CORBA.Any(CORBA.TC_float, val))])]))
        self.comp_obj.configure([seq])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(seq))
        
        #test that an event is not created if the value is not changed
        self.comp_obj.configure([seq])
        event = consumerPort.getEvent()
        if event:
            self.fail("An event was generated for " + str(propID) + " when the value was not changed")
        
        #test that an event is created when the value is changed
        val = 6
        flag = False
        phrase = 'goodbye'
        seq = ossie.cf.CF.DataType(id=propID, value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/AnySeq:1.0"), 
                                [CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), 
                                   [ossie.cf.CF.DataType(id='ss_stringSimple', value=CORBA.Any(CORBA.TC_string, phrase)), 
                                    ossie.cf.CF.DataType(id='ss_boolSimple', value=CORBA.Any(CORBA.TC_boolean, flag)),
                                    ossie.cf.CF.DataType(id='ss_shortSimple', value=CORBA.Any(CORBA.TC_short, val)),
                                    ossie.cf.CF.DataType(id='ss_floatSimple', value=CORBA.Any(CORBA.TC_float, val))])]))
        self.comp_obj.configure([seq])
        event = consumerPort.getEvent()
        if not event:
            self.fail("No event was generated for " + str(propID) + " for value " + str(seq))
        
        #test that the configure worked properly
        ret = self.comp_obj.query([ossie.cf.CF.DataType(id='eventStructSeq', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/AnySeq:1.0"), []))])[0]
        propDict = properties.prop_to_dict(ret)[propID][0]
        self.assertEquals(propDict['ss_shortSimple'], val, 
                          msg='configure failed for ' + str(propID) + ', ' + str(propDict['ss_shortSimple']) + ' != ' + str(val))
        self.assertEquals(propDict['ss_floatSimple'], val, 
                          msg='configure failed for ' + str(propID) + ', ' + str(propDict['ss_floatSimple']) + ' != ' + str(val))
        self.assertEquals(propDict['ss_stringSimple'], phrase, 
                          msg='configure failed for ' + str(propID) + ', ' + str(propDict['ss_stringSimple']) + ' != ' + str(phrase))
        self.assertEquals(propDict['ss_boolSimple'], flag, 
                          msg='configure failed for ' + str(propID) + ', ' + str(propDict['ss_boolSimple']) + ' != ' + str(flag))
        
        
    def testEventPortSendFunction(self):
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        
        consumerPort = MessageConsumerPort()
        self.eventPort = self.comp_obj.getPort('propEvent')
        self.eventPort.connectPort(consumerPort._this(), 'some_id')
        
        propIDs = ['eventShortSimple',
                'eventStringSimple',
                'eventBoolSimple',
                'eventUlongSimple',
                'eventFloatSimple',
                'eventOctetSimple',
                'eventCharSimple',
                'eventUshortSimple',
                'eventDoubleSimple',
                'eventLongSimple',
                'eventLonglongSimple',
                'eventUlonglongSimple',
                'eventStringSeq',
                'eventBoolSeq',
                'eventUlongSeq',
                'eventFloatSeq',
                'eventOctetSeq',
                'eventCharSeq',
                'eventUshortSeq',
                'eventDoubleSeq',
                'eventLongSeq',
                'eventLonglongSeq',
                'eventUlonglongSeq',
                'eventShortSeq',
                'eventStruct',
                'eventStructSeq']
        
        for propID in propIDs:
            #test that the internal code sends out an event for each of these properties
            self.comp_obj.configure([ossie.cf.CF.DataType(id='propToSend', value=CORBA.Any(CORBA.TC_string, propID))])
            eventSent = False
            while not eventSent:
                ret = self.comp_obj.query([ossie.cf.CF.DataType(id='eventSent',value=any.to_any(None))])[0]
                if ret.value.value():
                    eventSent = True
            time.sleep(.003) #allow the event send to complete in the other thread
            event = consumerPort.getEvent()
            if not event:
                self.fail("No event was generated for " + str(propID))
        
if __name__ == "__main__":
    ossie.utils.testing.main("../event_props.spd.xml") # By default tests all implementations
