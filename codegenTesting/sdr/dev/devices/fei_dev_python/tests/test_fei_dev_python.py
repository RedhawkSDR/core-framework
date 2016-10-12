#!/usr/bin/env python

import ossie.utils.testing
from ossie.utils import sb
import frontend
import redhawk.frontendInterfaces
from omniORB import any as _any
from ossie.cf import CF


class DeviceTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the device.
    SPD_FILE = '../fei_dev_python.spd.xml'

    # setUp is run before every function preceded by "test" is executed
    # tearDown is run after every function preceded by "test" is executed
    
    # self.comp is a device using the sandbox API
    # to create a data source, the package sb contains data sources like DataSource or FileSource
    # to create a data sink, there are sinks like DataSink and FileSink
    # to connect the component to get data from a file, process it, and write the output to a file, use the following syntax:
    #  src = sb.FileSource('myfile.dat')
    #  snk = sb.DataSink()
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
        
        for _port in self.comp.ports:
            if _port.name == 'DigitalTuner_in':
                port = _port
                break
        
        print self.comp.frontend_tuner_status
        self.assertEquals(len(self.comp.frontend_tuner_status),2)
        alloc = frontend.createTunerAllocation(tuner_type='RX_DIGITIZER',allocation_id='hello',center_frequency=100)
        self.assertRaises(redhawk.frontendInterfaces.FRONTEND.FrontendException, port.ref.getTunerEnable, 'hello')
        
        listen_alloc = frontend.createTunerListenerAllocation('hello','hello_listen')
        listen_2_alloc = frontend.createTunerListenerAllocation('hello','hello_2_listen')
        
        retval = self.comp.allocateCapacity(listen_alloc)
        self.assertEquals(retval, False)
        
        retval = self.comp.allocateCapacity(alloc)
        self.assertEquals(retval, True)
        
        retval = self.comp.allocateCapacity(listen_alloc)
        self.assertEquals(retval, True)
        retval = self.comp.allocateCapacity(listen_2_alloc)
        self.assertEquals(retval, True)

        retval = port.ref.getTunerEnable('hello')
        self.assertEquals(retval, True)
        sample_rate = port.ref.getTunerOutputSampleRate('hello')
        self.assertEquals(sample_rate, 1)
        port.ref.setTunerOutputSampleRate('hello',5)
        sample_rate = port.ref.getTunerOutputSampleRate('hello')
        self.assertEquals(sample_rate, 5)
        port.ref.setTunerEnable('hello',False)
        retval = port.ref.getTunerEnable('hello')
        self.assertEquals(retval, False)
        port.ref.setTunerEnable('hello',True)
        retval = port.ref.getTunerEnable('hello')
        self.assertEquals(retval, True)
        _id = port.ref.getTunerRfFlowId('hello')
        self.assertEquals(_id, 'foo')
        
        self.assertEquals(self.comp.frontend_tuner_status[0].enabled, True)
        self.assertEquals(self.comp.frontend_tuner_status[0].center_frequency, 100)
        
        self.assertEquals(len(self.comp.connectionTable), 3)
        
        allocation_id_csv = self.comp.frontend_tuner_status[0].allocation_id_csv
        allocations = allocation_id_csv.split(',')
        self.assertEquals(len(allocations), 3)
        self.assertEquals('hello' in allocations, True)
        self.assertEquals('hello_2_listen' in allocations, True)
        self.assertEquals('hello_listen' in allocations, True)
        
        listen_alloc['FRONTEND::listener_allocation'].pop('FRONTEND::listener_allocation::existing_allocation_id')
        _old_id = [CF.DataType(id='FRONTEND::listener_allocation::listener_allocation_id',value=_any.to_any(listen_alloc['FRONTEND::listener_allocation']['FRONTEND::listener_allocation::listener_allocation_id']))]
        _listen_alloc = [CF.DataType(id='FRONTEND::listener_allocation',value=_any.to_any(_old_id))]
        self.comp.ref.deallocateCapacity(_listen_alloc)
        
        self.assertEquals(self.comp.frontend_tuner_status[0].enabled, True)
        self.assertEquals(self.comp.frontend_tuner_status[0].center_frequency, 100)
        
        allocation_id_csv = self.comp.frontend_tuner_status[0].allocation_id_csv
        allocations = allocation_id_csv.split(',')
        self.assertEquals(len(allocations), 2)
        self.assertEquals('hello' in allocations, True)
        self.assertEquals('hello_2_listen' in allocations, True)
        self.assertEquals('hello_listen' in allocations, False)
        
        self.comp.deallocateCapacity(alloc)
        self.assertEquals(self.comp.frontend_tuner_status[0].enabled, False)
        self.assertEquals(self.comp.frontend_tuner_status[0].center_frequency, 0)

        self.assertEquals(self.comp.frontend_tuner_status[0].enabled, False)
        self.assertEquals(self.comp.frontend_tuner_status[0].center_frequency, 0)
        
        allocation_id_csv = self.comp.frontend_tuner_status[0].allocation_id_csv
        allocations = allocation_id_csv.split(',')
        self.assertEquals(len(allocations), 1)
        self.assertEquals(allocations[0], '')
        self.assertEquals(len(self.comp.connectionTable), 0)

        self.assertRaises(redhawk.frontendInterfaces.FRONTEND.FrontendException, port.ref.getTunerEnable, 'hello')

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
