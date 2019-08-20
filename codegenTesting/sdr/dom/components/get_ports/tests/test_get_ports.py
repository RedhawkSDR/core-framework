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

import ossie.utils.testing
from ossie.utils import sb
from ossie.cf import CF

class ComponentTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the component.
    SPD_FILE = '../get_ports.spd.xml'

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
        self.comp = sb.launch(self.spd_file, impl=self.impl)
    
    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        sb.release()

    def testBasicBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.start()
        self.comp.stop()

    def getPortDefinition(self, name, scd):
        port_def = None
        for port in scd.get_componentfeatures().get_ports().get_uses():
            if port.get_usesname() == name:
                port_def = { 'repid': port.get_repid(),
                             'description': port.get_description(),
                             'direction': CF.PortSet.DIRECTION_USES }
                break

        for port in scd.get_componentfeatures().get_ports().get_provides():
            if port.get_providesname() == name:
                if port_def is not None:
                    # Port was already found in uses ports, so it must be bi-directional
                    port_def['direction'] = CF.PortSet.DIRECTION_BIDIR
                    if not port_def['description']:
                        port_def['description'] = port.get_description()
                else:
                    port_def = { 'repid': port.get_repid(),
                                 'description': port.get_description(),
                                 'direction': CF.PortSet.DIRECTION_PROVIDES }
                break

        return port_def

    def testGetPortSet(self):
        for port in self.comp.getPortSet():
            # Make sure the port object is non-nil
            self.assertNotEquals(port.obj_ptr, None)

            # Look up the XML definition, making sure it's valid
            port_def = self.getPortDefinition(port.name, self.comp._scd)
            self.assertNotEquals(port_def, None)

            # Check that the runtime info matches the XML
            self.assertEquals(port.repid, port_def['repid'])
            description = port_def['description']
            if description is None:
                description = ''
            self.assertEquals(port.description, description)
            self.assertEquals(port.direction, port_def['direction'])

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
