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
from omniORB import URI, any, CORBA
from ossie.cf import CF
from _unitTestHelpers import runtestHelpers

java_support = runtestHelpers.haveJavaSupport('../Makefile')

class SADPropertiesTest(scatest.CorbaTestCase):
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

    def _createApp(self, extra):
        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)

        if java_support:
            sadpath = "/waveforms/ExternalProperties/ExternalProperties"+extra+".sad.xml"
        else:
            sadpath = "/waveforms/ExternalProperties/ExternalProperties"+extra+"NoJava.sad.xml"
        self._domMgr.installApplication(sadpath)
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        appFact = self._domMgr._get_applicationFactories()[0]

        try:
            self._app = appFact.create(appFact._get_name(), [], [])
        except:
            self.fail("Did not create application ")

    def _getComponents(self):
        # Get references to the waveform's components.
        components = {}
        for compName in self._app._get_componentNamingContexts():
            usageName = compName.elementId.split('/')[-1]
            components[usageName] = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        return components

    def test_ExternalProps(self):
        # Makes sure that duplicate external property names throws an error
        self.assertRaises(CF.DomainManager.ApplicationInstallationError, self._createApp, 'Duplicate')
        self.assertRaises(CF.DomainManager.ApplicationInstallationError, self._createApp, 'Duplicate2')

        self._createApp("")

        # Internal property IDs should fail
        pythonProp= CF.DataType(id="DCE:b8f43ac8-26b5-40b3-9102-d127b84f9e4b", value=CORBA.Any(CORBA.TC_string, "test"))
        javaProp = CF.DataType(id="ulong_prop", value=CORBA.Any(CORBA.TC_ulong, 1))
        self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure, [javaProp, pythonProp])

        # will throw partial because cppProp is in AC
        cppProp = CF.DataType(id="DCE:9d1e3621-27ca-4cd0-909d-90b7448b8f71", value=CORBA.Any(CORBA.TC_long, -1))
        self.assertRaises(CF.PropertySet.PartialConfiguration, self._app.configure, [pythonProp, cppProp, javaProp])

        # Make sure cppProp was set
        cppProp = CF.DataType(id="DCE:9d1e3621-27ca-4cd0-909d-90b7448b8f71", value=any.to_any(None))
        retProp = self._app.query([cppProp])[0]
        self.assertEquals(retProp.value.value(), -1)

        cppProp = CF.DataType(id="ext_prop_long", value=CORBA.Any(CORBA.TC_long, -111))
        pythonProp= CF.DataType(id="ext_prop_string", value=CORBA.Any(CORBA.TC_string, "hello world"))
        javaProp = CF.DataType(id="ext_prop_ulong", value=CORBA.Any(CORBA.TC_ulong, 111))

        # Configure all
        props = [pythonProp, cppProp]
        number_props = 5
        to_find = 2
        if java_support:
            props.append(javaProp)
            number_props = 6
            to_find = 3
        self._app.configure(props)
        # Make sure all were set
        found = 0
        props = self._app.query([])
        # Should have 3 external properties and 3 AC properties (the 4th AC prop is promoted as external)
        self.assertEquals(len(props), number_props)
        for p in props:
            if p.id == "ext_prop_long":
                self.assertEquals(p.value.value(), -111)
                found += 1
            elif p.id == "ext_prop_string":
                self.assertEquals(p.value.value(), "hello world")
                found += 1
            elif p.id == "ext_prop_ulong":
                self.assertEquals(p.value.value(), 111)
                found += 1
       # Make sure all 3 external prop IDs were found
        if not found == to_find:
            self.fail('Unable to query() all required external property IDs')

        # Can still configure other AC internal props
        cppProp = CF.DataType(id="DCE:4e7c1977-5f53-4061-bae7-cb8c1072f4b7", value=CORBA.Any(CORBA.TC_string, "Hello world"))
        self._app.configure([cppProp])
        cppProp = CF.DataType(id="DCE:4e7c1977-5f53-4061-bae7-cb8c1072f4b7", value=any.to_any(None))
        retProp = self._app.query([cppProp])[0]
        self.assertEquals(retProp.value.value(), "Hello world")

        # Can not configure/query other components internal props
        pythonProp = CF.DataType(id="test_float", value=CORBA.Any(CORBA.TC_float, 1.11))
        javaProp = CF.DataType(id="DCE:3303ee57-70bb-4325-84ad-fb7fd333c44a",  value=any.to_any([1, 1]))
        self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure, [pythonProp, javaProp])
        self.assertRaises(CF.UnknownProperties, self._app.query, [pythonProp, javaProp])

        # Make sure can do individual queries on external properties
        pythonProp= CF.DataType(id="ext_prop_string", value=CORBA.Any(CORBA.TC_string, "HELLO WORLD"))
        javaProp = CF.DataType(id="ext_prop_ulong", value=CORBA.Any(CORBA.TC_ulong, 222))
        props = [pythonProp]
        if java_support:
            props.append(javaProp)
        self._app.configure(props)
        pythonProp= CF.DataType(id="ext_prop_string", value=any.to_any(None))
        javaProp = CF.DataType(id="ext_prop_ulong", value=any.to_any(None))
        if java_support:
            pythonRet = self._app.query([pythonProp, javaProp])[0]
            javaRet = self._app.query([pythonProp, javaProp])[1]
            self.assertEquals(javaRet.value.value(), 222)
        else:
            pythonRet = self._app.query([pythonProp])[0]
        self.assertEquals(pythonRet.value.value(), "HELLO WORLD")

        # Individual queries of mix of AC & external properties
        cppProp = CF.DataType(id="ext_prop_long", value=CORBA.Any(CORBA.TC_long, -333))
        cppPropInternal = CF.DataType(id="DCE:4e7c1977-5f53-4061-bae7-cb8c1072f4b7", value=CORBA.Any(CORBA.TC_string, "HELLO WORLD2"))
        pythonProp= CF.DataType(id="ext_prop_string", value=CORBA.Any(CORBA.TC_string, "hello world2"))
        javaProp = CF.DataType(id="ext_prop_ulong", value=CORBA.Any(CORBA.TC_ulong, 333))
        if java_support:
            self._app.configure([pythonProp, javaProp, cppProp, cppPropInternal])
        else:
            self._app.configure([pythonProp, cppProp, cppPropInternal])

        cppProp = CF.DataType(id="ext_prop_long", value=any.to_any(None))
        cppPropInternal = CF.DataType(id="DCE:4e7c1977-5f53-4061-bae7-cb8c1072f4b7", value=any.to_any(None))
        pythonProp= CF.DataType(id="ext_prop_string", value=any.to_any(None))
        javaProp = CF.DataType(id="ext_prop_ulong", value=any.to_any(None))

        if java_support:
            pythonRet = self._app.query([pythonProp, javaProp, cppPropInternal, cppProp])[0]
            javaRet = self._app.query([pythonProp, javaProp, cppPropInternal, cppProp])[1]
            cppRet2 = self._app.query([pythonProp, javaProp, cppPropInternal, cppProp])[2]
            cppRet = self._app.query([pythonProp, javaProp, cppPropInternal, cppProp])[3]
        else:
            pythonRet = self._app.query([pythonProp, cppPropInternal, cppProp])[0]
            cppRet2 = self._app.query([pythonProp, cppPropInternal, cppProp])[1]
            cppRet = self._app.query([pythonProp, cppPropInternal, cppProp])[2]

        self.assertEquals(pythonRet.value.value(), "hello world2")
        if java_support:
            self.assertEquals(javaRet.value.value(), 333)
        self.assertEquals(cppRet2.value.value(), "HELLO WORLD2")
        self.assertEquals(cppRet.value.value(), -333)


    def test_externalAndACProp(self):
        self._createApp("")

        # Make sure external props that are in the AC can be accessed by either ID
        cppPropExt = CF.DataType(id="ext_prop_long", value=CORBA.Any(CORBA.TC_long, -11))
        cppPropQueryExt = CF.DataType(id="ext_prop_long", value=any.to_any(None))
        cppProp = CF.DataType(id="DCE:9d1e3621-27ca-4cd0-909d-90b7448b8f71", value=CORBA.Any(CORBA.TC_long, -22))
        cppPropQuery = CF.DataType(id="DCE:9d1e3621-27ca-4cd0-909d-90b7448b8f71", value=any.to_any(None))

        self._app.configure([cppProp])
        self.assertEquals(self._app.query([cppPropQuery])[0].value.value(), -22)
        self.assertEquals(self._app.query([cppPropQueryExt])[0].value.value(), -22)

        self._app.configure([cppPropExt])
        self.assertEquals(self._app.query([cppPropQuery])[0].value.value(), -11)
        self.assertEquals(self._app.query([cppPropQueryExt])[0].value.value(), -11)

    def test_externalAcConflict(self):
        # External property name that is the same as an Assembly Controller property should throw an error
        self.assertRaises(CF.DomainManager.ApplicationInstallationError, self._createApp, 'AcExternalConflict')

    def test_badInternalId(self):
        extra = 'BadInternal'

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)

        sadpath = "/waveforms/ExternalProperties/ExternalProperties"+extra+".sad.xml"
        self._domMgr.installApplication(sadpath)
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        appFact = self._domMgr._get_applicationFactories()[0]

        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, appFact.create, appFact._get_name(), [], [])

    def test_badCompRef(self):
        extra = 'BadCompRef'

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)

        if java_support:
            sadpath = '/waveforms/ExternalProperties/ExternalProperties'+extra+'.sad.xml'
        else:
            sadpath = '/waveforms/ExternalProperties/ExternalProperties'+extra+'NoJava.sad.xml'
        self._domMgr.installApplication(sadpath)
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        appFact = self._domMgr._get_applicationFactories()[0]

        # Bad compref tag in externalproperties should throw appropriate error
        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, appFact.create, appFact._get_name(), [], [])

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)

    def test_ExternalPropOverride(self):
        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)

        if java_support:
            self._domMgr.installApplication('/waveforms/ExternalProperties/ExternalProperties.sad.xml')
        else:
            self._domMgr.installApplication('/waveforms/ExternalProperties/ExternalPropertiesNoJava.sad.xml')
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        appFact = self._domMgr._get_applicationFactories()[0]

        props = []
        if java_support:
            props.append(CF.DataType(id='ext_prop_ulong', value=any.to_any(123456)))
        props.append(CF.DataType(id='DCE:b8f43ac8-26b5-40b3-9102-d127b84f9e4b', value=any.to_any("Override_value")))
        props.append(CF.DataType(id='DCE:0a3663dd-7747-4b7f-b9cb-20f1e52e7089', value=any.to_any(98765)))
        props.append(CF.DataType(id='test_float', value=any.to_any(99.9)))
        app = appFact.create(appFact._get_name(), props, [])

        res = app.query([])
        for r in res:
            if r.id == 'ext_prop_ulong':
                self.assertEquals(123456, r.value.value())
            elif r.id == 'DCE:b8f43ac8-26b5-40b3-9102-d127b84f9e4b':
                self.assertEquals('Override_value', r.value.value())
            elif r.id == 'DCE:0a3663dd-7747-4b7f-b9cb-20f1e52e7089':
                self.assertEquals(98765, r.value.value())
            elif r.id == 'test_float':
                self.fail('The property "test_float" was not marked as external')

        # Props that are not external should not be configured
        comps = app._get_registeredComponents()
        for c in comps:
            if c.identifier == 'TestPythonProps_1:ExternalPropertiesNoJava_1' or c.identifier == 'TestPythonProps_1:ExternalProperties_1':
                comp = c
        res = comp.componentObject.query([])
        for r in res:
            if r.id == 'test_float':
                self.assertAlmostEquals(1.234, r.value.value())
                self.assertNotAlmostEquals(99.9, r.value.value())
