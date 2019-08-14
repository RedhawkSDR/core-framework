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
import frontend, time, os

class DeviceTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the device.
    SPD_FILE = '../eos_test.spd.xml'

    # setUp is run before every function preceded by "test" is executed
    # tearDown is run after every function preceded by "test" is executed
    
    # self.comp is a device using the sandbox API
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
        # Launch the device, using the selected implementation
        self.filename = 'somefile.tst'
        try:
            os.remove(self.filename)
        except:
            pass
        self.fp = open(self.filename,'w')
        self.comp = sb.launch(self.spd_file, impl=self.impl, stdout=self.fp)
    
    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        sb.release()
        try:
            self.fp.close()
        except:
            pass
        try:
            os.remove(self.filename)
        except:
            pass

    def testBasicBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        sb.start()
        alloc = frontend.createTunerAllocation(tuner_type='RX_DIGITIZER',allocation_id='master',center_frequency=100)
        listen_alloc = frontend.createTunerListenerAllocation('master','slave')
        another_listen_alloc = frontend.createTunerListenerAllocation('master','another_slave')
        self.comp.allocateCapacity(alloc)
        self.comp.allocateCapacity(listen_alloc)
        self.comp.allocateCapacity(another_listen_alloc)

        master = sb.StreamSink()
        slave = sb.StreamSink()
        another_slave = sb.StreamSink()
        self.comp.connect(master,connectionId='master')
        self.comp.connect(slave,connectionId='slave')
        self.comp.connect(another_slave,connectionId='another_slave')

        def get_eos(streamSink, **kwargs):
            streamData = streamSink.read(**kwargs)
            if streamData:
                return streamData.eos
            return False

        time.sleep(1)
        self.assertEquals(get_eos(master), False)
        self.assertEquals(get_eos(slave), False)
        self.assertEquals(get_eos(another_slave), False)

        self.comp.deallocateCapacity(listen_alloc)
        self.assertEquals(get_eos(master), False)
        # Save result so we dont call read() twice after eos.
        eos_slave = get_eos(slave, timeout=1, eos=True)
        self.assertEquals(eos_slave, True)
        self.assertEquals(get_eos(another_slave), False)

        self.comp.deallocateCapacity(alloc)
        self.assertEquals(get_eos(master, timeout=1, eos=True), True)
        self.assertEquals(eos_slave, True)
        self.assertEquals(get_eos(another_slave, timeout=1, eos=True), True)

        sb.release()
        self.fp.close()
        self.fp = open(self.filename,'r')
        stuff = self.fp.read()
        self.fp.close()
        self.assertEquals(stuff.find('the application attempted to invoke an operation on a nil reference'), -1)

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
