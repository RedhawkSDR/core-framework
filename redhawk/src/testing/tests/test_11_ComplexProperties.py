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
from omniORB import CORBA
import numpy

class _DataTypeTest:
    def __init__(self, id, default, override, typecode):
        self.id = id
        self.default = default
        self.override = override
        self.typecode = typecode



class _TestVector:
    '''
    def test_complexBoolean(self):
        testStruct = (_DataTypeTest("complexBooleanProp",
                                    CF.complexBoolean(False, True),
                                    CF.complexBoolean(True, False),
                                    CF._tc_complexBoolean))
        self._runTest(testStruct)

    def test_complexULong(self):
        testStruct = (_DataTypeTest("complexULongProp",
                                    CF.complexULong(4, 5),
                                    CF.complexULong(2, 3),
                                    CF._tc_complexULong))
        self._runTest(testStruct)

    def test_complexShort(self):
        testStruct = (_DataTypeTest("complexShortProp",
                                    CF.complexShort(4,5),
                                    CF.complexShort(2,3),
                                    CF._tc_complexShort))
        self._runTest(testStruct)
    '''
    def test_complexFloat(self):
        testStruct = (_DataTypeTest("complexFloatProp",
                                    CF.complexFloat(4.0, 5.0),
                                    CF.complexFloat(2.0, 3.0),
                                    CF._tc_complexFloat))
        self._runTest(testStruct)
    '''
    def test_complexOctet(self):
        testStruct = (_DataTypeTest("complexOctetProp",
                                    CF.complexOctet(4, 5),
                                    CF.complexOctet(2, 3),
                                    CF._tc_complexOctet))
        self._runTest(testStruct)

    def test_complexUShort(self):
        testStruct = (_DataTypeTest("complexUShort",
                                    CF.complexUShort(4, 5),
                                    CF.complexUShort(2, 3),
                                    CF._tc_complexUShort))
        self._runTest(testStruct)

    def test_complexDouble(self):
        testStruct = (_DataTypeTest("complexDouble",
                                    CF.complexDouble(4.0, 5.0),
                                    CF.complexDouble(2.0, 3.0),
                                    CF._tc_complexDouble))
        self._runTest(testStruct)

    def test_complexLong(self):
        testStruct = (_DataTypeTest("complexLong",
                                    CF.complexLong(4, 5),
                                    CF.complexLong(2, 3),
                                    CF._tc_complexLong))
        self._runTest(testStruct)

    def test_complexLongLong(self):
        testStruct = (_DataTypeTest("complexLongLong",
                                    CF.complexLongLong(4, 5),
                                    CF.complexLongLong(2, 3),
                                    CF._tc_complexLongLong))
        self._runTest(testStruct)

    def test_complexULongLong(self):
        testStruct = (_DataTypeTest("complexULongLong",
                                    CF.complexULongLong(4, 5),
                                    CF.complexULongLong(2, 3),
                                    CF._tc_complexULongLong))
        self._runTest(testStruct)
    '''
#    def test_complexChar(self):
#        testStruct = (_DataTypeTest(
#            "complexCharProp",
#            CF.complexChar(0, 1),
#            CF.complexChar(2, 3),
#            CF._tc_complexChar))
#        self._runTest(testStruct)

class SetupCommon:
    def _compareComplexValues(self, val1, val2):
        self.assertEquals(val1.real, val2.real)
        self.assertEquals(val1.imag, val2.imag)
    def setUp_(self, sadpath):
        domBooter, self._domMgr = self.launchDomainManager(debug=self.debuglevel)
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml", debug=self.debuglevel)
        self._app = None
        if self._domMgr:
            try:
                self._domMgr.installApplication(sadpath)
                appFact = self._domMgr._get_applicationFactories()[0]
                self._app = appFact.create(appFact._get_name(), [], [])
            except:
                pass

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


class CppPropertiesSADOverridesTest(scatest.CorbaTestCase, _TestVector, SetupCommon):
    def setUp(self):
        SetupCommon.setUp_(self, sadpath = "/waveforms/TestComplexPropsSADOverrides/TestComplexPropsSADOverrides.sad.xml")

    def tearDown(self):
        SetupCommon.tearDown_(self)

    def preconditions(self):
        SetupCommon.preconditions_(self)

    def _runTest(self, dataTypeTest):
        # Create a property structure
        prop = CF.DataType(id = dataTypeTest.id,
                           value = CORBA.Any(dataTypeTest.typecode,
                                             dataTypeTest.default))
        # Check the default property value via query
        defaultProps = self._app.query([prop])
        self._compareComplexValues(defaultProps[0].value.value(), dataTypeTest.override)


class SandboxTest(scatest.CorbaTestCase, _TestVector, SetupCommon):
    def setUp(self):
        SetupCommon.setUp_(self, sadpath = "/waveforms/TestComplexPropsWaveform/TestComplexPropsWaveform.sad.xml")

    def tearDown(self):
        SetupCommon.tearDown_(self)

    def preconditions(self):
        SetupCommon.preconditions_(self)

    def _runTest(self, dataTypeTest):
        '''
        1.  Check the default value of the property via the query method.
        2.  Configure the property with an override value.
        3.  Query the property to make sure the override value has been set.

        '''
        # Create a property structure
        prop = CF.DataType(id = dataTypeTest.id,
                           value = CORBA.Any(dataTypeTest.typecode,
                                             dataTypeTest.override))

        # Check the default property value via query
        defaultProps = self._app.query([prop])
        self._compareComplexValues(defaultProps[0].value.value(), dataTypeTest.default)

        # Call configure with the property with an override value
        # then check, via query, if the configuration worked
        self._app.configure([prop])
        newProps = self._app.query([prop])
        self._compareComplexValues(newProps[0].value.value(), dataTypeTest.override)

    def _queryDefaults(self, component):
        defaults = {"boolean"   : component.complexBooleanProp,
                    "ulong"     : component.complexULongProp,
                    "short"     : component.complexShortProp,
                    "float"     : component.complexFloatProp,
                    "octet"     : component.complexOctetProp,
                    "ushort"    : component.complexUShort,
                    "double"    : component.complexDouble,
                    "long"      : component.complexLong,
                    "longlong"  : component.complexLongLong,
                    "ulonglong" : component.complexULongLong}
        # TODO: char
        #"char"      : component.complexCharProp,
        #"char"      : numpy.complex(0,1),
        return defaults

    def test_sandboxComplexProps(self):
        from ossie.utils import sb

        # values from the component PRF file
        expectedDefaults = {
            "boolean"   : numpy.complex(False, True),
            "ulong"     : numpy.complex(4,5),
            "short"     : numpy.complex(4,5),
            "float"     : numpy.complex(4.,5.),
            "octet"     : numpy.complex(4,5),
            "ushort"    : numpy.complex(4,5),
            "double"    : numpy.complex(4.,5.),
            "long"      : numpy.complex(4,5),
            "longlong"  : numpy.complex(4,5),
            "ulonglong" : numpy.complex(4,5)}


        '''
            "cFloatSeq"       : component.complexFloatSeq,
            "cFloatStruct"    : component.complexFloatStruct,
            "cFloatStructSeq" : component.complexFloatStructSeq}
            "cFloatSeq"       : [CF.complexFloat(real=1.0, imag=0.0),
                                 CF.complexFloat(real=1.0, imag=0.0),
                                 CF.complexFloat(real=1.0, imag=0.0)],
            "cFloatStruct"    : {"complexFloatStructMember": CF.complexFloat(real=1.0, imag=0.0)},
            "cFloatStructSeq" : [{"complexFloatStructMember": CF.complexFloat(real=1.0, imag=0.0)}]}
        '''

        # Create an instance of the test component in all 3 languages
        components = {"cpp"   : sb.launch("TestComplexProps", impl="cpp"),
                      "python": sb.launch("TestComplexProps", impl="python"),
                      "java"  : sb.launch("TestComplexProps", impl="java")}

        sb.start()

        for language in components.keys():
            # allow for visual inspection of complex sequences
            # TODO: replace this with an automated comparison
            print language
            print components[language].complexFloatProp
            print "simple struct member"
            print components[language].FloatStruct.FloatStructMember
            components[language].FloatStruct.FloatStructMember = 9
            print components[language].FloatStruct.FloatStructMember
            print "complex struct member"
            print components[language].complexFloatStruct.complexFloatStructMember
            components[language].complexFloatStruct.complexFloatStructMember = complex(9,10)
            print components[language].complexFloatStruct.complexFloatStructMember


            print components[language].complexFloatSequence
            components[language].complexFloatSequence = [complex(6,7)]*3
            print components[language].complexFloatSequence
            print ""


        for componentKey in components.keys():
            # loop through all three languages and query for the default
            # property values
            defaults = self._queryDefaults(components[componentKey])
            for key in defaults.keys():
                # Loop through the default property values and compare them
                # to the expected values.
                self._compareComplexValues(defaults[key], expectedDefaults[key])

        sb.domainless._cleanUpLaunchedComponents()
