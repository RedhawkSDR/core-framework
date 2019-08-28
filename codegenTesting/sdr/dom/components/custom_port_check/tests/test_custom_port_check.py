#!/usr/bin/env python

import time
import ossie.utils.testing
from ossie.utils import sb
from ossie.cf import CF, CF__POA

class FilePort(CF__POA.File):
    def __init__(self):
        pass
    def read(self, length):
        return ''
    def write(self, data):
        pass

class PropertyEmitterPort(CF__POA.PropertyEmitter):
    def __init__(self):
        pass
    def registerPropertyListener(self, obj, string_seq, interval):
        return 'hello'

class TestableObjectPort(CF__POA.TestableObject):
    def __init__(self):
        pass
    def runTest(self, test_number, prop_seq):
        return prop_seq

class ComponentTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the component.
    SPD_FILE = '../custom_port_check.spd.xml'

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
        self._filePort = FilePort()
        self._propertyEmitterPort = PropertyEmitterPort()
        self._testableObjectPort = TestableObjectPort()
        self._filePort_2 = FilePort()
        self._propertyEmitterPort_2 = PropertyEmitterPort()
        self._testableObjectPort_2 = TestableObjectPort()
    
    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        sb.release()

    def testBasicBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.start()
        time.sleep(0.5)
        self.comp.stop()
        self.assertEquals(self.comp.inout_state, "No connections available.")
        self.assertEquals(self.comp.retval_state, "No connections available.")
        self.assertEquals(self.comp.out_state, "No connections available.")
        self.assertEquals(self.comp.in_state, "ok")
        self.assertEquals(self.comp.bad_connection, "No connections available.")
        testableobject_port = self.comp.getPort('testableobject_out')
        propertyemitter_port = self.comp.getPort('propertyemitter_out')
        file_port = self.comp.getPort('file_out')
        testableobject_port.connectPort(self._testableObjectPort._this(), 'abc')
        propertyemitter_port.connectPort(self._propertyEmitterPort._this(), 'abc')
        file_port.connectPort(self._filePort._this(), 'abc')
        time.sleep(0.5)
        self.comp.start()
        time.sleep(0.5)
        self.comp.stop()
        self.assertEquals(self.comp.inout_state, "ok")
        self.assertEquals(self.comp.retval_state, "ok")
        self.assertEquals(self.comp.out_state, "ok")
        self.assertEquals(self.comp.in_state, "ok")
        self.assertEquals(self.comp.bad_connection, "The requested connection id (invalid_connectionid) does not exist.Connections available: abc")
        testableobject_port.connectPort(self._testableObjectPort_2._this(), 'def')
        propertyemitter_port.connectPort(self._propertyEmitterPort_2._this(), 'def')
        file_port.connectPort(self._filePort_2._this(), 'def')
        time.sleep(0.5)
        self.comp.start()
        time.sleep(0.5)
        self.comp.stop()
        self.assertEquals(self.comp.inout_state, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.retval_state, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.out_state, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.in_state, "ok")
        self.assertEquals(self.comp.bad_connection, "The requested connection id (invalid_connectionid) does not exist.Connections available: abc, def")

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
