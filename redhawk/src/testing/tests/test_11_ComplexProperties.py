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
from _unitTestHelpers import scatest
from ossie.cf import CF
from ossie.utils import sb
from omniORB import CORBA
import numpy
from _unitTestHelpers import runtestHelpers

java_support = runtestHelpers.haveJavaSupport('../Makefile')

class _DataTypeTest:
    def __init__(self, id, default, override, typecode):
        self.id = id
        self.default = default
        self.override = override
        self.typecode = typecode

def _compareComplex(parent, item_1, item_2):
    parent.assertEquals(item_1.real, item_2.real)
    parent.assertEquals(item_1.imag, item_2.imag)

def _compareStructs(parent, struct_1, struct_2):
    for _struct_mem_idx in range(len(struct_2)):
        if type(struct_2[_struct_mem_idx]) == list:
            for idx_val in range(len(struct_2[_struct_mem_idx])):
                _compareComplex(parent, struct_1[_struct_mem_idx].value._v[idx_val], struct_2[_struct_mem_idx][idx_val])
        else:
            _compareComplex(parent, struct_1[_struct_mem_idx].value._v, struct_2[_struct_mem_idx])

def _compareComplexValues(parent, val1, val2):
    if type(val2) == list:
        if len(val2) != 0:
            if type(val2[0]) == tuple: # sequence of structs
                for _struct_seq_idx in range(len(val2)):
                    _compareStructs(parent, val1[_struct_seq_idx]._v, val2[_struct_seq_idx])
            else:
                for idx_val in range(len(val1)):
                    _compareComplex(parent, val1[idx_val], val2[idx_val])
    elif type(val2) == tuple:
        _compareStructs(parent, val1, val2)
    else:
        _compareComplex(parent, val1, val2)

class _TestVector:

    def test_complexOverrides(self):
        testStruct = [(_DataTypeTest("complexBooleanProp", CF.complexBoolean(False, True), CF.complexBoolean(True, False), CF._tc_complexBoolean)),
                      (_DataTypeTest("complexULongProp", CF.complexULong(4, 5), CF.complexULong(2, 3), CF._tc_complexULong)),
                      (_DataTypeTest("complexShortProp", CF.complexShort(4,5), CF.complexShort(2,3), CF._tc_complexShort)),
                      (_DataTypeTest("complexFloatProp", CF.complexFloat(4.0, 5.0), CF.complexFloat(2.0, 3.0), CF._tc_complexFloat)),
                      (_DataTypeTest("complexOctetProp", CF.complexOctet(4, 5), CF.complexOctet(2, 3), CF._tc_complexOctet)),
                      (_DataTypeTest("complexUShort", CF.complexUShort(4, 5), CF.complexUShort(2, 3), CF._tc_complexUShort)),
                      (_DataTypeTest("complexDouble", CF.complexDouble(4.0, 5.0), CF.complexDouble(2.0, 3.0), CF._tc_complexDouble)),
                      (_DataTypeTest("complexLong", CF.complexLong(4, 5), CF.complexLong(2, 3), CF._tc_complexLong)),
                      (_DataTypeTest("complexLongLong", CF.complexLongLong(4, 5), CF.complexLongLong(2, 3), CF._tc_complexLongLong)),
                      (_DataTypeTest("complexULongLong", CF.complexULongLong(4, 5), CF.complexULongLong(2, 3), CF._tc_complexULongLong)),
                      (_DataTypeTest("complexFloatSequence", [CF.complexFloat(6, 7), CF.complexFloat(4, 5), CF.complexFloat(8, 9)], [CF.complexFloat(1, 2), CF.complexFloat(10, 20)], None)),
                      (_DataTypeTest("complexFloatStruct", (CF.complexFloat(6, 7), [CF.complexFloat(3, 4)]), (CF.complexFloat(6, 7), [CF.complexFloat(-5, 5), CF.complexFloat(9, -8), CF.complexFloat(-13, -24), CF.complexFloat(21, -22), CF.complexFloat(31, 0), CF.complexFloat(0, 431), CF.complexFloat(0, -567), CF.complexFloat(-3567, 0), CF.complexFloat(-5.25, 5.25), CF.complexFloat(9.25, -8.25)]), ['complexFloatStructMember', 'complexFloatStruct::complex_float_seq'])),
                      (_DataTypeTest("complexFloatStructSequence", [(CF.complexFloat(9, 4), [CF.complexFloat(6, 5)])], [(CF.complexFloat(32, 33), [CF.complexFloat(45, 55), CF.complexFloat(69, 78)]), (CF.complexFloat(42, 43), [CF.complexFloat(145, 155), CF.complexFloat(169, 178), CF.complexFloat(279, 998)])], ['complexFloatStructSequenceMemberMemember', 'complexFloatStructSequence::complex_float_seq']))]

        self._runTest(testStruct)

class SetupCommon:
    def setUp_(self, sadpath):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self._app = None
        if self._domMgr:
            try:
                self._domMgr.installApplication(sadpath)
                appFact = self._domMgr._get_applicationFactories()[0]
                self._app = appFact.create(appFact._get_name(), [], [])
            except:
                pass
        self.assertNotEqual(self._app, None)

    def tearDown_(self):
        if self._app:
            self._app.stop()
            self._app.releaseObject()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def preconditions_(self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")
        self.assertNotEqual(self._app, None, "Application not created")


class CppPropertiesSADOverridesLaunchTest(scatest.CorbaTestCase, _TestVector, SetupCommon):
    def setUp(self):
        SetupCommon.setUp_(self, sadpath = "/waveforms/TestComplexPropsSADOverrides/TestComplexPropsSADOverrides.sad.xml")

    def tearDown(self):
        SetupCommon.tearDown_(self)

    def preconditions(self):
        SetupCommon.preconditions_(self)

    def _runTest(self, dataTypeTests):
        for dataTypeTest in dataTypeTests:
            prop = CF.DataType(id = dataTypeTest.id, value = any.to_any(None))
            defaultProps = self._app.query([prop])
            _compareComplexValues(self, defaultProps[0].value.value(), dataTypeTest.override)

class CppPropertiesSADConfigureTest(scatest.CorbaTestCase, _TestVector, SetupCommon):
    def setUp(self):
        SetupCommon.setUp_(self, sadpath = "/waveforms/TestComplexPropsWaveform/TestComplexPropsWaveform.sad.xml")

    def tearDown(self):
        SetupCommon.tearDown_(self)

    def preconditions(self):
        SetupCommon.preconditions_(self)

    def _runTest(self, dataTypeTests):
        '''
        1.  Check the default value of the property via the query method.
        2.  Configure the property with an override value.
        3.  Query the property to make sure the override value has been set.

        '''
        # Create a property structure
        for dataTypeTest in dataTypeTests:
            if dataTypeTest.typecode == None:
                _anyvalue = any.to_any(dataTypeTest.override)
            elif type(dataTypeTest.typecode) == list:
                if type(dataTypeTest.override) == tuple:
                    _val = []
                    for name_idx in range(len(dataTypeTest.typecode)):
                        _val.append(CF.DataType(id=dataTypeTest.typecode[name_idx], value=any.to_any(dataTypeTest.override[name_idx])))
                    _anyvalue = any.to_any(_val)
                elif type(dataTypeTest.override) == list:
                    _val = []
                    for _override in dataTypeTest.override:
                        _inner_val = []
                        for name_idx in range(len(dataTypeTest.typecode)):
                            _inner_val.append(CF.DataType(id=dataTypeTest.typecode[name_idx], value=any.to_any(_override[name_idx])))
                        _val.append(any.to_any(_inner_val))
                    _anyvalue = any.to_any(_val)
                else:
                    a=b
            else:
                _anyvalue = CORBA.Any(dataTypeTest.typecode, dataTypeTest.override)
            query_prop = CF.DataType(id = dataTypeTest.id, value = any.to_any(None))
            configure_prop = CF.DataType(id = dataTypeTest.id, value = _anyvalue)

            # Check the default property value via query
            defaultProps = self._app.query([query_prop])
            _compareComplexValues(self, defaultProps[0].value.value(), dataTypeTest.default)

            # Call configure with the property with an override value
            # then check, via query, if the configuration worked
            self._app.configure([configure_prop])
            newProps = self._app.query([query_prop])
            _compareComplexValues(self, newProps[0].value.value(), dataTypeTest.override)

class CppPropertiesSADCreateTest(scatest.CorbaTestCase, _TestVector, SetupCommon):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")


    def tearDown(self):
        SetupCommon.tearDown_(self)

    def preconditions(self):
        SetupCommon.preconditions_(self)

    def _runTest(self, dataTypeTests):
        sadpath = "/waveforms/TestComplexPropsWaveform/TestComplexPropsWaveform.sad.xml"
        '''
        1.  Create the set of properties with the override values.
        2.  Create the application specifying the override values.
        3.  Query each property to make sure the override value has been set.

        '''
        initProps = []

        # Create a property structure with the override values for the create call
        for dataTypeTest in dataTypeTests:
            if dataTypeTest.typecode == None:
                _anyvalue = any.to_any(dataTypeTest.override)
            elif type(dataTypeTest.typecode) == list:
                if type(dataTypeTest.override) == tuple:
                    _val = []
                    for name_idx in range(len(dataTypeTest.typecode)):
                        _val.append(CF.DataType(id=dataTypeTest.typecode[name_idx], value=any.to_any(dataTypeTest.override[name_idx])))
                    _anyvalue = any.to_any(_val)
                elif type(dataTypeTest.override) == list:
                    _val = []
                    for _override in dataTypeTest.override:
                        _inner_val = []
                        for name_idx in range(len(dataTypeTest.typecode)):
                            _inner_val.append(CF.DataType(id=dataTypeTest.typecode[name_idx], value=any.to_any(_override[name_idx])))
                        _val.append(any.to_any(_inner_val))
                    _anyvalue = any.to_any(_val)
                else:
                    a=b
            else:
                _anyvalue = CORBA.Any(dataTypeTest.typecode, dataTypeTest.override)
            initProps.append(CF.DataType(id = dataTypeTest.id, value = _anyvalue))

        self._app = None
        if self._domMgr:
            try:
                self._domMgr.installApplication(sadpath)
                appFact = self._domMgr._get_applicationFactories()[0]
                self._app = appFact.create(appFact._get_name(), initProps, [])
            except:
                pass
        self.assertNotEqual(self._app, None)

        # Create a property structure
        for dataTypeTest in dataTypeTests:
            if dataTypeTest.typecode == None:
                _anyvalue = any.to_any(dataTypeTest.override)
            elif type(dataTypeTest.typecode) == list:
                if type(dataTypeTest.override) == tuple:
                    _val = []
                    for name_idx in range(len(dataTypeTest.typecode)):
                        _val.append(CF.DataType(id=dataTypeTest.typecode[name_idx], value=any.to_any(dataTypeTest.override[name_idx])))
                    _anyvalue = any.to_any(_val)
                elif type(dataTypeTest.override) == list:
                    _val = []
                    for _override in dataTypeTest.override:
                        _inner_val = []
                        for name_idx in range(len(dataTypeTest.typecode)):
                            _inner_val.append(CF.DataType(id=dataTypeTest.typecode[name_idx], value=any.to_any(_override[name_idx])))
                        _val.append(any.to_any(_inner_val))
                    _anyvalue = any.to_any(_val)
                else:
                    a=b
            else:
                _anyvalue = CORBA.Any(dataTypeTest.typecode, dataTypeTest.override)
            query_prop = CF.DataType(id = dataTypeTest.id, value = any.to_any(None))

            # Check the property value via query is the overriden property value, not the default value
            defaultProps = self._app.query([query_prop])
            _compareComplexValues(self, defaultProps[0].value.value(), dataTypeTest.override)

class _SandboxDataTypeTest:
    def __init__(self, _id, expected, typecode):
        self._id = _id
        self.expected = expected
        self.typecode = typecode

class SandboxTests(scatest.CorbaTestCase):
    def setUp(self):
        if sb.domainless._sandbox:
            sb.domainless._sandbox.shutdown()
            sb.domainless._sandbox = None

    def tearDown(self):
        sb.domainless._getSandbox().shutdown()

    def test_sandboxComplexProps(self):
        testSet = [(_SandboxDataTypeTest(["complexBooleanProp"], CF.complexBoolean(False, True), CF._tc_complexBoolean)),
                      (_SandboxDataTypeTest(["complexULongProp"], CF.complexULong(4, 5), CF._tc_complexULong)),
                      (_SandboxDataTypeTest(["complexShortProp"], CF.complexShort(4,5), CF._tc_complexShort)),
                      (_SandboxDataTypeTest(["complexFloatProp"], CF.complexFloat(4.0, 5.0), CF._tc_complexFloat)),
                      (_SandboxDataTypeTest(["complexOctetProp"], CF.complexOctet(4, 5), CF._tc_complexOctet)),
                      (_SandboxDataTypeTest(["complexUShort"], CF.complexUShort(4, 5), CF._tc_complexUShort)),
                      (_SandboxDataTypeTest(["complexDouble"], CF.complexDouble(4.0, 5.0), CF._tc_complexDouble)),
                      (_SandboxDataTypeTest(["complexLong"], CF.complexLong(4, 5), CF._tc_complexLong)),
                      (_SandboxDataTypeTest(["complexLongLong"], CF.complexLongLong(4, 5), CF._tc_complexLongLong)),
                      (_SandboxDataTypeTest(["complexULongLong"], CF.complexULongLong(4, 5), CF._tc_complexULongLong)),
                      (_SandboxDataTypeTest(["complexFloatSequence"], [CF.complexFloat(6, 7), CF.complexFloat(4, 5), CF.complexFloat(8, 9)], None)),
                      (_SandboxDataTypeTest(['complexFloatStruct', ['complexFloatStructMember', 'complexFloatStruct::complex_float_seq']], (CF.complexFloat(6, 7), [CF.complexFloat(3, 4)]), None)),
                      (_SandboxDataTypeTest(['complexFloatStructSequence', ['complexFloatStructSequenceMemberMemember', 'complexFloatStructSequence::complex_float_seq']], [(CF.complexFloat(9, 4), [CF.complexFloat(6, 5)])], None))]

        # Create an instance of the test component in all 3 languages
        components = {"cpp"   : sb.launch("TestComplexProps", impl="cpp"),
                      "python": sb.launch("TestComplexProps", impl="python")}
        if java_support:
            components["java"] = sb.launch("TestComplexProps", impl="java")

        sb.start()

        prop_idx = {}
        for idx in range(len(components['cpp']._properties)):
            prop_idx[components['cpp']._properties[idx].id] = idx
        for language in components.keys():
            for _test in testSet:
                _prop = components[language]._properties[prop_idx[_test._id[0]]]
                _value = _prop._queryValue().value()
                _compareComplexValues(self, _value, _test.expected)
            components[language].releaseObject()

    def test_complexLoadSADFile(self):
        testSet = [(_SandboxDataTypeTest(["complexBooleanProp"], CF.complexBoolean(False, True), CF._tc_complexBoolean)),
                      (_SandboxDataTypeTest(["complexULongProp"], CF.complexULong(4, 5), CF._tc_complexULong)),
                      (_SandboxDataTypeTest(["complexShortProp"], CF.complexShort(4,5), CF._tc_complexShort)),
                      (_SandboxDataTypeTest(["complexFloatProp"], CF.complexFloat(4.0, 5.0), CF._tc_complexFloat)),
                      (_SandboxDataTypeTest(["complexOctetProp"], CF.complexOctet(4, 5), CF._tc_complexOctet)),
                      (_SandboxDataTypeTest(["complexUShort"], CF.complexUShort(4, 5), CF._tc_complexUShort)),
                      (_SandboxDataTypeTest(["complexDouble"], CF.complexDouble(4.0, 5.0), CF._tc_complexDouble)),
                      (_SandboxDataTypeTest(["complexLong"], CF.complexLong(4, 5), CF._tc_complexLong)),
                      (_SandboxDataTypeTest(["complexLongLong"], CF.complexLongLong(4, 5), CF._tc_complexLongLong)),
                      (_SandboxDataTypeTest(["complexULongLong"], CF.complexULongLong(4, 5), CF._tc_complexULongLong)),
                      (_SandboxDataTypeTest(["complexFloatSequence"], [CF.complexFloat(6, 7), CF.complexFloat(4, 5), CF.complexFloat(8, 9)], None)),
                      (_SandboxDataTypeTest(['complexFloatStruct', ['complexFloatStructMember', 'complexFloatStruct::complex_float_seq']], (CF.complexFloat(6, 7), [CF.complexFloat(3, 4)]), None)),
                      (_SandboxDataTypeTest(['complexFloatStructSequence', ['complexFloatStructSequenceMemberMemember', 'complexFloatStructSequence::complex_float_seq']], [(CF.complexFloat(9, 4), [CF.complexFloat(6, 5)])], None))]

        retval = sb.loadSADFile('sdr/dom/waveforms/TestComplexPropsWaveform/TestComplexPropsWaveform.sad.xml')
        self.assertEquals(retval, True)
        comp_ac = sb.getComponent('TestComplexProps_1')

        sb.start()

        prop_idx = {}
        for idx in range(len(comp_ac._propertySet)):
            prop_idx[comp_ac._propertySet[idx].id] = idx
        for _test in testSet:
            if _test._id[0]=='complexBooleanProp':
                continue
            _prop = comp_ac._propertySet[prop_idx[_test._id[0]]]
            _value = _prop._queryValue().value()
            _compareComplexValues(self, _value, _test.expected)

    def test_complexLoadSADFileOverride(self):
        testSet = [(_SandboxDataTypeTest(["complexBooleanProp"], CF.complexBoolean(True, False), CF._tc_complexBoolean)),
                      (_SandboxDataTypeTest(["complexULongProp"], CF.complexULong(2, 3), CF._tc_complexULong)),
                      (_SandboxDataTypeTest(["complexShortProp"], CF.complexShort(2, 3), CF._tc_complexShort)),
                      (_SandboxDataTypeTest(["complexFloatProp"], CF.complexFloat(2, 3), CF._tc_complexFloat)),
                      (_SandboxDataTypeTest(["complexOctetProp"], CF.complexOctet(2, 3), CF._tc_complexOctet)),
                      (_SandboxDataTypeTest(["complexUShort"], CF.complexUShort(2, 3), CF._tc_complexUShort)),
                      (_SandboxDataTypeTest(["complexDouble"], CF.complexDouble(2, 3), CF._tc_complexDouble)),
                      (_SandboxDataTypeTest(["complexLong"], CF.complexLong(2, 3), CF._tc_complexLong)),
                      (_SandboxDataTypeTest(["complexLongLong"], CF.complexLongLong(2, 3), CF._tc_complexLongLong)),
                      (_SandboxDataTypeTest(["complexULongLong"], CF.complexULongLong(2, 3), CF._tc_complexULongLong)),
                      (_SandboxDataTypeTest(["complexFloatSequence"], [CF.complexFloat(1, 2), CF.complexFloat(10, 20)], None)),
                      (_SandboxDataTypeTest(['complexFloatStruct', ['complexFloatStructMember', 'complexFloatStruct::complex_float_seq']], (CF.complexFloat(6, 7), [CF.complexFloat(-5, 5), CF.complexFloat(9, -8), CF.complexFloat(-13, -24), CF.complexFloat(21, -22), CF.complexFloat(31, 0), CF.complexFloat(0, 431), CF.complexFloat(0, -567), CF.complexFloat(-3567, 0), CF.complexFloat(-5.25, 5.25), CF.complexFloat(9.25, -8.25)]), None)),
                      (_SandboxDataTypeTest(['complexFloatStructSequence', ['complexFloatStructSequenceMemberMemember', 'complexFloatStructSequence::complex_float_seq']], [(CF.complexFloat(32, 33), [CF.complexFloat(45, 55), CF.complexFloat(69, 78)]), (CF.complexFloat(42, 43), [CF.complexFloat(145, 155), CF.complexFloat(169, 178), CF.complexFloat(279, 998)])], None))]

        retval = sb.loadSADFile('sdr/dom/waveforms/TestComplexPropsSADOverrides/TestComplexPropsSADOverrides.sad.xml')
        self.assertEquals(retval, True)
        comp_ac = sb.getComponent('TestComplexProps_1')

        sb.start()

        prop_idx = {}
        for idx in range(len(comp_ac._propertySet)):
            prop_idx[comp_ac._propertySet[idx].id] = idx
        for _test in testSet:
            if _test._id[0]=='complexBooleanProp':
                continue
            _prop = comp_ac._propertySet[prop_idx[_test._id[0]]]
            _value = _prop._queryValue().value()
            _compareComplexValues(self, _value, _test.expected)
