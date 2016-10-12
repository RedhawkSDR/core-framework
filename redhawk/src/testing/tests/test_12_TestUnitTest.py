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
import tempfile
import os
from omniORB import CORBA, any
import ossie.utils.testing as testing

class UnitTestTest(scatest.OssieTestCase):
    """Test the component unittest module"""

    def test_GetPropertySet(self):
        testing.setSoftPkg(os.path.join(scatest.getSdrPath(), "dom", "components", "CommandWrapper", "CommandWrapper.spd.xml"))
        testing.setImplId("DCE:535a68a7-64e4-424b-9045-5ffc7659bf9d")
        testing.RHComponentTestCase.runTest = lambda : None
        tc = testing.RHComponentTestCase()
        tc.setUp()

        execparams = tc.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"))
        execparams = dict([(p.id, any.from_any(p.value)) for p in execparams])
        self.assertEqual(execparams.has_key("EXEC_PARAM_1"), True)
        self.assertEqual(execparams.has_key("EXEC_PARAM_2"), True)
        self.assertEqual(execparams.has_key("EXEC_PARAM_4"), True)
        self.assertEqual(execparams.has_key("SOMEOBJREF"), True)

        self.assertEqual(execparams["EXEC_PARAM_1"], "Test1")
        self.assertEqual(execparams["EXEC_PARAM_2"], 2)
        self.assertEqual(execparams["EXEC_PARAM_4"], True)
        self.assertEqual(execparams["SOMEOBJREF"], None)

        execparams = tc.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(p.id, any.from_any(p.value)) for p in execparams])
        self.assertEqual(execparams.has_key("EXEC_PARAM_1"), True)
        self.assertEqual(execparams.has_key("EXEC_PARAM_2"), True)
        self.assertEqual(execparams.has_key("EXEC_PARAM_4"), True)
        self.assertEqual(execparams.has_key("SOMEOBJREF"), False)

        self.assertEqual(execparams["EXEC_PARAM_1"], "Test1")
        self.assertEqual(execparams["EXEC_PARAM_2"], 2)
        self.assertEqual(execparams["EXEC_PARAM_4"], True)

        cfgparams = tc.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"))
        cfgparams = dict([(p.id, any.from_any(p.value)) for p in cfgparams])
        self.assertEqual(cfgparams.has_key("DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e"), True)
        self.assertEqual(cfgparams.has_key("DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53"), True)
        self.assertEqual(cfgparams.has_key("DCE:ffe634c9-096d-425b-86cc-df1cce50612f"), True)
        self.assertEqual(cfgparams.has_key("DCE:fa8c5924-845c-484a-81df-7941f2c5baa9"), True)
        self.assertEqual(cfgparams.has_key("DCE:a7de97ee-1e78-45e9-8e2b-204c141656fc"), True)
        self.assertEqual(cfgparams.has_key("DCE:9ec6e2ff-6a4f-4452-8f38-4df47d6eebc1"), True)
        self.assertEqual(cfgparams.has_key("DCE:cf623573-a09d-4fb1-a2ae-24b0b507115d"), True)
        self.assertEqual(cfgparams.has_key("DCE:6ad84383-49cf-4017-b7ca-0ec4c4917952"), True)

        propparams = tc.getPropertySet(kinds=("property",), modes=("readwrite", "writeonly"))
        propparams = dict([(p.id, any.from_any(p.value)) for p in propparams])
        self.assertEqual(propparams.has_key("configure_prop_notset"), True)

        self.assertEqual(cfgparams["DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e"], "/bin/echo")
        self.assertEqual(cfgparams["DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53"], ["Hello World"])
        self.assertEqual(cfgparams["DCE:ffe634c9-096d-425b-86cc-df1cce50612f"], [{"id": 'item1', 'value': 'value1'},
                                                                                 {"id": 'item2', 'value': 100},
                                                                                 {"id": 'item3', 'value': 3.14156}
                                                                                ])
        self.assertEqual(cfgparams["DCE:fa8c5924-845c-484a-81df-7941f2c5baa9"], 10000)
        self.assertEqual(cfgparams["DCE:a7de97ee-1e78-45e9-8e2b-204c141656fc"], 12345678901)
        self.assertEqual(cfgparams["DCE:9ec6e2ff-6a4f-4452-8f38-4df47d6eebc1"], 11111111111)
        self.assertEqual(cfgparams["DCE:cf623573-a09d-4fb1-a2ae-24b0b507115d"], 500.0)
        self.assertEqual(cfgparams["DCE:6ad84383-49cf-4017-b7ca-0ec4c4917952"], None)

    def test_SimpleLaunch(self):
        os.environ['OSSIEHOME'] = 'foo'
        # Simulate RHComponentTestProgram
        testing.setSoftPkg(os.path.join(scatest.getSdrPath(), "dom", "components", "CommandWrapper", "CommandWrapper.spd.xml"))
        testing.setImplId("DCE:535a68a7-64e4-424b-9045-5ffc7659bf9d")

        testing.RHComponentTestCase.runTest = lambda : None
        tc = testing.RHComponentTestCase()
        tc.setUp()

        self.assertEqual(tc.comp_obj, None)
        self.assertEqual(tc.comp, None)

        tc.launch(ossiehome=os.getcwd()+'/../')
        pid = None
        comp_obj = None
        try:
            self.assertNotEqual(tc.comp_obj, None)
            self.assertNotEqual(tc.comp, None)
            comp_obj = tc.comp_obj
            self.assertEqual(comp_obj._non_existent(), False)
            self.assertEqual(comp_obj._is_a("IDL:CF/Resource:1.0"), True)
        finally:
            tc.tearDown()

        self.assertNotEqual(comp_obj, None)
        try:
            nonExistent = comp_obj._non_existent()
        except CORBA.TRANSIENT:
            nonExistent = True
        self.assertEqual(nonExistent, True)
        self.assertEqual(tc.comp_obj, None)

    def test_DeviceLaunch(self):
        # Simulate RHComponentTestProgram
        testing.setSoftPkg(os.path.join(scatest.getSdrPath(), "dev", "devices", "BasicTestDevice", "BasicTestDevice.spd.xml"))
        testing.setImplId("DCE:0ef71fab-731d-4ee1-a528-a6da2207e0c5")

        testing.RHComponentTestCase.runTest = lambda : None
        tc = testing.RHComponentTestCase()
        tc.setUp()

        self.assertEqual(tc.comp_obj, None)
        self.assertEqual(tc.comp, None)

        tc.launch(ossiehome=os.getcwd()+'/../')
        comp_obj = None
        try:
            self.assertNotEqual(tc.comp_obj, None)
            self.assertNotEqual(tc.comp, None)
            comp_obj = tc.comp_obj
            self.assertEqual(comp_obj._non_existent(), False)
            self.assertEqual(tc.comp_obj._is_a("IDL:CF/Resource:1.0"), True)
            self.assertEqual(tc.comp_obj._is_a("IDL:CF/Device:1.0"), True)
            self.assertEqual(tc.comp_obj._is_a("IDL:CF/LoadableDevice:1.0"), True)
            self.assertEqual(tc.comp_obj._is_a("IDL:CF/ExecutableDevice:1.0"), True)
        finally:
            tc.tearDown()

        self.assertNotEqual(comp_obj, None)
        try:
            nonExistent = comp_obj._non_existent()
        except CORBA.TRANSIENT:
            nonExistent = True
        self.assertEqual(nonExistent, True)
        self.assertEqual(tc.comp_obj, None)


    def test_GetPropertySetBackwardsCompatibility(self):
        testing.setSoftPkg(os.path.join(scatest.getSdrPath(), "dom", "components", "CommandWrapper", "CommandWrapper.spd.xml"))
        testing.setImplId("DCE:535a68a7-64e4-424b-9045-5ffc7659bf9d")
        testing.ScaComponentTestCase.runTest = lambda : None
        tc = testing.ScaComponentTestCase()
        tc.setUp()

        execparams = tc.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"))
        execparams = dict([(p.id, any.from_any(p.value)) for p in execparams])
        self.assertEqual(execparams.has_key("EXEC_PARAM_1"), True)
        self.assertEqual(execparams.has_key("EXEC_PARAM_2"), True)
        self.assertEqual(execparams.has_key("EXEC_PARAM_4"), True)
        self.assertEqual(execparams.has_key("SOMEOBJREF"), True)

        self.assertEqual(execparams["EXEC_PARAM_1"], "Test1")
        self.assertEqual(execparams["EXEC_PARAM_2"], 2)
        self.assertEqual(execparams["EXEC_PARAM_4"], True)
        self.assertEqual(execparams["SOMEOBJREF"], None)

        execparams = tc.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(p.id, any.from_any(p.value)) for p in execparams])
        self.assertEqual(execparams.has_key("EXEC_PARAM_1"), True)
        self.assertEqual(execparams.has_key("EXEC_PARAM_2"), True)
        self.assertEqual(execparams.has_key("EXEC_PARAM_4"), True)
        self.assertEqual(execparams.has_key("SOMEOBJREF"), False)

        self.assertEqual(execparams["EXEC_PARAM_1"], "Test1")
        self.assertEqual(execparams["EXEC_PARAM_2"], 2)
        self.assertEqual(execparams["EXEC_PARAM_4"], True)

        cfgparams = tc.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"))
        cfgparams = dict([(p.id, any.from_any(p.value)) for p in cfgparams])
        self.assertEqual(cfgparams.has_key("DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e"), True)
        self.assertEqual(cfgparams.has_key("DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53"), True)
        self.assertEqual(cfgparams.has_key("DCE:ffe634c9-096d-425b-86cc-df1cce50612f"), True)
        self.assertEqual(cfgparams.has_key("DCE:fa8c5924-845c-484a-81df-7941f2c5baa9"), True)
        self.assertEqual(cfgparams.has_key("DCE:a7de97ee-1e78-45e9-8e2b-204c141656fc"), True)
        self.assertEqual(cfgparams.has_key("DCE:9ec6e2ff-6a4f-4452-8f38-4df47d6eebc1"), True)
        self.assertEqual(cfgparams.has_key("DCE:cf623573-a09d-4fb1-a2ae-24b0b507115d"), True)
        self.assertEqual(cfgparams.has_key("DCE:6ad84383-49cf-4017-b7ca-0ec4c4917952"), True)

        propparams = tc.getPropertySet(kinds=("property",), modes=("readwrite", "writeonly"))
        propparams = dict([(p.id, any.from_any(p.value)) for p in propparams])
        self.assertEqual(propparams.has_key("configure_prop_notset"), True)

        self.assertEqual(cfgparams["DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e"], "/bin/echo")
        self.assertEqual(cfgparams["DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53"], ["Hello World"])
        self.assertEqual(cfgparams["DCE:ffe634c9-096d-425b-86cc-df1cce50612f"], [{"id": 'item1', 'value': 'value1'},
                                                                                 {"id": 'item2', 'value': 100},
                                                                                 {"id": 'item3', 'value': 3.14156}
                                                                                ])
        self.assertEqual(cfgparams["DCE:fa8c5924-845c-484a-81df-7941f2c5baa9"], 10000)
        self.assertEqual(cfgparams["DCE:a7de97ee-1e78-45e9-8e2b-204c141656fc"], 12345678901)
        self.assertEqual(cfgparams["DCE:9ec6e2ff-6a4f-4452-8f38-4df47d6eebc1"], 11111111111)
        self.assertEqual(cfgparams["DCE:cf623573-a09d-4fb1-a2ae-24b0b507115d"], 500.0)
        self.assertEqual(cfgparams["DCE:6ad84383-49cf-4017-b7ca-0ec4c4917952"], None)

    def test_SimpleLaunchBackwardsCompatibility(self):
        os.environ['OSSIEHOME'] = 'foo'
        # Simulate RHComponentTestProgram
        testing.setSoftPkg(os.path.join(scatest.getSdrPath(), "dom", "components", "CommandWrapper", "CommandWrapper.spd.xml"))
        testing.setImplId("DCE:535a68a7-64e4-424b-9045-5ffc7659bf9d")

        testing.ScaComponentTestCase.runTest = lambda : None
        tc = testing.ScaComponentTestCase()
        tc.setUp()

        self.assertEqual(tc.comp_obj, None)
        self.assertEqual(tc.comp, None)

        tc.launch(ossiehome=os.getcwd()+'/../')
        pid = None
        comp_obj = None
        try:
            self.assertNotEqual(tc.comp_obj, None)
            self.assertNotEqual(tc.comp, None)
            comp_obj = tc.comp_obj
            self.assertEqual(comp_obj._non_existent(), False)
            self.assertEqual(comp_obj._is_a("IDL:CF/Resource:1.0"), True)
        finally:
            tc.tearDown()

        self.assertNotEqual(comp_obj, None)
        try:
            nonExistent = comp_obj._non_existent()
        except CORBA.TRANSIENT:
            nonExistent = True
        self.assertEqual(nonExistent, True)
        self.assertEqual(tc.comp_obj, None)

    def test_DeviceLaunchBackwardsCompatibility(self):
        # Simulate RHComponentTestProgram
        testing.setSoftPkg(os.path.join(scatest.getSdrPath(), "dev", "devices", "BasicTestDevice", "BasicTestDevice.spd.xml"))
        testing.setImplId("DCE:0ef71fab-731d-4ee1-a528-a6da2207e0c5")

        testing.ScaComponentTestCase.runTest = lambda : None
        tc = testing.ScaComponentTestCase()
        tc.setUp()

        self.assertEqual(tc.comp_obj, None)
        self.assertEqual(tc.comp, None)

        tc.launch(ossiehome=os.getcwd()+'/../')
        comp_obj = None
        try:
            self.assertNotEqual(tc.comp_obj, None)
            self.assertNotEqual(tc.comp, None)
            comp_obj = tc.comp_obj
            self.assertEqual(comp_obj._non_existent(), False)
            self.assertEqual(tc.comp_obj._is_a("IDL:CF/Resource:1.0"), True)
            self.assertEqual(tc.comp_obj._is_a("IDL:CF/Device:1.0"), True)
            self.assertEqual(tc.comp_obj._is_a("IDL:CF/LoadableDevice:1.0"), True)
            self.assertEqual(tc.comp_obj._is_a("IDL:CF/ExecutableDevice:1.0"), True)
        finally:
            tc.tearDown()

        self.assertNotEqual(comp_obj, None)
        try:
            nonExistent = comp_obj._non_existent()
        except CORBA.TRANSIENT:
            nonExistent = True
        self.assertEqual(nonExistent, True)
        self.assertEqual(tc.comp_obj, None)


