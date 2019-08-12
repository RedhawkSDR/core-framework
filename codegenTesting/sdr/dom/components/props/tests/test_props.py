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
import struct
from ossie.utils import sb

class ComponentTestsNew(ossie.utils.testing.RHTestCase):
    SPD_FILE = '../props.spd.xml'

    # setUp is run before every function preceded by "test" is executed
    # tearDown is run after every function preceded by "test" is executed
    
    # self.comp is a component using the sandbox API
    # to create a data source, the package sb contains sources like StreamSource or FileSource
    # to create a data sink, there are sinks like StreamSink and FileSink
    # to connect the component to get data from a file, process it, and write the output to a file, use the following syntax:
    #  src = sb.FileSource('myfile.dat')
    #  snk = sb.StreamSink()
    #  src.connect(self.comp)
    #  self.comp.connect(snk)
    #  sb.start()
    #
    # components/sources/sinks need to be started. Individual components or elements can be started
    #  src.start()
    #  self.comp.start()
    #
    # every component/elements in the sandbox can be started
    #  sb.start()

    def setUp(self):
        # Launch the component, using the selected implementation
        self.comp = sb.launch(self.spd_file, impl=self.impl, initialize=False, configure=None)
    
    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        sb.release()

    def testStructSeqDefVal(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.assertEqual(len(self.comp.foo),3)
        self.assertEqual(self.comp.foo[0].x,"c")
        self.assertEqual(self.comp.foo[0].y,"d")
        self.assertEqual(self.comp.foo[1].x,"e")
        self.assertEqual(self.comp.foo[1].y,"f")
        self.assertEqual(self.comp.foo[2].x,"g")
        self.assertEqual(self.comp.foo[2].y,"h")
        self.assertEqual(len(self.comp.structSeqPropDefValue),2)
        self.assertEqual(self.comp.structSeqPropDefValue[0].structSeqStringSimpleDefValue,"foo")
        self.assertEqual(self.comp.structSeqPropDefValue[0].structSeqBoolSimpleDefValue,False)
        self.assertEqual(self.comp.structSeqPropDefValue[0].structSeqShortSimpleDefValue,4)
        self.assertEqual(self.comp.structSeqPropDefValue[0].structSeqFloatSimpleDefValue,10)
        self.assertEqual(self.comp.structSeqPropDefValue[0].structSeqCharSimpleDefValue,"A")
        self.assertEqual(self.comp.structSeqPropDefValue[0].structSeqOctetSimpleDefValue,1)
        self.assertEqual(self.comp.structSeqPropDefValue[1].structSeqStringSimpleDefValue,"bar")
        self.assertEqual(self.comp.structSeqPropDefValue[1].structSeqBoolSimpleDefValue,True)
        self.assertEqual(self.comp.structSeqPropDefValue[1].structSeqShortSimpleDefValue,3)
        self.assertEqual(self.comp.structSeqPropDefValue[1].structSeqFloatSimpleDefValue,20)
        self.assertEqual(self.comp.structSeqPropDefValue[1].structSeqCharSimpleDefValue,"B")
        self.assertEqual(self.comp.structSeqPropDefValue[1].structSeqOctetSimpleDefValue,2)

class ComponentTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all component implementations in props"""

    def testScaBasicBehavior(self):
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        #######################################################################
        # Verify the basic state of the component
        self.assertNotEqual(self.comp_obj, None)
        self.assertEqual(self.comp_obj._non_existent(), False)
        self.assertEqual(self.comp_obj._is_a("IDL:CF/Resource:1.0"), True)
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        
#################################################
################## WARNING ######################
#################################################
## The getPropertySet function does not work as 
## it is expected to.  It confuses types and
## uses a different structure to configure than
## a typical configure would use.  This causes
## issues mostly with the sequence type props
#################################################
        configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        self.comp_obj.configure(configureProps)
        
        
        #######################################################################
        # Validate that query returns all expected parameters
        # Query of '[]' should return the following set of properties
        expectedProps = []
        expectedProps.extend(self.getPropertySet(kinds=("configure", "execparam"), modes=("readwrite", "readonly"), includeNil=True))
        expectedProps.extend(self.getPropertySet(kinds=("allocate",), action="external", includeNil=True))
        props = self.comp_obj.query([])
        props = dict((x.id, any.from_any(x.value)) for x in props)
        # Query may return more than expected, but not less
        for expectedProp in expectedProps:
            self.assertEquals(props.has_key(expectedProp.id), True)
        
        #######################################################################
        # Verify that all expected ports are available
        for port in self.scd.get_componentfeatures().get_ports().get_uses():
            port_obj = self.comp_obj.getPort(str(port.get_usesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a("IDL:CF/Port:1.0"),  True)
            
        for port in self.scd.get_componentfeatures().get_ports().get_provides():
            port_obj = self.comp_obj.getPort(str(port.get_providesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a(port.get_repid()),  True)
            
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp_obj.start()
        self.comp_obj.stop()
        
        #######################################################################
        # Simulate regular component shutdown
        self.comp_obj.releaseObject()
        
        
    def testConfigureQuerySimples(self):   
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        
        numTestRuns = 10
        val = 15
        charval = 'a'
        chardata = struct.pack('1c', charval)

        flag = True
        for x in range(numTestRuns):
            phrase = 'blah' + str(val)
            flag = not flag
            
            self.comp_obj.configure([ossie.cf.CF.DataType(id='stringSimple', value=CORBA.Any(CORBA.TC_string, phrase))])
            # Need to reset boolSimple value so value will definitely change
            self.comp_obj.configure([ossie.cf.CF.DataType(id='boolSimple', value=any.to_any(None))])
            self.comp_obj.configure([ossie.cf.CF.DataType(id='boolSimple', value=CORBA.Any(CORBA.TC_boolean, flag))])
            self.comp_obj.configure([ossie.cf.CF.DataType(id='ulongSimple', value=CORBA.Any(CORBA.TC_ulong, val))])
            self.comp_obj.configure([ossie.cf.CF.DataType(id='shortSimple', value=CORBA.Any(CORBA.TC_short, val))])
            self.comp_obj.configure([ossie.cf.CF.DataType(id='floatSimple', value=CORBA.Any(CORBA.TC_float, val))])
            self.comp_obj.configure([ossie.cf.CF.DataType(id='octetSimple', value=CORBA.Any(CORBA.TC_octet, val))])
            self.comp_obj.configure([ossie.cf.CF.DataType(id='charSimple', value=CORBA.Any(CORBA.TC_char, chardata))])
            self.comp_obj.configure([ossie.cf.CF.DataType(id='ushortSimple', value=CORBA.Any(CORBA.TC_ushort, val))])
            self.comp_obj.configure([ossie.cf.CF.DataType(id='doubleSimple', value=CORBA.Any(CORBA.TC_double, val))])
            self.comp_obj.configure([ossie.cf.CF.DataType(id='longSimple', value=CORBA.Any(CORBA.TC_long, val))])
            self.comp_obj.configure([ossie.cf.CF.DataType(id='longlongSimple', value=CORBA.Any(CORBA.TC_longlong, val))])
            self.comp_obj.configure([ossie.cf.CF.DataType(id='ulonglongSimple', value=CORBA.Any(CORBA.TC_ulonglong, val))])
            
            simple_list = self.comp_obj.query([
                                       ossie.cf.CF.DataType(id=str('stringSimple'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('boolSimple'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('ulongSimple'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('shortSimple'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('floatSimple'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('octetSimple'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('charSimple'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('ushortSimple'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('doubleSimple'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('longSimple'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('longlongSimple'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('ulonglongSimple'),value=any.to_any(None))
                                               ])

            
            simple_dict = properties.props_to_dict(simple_list)

            #the onconfigure for the stringSimple is supposed to double the phrase
            self.assertEquals(simple_dict['stringSimple'], phrase*2, msg=str(simple_dict['stringSimple']) + ' != ' + str(phrase*2) + '  for stringSimple')
            simple_dict.pop('stringSimple')
            
            #the onconfigure for the boolSimple is supposed to flip the flag
            self.assertEquals(simple_dict['boolSimple'], not flag, msg=str(simple_dict['boolSimple']) + ' != ' + str(not flag) + '  for boolSimple')
            simple_dict.pop('boolSimple')
            self.comp_obj.configure([ossie.cf.CF.DataType(id='boolSimple', value=CORBA.Any(CORBA.TC_boolean, flag))])
            
            #the onconfigure for the charSimple is supposed raise the case
            self.assertEquals(simple_dict['charSimple'], charval.upper(), msg=str(simple_dict['charSimple']) + ' != ' + str(charval.upper()) + '  for charSimple')
            simple_dict.pop('charSimple')
            
            #the rest of the onconfigures are supposed to double the value
            for x in simple_dict:
                self.assertEquals(simple_dict[x], val*2, msg=str(simple_dict[x]) + ' != ' + str(val*2) + '  for ' + x)
            val = val+1
    
    
    def testConfigureQuerySeqs(self):
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        
        values = [1,2,3,4,5]
        phrases = ['one','two','three','four','five']
        phrase = 'testing'
        octetData = struct.pack('5b', *[x for x in values]) #octet data must be packed bytes
        flags = [True, False, True, False]
        
        values_r = copy.deepcopy(values)
        values_r.reverse()
        phrases_r = copy.deepcopy(phrases)
        phrases_r.reverse()
        phrase_r = ([x for x in phrase])
        phrase_r.reverse()
        flags_r = copy.deepcopy(flags)
        flags_r.reverse()
        
        numTestRuns = 10
        for x in range(numTestRuns):    
            stringset = ossie.cf.CF.DataType(id='stringSeq', value=any.to_any(phrases))
            stringset.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.StringSeq)
            self.comp_obj.configure([stringset])
              
            boolset = ossie.cf.CF.DataType(id='boolSeq', value=any.to_any(flags))  
            boolset.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.BooleanSeq)
            self.comp_obj.configure([boolset])
  
            ulongset = ossie.cf.CF.DataType(id='ulongSeq', value=any.to_any(values))
            ulongset.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.ULongSeq)
            self.comp_obj.configure([ulongset])
            
            shortset = ossie.cf.CF.DataType(id='shortSeq', value=any.to_any(values))
            shortset.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.ShortSeq)
            self.comp_obj.configure([shortset])
            
            floatset = ossie.cf.CF.DataType(id='floatSeq', value=any.to_any(values))
            floatset.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.FloatSeq)
            self.comp_obj.configure([floatset])
            
            charset = ossie.cf.CF.DataType(id='charSeq', value=any.to_any(phrase))
            charset.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.CharSeq)
            self.comp_obj.configure([charset])
            
            octetset = ossie.cf.CF.DataType(id='octetSeq', value=any.to_any(octetData))
            octetset.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.OctetSeq)
            self.comp_obj.configure([octetset])
            
            ushortset = ossie.cf.CF.DataType(id='ushortSeq', value=any.to_any(values))
            ushortset.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.UShortSeq)
            self.comp_obj.configure([ushortset])
            
            doubleset = ossie.cf.CF.DataType(id='doubleSeq', value=any.to_any(values))
            doubleset.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.DoubleSeq)
            self.comp_obj.configure([doubleset])
            
            longset = ossie.cf.CF.DataType(id='longSeq', value=any.to_any(values))
            longset.value._t = tcInternal.typeCodeFromClassOrRepoId(CORBA.LongSeq)
            self.comp_obj.configure([longset])
            
            longlongset = ossie.cf.CF.DataType(id='longlongSeq', value=any.to_any(values))
            longlongset.value._t = tcInternal.typeCodeFromClassOrRepoId(PortTypes.LongLongSequence)
            self.comp_obj.configure([longlongset])
            
            ulonglongset = ossie.cf.CF.DataType(id='ulonglongSeq', value=any.to_any(values))
            ulonglongset.value._t = tcInternal.typeCodeFromClassOrRepoId(PortTypes.UlongLongSequence)
            self.comp_obj.configure([ulonglongset])
    
            simple_list = self.comp_obj.query([
                                       ossie.cf.CF.DataType(id=str('stringSeq'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('boolSeq'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('ulongSeq'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('shortSeq'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('floatSeq'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('charSeq'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('octetSeq'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('ushortSeq'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('doubleSeq'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('longSeq'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('longlongSeq'),value=any.to_any(None)),
                                       ossie.cf.CF.DataType(id=str('ulonglongSeq'),value=any.to_any(None))
                                            ])         
            
            simple_dict = properties.props_to_dict(simple_list)
            
            #the onconfigure for the stringSeq is supposed to reverse the order of the phrases
            self.assertEquals(simple_dict['stringSeq'], phrases_r, msg=str(simple_dict['stringSeq']) + ' != ' + str(phrases_r) + '  for stringSeq')
            simple_dict.pop('stringSeq')
            
            #the onconfigure for the boolSeq is supposed to reverse the order of the flags
            self.assertEquals(simple_dict['boolSeq'], flags_r, msg=str(simple_dict['boolSeq']) + ' != ' + str(flags_r) + '  for boolSeq')
            simple_dict.pop('boolSeq')
            
            #the onconfigure for the charSeq is supposed to reverse the order of the phrase
            self.assertEquals([x for x in simple_dict['charSeq']], phrase_r, msg=str(simple_dict['charSeq']) + ' != ' + str(phrase_r) + '  for charSeq')
            simple_dict.pop('charSeq')
           
            #the onconfigure for the octetSeq is supposed to reverse the order of the phrase
            self.assertEquals([x for x in struct.unpack('5b', simple_dict['octetSeq'])], values_r, 
                              msg=str(struct.unpack('5b', simple_dict['octetSeq'])) + ' != ' + str(values_r) + '  for octetSeq')
            simple_dict.pop('octetSeq')
            
            #the rest of the onconfigures are supposed to reverse the values in the list
            for x in simple_dict:
                self.assertEquals(simple_dict[x], values_r, msg=str(simple_dict[x]) + ' != ' + str(values_r) + '  for ' + x)
    
    
    def testConfigureQueryStructs(self):
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        
        val = 5   
        phrase = 'blah'
        flag = True
        charval = 'a'
        
        self.comp_obj.configure([ossie.cf.CF.DataType(id='structProp', value=CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), 
                                        [ossie.cf.CF.DataType(id='structStringSimple', value=CORBA.Any(CORBA.TC_string, phrase)), 
                                         ossie.cf.CF.DataType(id='structBoolSimple', value=CORBA.Any(CORBA.TC_boolean, flag)), 
                                         ossie.cf.CF.DataType(id='structUlongSimple', value=CORBA.Any(CORBA.TC_ulong, val)), 
                                         ossie.cf.CF.DataType(id='structShortSimple', value=CORBA.Any(CORBA.TC_short, val)), 
                                         ossie.cf.CF.DataType(id='structFloatSimple', value=CORBA.Any(CORBA.TC_float, val)), 
                                         ossie.cf.CF.DataType(id='structOctetSimple', value=CORBA.Any(CORBA.TC_octet, val)), 
                                         ossie.cf.CF.DataType(id='structUshortSimple', value=CORBA.Any(CORBA.TC_ushort, val)), 
                                         ossie.cf.CF.DataType(id='structDoubleSimple', value=CORBA.Any(CORBA.TC_double, val)), 
                                         ossie.cf.CF.DataType(id='structLongSimple', value=CORBA.Any(CORBA.TC_long, val)), 
                                         ossie.cf.CF.DataType(id='structLonglongSimple', value=CORBA.Any(CORBA.TC_longlong, val)), 
                                         ossie.cf.CF.DataType(id='structUlonglongSimple', value=CORBA.Any(CORBA.TC_ulonglong, val)),
                                         ossie.cf.CF.DataType(id='structCharSimple', value=CORBA.Any(CORBA.TC_char, charval))]))])

        ret = self.comp_obj.query([ossie.cf.CF.DataType(id='structProp', value=CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), 
                                        [ossie.cf.CF.DataType(id='structShortSimple', value=any.to_any( None)), 
                                         ossie.cf.CF.DataType(id='structFloatSimple', value=any.to_any( None)), 
                                         ossie.cf.CF.DataType(id='structOctetSimple', value=any.to_any( None)), 
                                         ossie.cf.CF.DataType(id='structUlongSimple', value=any.to_any( None)), 
                                         ossie.cf.CF.DataType(id='structUshortSimple', value=any.to_any( None)), 
                                         ossie.cf.CF.DataType(id='structStringSimple', value=any.to_any( None)), 
                                         ossie.cf.CF.DataType(id='structDoubleSimple', value=any.to_any( None)), 
                                         ossie.cf.CF.DataType(id='structLonglongSimple', value=any.to_any( None)), 
                                         ossie.cf.CF.DataType(id='structBoolSimple', value=any.to_any( None)), 
                                         ossie.cf.CF.DataType(id='structLongSimple', value=any.to_any( None)), 
                                         ossie.cf.CF.DataType(id='structUlonglongSimple', value=any.to_any( None)),
                                         ossie.cf.CF.DataType(id='structCharSimple', value=any.to_any( None))]))])
        
        for x in ret[0].value.value():
            if x.id == 'structBoolSimple':
                boolCheck = x
                break

        self.assertEquals(x.value.value(), not flag, msg=str(x.value.value()) + ' != ' + str(not flag) + '  for structBoolSimple')       
        ret[0].value.value().remove(x)
                
        for y in ret[0].value.value():
            if y.id == 'structStringSimple':
                boolCheck = y
                break
                
        self.assertEquals(y.value.value(), phrase*2, msg=str(y.value.value()) + ' != ' + str(phrase*2) + '  for structStringSimple')
        ret[0].value.value().remove(y)
        
        for xx in ret[0].value.value():
            if xx.id == 'structCharSimple':
                boolCheck = xx
                break
                
        self.assertEquals(xx.value.value(), charval.upper(), msg=str(xx.value.value()) + ' != ' + charval.upper() + '  for structCharSimple')
        ret[0].value.value().remove(xx)
        
        for z in ret[0].value.value():
            self.assertEquals(z.value.value(), val*2, msg=str(z.value.value()) + ' != ' + str(val*2) + '  for ' + str(z.id))

    def testConfigureQueryStructSeqs(self):
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        
        props = self.comp_obj.query([])
        
        val = 5   
        phrase = 'blah'
        flag = True   
            
        seq = ossie.cf.CF.DataType(id='structSeqProp', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/AnySeq:1.0"), 
                                [CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), 
                                           [ossie.cf.CF.DataType(id='structSeqStringSimple', value=CORBA.Any(CORBA.TC_string, phrase)), 
                                            ossie.cf.CF.DataType(id='structSeqBoolSimple', value=CORBA.Any(CORBA.TC_boolean, flag)), 
                                            ossie.cf.CF.DataType(id='structSeqShortSimple', value=CORBA.Any(CORBA.TC_short, val)), 
                                            ossie.cf.CF.DataType(id='structSeqFloatSimple', value=CORBA.Any(CORBA.TC_float, val))])]))
        
        instance = seq.value._v[0]
        for x in range(4):
            seq.value._v.append(copy.deepcopy(instance))
    
        self.comp_obj.configure([seq])
        ret = self.comp_obj.query([ossie.cf.CF.DataType(id='structSeqProp', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/AnySeq:1.0"), []))])
        
        for struct in ret[0].value.value():
            for simple in struct.value():
                if simple.id == 'structSeqStringSimple':
                    self.assertEquals(simple.value.value(), phrase * 2, msg=str(simple.value.value()) + ' != ' + str(phrase*2) + '  for structSeqStringSimple')
                elif simple.id == 'structSeqBoolSimple':
                    self.assertEquals(simple.value.value(), not flag, msg=str(simple.value.value()) + ' != ' + str(not flag) + '  for structSeqBoolSimple')
                else:
                    self.assertEquals(simple.value.value(), val * 2, msg=str(simple.value.value()) + ' != ' + str(val*2) + '  for ' + str(simple.id))
       
    def testAccessRules(self):   
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        
        #try to configure a read only property should fail
        self.assertRaises(resource.CF.PropertySet.InvalidConfiguration, self.comp_obj.configure, 
                          [ossie.cf.CF.DataType(id='simpleROnly', value=CORBA.Any(CORBA.TC_short, 0))])
        
        #try to query a write only property should fail
        self.assertRaises(ossie.cf.CF.UnknownProperties, self.comp_obj.query, 
                      [ossie.cf.CF.DataType(id='simpleWOnly', value=any.to_any(None))])

        #try to configure an execparam should fail if it isn't configure as well
        self.assertRaises(resource.CF.PropertySet.InvalidConfiguration, self.comp_obj.configure, 
                          [ossie.cf.CF.DataType(id='simpleExec', value=CORBA.Any(CORBA.TC_short, 0))])

#THIS TEST IS NOT BEING USED.  It was determined by the REDHAWK Team that this is a valid action due
#to the fact that it may be unclear to someone that they need to check the configure box in order
#to be able to configure/query an execparam, but that it is a justifiable assumption that you would
#be able to query an execparam no matter what
#        #try to query an execparam should fail if it isn't configure as well
#        self.assertRaises(ossie.cf.CF.UnknownProperties, self.comp_obj.query, 
#                          [ossie.cf.CF.DataType(id='simpleExec', value=any.to_any(None))])
        
        #try to configure and query and execparam that is also configure should not fail
        confval = 100
        self.comp_obj.configure([ossie.cf.CF.DataType(id='simpleConfNExec', value=CORBA.Any(CORBA.TC_short, confval))])
        curval = self.comp_obj.query([ossie.cf.CF.DataType(id='simpleConfNExec', value=any.to_any(None))])[0].value.value()
        self.assertEquals(confval, curval)
        
        #try to configure and query and the kitchen sink, should work fine
        confval = 'blah'
        self.comp_obj.configure([ossie.cf.CF.DataType(id='simpleKitchenSink', value=CORBA.Any(CORBA.TC_string, confval))])
        curval = self.comp_obj.query([ossie.cf.CF.DataType(id='simpleKitchenSink', value=any.to_any(None))])[0].value.value()
        self.assertEquals(confval, curval)
        
        
#    def testRange(self):
#        #######################################################################
#        # Launch the component with the default execparams
#        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
#        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
#        self.launch(execparams)
#        
#        #######################################################################
#        # Simulate regular component startup
#        # Verify that initialize nor configure throw errors
#        self.comp_obj.initialize()
#        
#        ####MAKE SURE A NUMBER IN RANGE WORKS
#        #range is -50 to 50
#        confval = 25
#        self.comp_obj.configure([ossie.cf.CF.DataType(id='simpleRange', value=CORBA.Any(CORBA.TC_short, confval))])
#        curval = self.comp_obj.query([ossie.cf.CF.DataType(id='simpleRange', value=any.to_any(None))])[0].value.value()
#        self.assertEquals(confval, curval)
#
#        ####MAKE SURE A NUMBER OUT OF RANGE FAILS
#        #range is -50 to 50
#        self.assertRaises(resource.CF.PropertySet.InvalidConfiguration, self.comp_obj.configure, 
#                          [ossie.cf.CF.DataType(id='simpleRange', value=CORBA.Any(CORBA.TC_short, -100))])
#        self.assertRaises(resource.CF.PropertySet.InvalidConfiguration, self.comp_obj.configure, 
#                          [ossie.cf.CF.DataType(id='simpleRange', value=CORBA.Any(CORBA.TC_short, 100))])    
#             
#             
#    def testEnum(self):
#        #######################################################################
#        # Launch the component with the default execparams
#        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
#        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
#        self.launch(execparams)
#        
#        #######################################################################
#        # Simulate regular component startup
#        # Verify that initialize nor configure throw errors
#        self.comp_obj.initialize()
#        
#        ####MAKE SURE A VALID OPTION WORKS
#        #options are 'good', 'bad', 'fair'
#        confphrase = 'good'
#        self.comp_obj.configure([ossie.cf.CF.DataType(id='simpleEnum', value=CORBA.Any(CORBA.TC_string, confphrase))])
#        curval = self.comp_obj.query([ossie.cf.CF.DataType(id='simpleEnum', value=any.to_any(None))])[0].value.value()
#        self.assertEquals(confphrase, curval)
#        
#        ###MAKE SURE AN INVLAID OPTION FAILS
#        #options are 'good', 'bad', 'fair'
##++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
##++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#        confphrase = 'blah'
#        try:
#            self.comp_obj.configure([ossie.cf.CF.DataType(id='simpleEnum', value=CORBA.Any(CORBA.TC_string, confphrase))])
#        except:
#            pass
##        else:
##            curval = self.comp_obj.query([ossie.cf.CF.DataType(id='simpleEnum', value=any.to_any(None))])[0].value.value()
##            self.assertNotEquals(confphrase, curval)
##++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
##++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#
##################################
##TODO
##UNCOMMENT BELOW AND ERASE ABOVE
##################################             
##        ##THIS CODE WILL REPLACE CODE ABOVE WHEN ENUMERATIONS ARE PROPERLY ENFORCED
##        self.assertRaises(resource.CF.PropertySet.InvalidConfiguration, self.comp_obj.configure, 
##                          [ossie.cf.CF.DataType(id='simpleEnum', value=CORBA.Any(CORBA.TC_short, 'blah'))])
#
#
#    def testExecParam(self):
#        #######################################################################
#        # Launch the component with the default execparams
#        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
#        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
#        self.launch(execparams)
#        
#        #the value of the execparam should be 10 in the prf
#        curval = self.comp_obj.query([ossie.cf.CF.DataType(id='simpleExec', value=any.to_any(None))])[0].value.value()
#        self.assertEquals(curval, 10)
    
if __name__ == "__main__":
    ossie.utils.testing.main("../props.spd.xml") # By default tests all implementations
