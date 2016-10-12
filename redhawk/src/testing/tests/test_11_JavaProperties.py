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
import threading
import time
import scatest
from ossie.cf import CF
from omniORB import CORBA, any
from ossie.properties import simple_property

class struct_val(object):
    item_long = simple_property(id_="item_long", type_="long")
    item_string = simple_property(id_="item_string", type_="string")

    def __init__(self, item_long=0, item_string=""):
        self.item_long = item_long
        self.item_string = item_string


class JavaPropertiesTest(scatest.CorbaTestCase):

    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager(debug=9)
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", debug=9)
        self._app = None
        if self._domMgr:
            try:
                sadpath = "/waveforms/TestJavaProps/TestJavaProps.sad.xml"
                self._domMgr.installApplication(sadpath)
                appFact = self._domMgr._get_applicationFactories()[0]
                self._app = appFact.create(appFact._get_name(), [], [])
            except:
                pass

    def _structToProps(self, value):
        properties = []
        for name, attr in struct_val.__dict__.items():
            if type(attr) is simple_property: 
                field_value = attr.get(value)
                properties.append(CF.DataType(id=attr.id_, value=attr._toAny(field_value)))

        return properties

    def _structsToAny(self, values):
        anySeq = [any.to_any(self._structToProps(v)) for v in values]
        return any.to_any(anySeq)

    def _propsToDict(self, props):
        return dict([(dt['id'], dt['value']) for dt in props])

    def _anyToDicts(self, value):
        return [self._propsToDict(v) for v in any.from_any(value)]

    def tearDown(self):
        if self._app:
            self._app.stop()
            self._app.releaseObject()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def test_EmptyQuery (self):
        results = self._app.query([])
        ids = []
        for result in results:
            ids.append(result.id)
        self.assertEqual("DCE:3303ee57-70bb-4325-84ad-fb7fd333c44a" in ids, True)
        self.assertEqual("DCE:dd8d450f-d377-4c2c-8c3c-207e42dae017" in ids, True)
        self.assertEqual("DCE:d933f8ba-9e79-4d2c-a1b5-9fb9da0ea740" in ids, True)
        self.assertEqual("exec_param" in ids, True)
        self.assertEqual("DCE:23a6d333-55fb-4425-a102-185e6e998782" in ids, True)
        self.assertEqual(len(results), 5)
        
        self._app.configure([CF.DataType(id='DCE:3303ee57-70bb-4325-84ad-fb7fd333c44a',value=any.to_any([1,2,3,4]))])
        self._app.start()

    def test_StructSequenceProps (self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")
        self.assertNotEqual(self._app, None, "Failed to launch app")

        # The TestJavaProps component is also the assembly controller, so we can
        # query it via the application.
        prop = self._app.query([CF.DataType("DCE:d933f8ba-9e79-4d2c-a1b5-9fb9da0ea740", any.to_any(None))])[0]

        # Check that the struct sequence matches the expected value from the SAD.
        value = any.from_any(prop.value)
        self.assertEqual(len(value), 2)
        d = [dict([(dt['id'], dt['value']) for dt in v]) for v in value]
        self.assertEqual(d, [{'item_string':'five', 'item_long':5}, {'item_string':'seven', 'item_long':7}])

        # Try configuring a new structsequence value and checking that it comes back correct.
        newvalue = [{"item_long":12, "item_string":"twelve"}, {"item_long":9, "item_string":"nine"}]
        newprop = CF.DataType("DCE:d933f8ba-9e79-4d2c-a1b5-9fb9da0ea740", self._structsToAny([struct_val(**d) for d in newvalue]))
        self._app.configure([newprop])
        prop = self._app.query([CF.DataType("DCE:d933f8ba-9e79-4d2c-a1b5-9fb9da0ea740", any.to_any(None))])[0]
        value = self._anyToDicts(prop.value)
        self.assertEqual(value, newvalue)

