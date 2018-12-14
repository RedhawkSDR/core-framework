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

from omniORB.any import to_any, from_any

from _unitTestHelpers import scatest
from ossie.cf import CF

class ComponentHostTest(scatest.CorbaTestCase):
    def setUp(self):
        self.launchDomainManager()
        self.launchDeviceManager('/nodes/test_GPP_node/DeviceManager.dcd.xml')

    def test_BasicShared(self):
        self.assertNotEqual(self._domainManager, None)

        app = self._domainManager.createApplication('/waveforms/BasicSharedWave/BasicSharedWave.sad.xml',
                                                    'BasicSharedWave', [], [])

        comps = app._get_registeredComponents()
        self.assertEqual(len(comps), 2)

        request = [CF.DataType('pid', to_any(None))]
        props1 = comps[0].componentObject.query(request)
        props2 = comps[1].componentObject.query(request)

        self.assertEqual(len(props1), 1)
        self.assertEqual(props1[0].id, 'pid')
        self.assertEqual(len(props2), 1)
        self.assertEqual(props2[0].id, 'pid')
        self.assertEqual(from_any(props1[0].value), from_any(props2[0].value))

    def test_CollocBasicShared(self):
        self.assertNotEqual(self._domainManager, None)

        app = self._domainManager.createApplication('/waveforms/BasicSharedCollocWave/BasicSharedCollocWave.sad.xml',
                                                    'BasicSharedCollocWave', [], [])

        comps = app._get_registeredComponents()
        self.assertEqual(len(comps), 2)

        request = [CF.DataType('pid', to_any(None))]
        props1 = comps[0].componentObject.query(request)
        props2 = comps[1].componentObject.query(request)

        self.assertEqual(len(props1), 1)
        self.assertEqual(props1[0].id, 'pid')
        self.assertEqual(len(props2), 1)
        self.assertEqual(props2[0].id, 'pid')
        self.assertEqual(from_any(props1[0].value), from_any(props2[0].value))
