#!/usr/bin/env python

import ossie.utils.testing
from ossie.utils import sb
import frontend
from ossie.cf import CF
from bulkio.bulkioInterfaces import BULKIO
import time
from omniORB import any as _any

class DeviceTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the device.
    SPD_FILE = '../digitizer.spd.xml'

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
        self.comp = sb.launch(self.spd_file, impl=self.impl)
    
    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        sb.release()

    def testBasicBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.start()
        self.comp.stop()
    
    def testQuery(self):
        prop = CF.DataType(id='FRONTEND::tuner_status', value=_any.to_any(None))
        retval = self.comp.query([prop])
        self.assertEquals(len(retval[0].value._v), 3)

        retval = self.comp.query([])
        for entry in retval:
            if entry.id == 'FRONTEND::tuner_status':
                self.assertEquals(len(entry.value._v), 3)

    def testAlloc(self):
        alloc=frontend.createTunerAllocation(tuner_type='RDC', returnDict=False)
        retval=self.comp.allocate([alloc])
        data_sink = sb.DataSink()
        data_port = retval[0].data_ports[0].port_ref._narrow(BULKIO.UsesPortStatisticsProvider)
        data_port.connectPort(data_sink.getPort('shortIn'), 'connection_id')
        time.sleep(1)
        data = data_sink.getData()
        self.assertTrue(len(data)>0)


if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
