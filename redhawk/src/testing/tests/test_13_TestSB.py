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
from ossie.cf import CF
from ossie.cf import StandardEvent
from omniORB import CORBA, any
import os
import Queue
from ossie.utils import sb
from ossie.utils import type_helpers
globalsdrRoot = os.environ['SDRROOT']
import sys
import time
import copy
import ossie.utils.bulkio.bulkio_helpers as _bulkio_helpers

def _initSourceAndSink(dataFormat):

    source = sb.DataSource(dataFormat = dataFormat)
    sink   = sb.DataSink()

    usesPortName = source.supportedPorts[dataFormat]["portDict"]["Port Name"]
    providesPortName = sink.supportedPorts[dataFormat]["portDict"]["Port Name"]

    source.connect(
        sink,
        usesPortName     = usesPortName,
        providesPortName = providesPortName)

    source.start()
    sink.start()

    return source, sink

class SBTestTest(scatest.CorbaTestCase):
    def setUp(self):
        sb.setDEBUG(True)
        self.test_comp = "Sandbox"
        # Flagrant violation of sandbox API: if the sandbox singleton exists,
        # clean up previous state and dispose of it.
        if sb.domainless._sandbox:
            sb.domainless._sandbox.shutdown()
            sb.domainless._sandbox = None

    def assertComponentCount(self, count):
        self.assertEquals(len(sb.domainless._getSandbox().getComponents()), count)

    def tearDown(self):
        sb.domainless._getSandbox().shutdown()
        sb.setDEBUG(False)
        os.environ['SDRROOT'] = globalsdrRoot

    def test_catalog(self):
        # Store orig sdrroot
        sdrRoot = os.environ.pop('SDRROOT')

        # No SDRROOT env (should default to current directory).
        self.assertEquals(sb.catalog(), [])

        # Bad Sdr root env; dispose of existing sandbox instance to test.
        sb.domainless._sandbox = None
        os.environ["SDRROOT"] = ""
        self.assertEquals(sb.catalog(), [])

        # Good SDRROOT
        sb.setSDRROOT(sdrRoot)
        self.assertNotEquals(len(sb.catalog()), 0)

        # Bad search path
        self.assertEquals(len(sb.catalog("my_path")), 0)

        # Search path with no usable files
        self.assertEquals(len(sb.catalog("jackhammer")), 0)

        # Restore sdrroot
        os.environ['SDRROOT'] = sdrRoot

    def test_softpkgDepSingle(self):
        c = sb.launch('ticket_490_single')
        self.assertNotEquals(c, None)

    def test_softpkgDepNone(self):
        c = sb.launch('ticket_490_none')
        self.assertNotEquals(c, None)

    def test_softpkgDepDouble(self):
        c = sb.launch('ticket_490_double')
        self.assertNotEquals(c, None)

    def test_setSDRROOT(self):
        # None type
        self.assertRaises(TypeError, sb.setSDRROOT, None)

        # Bad dir should not change root
        sdrroot = sb.getSDRROOT()
        self.assertRaises(AssertionError, sb.setSDRROOT, 'TEMP_PATH')
        self.assertEquals(sdrroot, sb.getSDRROOT())

        # Good dir with no dev/dom should not change root
        self.assertRaises(AssertionError, sb.setSDRROOT, 'jackhammer')
        self.assertEquals(sdrroot, sb.getSDRROOT())

        # New root
        sb.setSDRROOT('sdr')
        self.assertEquals(sb.getSDRROOT(), 'sdr')

        # Restore sdrroot
        sb.setSDRROOT(os.environ['SDRROOT'])


    def test_componentInit(self):
        # Bad descriptors
        self.assertRaises(TypeError, sb.launch)
        self.assertRaises(ValueError, sb.launch, "test_name")

        # Make sure only one instance name and refid can be used
        comp = sb.launch(self.test_comp, "comp")
        comp.api()
        refid = comp._refid
        self.assertRaises(ValueError, sb.launch, self.test_comp, "comp")
        self.assertRaises(ValueError, sb.launch, self.test_comp, "new_comp", refid)

        # Only 1 component should be running
        self.assertComponentCount(1)

    def test_relativePath(self):
        comp = sb.launch('sdr/dom/components/Sandbox/Sandbox.spd.xml')
        self.assertComponentCount(1)

    def test_nestedSoftPkgDeps(self):
        cwd = os.getcwd()
        depLibraryPath = cwd + "/sdr/dom/components/softpkgNestedDep/spdNestedDepLibrary"
        if not depLibraryPath in sys.path:
            sys.path.append(depLibraryPath)
        if os.environ.has_key('PYTHONPATH'):
            os.environ['PYTHONPATH'] = "%s:%s" % (depLibraryPath, os.environ['PYTHONPATH'])
        else:
            os.environ['PYTHONPATH'] = "%s" % (depLibraryPath)

        comp= sb.launch('sdr/dom/components/CommandWrapperNestedSPDDep/CommandWrapperNestedSPDDep.spd.xml')
        self.assertComponentCount(1)

    def initValues(self, comp):
        # Init values
        self.assertEquals(comp.my_bool_true, True)
        self.assertEquals(comp.my_bool_false, False)
        self.assertEquals(comp.my_bool_empty, None)
        self.assertEquals(comp.my_long, 10)
        self.assertEquals(comp.my_long_empty, None)
        self.assertEquals(comp.my_str, "Hello World!")
        self.assertEquals(comp.my_str_empty, None)
        self.assertEquals(comp.my_struct.bool_true, True)
        self.assertEquals(comp.my_struct.bool_false, False)

        self.assertEquals(comp.my_struct.bool_empty, None)
        self.assertEquals(comp.my_struct.long_s, None)
        self.assertEquals(comp.my_struct.str_s, None)
        self.assertEquals(comp.my_seq_bool[0], True)
        self.assertEquals(comp.my_seq_bool[1], False)
        self.assertEquals(comp.my_seq_str[0], "one")
        self.assertEquals(comp.my_seq_str[1], "")
        self.assertEquals(comp.my_seq_str[2], "three")
        self.assertEquals(comp.my_long_enum, None)
        self.assertEquals(comp.my_bool_enum, None)
        self.assertEquals(comp.my_str_enum, None)
        self.assertEquals(comp.my_struct.enum_long, None)
        self.assertEquals(comp.my_struct.enum_bool, None)
        self.assertEquals(comp.my_struct.enum_str, None)


    def test_simpleComp(self):
        comp = sb.launch(self.test_comp)
        comp.api()

        # Check the init values
        self.initValues(comp)
        
        # Change values
        comp.my_bool_true = False
        comp.my_bool_false = True
        comp.my_long = 22
        comp.my_str = "new"
        comp.my_struct.bool_true = False
        comp.my_struct.bool_false = True
        comp.my_struct.long_s = 33
        comp.my_seq_bool[0] = False
        comp.my_seq_bool[1] = True
        comp.my_seq_str[0] = "1"
        comp.my_seq_str[1] = "2"
        comp.my_seq_str[2] = "3"
        self.assertEquals(comp.my_bool_true, False)
        self.assertEquals(comp.my_bool_false, True)
        self.assertEquals(comp.my_long, 22)
        self.assertEquals(comp.my_str, "new")
        self.assertEquals(comp.my_struct.bool_true, False)
        self.assertEquals(comp.my_struct.bool_false, True)
        self.assertEquals(comp.my_struct.long_s, 33)
        self.assertEquals(comp.my_seq_bool[0], False)
        self.assertEquals(comp.my_seq_bool[1], True)
        self.assertEquals(comp.my_seq_str[0], "1")
        self.assertEquals(comp.my_seq_str[1], "2")
        self.assertEquals(comp.my_seq_str[2], "3")

        # Checks invalid simple enumeration changes
        comp.my_long_enum = 10
        comp.my_long_enum = "one"
        comp.my_long_enum = "3"
        comp.my_long_enum = 11.1
        comp.my_long_enum = False
        self.assertEquals(comp.my_long_enum, None)
        comp.my_str_enum = 10
        comp.my_str_enum = "one"
        comp.my_str_enum = 11.1
        comp.my_str_enum = False
        self.assertEquals(comp.my_str_enum, None)
        comp.my_bool_enum = 10
        comp.my_bool_enum = "one"
        comp.my_bool_enum = 11.1
        comp.my_bool_enum = "invalid"
        self.assertEquals(comp.my_bool_enum, None)

        # Checks invalid struct enumeration changes
        comp.my_struct.enum_long = 10
        comp.my_struct.enum_long = "one"
        comp.my_struct.enum_long = "3"
        comp.my_struct.enum_long = 11.1
        comp.my_struct.enum_long = False
        self.assertEquals(comp.my_struct.enum_long, None)
        comp.my_struct.enum_str = 10
        comp.my_struct.enum_str = "one"
        comp.my_struct.enum_str = 11.1
        comp.my_struct.enum_str = False
        self.assertEquals(comp.my_struct.enum_str, None)
        comp.my_struct.enum_bool = 10
        comp.my_struct.enum_bool = "one"
        comp.my_struct.enum_bool = 11.1
        comp.my_struct.enum_bool = "valid"
        self.assertEquals(comp.my_struct.enum_bool, None)

        # Checks changes to simple enumerations
        comp.my_long_enum = "ONE"
        self.assertEquals(comp.my_long_enum, 11)
        comp.my_long_enum = "Two"
        self.assertEquals(comp.my_long_enum, 11)
        comp.my_long_enum = 33
        self.assertEquals(comp.my_long_enum, 33)
        comp.my_long_enum = 34
        self.assertEquals(comp.my_long_enum, 33)

        comp.my_str_enum = "ONE"
        self.assertEquals(comp.my_str_enum, "FIRST")
        comp.my_str_enum = "two"
        self.assertEquals(comp.my_str_enum, "FIRST")
        comp.my_str_enum = "SECOND"
        self.assertEquals(comp.my_str_enum, "SECOND")
        comp.my_str_enum = "third"
        self.assertEquals(comp.my_str_enum, "SECOND")

        comp.my_bool_enum = "VALID"
        self.assertEquals(comp.my_bool_enum, True)
        comp.my_bool_enum = "false"
        self.assertEquals(comp.my_bool_enum, True)
        comp.my_bool_enum = False
        self.assertEquals(comp.my_bool_enum, False)
        comp.my_bool_enum = True
        self.assertEquals(comp.my_bool_enum, True)
        comp.my_bool_enum = "INVALID"
        self.assertEquals(comp.my_bool_enum, False)

        # Checks changes to struct enumerations
        comp.my_struct.enum_long = "ONE"
        self.assertEquals(comp.my_struct.enum_long, 1)
        comp.my_struct.enum_long = "Two"
        self.assertEquals(comp.my_struct.enum_long, 1)
        comp.my_struct.enum_long = 3
        self.assertEquals(comp.my_struct.enum_long, 3)
        comp.my_struct.enum_long = 4
        self.assertEquals(comp.my_struct.enum_long, 3)

        comp.my_struct.enum_str = "ONE"
        self.assertEquals(comp.my_struct.enum_str, "first")
        comp.my_struct.enum_str = "two"
        self.assertEquals(comp.my_struct.enum_str, "first")
        comp.my_struct.enum_str = "second"
        self.assertEquals(comp.my_struct.enum_str, "second")
        comp.my_struct.enum_str = "THIRD"
        self.assertEquals(comp.my_struct.enum_str, "second")

        comp.my_struct.enum_bool = "VALID"
        self.assertEquals(comp.my_struct.enum_bool, True)
        comp.my_struct.enum_bool = "false"
        self.assertEquals(comp.my_struct.enum_bool, True)
        comp.my_struct.enum_bool = False
        self.assertEquals(comp.my_struct.enum_bool, False)
        comp.my_struct.enum_bool = True
        self.assertEquals(comp.my_struct.enum_bool, True)
        comp.my_struct.enum_bool = "INVALID"
        self.assertEquals(comp.my_struct.enum_bool, False)
        
        #Components which have properties with the same id where one is
        #enumerated and one is not. checks the id's can not be confused and enum 
        #values can not given to the non enumerated property
        sandbox = sb.launch("Sandbox")
        testProp = sb.launch("TestComponentProperty")
        self.assertRaises(TypeError, testProp.my_long_enum.configureValue, "TWO")
        self.assertEquals(testProp.my_long_enum, 11)
        testProp.my_struct.long_s = 1
        self.assertRaises(TypeError, testProp.my_struct.enum_long.configureValue, "THREE")
        self.assertEquals(testProp.my_struct.long_s,1)
        sandbox.my_struct.enum_str = "TWO"
        self.assertEquals(sandbox.my_struct.enum_str,"second")
        sandbox.my_struct.enum_str = "fourth"
        self.assertEquals(sandbox.my_struct.enum_str,"second")

        #Test _properties query works for getting a properties
        #kinds, action, type, value, defualt value, typecode and id
        self.assertEquals(testProp.simpleExecparam.kinds,['execparam'])
        self.assertEquals(testProp.simpleExecparam.action,"eq")
        self.assertEquals(testProp.seqTest.kinds,["configure"])
        self.assertEquals(testProp.structSeqTest[0].simpleShort,3)
        self.assertEquals(testProp.structSeqTest[1].simpleFloat.defValue, 2)
        self.assertEquals(testProp.simpleExecparam.id,"simpleExecparam")
        self.assertEquals(testProp.structSeqTest.type,"structSeq")
        self.assertEquals(testProp.simpleExecparam.typecode,CORBA.TC_float)



        # Make sure reset works properly
        sb.reset()
        self.initValues(comp)

    def test_illegalPropertyNames(self):
        comp = sb.launch(self.test_comp)
        comp.api()

        self.initValues(comp)

        # Makes sure that no getters cause exceptions
        comp.escape__simple
        comp.escape__struct.es__1
        comp.escape__struct.es__2
        comp.escape__struct.normal
        comp.my_struct.es__3
        comp.my_struct_seq
        comp.escape__structseq
        comp.my_struct_seq[0].simp__bad
        comp.my_struct_seq[1].simp_bool
        comp.escape__structseq[0].val__1
        comp.escape__structseq[1].val_2

        # Makes sure that no setters cause exceptions
        comp.escape__simple = 1234
        comp.escape__struct.es__1 = 5678
        comp.escape__struct.es__2 = 4321
        comp.escape__struct.normal = 8765
        comp.my_struct.es__3 = 333
        comp.my_struct_seq[0].simp_bool = True
        comp.my_struct_seq[1].simp__bad = 11
        comp.escape__structseq[0].val_2 = "test"
        comp.escape__structseq[1].val__1 = 22

        # Makes sure that values got set properly
        self.assertEquals(comp.escape__simple, 1234)
        self.assertEquals(comp.escape__struct.es__1, 5678)
        self.assertEquals(comp.escape__struct.es__2, 4321)
        self.assertEquals(comp.escape__struct.normal, 8765)
        self.assertEquals(comp.my_struct.es__3, 333)
        self.assertEquals(comp.my_struct_seq[0].simp_bool, True)
        self.assertEquals(comp.my_struct_seq[1].simp__bad, 11)
        self.assertEquals(comp.escape__structseq[0].val_2, "test")
        self.assertEquals(comp.escape__structseq[1].val__1, 22)

    def test_loadSADFile(self):
        retval = sb.loadSADFile('sdr/dom/waveforms/ticket_462_w/ticket_462_w.sad.xml')
        self.assertEquals(retval, True)
        comp_ac = sb.getComponent('ticket_462_ac_1')
        self.assertNotEquals(comp_ac, None)
        comp = sb.getComponent('ticket_462_1')
        self.assertNotEquals(comp, None)
        self.assertEquals(comp_ac.my_simple, "foo")
        self.assertEquals(comp_ac.my_seq, ["initial value"])
        self.assertEquals(comp_ac.basic_struct.some_simple, '4')
        self.assertEquals(comp.over_simple, "override")
        self.assertEquals(comp.over_struct_seq, [{'a_word': 'something', 'a_number': 1}])

    def test_loadSADFileUses_p(self):
        retval = sb.loadSADFile('sdr/dom/waveforms/SADUsesDeviceWave/SADUsesDeviceWaveConnectionDevProvides.sad.xml')
        self.assertEquals(retval, True)

    def test_loadSADFileUses_u(self):
        retval = sb.loadSADFile('sdr/dom/waveforms/SADUsesDeviceWave/SADUsesDeviceWaveConnectionDevUses.sad.xml')
        self.assertEquals(retval, True)

    def test_loadSADFilePartialOverloadStruct(self):
        # This tests partially overloading a struct or struct seq property in sad
        retval = sb.loadSADFile('sdr/dom/waveforms/more_ticket_462/more_ticket_462.sad.xml')
        comp = sb.getComponent('another_ticket_462_1')
        self.assertNotEquals(comp, None)
        self.assertEquals(comp.simple_prop, "This is a string")
        self.assertEquals(comp.seq_prop, [9,8,7])
        self.assertEquals(comp.struct_prop.prop_two, 'string overload')
        self.assertEquals(comp.struct_seq_prop[0].prop_five, 1)
        self.assertEquals(comp.struct_seq_prop[0].prop_six, 123.0)
        self.assertEquals(comp.struct_seq_prop[1].prop_four, "anotherString")
        self.assertEquals(comp.struct_seq_prop[1].prop_six, 345.0)
        self.assertEquals(comp.struct_seq_prop[2].prop_four, "string_overload")
        self.assertEquals(comp.struct_seq_prop[2].prop_five, 3)
        self.assertEquals(comp.struct_seq_prop[2].prop_six, 678.0)
        self.assertEquals(comp.struct_seq_prop[3].prop_six, 987.0)

    def test_loadSADFileNoOverriddenProperties(self):
        retval = sb.loadSADFile('sdr/dom/waveforms/ticket_841_and_854/ticket_841_and_854.sad.xml')
        self.assertEquals(retval, True)
        comp_ac = sb.getComponent('Sandbox_1')
        self.assertNotEquals(comp_ac, None)
        self.assertEquals(comp_ac.my_long, 10)

    def test_loadSADFile_overload_create(self):
        retval = sb.loadSADFile('sdr/dom/waveforms/ticket_462_w/ticket_462_w.sad.xml', props={'my_simple':'not foo','over_simple':'not override'})
        #retval = sb.loadSADFile('sdr/dom/waveforms/ticket_462_w/ticket_462_w.sad.xml', props=[{'my_simple':'not foo'},{'over_simple':'not override'}])
        self.assertEquals(retval, True)
        comp_ac = sb.getComponent('ticket_462_ac_1')
        self.assertNotEquals(comp_ac, None)
        comp = sb.getComponent('ticket_462_1')
        self.assertNotEquals(comp, None)
        self.assertEquals(comp_ac.my_simple, "not foo")
        self.assertEquals(comp_ac.my_seq, ["initial value"])
        self.assertEquals(comp_ac.basic_struct.some_simple, '4')
        self.assertEquals(comp.over_simple, "override")
        self.assertEquals(comp.over_struct_seq, [{'a_word': 'something', 'a_number': 1}])

    def test_simplePropertyRange(self):
        comp = sb.launch('TestPythonPropsRange')
        comp.api()

        # Test upper range
        comp.my_octet_name = 255
        comp.my_short_name = 32767
        comp.my_ushort_name = 65535
        comp.my_long_name = 2147483647
        comp.my_ulong_name = 4294967295
        comp.my_longlong_name = 9223372036854775807
        comp.my_ulonglong_name = 18446744073709551615
        self.assertEquals(comp.my_octet_name, 255)
        self.assertEquals(comp.my_short_name, 32767)
        self.assertEquals(comp.my_ushort_name, 65535)
        self.assertEquals(comp.my_long_name, 2147483647)
        self.assertEquals(comp.my_ulong_name, 4294967295)
        self.assertEquals(comp.my_longlong_name, 9223372036854775807)
        self.assertEquals(comp.my_ulonglong_name, 18446744073709551615)

        # Test lower range
        comp.my_octet_name = 0
        comp.my_short_name = -32768
        comp.my_ushort_name = 0
        comp.my_long_name = -2147483648
        comp.my_ulong_name = 0
        comp.my_longlong_name = -9223372036854775808
        comp.my_ulonglong_name = 0
        self.assertEquals(comp.my_octet_name, 0)
        self.assertEquals(comp.my_short_name, -32768)
        self.assertEquals(comp.my_ushort_name, 0)
        self.assertEquals(comp.my_long_name, -2147483648)
        self.assertEquals(comp.my_ulong_name, 0)
        self.assertEquals(comp.my_longlong_name, -9223372036854775808)
        self.assertEquals(comp.my_ulonglong_name, 0)

        # Test one beyond upper bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_octet_name.configureValue, 256)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_short_name.configureValue, 32768)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ushort_name.configureValue, 65536)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_long_name.configureValue, 2147483648)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ulong_name.configureValue, 4294967296)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_longlong_name.configureValue, 9223372036854775808)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ulonglong_name.configureValue, 18446744073709551616)

        # Test one beyond lower bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_octet_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_short_name.configureValue, -32769)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ushort_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_long_name.configureValue, -2147483649)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ulong_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_longlong_name.configureValue, -9223372036854775809)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ulonglong_name.configureValue, -1)


    def test_structPropertyRange(self):
        comp = sb.launch('TestPythonPropsRange')
        comp.api()

        # Test upper range
        comp.my_struct_name.struct_octet_name = 255
        comp.my_struct_name.struct_short_name = 32767
        comp.my_struct_name.struct_ushort_name = 65535
        comp.my_struct_name.struct_long_name = 2147483647
        comp.my_struct_name.struct_ulong_name = 4294967295
        comp.my_struct_name.struct_longlong_name = 9223372036854775807
        comp.my_struct_name.struct_ulonglong_name = 18446744073709551615
        self.assertEquals(comp.my_struct_name.struct_octet_name, 255)
        self.assertEquals(comp.my_struct_name.struct_short_name, 32767)
        self.assertEquals(comp.my_struct_name.struct_ushort_name, 65535)
        self.assertEquals(comp.my_struct_name.struct_long_name, 2147483647)
        self.assertEquals(comp.my_struct_name.struct_ulong_name, 4294967295)
        self.assertEquals(comp.my_struct_name.struct_longlong_name, 9223372036854775807)
        self.assertEquals(comp.my_struct_name.struct_ulonglong_name, 18446744073709551615)

        # Test lower range
        comp.my_struct_name.struct_octet_name = 0
        comp.my_struct_name.struct_short_name = -32768
        comp.my_struct_name.struct_ushort_name = 0
        comp.my_struct_name.struct_long_name = -2147483648
        comp.my_struct_name.struct_ulong_name = 0
        comp.my_struct_name.struct_longlong_name = -9223372036854775808
        comp.my_struct_name.struct_ulonglong_name = 0
        self.assertEquals(comp.my_struct_name.struct_octet_name, 0)
        self.assertEquals(comp.my_struct_name.struct_short_name, -32768)
        self.assertEquals(comp.my_struct_name.struct_ushort_name, 0)
        self.assertEquals(comp.my_struct_name.struct_long_name, -2147483648)
        self.assertEquals(comp.my_struct_name.struct_ulong_name, 0)
        self.assertEquals(comp.my_struct_name.struct_longlong_name, -9223372036854775808)
        self.assertEquals(comp.my_struct_name.struct_ulonglong_name, 0)

        # Test one beyond upper bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_octet_name.configureValue, 256)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_short_name.configureValue, 32768)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ushort_name.configureValue, 65536)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_long_name.configureValue, 2147483648)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ulong_name.configureValue, 4294967296)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_longlong_name.configureValue, 9223372036854775808)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ulonglong_name.configureValue, 18446744073709551616)

        # Test one beyond lower bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_octet_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_short_name.configureValue, -32769)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ushort_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_long_name.configureValue, -2147483649)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ulong_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_longlong_name.configureValue, -9223372036854775809)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ulonglong_name.configureValue, -1)

        # Makes sure the struct can be set without error
        # NB: This test used to use names instead of ids, which silently failed in 1.8.
        new_value = {'struct_octet': 100, 'struct_short': 101, 'struct_ushort': 102, 'struct_long': 103,
                     'struct_ulong': 104, 'struct_longlong': 105, 'struct_ulonglong': 106}
        comp.my_struct_name = new_value
        self.assertEquals(comp.my_struct_name, new_value)

    def test_seqPropertyRange(self):
        comp = sb.launch('TestPythonPropsRange')
        comp.api()

        # Test upper and lower bounds
        comp.seq_octet_name[0] = 0
        comp.seq_octet_name[1] = 255
        comp.seq_short_name[0] = -32768
        comp.seq_short_name[1] = 32767
        comp.seq_ushort_name[0] = 0
        comp.seq_ushort_name[1] = 65535
        comp.seq_long_name[0] = -2147483648
        comp.seq_long_name[1] = 2147483647
        comp.seq_ulong_name[0] = 0
        comp.seq_ulong_name[1] = 4294967295
        comp.seq_longlong_name[0] = -9223372036854775808
        comp.seq_longlong_name[1] = 9223372036854775807
        comp.seq_ulonglong_name[0] = 0
        #comp.seq_ulonglong_name[1] = 18446744073709551615
        self.assertEquals(comp.seq_octet_name[0], 0)
        self.assertEquals(comp.seq_octet_name[1], 255)
        self.assertEquals(comp.seq_short_name[0], -32768)
        self.assertEquals(comp.seq_short_name[1], 32767)
        self.assertEquals(comp.seq_ushort_name[0], 0)
        self.assertEquals(comp.seq_ushort_name[1], 65535)
        self.assertEquals(comp.seq_long_name[0], -2147483648)
        self.assertEquals(comp.seq_long_name[1], 2147483647)
        self.assertEquals(comp.seq_ulong_name[0], 0)
        self.assertEquals(comp.seq_ulong_name[1], 4294967295)
        self.assertEquals(comp.seq_longlong_name[0], -9223372036854775808)
        self.assertEquals(comp.seq_longlong_name[1], 9223372036854775807)
        self.assertEquals(comp.seq_ulonglong_name[0], 0)
        #self.assertEauals(comp.seq_ulonglong_name[1], 18446744073709551615)

        # Test one beyond upper bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_octet.configureValue, [0, 256])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_short.configureValue, [0, 32768])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ushort.configureValue, [0, 65536])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_long.configureValue, [0, 2147483648])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ulong.configureValue, [0, 4294967296])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_longlong.configureValue, [0, 9223372036854775808])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ulonglong.configureValue, [0, 18446744073709551616])

        # Test one beyond lower bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_octet.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_short.configureValue, [-32769, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ushort.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_long.configureValue, [-2147483649, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ulong.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_longlong.configureValue, [-9223372036854775809, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ulonglong.configureValue, [-1, 0])

        # Tests char and octet sequences
        self.assertRaises(TypeError, comp.seq_char_name.configureValue, ['A','BB'])
        self.assertRaises(TypeError, comp.seq_char_name.configureValue, ['a', 1])
        self.assertRaises(TypeError, comp.seq_octet_name.configureValue, [1, 'a'])

        comp.seq_char_name[0] = 'X'
        comp.seq_char_name[1] = 'Y'
        self.assertEquals(comp.seq_char_name[0], 'X')
        self.assertEquals(comp.seq_char_name[1], 'Y')


    def test_structSeqPropertyRange(self):
        comp = sb.launch('TestPythonPropsRange')
        comp.api()

        # Test upper and lower bounds
        comp.my_structseq_name[0].ss_octet_name = 255
        comp.my_structseq_name[1].ss_octet_name = 0
        comp.my_structseq_name[0].ss_short_name = 32767
        comp.my_structseq_name[1].ss_short_name = -32768
        comp.my_structseq_name[0].ss_ushort_name = 65535
        comp.my_structseq_name[1].ss_ushort_name = 0
        comp.my_structseq_name[0].ss_long_name = 2147483647
        comp.my_structseq_name[1].ss_long_name = -2147483648
        comp.my_structseq_name[0].ss_ulong_name = 4294967295
        comp.my_structseq_name[1].ss_ulong_name = 0
        comp.my_structseq_name[0].ss_longlong_name = 9223372036854775807
        comp.my_structseq_name[1].ss_longlong_name = -9223372036854775808
        comp.my_structseq_name[0].ss_ulonglong_name = 18446744073709551615
        comp.my_structseq_name[1].ss_ulonglong_name = 0
        self.assertEquals(comp.my_structseq_name[0].ss_octet_name, 255)
        self.assertEquals(comp.my_structseq_name[1].ss_octet_name, 0)
        self.assertEquals(comp.my_structseq_name[0].ss_short_name, 32767)
        self.assertEquals(comp.my_structseq_name[1].ss_short_name, -32768)
        self.assertEquals(comp.my_structseq_name[0].ss_ushort_name, 65535)
        self.assertEquals(comp.my_structseq_name[1].ss_ushort_name, 0)
        self.assertEquals(comp.my_structseq_name[0].ss_long_name, 2147483647)
        self.assertEquals(comp.my_structseq_name[1].ss_long_name, -2147483648)
        self.assertEquals(comp.my_structseq_name[0].ss_ulong_name, 4294967295)
        self.assertEquals(comp.my_structseq_name[1].ss_ulong_name, 0)
        self.assertEquals(comp.my_structseq_name[0].ss_longlong_name, 9223372036854775807)
        self.assertEquals(comp.my_structseq_name[1].ss_longlong_name, -9223372036854775808)
        self.assertEquals(comp.my_structseq_name[0].ss_ulonglong_name, 18446744073709551615)
        self.assertEquals(comp.my_structseq_name[1].ss_ulonglong_name, 0)

        # Test one beyond upper bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_octet_name.configureValue, 256)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_short_name.configureValue, 32768)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_ushort_name.configureValue, 65536)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_long_name.configureValue, 2147483648)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_ulong_name.configureValue, 4294967296)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_longlong_name.configureValue, 9223372036854775808)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_ulonglong_name.configureValue, 18446744073709551616)

        # Test one beyond lower bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_octet_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_short_name.configureValue, -32769)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_ushort_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_long_name.configureValue, -2147483649)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_ulong_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_longlong_name.configureValue, -9223372036854775809)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_ulonglong_name.configureValue, -1)

        # Make sure entire struct seq can be set without error
        new_value = [{'ss_octet': 100, 'ss_short': 101, 'ss_ushort': 102, 'ss_long': 103,
                      'ss_ulong': 104, 'ss_longlong': 105, 'ss_ulonglong': 106},
                     {'ss_octet': 107, 'ss_short': 108, 'ss_ushort': 109, 'ss_long': 110,
                      'ss_ulong': 111, 'ss_longlong': 112, 'ss_ulonglong': 113}]
        comp.my_structseq_name = new_value
        self.assertEqual(comp.my_structseq_name, new_value)

        # Make sure individual structs can be set without error
        # NB: This test used to use names instead of ids, which silently failed in 1.8.
        for item in new_value:
            for name in item.iterkeys():
                item[name] = item[name] + 100
        comp.my_structseq_name[0] = new_value[0]
        comp.my_structseq_name[1] = new_value[1]
        self.assertEqual(comp.my_structseq_name, new_value)

    def test_readOnlyProps(self):
        comp = sb.launch('Sandbox')
        comp.api()

        # Properties should be able to be read, but not set, and all should throw the saem exception
        exception = None
        comp.readonly_simp
        try:
            comp.readonly_simp = 'bad'
        except Exception, e:
            self.assertEquals(e.__class__, Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')

        # Makes sure both ways to set struct members throws same error
        comp.readonly_struct.readonly_struct_simp
        try:
            comp.readonly_struct.readonly_struct_simp = 'bad'
        except Exception, e:
            self.assertEquals(e.__class__, Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')
        try:
            comp.readonly_struct = {'readonly_struct_simp':'bad'}
        except Exception, e:
            self.assertEquals(e.__class__, Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')

        # Makes sure both ways to set seqs throws same error
        comp.readonly_seq[0]
        comp.readonly_seq[1]
        try:
            comp.readonly_seq[1] = 'bad'
        except Exception, e:
            self.assertEquals(e.__class__, Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')
        try:
            comp.readonly_seq = ['bad1', 'bad2']
        except Exception, e:
            self.assertEquals(e.__class__, Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')

        # Make sure all ways to set structseq throws the same error
        comp.readonly_structseq[0].readonly_s
        comp.readonly_structseq[1].readonly_s
        try:
            comp.readonly_structseq[0].readonly_s = 'bad'
        except Exception, e:
            self.assertEquals(e.__class__, Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')
        try:
            comp.readonly_structseq[1] = {'readonly_s':'bad'}
        except Exception, e:
            self.assertEquals(e.__class__, Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')
        try:
            comp.readonly_structseq = [{'readonly_s':'bad1'}, {'readonly_s':'bad2'}]
        except Exception, e:
            self.assertEquals(e.__class__, Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')

        # Nothing should have changed
        self.assertEquals(comp.readonly_simp, 'Read only simple prop')
        self.assertEquals(comp.readonly_struct.readonly_struct_simp, 'read only struct property')
        self.assertEquals(comp.readonly_seq[0], 'read only')
        self.assertEquals(comp.readonly_seq[1], 'sequence property')
        self.assertEquals(comp.readonly_structseq[0].readonly_s, 'read only')
        self.assertEquals(comp.readonly_structseq[1].readonly_s, 'struct seq property')

    def test_DuplicateNames(self):
        """
        Tests that duplicate property names that belong to different scopes
        (e.g., fields in different structs) can be accessed by the base name,
        rather than the "uniquified" version.
        """
        comp = sb.launch('struct_fields')

        comp.first.first = -1
        comp.first.second = 'TEST'
        comp.second.first = 1e6
        comp.second.second = True

        for prop in comp.query([]):
            if prop.id == 'first':
                value = comp.first.fromAny(prop.value)
                self.assertEqual(value['first::first'], -1)
                self.assertEqual(value['first::second'], 'TEST')
            elif prop.id == 'second':
                value = comp.second.fromAny(prop.value)
                self.assertEqual(value['second::first'], 1e6)
                self.assertEqual(value['second::second'], True)

    def test_StructSetFromDict(self):
        """
        Tests that setting struct properties from dictionaries supports both
        names and IDs for the keys.
        """
        comp = sb.launch('struct_fields')

        # The first property gets names, the second gets IDs
        props = {'first':{'first':-1, 'second':'TEST'},
                 'second':{'second::first':1e6, 'second::second':True}}
        comp.configure(props)

        for prop in comp.query([]):
            if prop.id == 'first':
                value = comp.first.fromAny(prop.value)
                self.assertEqual(value['first::first'], props['first']['first'])
                self.assertEqual(value['first::second'], props['first']['second'])
            elif prop.id == 'second':
                value = comp.second.fromAny(prop.value)
                first = 'second::first'
                self.assertEqual(value[first], props['second'][first])
                second = 'second::second'
                self.assertEqual(value[second], props['second'][second])

    def test_DefaultPropertyKinds(self):
        """
        Try to set a property of each type (simple, struct, etc.) to ensure
        that if the PRF does not specify a kind, it defaults to configure.
        """
        comp = sb.launch('default_kinds')

        comp.long_prop = 1
        comp.floatseq_prop = [1,2,3]
        comp.struct_prop.string_field = 'TEST'
        comp.endpoints = [{}]

    def test_Services(self):
        service = sb.launch(sb.getSDRROOT() + '/dev/services/BasicService_java/BasicService_java.spd.xml')
        comp = sb.launch('ServiceComponent')
        comp.connect(service)
        self.assertEquals(len(sb.domainless.ConnectionManager.instance().getConnections().keys()), 1)

        # Check that all the parameters got set correctly
        props = service.query([])
        d = dict([(p.id, any.from_any(p.value)) for p in props])
        self.assertEqual(d["SERVICE_NAME"], "BasicService_java_1")
        self.assertEqual(d["PARAM1"], "ABCD")
        self.assertEqual(d["PARAM2"], 42)
        self.assertAlmostEqual(d["PARAM3"], 3.1459)
        self.assertEqual(d["PARAM4"], False)
        self.assertEqual(d["PARAM5"], "Hello World")
        self.assertEqual(d.has_key("PARAM6"), False)


    def test_IDMChannel(self):
        device_spd = sb.getSDRROOT() + '/dev/devices/BasicTestDevice/BasicTestDevice.spd.xml'
        device = sb.launch(device_spd)

        channel = sb.getEventChannel('IDM_Channel')

        # Push all events onto a queue
        event_queue = Queue.Queue()
        channel.eventReceived.addListener(event_queue.put)

        # Lock and unlock the device; this should create an event
        device._set_adminState(CF.Device.LOCKED)

        # Wait up to a second, just in case
        try:
            event = event_queue.get(True, 1.0)
        except Queue.Empty:
            self.fail('Device admin state change message was never received')

        # Unpack the event and check that it matches our expectations
        event = event.value()
        self.assertEqual(event.producerId, device._refid)
        self.assertEqual(event.sourceId, device._refid)
        self.assertEqual(event.stateChangeCategory, StandardEvent.ADMINISTRATIVE_STATE_EVENT)
        self.assertEqual(event.stateChangeFrom, StandardEvent.UNLOCKED)
        self.assertEqual(event.stateChangeTo, StandardEvent.LOCKED)

    def test_EventChannels(self):
        channel_name = 'properties'

        # Check that getEventChannel raises an exception when the channel
        # doesn't exist (and it shouldn't yet)
        self.assertRaises(NameError, sb.getEventChannel, channel_name)

        # Create the event channel and ensure that a duplicate create call
        # raises an exception if exclusive is True
        channel = sb.createEventChannel(channel_name)
        self.assertRaises(NameError, sb.createEventChannel, channel_name, True)

        # Make sure getEventChannel works now
        self.assertEqual(channel, sb.getEventChannel(channel_name))

        # Push all events onto a queue
        event_queue = Queue.Queue()
        channel.eventReceived.addListener(event_queue.put)

        # Connect a component that emits events to the channel
        comp = sb.launch('PropertyChangeEvents')
        comp.connect(channel)
        self.assertEqual(channel.supplier_count, 1)

        # Trigger a property event and wait up to a second for it to be
        # received
        comp.myprop = 2
        try:
            event = event_queue.get(True, 1.0)
        except Queue.Empty:
            self.fail('Property change event not received')

        # Check that the event matches our expectations
        event = event.value()
        self.assertEqual(event.sourceId, comp._refid)
        self.assertEqual(event.sourceName, comp._instanceName)
        self.assertEqual(len(event.properties), 1)
        prop = event.properties[0]
        self.assertEqual(prop.id, 'myprop')
        self.assertEqual(any.from_any(prop.value), 2)

        # Disconnect the event channel
        comp.disconnect(channel)
        self.assertEqual(channel.supplier_count, 0)

        # Clean up the channel
        channel.destroy()

        # Should no longer be able to use the channel--even a simple attribute
        # reference throws an exception
        self.assertRaises(ReferenceError, getattr, channel, 'destroy')

        # Check that the channel is really gone from the sandbox
        self.assertRaises(NameError, sb.getEventChannel, channel_name)

    def test_LaunchTimeout(self):
        """
        Test that the launch timeout can be adjusted to accomodate components
        or devices that take longer than 10 seconds to register with the
        sandbox.
        """
        try:
            comp = sb.launch('SlowComponent', execparams={'CREATE_DELAY': 15}, timeout=20)
        except RuntimeError:
            self.fail('Launch timeout was not honored')

    def test_ComplexSequenceFromRealSequence(self):
        """
        Test that assigning a sequence of real values to a complex sequence
        property works correctly.
        """
        # NB: The default value for 'complexCharProp' raises a non-fatal
        #     exception during launch. Overriding it avoids the error message,
        #     though in general, complex char properties should be avoided.
        comp = sb.launch('TestComplexProps', configure={'complexCharProp':('a','b')})
        value = range(4)
        try:
            comp.complexFloatSequence = value
        except:
            self.fail('Could not assign real sequence to complex sequence property')
        self.assertEqual(value, comp.complexFloatSequence)

    def test_DeviceAllocation(self):
        """
        Tests device allocation/deallocation using both dictionaries and lists
        of CF.DataTypes.
        """
        spd = os.path.join(sb.getSDRROOT(), 'dev/devices/CppTestDevice/CppTestDevice.spd.xml')
        dev = sb.launch(spd)

        # Save the initial state for checking that allocation is working
        load_average = dev.load_average.queryValue()
        shared_memory = dev.shared_memory.queryValue()

        # Allocate via dictionary
        dict_props = {'load_average': 1.25,
                      'memory_allocation': {'contiguous': False,
                                            'capacity': 1024,
                                            'memory_type': 'SHARED'} }
        self.assertTrue(dev.allocateCapacity(dict_props))
        self.assertEqual(dev.load_average, load_average+dict_props['load_average'])
        self.assertEqual(dev.shared_memory, shared_memory-dict_props['memory_allocation']['capacity'])

        dev.deallocateCapacity(dict_props)
        self.assertEqual(dev.load_average, load_average)
        self.assertEqual(dev.shared_memory, shared_memory)

        # Allocate with a list of CF.DataTypes
        cf_props = [self._propertyToDataType(dev, name, value) for name, value in dict_props.iteritems()]
        self.assertTrue(dev.allocateCapacity(cf_props))

        self.assertEqual(dev.load_average, load_average+dict_props['load_average'])
        self.assertEqual(dev.shared_memory, shared_memory-dict_props['memory_allocation']['capacity'])

        dev.deallocateCapacity(cf_props)
        self.assertEqual(dev.load_average, load_average)
        self.assertEqual(dev.shared_memory, shared_memory)

    def _propertyToDataType(self, comp, name, value):
        prop = getattr(comp, name)
        return CF.DataType(prop.id, prop.toAny(value))

    def test_Services(self):
        """
        Tests for the ability to launch and interact with services from the
        sandbox.
        """
        # Launch first instance by path to SPD
        spd = os.path.join(sb.getSDRROOT(), 'dev/services/BasicService/BasicService.spd.xml')
        service1 = sb.launch(spd)
        service1_name = service1._instanceName
        service1_id = service1._refid

        # Launch second instance by ID (added in 1.10)
        service2 = sb.launch('BasicService', execparams={'PARAM4':True,'PARAM5':'Message'})
        service2_name = service2._instanceName
        service2_id = service2._refid

        # Check that the generated service names and IDs are unique
        self.assertNotEqual(service1_name, service2_name)
        self.assertNotEqual(service1_id, service2_id)

        # Try the CORBA API (BasicService is a property set)
        props = service1.query([])

        # Check that the execparams actually got overridden
        props = dict((str(prop.id), prop.value._v) for prop in service2.query([]))
        self.assertEqual(props['PARAM4'], True)
        self.assertEqual(props['PARAM5'], 'Message')

        # Make sure you can retrieve them by name
        serviceA = sb.getService(service1_name)
        serviceB = sb.getService(service2_name)

        # Make sure we got the same ones back
        self.assertEqual(service1, serviceA)
        self.assertEqual(service2, serviceB)

    def test_ComplexListConversions(self):
        # Test interleaved-to-complex
        inData = range(4)
        outData = _bulkio_helpers.bulkioComplexToPythonComplexList(inData)
        self.assertEqual(outData,[complex(0,1),complex(2,3)])

        # Test complex-to-interleaved
        cxData = [complex(x+0.5,0) for x in xrange(4)]
        outData = _bulkio_helpers.pythonComplexListToBulkioComplex(cxData)
        self.assertEqual(outData, [0.5,0.0,1.5,0.0,2.5,0.0,3.5,0.0])

        # Ensure that conversion does not modify the original list
        self.assertTrue(isinstance(cxData[0],complex))

        # Test inline type conversion (should truncate)
        outDataInt = _bulkio_helpers.pythonComplexListToBulkioComplex(cxData, int)
        self.assertEqual(outDataInt[0], 0)
        self.assertEqual(outDataInt, [int(x) for x in outData])
    
    def test_apiBeforeLaunch(self):
        try:
            sb.api("TestCppProps")
            sb.api("SimpleDevice")
            sb.api("BasicService_java")
        except:
            self.fail("sb.api(<objectName>) failure")


class BulkioTest(unittest.TestCase):
    XMLDATA = """<body>
  <element tag=value/>
</body>"""

    TEMPFILE = 'testout.xml'

    def setUp(self):
        try:
            import bulkio
        except ImportError:
            raise ImportError('BULKIO is required for this test')

    def tearDown(self):
        try:
            os.unlink(self.TEMPFILE)
        except:
            pass

    def readFile(self, filename, strip=False):
        infile = open(filename, 'r')
        try:
            data = infile.read()
            if strip:
                data = data.strip()
            return data
        finally:
            infile.close()

    def _pushSRIThroughSourceAndSink(
        self,
        EOS          = True,
        dataFormat   = "float",
        streamID     = "testStreamID",
        sampleRate   = 2.0,
        delay        = 0.1,
        SRIKeywords  = []):
        # Note that the default parameters should be different
        # than the defaults in the DataSource/Sink to verify
        # that we are not just getting DataSource/Sink defaults.

        source, sink = _initSourceAndSink(dataFormat)

        data = range(10)
        source.push(
            data,
            EOS         = EOS,
            streamID    = streamID,
            sampleRate  = sampleRate,
            SRIKeywords = SRIKeywords)

        # give the data sink time to buffer the pushed data
        time.sleep(delay)

        returnedSRI = sink.sri()
        self.assertEquals(sink.eos(), EOS)
        self.assertEquals(returnedSRI.streamID, streamID)
        self.assertEquals(returnedSRI.xdelta, 1./sampleRate)
        self.assertEquals(returnedSRI.keywords, SRIKeywords)

    def _pushDataThroughSourceAndSink(
        self,
        data,
        dataFormat  = "float",
        delay       = 0.1,
        complexData = False):
        """
        Push data from DataSource to DataSink and verify that the data out
        of the DataSink is identical to the data being sent into the
        DataSource.
        """
        source, sink = _initSourceAndSink(dataFormat)

        originalData = copy.deepcopy(data)
        source.push(
            data,
            complexData = complexData)

        # give the data sink time to buffer the pushed data
        time.sleep(delay)
        receivedData = sink.getData()

        # Detect if we were passed a data list that had python complex types
        # in it
        def isComplex(x) : return type(x) == type(complex())
        if len(filter(isComplex, originalData)):
            # in this case, the DataSink will return bulkio complex data
            # convert the originalData to the bulkio data format.
            originalData = _bulkio_helpers.pythonComplexListToBulkioComplex(originalData)

            # make sure the mode flag was automatically set
            self.assertEquals(sink.sri().mode, True)

        self.assertEquals(receivedData, originalData)

    def test_DataSourceWithFormatConnect(self):
        src = sb.DataSource(dataFormat='short')
        sink = sb.DataSink()
        try:
            src.connect(sink)
        except Exception, e:
            self.fail('Automatic connect failed for source that had dataFormat passed in')
      
    def test_DataSourceAndSink(self):
        dummySource = sb.DataSource()
        supportedPorts = dummySource.supportedPorts

        self._pushSRIThroughSourceAndSink()

        scalarDataList  = range(8)
        complexDataList = [0, 1, 2+2j]

        for format in supportedPorts.keys():
            if format == "file":
                # TODO: add test for file
                continue
            elif format == "xml":
                # TODO: add test for xml
                continue
            elif format == "char":
                # TODO: add test for char
                continue
            elif format == "octet":
                # TODO: add test for char
                continue
            else:
                # test scalar data
                dataCopy = copy.deepcopy(scalarDataList)
                self._pushDataThroughSourceAndSink(
                    data         = dataCopy,
                    dataFormat   = format)

                # test complex data (both methods)
                dataCopy = copy.deepcopy(scalarDataList)
                self._pushDataThroughSourceAndSink(
                    data         = dataCopy,
                    dataFormat   = format,
                    complexData  = True)
                dataCopy = copy.deepcopy(complexDataList)
                self._pushDataThroughSourceAndSink(
                    data         = dataCopy,
                    dataFormat   = format)

    def test_XMLDataSource(self):
        source = sb.DataSource(dataFormat='xml')
        datasink = sb.DataSink()
        source.connect(datasink)
        sb.start()

        source.push(self.XMLDATA, EOS=True)

        # Retrieve the data
        data = datasink.getData(eos_block=True)
        self.assertEqual(len(data), 1)
        data = data[0]
        self.assertEqual(self.XMLDATA, data)

    def test_XMLDataSink(self):
        """
        Test DataSink with XML data.
        """
        sink = sb.DataSink()
        sb.start()

        # Push directly into the port to test without a source
        port = sink.getPort('xmlIn')
        port.pushPacket(self.XMLDATA, True, 'xml')

        # Retrieve the data
        data = sink.getData(eos_block=True)
        self.assertEqual(len(data), 1)
        data = data[0]
        self.assertEqual(self.XMLDATA, data)

    def test_XMLFileSink(self):
        sink = sb.FileSink(self.TEMPFILE)
        sb.start()

        # Push directly into the port to test without a source
        port = sink.getPort('xmlIn')
        port.pushPacket(self.XMLDATA, True, 'xml')

        sink.waitForEOS()

        # Compare the output against the original data
        data = self.readFile(self.TEMPFILE, strip=True)
        self.assertEqual(self.XMLDATA, data)

    def test_XMLFileSource(self):
        infile = os.path.join(sb.getSDRROOT(), 'dom/mgr/DomainManager.spd.xml')

        source = sb.FileSource(infile, dataFormat='xml')
        sink = sb.FileSink(self.TEMPFILE)
        source.connect(sink)
        sb.start()

        sink.waitForEOS()
        
        # Check that the input and output files match
        xmldata = self.readFile(infile)
        data = self.readFile(self.TEMPFILE)
        self.assertEqual(xmldata, data)


        #TODO if BULKIO ever gets folded into core framework these tests can be used
        # to add them proper components must be created
        # 1 with multiple good connections
        # 1 with no good connections
        # 1 with a good connection

#    def test_connections(self):
#        a = sb.launch(self.test_comp)
#        b = sb.launch("SandBoxTest2")
#        no_connections = sb.launch("SimpleComponent")
#        multiple_connections = sb.launch("SandBoxTestMultipleConnections")
#
#        names = [a._componentName,              \
#                 b._componentName,              \
#                 no_connections._componentName, \
#                 multiple_connections._componentName]
#
#        # Poor args
#        self.assertRaises(AssertionError, a.connect, b, "")
#        self.assertRaises(AssertionError, a.connect, b, None, "")
#        self.assertRaises(AssertionError, a.connect, b, "", "")
#        self.assertRaises(AssertionError, a.connect, b, "long_in", "")
#        self.assertRaises(AssertionError, a.connect, b, "", "long_out")
#        self.assertRaises(AssertionError, a.connect, no_connections)
#
#        # Good connections
#        self.assertEquals(a.connect(b), True)
#        self.assertEquals(a.connect(b, "long_in"), True)
#        self.assertEquals(a.connect(b, None, "long_out"), True)
#        self.assertEquals(len(sb.domainless._currentState['Component Connections'].keys()), 3)
#
#        # Disconnect
#        a.disconnect(no_connections)
#        a.disconnect(None)
#        self.assertEquals(len(sb.domainless._currentStatep['Component Connections'].keys()), 3)
#        a.disconnect(b)
#        self.assertEquals(len(sb.domainless._currentState['Component Connections'].keys()), 0)
#
#        # Makes sure that connection keys are unique
#        self.assertEquals(a.connect(b, "long_in", "long_out", "my_conn"), True)
#        self.assertEquals(len(sb.domainless._currentState['Component Connections'].keys()), 1)
#        self.assertRaises(AssertionError, a.connect, b, "long_in", "long_out", "my_conn")
#        self.assertEquals(len(sb.domainless._currentState['Component Connections'].keys()), 1)
#
#        a.disconnect(b)
#
#        # Multiple good connections
#        self.assertRaises(AssertionError, a.connect, multiple_connections)
#
#        #### TEST for a.connectd diff ports BUG
#        ###TODO this test will fail until Issue #149 is fixed
#        #self.assertRaises(AssertionError, a.connect, multiple_connections, "long_in", "short_out")
#
#        # Tests getComponents()
#        self.assertEquals(sb.getComponent(None), None)
#        self.assertEquals(sb.getComponent(""), None)
#        for key in sb.domainless._currentState['Components Running'].keys():
#            temp = sb.getComponent(key)
#            self.assertNotEquals(temp, None)
#            self.assertEquals(temp._componentName in names, True)



