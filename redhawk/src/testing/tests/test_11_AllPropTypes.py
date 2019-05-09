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

from omniORB import any
import unittest
from _unitTestHelpers import scatest
from ossie.cf import CF, CF__POA
from ossie.utils import redhawk
from omniORB import CORBA
from ossie.utils import sb, rhtime, redhawk
from omniORB import tcInternal as _tcInternal
from ossie.cf import PortTypes as _PortTypes
import struct, time, os

globalsdrRoot = os.environ['SDRROOT']

class PropertyChangeListener_Receiver(CF__POA.PropertyChangeListener):
    def __init__(self):
        self.rcv_event = None

    def propertyChange( self, pce ) :
        self.rcv_event = pce

class TimeTest(scatest.CorbaTestCase):
    def setUp(self):
        sb.setDEBUG(False)
        self.test_comp = "Sandbox"
        # Flagrant violation of sandbox API: if the sandbox singleton exists,
        # clean up previous state and dispose of it.
        if sb.domainless._sandbox:
            sb.domainless._sandbox.shutdown()
            sb.domainless._sandbox = None

    def tearDown(self):
        sb.release()
        sb.setDEBUG(False)
        os.environ['SDRROOT'] = globalsdrRoot
    
    def basetest_getTime(self, comp_name):
        comp = sb.launch(comp_name)
        _prop=CF.DataType(id='prop',value=any.to_any(None))
        _retval = comp.query([_prop])
        self.assertEqual(_retval[0].value._v, 'value')
        self.assertEqual(len(_retval), 1)
        
        _retval = comp.query([])
        self.assertEqual(_retval[0].value._v, 'value')
        self.assertEqual(len(_retval), 1)
        
        _retval = comp.query([_prop, rhtime.queryTimestamp()])
        self.assertEqual(_retval[0].value._v, 'value')
        self.assertEqual(len(_retval), 2)
        
        myl = PropertyChangeListener_Receiver()
        t=float(0.5)
        regid=comp.registerPropertyListener( myl._this(), ['prop'],t)
        
        comp.prop = 'hello'
        time.sleep(1)
        
        _retval = comp.query([_prop])
        self.assertEqual(_retval[0].value._v, 'hello')
        self.assertEqual(myl.rcv_event.properties[0].value._v, 'hello')
        
        _retval = comp.query([rhtime.queryTimestamp()])
        self.assertEqual(len(_retval), 1)
        self.assertEqual(_retval[0].value._v.tcstatus, 1)
        _time1 = myl.rcv_event.timestamp.twsec + myl.rcv_event.timestamp.tfsec
        _time2 = _retval[0].value._v.twsec + _retval[0].value._v.tfsec
        between = True
        if _time2 - _time1 < 0.25 or _time2 - _time1 > 0.75:
            between = False
        self.assertEqual(between, True)
        
    def test_getTimeCpp(self):
        self.basetest_getTime('timeprop_cpp')

    def test_getTimePython(self):
        self.basetest_getTime('timeprop_py')

    @scatest.requireJava
    def test_getTimeJava(self):
        self.basetest_getTime('timeprop_java')

class UTCTimeTestWaveform(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self._rhDom = redhawk.attach(scatest.getTestDomainName())
        self.assertEquals(len(self._rhDom._get_applications()), 0)

    def tearDown(self):
        if self._app:
            self._app.stop()
            self._app.releaseObject()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def basetest_Now(self, app_name):
        self.basetest_Overload(app_name)
        prop = CF.DataType(id='simple1970',value=any.to_any(None))
        retval = self._app.comps[0].ref.query([prop])
        self.assertEqual(retval[0].value._v.twsec, 0)
        self.assertEqual(retval[0].value._v.tfsec, 0)
        prop = CF.DataType(id='simpleSeqDefNow',value=any.to_any(None))
        retval = self._app.comps[0].ref.query([prop])
        self.assertEquals(len(retval[0].value._v), 1)
        self.assertEquals(retval[0].value._v[0].tcstatus, 1)
        self.assertNotEquals(retval[0].value._v[0].twsec, 0)
        self.assertNotEquals(retval[0].value._v[0].tfsec, 0)
        prop = CF.DataType(id='simpleSeqNoDef',value=any.to_any(None))
        retval = self._app.comps[0].ref.query([prop])
        self.assertEquals(len(retval[0].value._v), 0)
        prop = CF.DataType(id='simpleSeq1970',value=any.to_any(None))
        retval = self._app.comps[0].ref.query([prop])
        self.assertEquals(len(retval[0].value._v), 1)
        self.assertEquals(retval[0].value._v[0].tcstatus, 1)
        self.assertEquals(retval[0].value._v[0].twsec, 0)
        self.assertEquals(retval[0].value._v[0].tfsec, 0)

    def basetest_Overload(self, app_name):
        self._app = self._rhDom.createApplication("/waveforms/"+app_name+"/"+app_name+".sad.xml")
        self.assertNotEqual(self._app, None)
        cur_time = rhtime.now()
        app_time = self._app.comps[0].rightnow.queryValue()
        _cur_time = cur_time.twsec + cur_time.tfsec
        _app_time = app_time.twsec + app_time.tfsec
        self.assertTrue(abs(_cur_time-_app_time)<1, True)

    def test_nowOverload(self):
        self.basetest_Overload('newtime_w')

    def test_nowWaveCpp(self):
        self.basetest_Now('time_cp_now_w')

    def test_nowWavePython(self):
        self.basetest_Now('time_py_now_w')

    @scatest.requireJava
    def test_nowWaveJava(self):
        self.basetest_Now('time_ja_now_w')

class UTCTimeTestSandbox(scatest.CorbaTestCase):
    def setUp(self):
        sb.setDEBUG(False)
        # Flagrant violation of sandbox API: if the sandbox singleton exists,
        # clean up previous state and dispose of it.
        if sb.domainless._sandbox:
            sb.domainless._sandbox.shutdown()
            sb.domainless._sandbox = None

    def tearDown(self):
        sb.release()
        sb.setDEBUG(False)
        os.environ['SDRROOT'] = globalsdrRoot

    def basetest_Now(self, comp_name):
        comp = sb.launch(comp_name)
        self.assertNotEqual(comp, None)
        cur_time = rhtime.now()
        comp_time = comp.rightnow.queryValue()
        _cur_time = cur_time.twsec + cur_time.tfsec
        _comp_time = comp_time.twsec + comp_time.tfsec
        self.assertTrue(abs(_cur_time-_comp_time)<1, True)
        prop = CF.DataType(id='simple1970',value=any.to_any(None))
        retval = comp.ref.query([prop])
        self.assertEqual(retval[0].value._v.twsec, 0)
        self.assertEqual(retval[0].value._v.tfsec, 0)

    def test_nowSbCpp(self):
        self.basetest_Now('time_cp_now')

    def test_nowSbPython(self):
        self.basetest_Now('time_py_now')

    @scatest.requireJava
    def test_nowSbJava(self):
        self.basetest_Now('time_ja_now')

class TestAllTypes(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self._app = None

    def tearDown(self):
        if self._app:
            self._app.stop()
            self._app.releaseObject()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def preconditions(self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")
        self.assertNotEqual(self._app, None, "Application not created")

    def launchApplication(self, language):
        if self._domMgr:
            try:
                sadpath = "/waveforms/Test"+language+"Props/"+language+"AllPropTypes.sad.xml"
                self._domMgr.installApplication(sadpath)
                # Gets the proper appFact
                for curr in self._domMgr._get_applicationFactories():
                    if curr._get_name().find(language) != -1:
                        appFact = curr
                self._app = appFact.create(appFact._get_name(), [], [])
            except:
                pass

    def _test_AllPropTypes(self, lang):
        self.launchApplication(lang)
        self.preconditions()
        res = self._app.query([])
        for r in res:
            self._app.configure([r])

    def test_AllPropTypesCpp(self):
        self._test_AllPropTypes('Cpp')

    def test_AllPropTypesPython(self):
        self._test_AllPropTypes('Python')

    @scatest.requireJava
    def test_AllPropTypesJava(self):
        self._test_AllPropTypes('Java')

    def checkValue(self, results, prop_id, value, same=True):
        for r in results:
            if r.id == prop_id:
                if prop_id == 'struct_vars':
                    if same:
                        self.assertEqual(r.value.value()[0].value.value(), value)
                    else:
                        self.assertNotEqual(r.value.value()[0].value.value(), value)
                elif prop_id == 'struct_seq':
                    found_id = False
                    for curr in r.value.value()[0].value():
                        if curr.id == 'struct_seq_string':
                            found_id = True
                            if same:
                                self.assertEqual(curr.value.value(), value)
                            else:
                                self.assertNotEqual(curr.value.value(), value)
                    self.assertEqual(found_id, True)
                elif prop_id.find('simple_sequence') != -1:
                    if same:
                        self.assertEqual(r.value.value()[0], value)
                    else:
                        self.assertNotEqual(r.value.value()[0], value)
                else:
                    if same:
                        self.assertEqual(r.value.value(), value)
                    else:
                        self.assertNotEqual(r.value.value(), value)

    def test_deploytime_sequenceExt(self):
        dom=redhawk.attach(self._domMgr._get_name())
        sadpath = "/waveforms/testWave/testWave.sad.xml"
        props3 = [CF.DataType(id='prop', value=any.to_any(['a']))]
        _app = dom.createApplication(sadpath, 'appname', props3)
        self.assertEqual(len(_app.prop), 1)
        self.assertEqual(_app.prop[0], 'a')

    def test_deploytime_sequenceDirect(self):
        dom=redhawk.attach(self._domMgr._get_name())
        sadpath = "/waveforms/testWave/testWave.sad.xml"
        props3 = [CF.DataType(id='a', value=any.to_any(['a']))]
        _app = dom.createApplication(sadpath, 'appname', props3)
        self.assertEqual(len(_app.prop), 1)
        self.assertEqual(_app.prop[0], 'a')

    def test_AllPropTypeCallbacks(self):
        languages = ['Cpp', 'Python']
        for lang in languages:
            self.launchApplication(lang)
            self.preconditions()

            res = self._app.query([CF.DataType(id='simple_string', value=any.to_any(None))])
            self._app.configure(res)
            res = self._app.query([])
            self.checkValue(res, 'simple_string', '42', same=False)

            res = self._app.query([])
            for r in res:
                if r.value._t == CORBA.TC_null:
                    if r.id == 'simple_boolean':
                        r.value = any.to_any(False)
                    elif r.id == 'simple_char':
                        r.value = any.to_any('o')
                        r.value._t = CORBA.TC_char
                    elif r.id == 'simple_string':
                        r.value = any.to_any('foo')
                        r.value._t = CORBA.TC_string
                    elif r.id == 'simple_double':
                        r.value = any.to_any(1.0)
                        r.value._t = CORBA.TC_double
                    elif r.id == 'simple_float':
                        r.value = any.to_any(1.0)
                        r.value._t = CORBA.TC_float
                    elif r.id == 'simple_short':
                        r.value = any.to_any(1)
                        r.value._t = CORBA.TC_short
                    elif r.id == 'simple_ushort':
                        r.value = any.to_any(1)
                        r.value._t = CORBA.TC_ushort
                    elif r.id == 'simple_long':
                        r.value = any.to_any(1)
                        r.value._t = CORBA.TC_long
                    elif r.id == 'simple_longlong':
                        r.value = any.to_any(1)
                        r.value._t = CORBA.TC_longlong
                    elif r.id == 'simple_ulong':
                        r.value = any.to_any(1)
                        r.value._t = CORBA.TC_ulong
                    elif r.id == 'simple_ulonglong':
                        r.value = any.to_any(1)
                        r.value._t = CORBA.TC_ulonglong
                    elif r.id == 'simple_objref':
                        r.value = any.to_any('o')
                        r.value._t = CORBA.TC_string
                    elif r.id == 'simple_octet':
                        r.value = any.to_any(1)
                        r.value._t = CORBA.TC_octet
                    elif r.id == 'simple_sequence_boolean':
                        r.value = any.to_any([])
                        r.value._t = CORBA.TypeCode("IDL:omg.org/CORBA/BooleanSeq:1.0")
                    elif r.id == 'simple_sequence_char':
                        r.value = any.to_any('')
                        r.value._t = CORBA.TypeCode("IDL:omg.org/CORBA/CharSeq:1.0")
                    elif r.id == 'simple_sequence_double':
                        r.value = any.to_any([])
                        r.value._t = CORBA.TypeCode("IDL:omg.org/CORBA/DoubleSeq:1.0")
                    elif r.id == 'simple_sequence_float':
                        r.value = any.to_any([])
                        r.value._t = CORBA.TypeCode("IDL:omg.org/CORBA/FloatSeq:1.0")
                    elif r.id == 'simple_sequence_long':
                        r.value = any.to_any([])
                        r.value._t = CORBA.TypeCode("IDL:omg.org/CORBA/LongSeq:1.0")
                    elif r.id == 'simple_sequence_longlong':
                        r.value = any.to_any([])
                        r.value._t = _tcInternal.typeCodeFromClassOrRepoId(_PortTypes.LongLongSequence)
                    elif r.id == 'simple_sequence_string':
                        r.value = any.to_any([])
                        r.value._t = CORBA.TypeCode("IDL:omg.org/CORBA/StringSeq:1.0")
                    elif r.id == 'simple_sequence_objref':
                        r.value = any.to_any([])
                        r.value._t = CORBA.TypeCode("IDL:omg.org/CORBA/StringSeq:1.0")
                    elif r.id == 'simple_sequence_octet':
                        r.value = any.to_any('')
                        r.value._t = CORBA.TypeCode("IDL:omg.org/CORBA/OctetSeq:1.0")
                    elif r.id == 'simple_sequence_short':
                        r.value = any.to_any([])
                        r.value._t = _tcInternal.typeCodeFromClassOrRepoId(CORBA.UShortSeq)
                    elif r.id == 'simple_sequence_ulong':
                        r.value = any.to_any([])
                        r.value._t = CORBA.TypeCode("IDL:omg.org/CORBA/ULongSeq:1.0")
                    elif r.id == 'simple_sequence_ulonglong':
                        r.value = any.to_any([])
                        r.value._t = _tcInternal.typeCodeFromClassOrRepoId(_PortTypes.UlongLongSequence)
                    elif r.id == 'simple_sequence_ushort':
                        r.value = any.to_any([])
                        r.value._t = CORBA.TypeCode("IDL:omg.org/CORBA/UShortSeq:1.0")
                if r.id == 'struct_vars':
                    for item in r.value._v:
                        if item.value._t != CORBA.TC_null:
                            continue
                        if item.id == 'struct_string':
                            item.value = any.to_any('foo')
                            item.value._t = CORBA.TC_string
                        elif item.id == 'struct_boolean':
                            item.value = any.to_any(False)
                        elif item.id == 'struct_ulong':
                            item.value = any.to_any(1)
                            item.value._t = CORBA.TC_ulong
                        elif item.id == 'struct_objref':
                            item.value = any.to_any('o')
                            item.value._t = CORBA.TC_string
                        elif item.id == 'struct_short':
                            item.value = any.to_any(1)
                            item.value._t = CORBA.TC_short
                        elif item.id == 'struct_float':
                            item.value = any.to_any(1.0)
                            item.value._t = CORBA.TC_float
                        elif item.id == 'struct_octet':
                            item.value = any.to_any(1)
                            item.value._t = CORBA.TC_octet
                        elif item.id == 'struct_char':
                            item.value = any.to_any('o')
                            item.value._t = CORBA.TC_char
                        elif item.id == 'struct_ushort':
                            item.value = any.to_any(1)
                            item.value._t = CORBA.TC_ushort
                        elif item.id == 'struct_double':
                            item.value = any.to_any(1.0)
                            item.value._t = CORBA.TC_double
                        elif item.id == 'struct_long':
                            item.value = any.to_any(1)
                            item.value._t = CORBA.TC_long
                        elif item.id == 'struct_longlong':
                            item.value = any.to_any(1)
                            item.value._t = CORBA.TC_longlong
                        elif item.id == 'struct_ulonglong':
                            item.value = any.to_any(1)
                            item.value._t = CORBA.TC_ulonglong
                if type(r.value._v) == int or type(r.value._v) == long or type(r.value._v) == float:
                    r.value._v = r.value._v + 1
                elif r.value._t == CORBA.TC_char:
                    r.value._v = 'o'
                elif type(r.value._v) == str:
                    r.value._v = 'foo'
                elif type(r.value._v) == bool:
                    r.value._v = False
                elif type(r.value._v) == list:
                    if r.id == 'simple_sequence_string':
                        r.value._v = ['foo']
                    elif r.id == 'simple_sequence_boolean':
                        r.value._v = [False]
                    elif r.id == 'simple_sequence_ulong':
                        r.value._v = [1]
                    elif r.id == 'simple_sequence_short':
                        r.value._v = [1]
                    elif r.id == 'simple_sequence_float':
                        r.value._v = [1.0]
                    elif r.id == 'simple_sequence_ushort':
                        r.value._v = [1]
                    elif r.id == 'simple_sequence_double':
                        r.value._v = [1.0]
                    elif r.id == 'simple_sequence_long':
                        r.value._v = [1]
                    elif r.id == 'simple_sequence_longlong':
                        r.value._v = [1]
                    elif r.id == 'simple_sequence_ulonglong':
                        r.value._v = [1]

                if r.id == 'struct_seq':
                    if r.value._v == []:
                        seq_vars = CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), [
                            CF.DataType(id='struct_seq_string', value=CORBA.Any(CORBA.TC_string, 'foo')),
                            CF.DataType(id='struct_seq_boolean', value=CORBA.Any(CORBA.TC_boolean, False)),
                            CF.DataType(id='struct_seq_ulong', value=CORBA.Any(CORBA.TC_ulong, 1)),
                            CF.DataType(id='struct_seq_objref', value=CORBA.Any(CORBA.TC_string, 'o')),
                            CF.DataType(id='struct_seq_short', value=CORBA.Any(CORBA.TC_short, 1)),
                            CF.DataType(id='struct_seq_float', value=CORBA.Any(CORBA.TC_float, 1.0)),
                            CF.DataType(id='struct_seq_octet', value=CORBA.Any(CORBA.TC_octet, 1)),
                            CF.DataType(id='struct_seq_char', value=CORBA.Any(CORBA.TC_char, 'o')),
                            CF.DataType(id='struct_seq_ushort', value=CORBA.Any(CORBA.TC_ushort, 1)),
                            CF.DataType(id='struct_seq_double', value=CORBA.Any(CORBA.TC_double, 1.0)),
                            CF.DataType(id='struct_seq_long', value=CORBA.Any(CORBA.TC_long, 1)),
                            CF.DataType(id='struct_seq_longlong', value=CORBA.Any(CORBA.TC_longlong, 1)),
                            CF.DataType(id='struct_seq_ulonglong', value=CORBA.Any(CORBA.TC_ulonglong, 1)),])
                        r = CF.DataType(id='struct_seq', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/AnySeq:1.0"), [any.to_any(seq_vars)]))
                self._app.configure([r])
            res = self._app.query([])

            self.checkValue(res, 'simple_string', '42')
            self.checkValue(res, 'simple_boolean', True)
            self.checkValue(res, 'simple_ulong', 43)
            self.checkValue(res, 'simple_objref', '44')
            self.checkValue(res, 'simple_short', 45)
            self.checkValue(res, 'simple_float', 46.0)
            self.checkValue(res, 'simple_octet', 47)
            self.checkValue(res, 'simple_char', struct.pack('b', 48))
            self.checkValue(res, 'simple_ushort', 49)
            self.checkValue(res, 'simple_double', 50.0)
            self.checkValue(res, 'simple_long', 51)
            self.checkValue(res, 'simple_longlong', 52)
            self.checkValue(res, 'simple_ulonglong', 53)
            self.checkValue(res, 'simple_sequence_string', '54')
            self.checkValue(res, 'simple_sequence_boolean', True)
            self.checkValue(res, 'simple_sequence_ulong', 55)
#            self.checkValue(res, 'simple_sequence_objref', '56')   Broken in python
            self.checkValue(res, 'simple_sequence_short', 57)
            self.checkValue(res, 'simple_sequence_float', 58)
#            self.checkValue(res, 'simple_sequence_octet', struct.pack('B', 59))    Broken in python
#            self.checkValue(res, 'simple_sequence_char', struct.pack('b', 60))     Borken in python
            self.checkValue(res, 'simple_sequence_ushort', 61)
            self.checkValue(res, 'simple_sequence_double', 62)
            self.checkValue(res, 'simple_sequence_long', 63)
            self.checkValue(res, 'simple_sequence_longlong', 64)
            self.checkValue(res, 'simple_sequence_ulonglong', 65)
            self.checkValue(res, 'struct_vars', '66')
            self.checkValue(res, 'struct_seq', '67')

    def test_appQueryWriteonly(self):
        dom=redhawk.attach(self._domMgr._get_name())
        sadpath = "/waveforms/comp_writeonly_external/comp_writeonly_external.sad.xml"
        _app = dom.createApplication(sadpath, 'appname')
        self.assertNotEqual(_app, None)
        retval = _app.query([])

