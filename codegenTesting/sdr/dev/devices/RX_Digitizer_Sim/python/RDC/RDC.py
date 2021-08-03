#!/usr/bin/env python3
#
#
# AUTO-GENERATED
#
# Source: None
from ossie.device import start_device
import logging

from frontend.tuner_device import validateRequestSingle, floatingPointCompare, validateRequestVsRFInfo

from .RDC_base import *

from data_generator import DataGenerator

class RDC_i(RDC_base):
    """<DESCRIPTION GOES HERE>"""
    MINFREQ = 50000000.0
    MAXFREQ = 3000000000.0
    AVAILABLE_BW_SR = ((2000000,2500000.0,8),(4000000,5000000.0,4),(8000000,10000000.0,2))

    def constructor(self):
        """
        This is called by the framework immediately after your device registers with the system.
        
        In general, you should add customization here and not in the __init__ constructor.  If you have 
        a custom port implementation you can override the specific implementation here with a statement
        similar to the following:
          self.some_port = MyPortImplementation()

        For a tuner device, the structure frontend_tuner_status needs to match the number
        of tuners that this device controls and what kind of device it is.
        The options for devices are: ANTENNA, RX, RX_ARRAY, DBOT, ABOT, ARDC, RDC, SRDC, DRDC, TX, TX_ARRAY, TDC
        
        An example of setting up this device as an ABOT would look like this:

        self.addChannels(1, "ABOT")
     
        The incoming request for tuning contains a string describing the requested tuner
        type. The string for the request must match the string in the tuner status.
        """
        # TODO add customization here.
        self.addChannels(1, "RDC")
        self.datagenerator = DataGenerator(self.port_dataShort_out)

        self.rfinfo = self._createEmptyRFInfo()

        self.datagenerator.keyword_dict['FRONTEND::DEVICE_ID'] = self._get_identifier()
        self.datagenerator.keyword_dict['FRONTEND::RF_FLOW_ID'] = self.rfinfo.rf_flow_id
        self.datagenerator.waveform_type = "Sine"

    def allocate(self, alloc_props):
        retval = super(RDC_i, self).allocate(alloc_props)
        #
        # if len(retval) > 0, add data and control ports. For example:
        # retval[0].data_ports = [CF.Device.PortDescription(self.port_dataShort_out._this(), 'dataShort_out', 'IDL:BULKIO/dataShort:1.0')]
        # retval[0].control_ports = [CF.Device.PortDescription(self.port_DigitalTuner_in._this(), 'DigitalTuner_in', 'IDL:FRONTEND/DigitalTuner:1.0')]
        #
        return retval

    def process(self):
        """
        Basic functionality:
        
            The process method should process a single "chunk" of data and then return. This method
            will be called from the processing thread again, and again, and again until it returns
            FINISH or stop() is called on the device.  If no work is performed, then return NOOP.
            
        StreamSRI:
            To create a StreamSRI object, use the following code (this generates a normalized SRI that does not flush the queue when full):
                sri = bulkio.sri.create("my_stream_id")

            To create a StreamSRI object based on tuner status structure index 'idx' and collector center frequency of 100:
                sri = frontend.sri.create("my_stream_id", self.frontend_tuner_status[idx], self._id, collector_frequency=100)

        PrecisionUTCTime:
            To create a PrecisionUTCTime object, use the following code:
                tstamp = bulkio.timestamp.now() 
  
        Ports:

            Each port instance is accessed through members of the following form: self.port_<PORT NAME>
            
            Data is passed to the serviceFunction through by reading from input streams
            (BulkIO only). UDP multicast (dataSDDS and dataVITA49) ports do not support
            streams.

            The input stream from which to read can be requested with the getCurrentStream()
            method. The optional argument to getCurrentStream() is a floating point number that
            specifies the time to wait in seconds. A zero value is non-blocking. A negative value
            is blocking.  Constants have been defined for these values, bulkio.const.BLOCKING and
            bulkio.const.NON_BLOCKING.

            More advanced uses of input streams are possible; refer to the REDHAWK documentation
            for more details.

            Input streams return data blocks that include the SRI that was in effect at the time
            the data was received, and the time stamps associated with that data.

            To send data using a BulkIO interface, create an output stream and write the
            data to it. When done with the output stream, the close() method sends and end-of-
            stream flag and cleans up.

            If working with complex data (i.e., the "mode" on the SRI is set to 1),
            the data block's complex attribute will return True. Data blocks provide a
            cxdata attribute that gives the data as a list of complex values:

                if block.complex:
                    outData = [val.conjugate() for val in block.cxdata]
                    outputStream.write(outData, block.getStartTime())

            Interactions with non-BULKIO ports are left up to the device developer's discretion.

        Messages:
    
            To receive a message, you need (1) an input port of type MessageEvent, (2) a message prototype described
            as a structure property of kind message, (3) a callback to service the message, and (4) to register the callback
            with the input port.
        
            Assuming a property of type message is declared called "my_msg", an input port called "msg_input" is declared of
            type MessageEvent, create the following code:
        
            def msg_callback(self, msg_id, msg_value):
                print msg_id, msg_value
        
            Register the message callback onto the input port with the following form:
            self.port_input.registerMessage("my_msg", RDC_i.MyMsg, self.msg_callback)
        
            To send a message, you need to (1) create a message structure, and (2) send the message over the port.
        
            Assuming a property of type message is declared called "my_msg", an output port called "msg_output" is declared of
            type MessageEvent, create the following code:
        
            msg_out = RDC_i.MyMsg()
            self.port_msg_output.sendMessage(msg_out)

    Accessing the Application and Domain Manager:
    
        Both the Application hosting this Component and the Domain Manager hosting
        the Application are available to the Component.
        
        To access the Domain Manager:
            dommgr = self.getDomainManager().getRef()
        To access the Application:
            app = self.getApplication().getRef()
        Properties:
        
            Properties are accessed directly as member variables. If the property name is baudRate,
            then accessing it (for reading or writing) is achieved in the following way: self.baudRate.

            To implement a change callback notification for a property, create a callback function with the following form:

            def mycallback(self, id, old_value, new_value):
                pass

            where id is the property id, old_value is the previous value, and new_value is the updated value.
            
            The callback is then registered on the component as:
            self.addPropertyChangeListener('baudRate', self.mycallback)

        Logging:

            The member _baseLog is a logger whose base name is the component (or device) instance name.
            New logs should be created based on this logger name.

            To create a new logger,
                my_logger = self._baseLog.getChildLogger("foo")

            Assuming component instance name abc_1, my_logger will then be created with the 
            name "abc_1.user.foo".
            
        Allocation:
            
            Allocation callbacks are available to customize a Device's response to an allocation request. 
            Callback allocation/deallocation functions are registered using the setAllocationImpl function,
            usually in the initialize() function
            For example, allocation property "my_alloc" can be registered with allocation function 
            my_alloc_fn and deallocation function my_dealloc_fn as follows:
            
            self.setAllocationImpl("my_alloc", self.my_alloc_fn, self.my_dealloc_fn)
            
            def my_alloc_fn(self, value):
                # perform logic
                return True # successful allocation
            
            def my_dealloc_fn(self, value):
                # perform logic
                pass
            
        Example:
        
            # This example assumes that the device has two ports:
            #   - A provides (input) port of type bulkio.InShortPort called dataShort_in
            #   - A uses (output) port of type bulkio.OutFloatPort called dataFloat_out
            # The mapping between the port and the class is found in the device
            # base class.
            # This example also makes use of the following Properties:
            #   - A float value called amplitude
            #   - A boolean called increaseAmplitude
            
            inputStream = self.port_dataShort_in.getCurrentStream()
            if not inputStream:
                return NOOP

            outputStream = self.port_dataFloat_out.getStream(inputStream.streamID)
            if not outputStream:
                outputStream = self.port_dataFloat_out.createStream(inputStream.sri)

            block = inputStream.read()
            if not block:
                if inputStream.eos():
                    outputStream.close()
                return NOOP

            if self.increaseAmplitude:
                scale = self.amplitude
            else:
                scale = 1.0
            outData = [float(val) * scale for val in block.data]

            if block.sriChanged:
                outputStream.sri = block.sri

            outputStream.write(outData, block.getStartTime())
            return NORMAL
            
        """

        # TODO fill in your code here
        self._baseLog.debug("process() example log message")
        return NOOP

    '''
    *************************************************************
    Functions servicing the tuner control port
    *************************************************************'''
    def getTunerType(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].tuner_type

    def getTunerDeviceControl(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        if self.getControlAllocationId(idx) == allocation_id:
            return True
        return False

    def getTunerGroupId(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].group_id

    def getTunerRfFlowId(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].rf_flow_id


    def setTunerCenterFrequency(self,allocation_id, freq):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        if allocation_id != self.getControlAllocationId(idx):
            raise FRONTEND.FrontendException(("ID "+str(allocation_id)+" does not have authorization to modify the tuner"))
        if freq<0: raise FRONTEND.BadParameterException("Center frequency cannot be less than 0")
        # set hardware to new value. Raise an exception if it's not possible
        self.frontend_tuner_status[idx].center_frequency = freq

    def getTunerCenterFrequency(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].center_frequency

    def setTunerBandwidth(self,allocation_id, bw):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        if allocation_id != self.getControlAllocationId(idx):
            raise FRONTEND.FrontendException(("ID "+str(allocation_id)+" does not have authorization to modify the tuner"))
        if bw<0: raise FRONTEND.BadParameterException("Bandwidth cannot be less than 0")
        # set hardware to new value. Raise an exception if it's not possible
        self.frontend_tuner_status[idx].bandwidth = bw

    def getTunerBandwidth(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].bandwidth

    def setTunerAgcEnable(self,allocation_id, enable):
        raise FRONTEND.NotSupportedException("setTunerAgcEnable not supported")

    def getTunerAgcEnable(self,allocation_id):
        raise FRONTEND.NotSupportedException("getTunerAgcEnable not supported")

    def setTunerGain(self,allocation_id, gain):
        raise FRONTEND.NotSupportedException("setTunerGain not supported")

    def getTunerGain(self,allocation_id):
        raise FRONTEND.NotSupportedException("getTunerGain not supported")

    def setTunerReferenceSource(self,allocation_id, source):
        raise FRONTEND.NotSupportedException("setTunerReferenceSource not supported")

    def getTunerReferenceSource(self,allocation_id):
        raise FRONTEND.NotSupportedException("getTunerReferenceSource not supported")

    def setTunerEnable(self,allocation_id, enable):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        if allocation_id != self.getControlAllocationId(idx):
            raise FRONTEND.FrontendException(("ID "+str(allocation_id)+" does not have authorization to modify the tuner"))
        # set hardware to new value. Raise an exception if it's not possible
        self.frontend_tuner_status[idx].enabled = enable

    def getTunerEnable(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].enabled


    def setTunerOutputSampleRate(self,allocation_id, sr):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        if allocation_id != self.getControlAllocationId(idx):
            raise FRONTEND.FrontendException(("ID "+str(allocation_id)+" does not have authorization to modify the tuner"))
        if sr<0: raise FRONTEND.BadParameterException("Sample rate cannot be less than 0")
        # set hardware to new value. Raise an exception if it's not possible
        self.frontend_tuner_status[idx].sample_rate = sr

    def getTunerOutputSampleRate(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].sample_rate

    def configureTuner(self, allocation_id, tunerSettings):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        if allocation_id != self.getControlAllocationId(idx):
            raise FRONTEND.FrontendException(("ID "+str(allocation_id)+" does not have authorization to modify the tuner"))
        # set provided tuner settings

    def getTunerSettings(self, allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        # return tuner settings in a CF.DataType sequence
        return []

    '''
    *************************************************************
    Functions servicing the RFInfo port(s)
    - port_name is the port over which the call was received
    *************************************************************'''
    def get_rf_flow_id(self,port_name):
        return ""

    def set_rf_flow_id(self,port_name, _id):
        pass

    def _createEmptyRFInfo(self):
        _antennainfo=FRONTEND.AntennaInfo('','','','')
        _freqrange=FRONTEND.FreqRange(0,0,[])
        _feedinfo=FRONTEND.FeedInfo('','',_freqrange)
        _sensorinfo=FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
        _range=FRONTEND.Range(0.0,0.0,[])
        _rfcapabilities=FRONTEND.RFCapabilities(_freqrange,_freqrange,_range,_range)
        _rfinfopkt=FRONTEND.RFInfoPkt('',0.0,0.0,0.0,False,_sensorinfo,[],[],[],_rfcapabilities,[])
        return _rfinfopkt

    def get_rfinfo_pkt(self,port_name):
        _antennainfo=FRONTEND.AntennaInfo('','','','')
        _freqrange=FRONTEND.FreqRange(0,0,[])
        _feedinfo=FRONTEND.FeedInfo('','',_freqrange)
        _sensorinfo=FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
        _rfcapabilities=FRONTEND.RFCapabilities(_freqrange,_freqrange)
        _rfinfopkt=FRONTEND.RFInfoPkt('',0.0,0.0,0.0,False,_sensorinfo,[],_rfcapabilities,[])
        return _rfinfopkt

    def set_rfinfo_pkt(self,port_name, pkt):
        pass

    def convert_rf_to_if(self,rf_freq):
        # Convert freq based on RF/IF of Analog Tuner
        if self.rfinfo.if_center_freq > 0:
            ifoffset = rf_freq - self.rfinfo.rf_center_freq
            if_freq =self.rfinfo.if_center_freq+ifoffset
        else:
            if_freq = rf_freq 

        return float(if_freq)
    
    # Start with smallest possible and see if that can satisfy request.
    # Sending 0 for don't care should pass because we are looking for requested to be less than available
    def findBestBWSR(self,requestedBW,requestedSR):
        for bw,sr,decimation in self.AVAILABLE_BW_SR:
            if bw>= requestedBW and sr>=requestedSR:
                return (bw,sr,decimation)
        return (False,False,False)

    '''
    *************************************************************
    Functions supporting tuning allocation
    *************************************************************'''
    def deviceEnable(self, fts, tuner_id):
        '''
        ************************************************************
        modify fts, which corresponds to self.frontend_tuner_status[tuner_id]
        Make sure to set the 'enabled' member of fts to indicate that tuner as enabled
        ************************************************************'''
        try:
            self.datagenerator.enableDataFlow()
            fts.enabled = True

        except Exception as e:
            self._log.exception("Got exception % s" %str(e))
            return False

        if fts.center_frequency == 112e6:
            print(self.getTunerStatus(fts.allocation_id_csv))

        return True

    def deviceDisable(self,fts, tuner_id):
        '''
        ************************************************************
        modify fts, which corresponds to self.frontend_tuner_status[tuner_id]
        Make sure to reset the 'enabled' member of fts to indicate that tuner as disabled
        ************************************************************'''
        self._log.debug(  "deviceDisable(): Disable the given tuner  *********")
        self.datagenerator.disableDataFlow()
        fts.enabled = False

        return


    def deviceSetTuning(self,request, fts, tuner_id):
        '''
        ************************************************************
        modify fts, which corresponds to self.frontend_tuner_status[tuner_id]
        
        The bandwidth, center frequency, and sampling rate that the hardware was actually tuned
        to needs to populate fts (to make sure that it meets the tolerance requirement. For example,
        if the tuned values match the requested values, the code would look like this:
        
        fts.bandwidth = request.bandwidth
        fts.center_frequency = request.center_frequency
        fts.sample_rate = request.sample_rate
        
        return True if the tuning succeeded, and False if it failed
        ************************************************************'''
        if fts.center_frequency == 111e6:
            raise Exception('bad center frequency')
        
        # Check that allocation can be satisfied
        if self.rfinfo.rf_flow_id !="":
            try:
                validateRequestVsRFInfo(request,self.rfinfo,1)
            except FRONTEND.BadParameterException as e:
                self._log.info("ValidateRequestVsRFInfo Failed: %s" %(str(e)))
                return False
                
        
        #Convert CF based on RFInfo. 
        tuneFreq = self.convert_rf_to_if(request.center_frequency)
        
        # Check the CF

        if not(validateRequestSingle(self.MINFREQ,self.MAXFREQ,tuneFreq)):
            self._log.debug( "Center Freq Does not fit %s, %s, %s" %(tuneFreq, self.MINFREQ , self.MAXFREQ))
            return False         
        # Check the BW/SR
        

        bw,sr,decimation = self.findBestBWSR(request.bandwidth,request.sample_rate)
        if not bw:
            self._log.debug( "Can't Satisfy BW and SR request")
            return False
 
        # Update Tuner Status
        fts.bandwidth = bw
        fts.center_frequency = request.center_frequency
        fts.sample_rate = sr
        fts.decimation = decimation
        print("deviceSetTuning(): 5")
                
        # Setup data Generator and start data for that tuner
        self.datagenerator.stream_id = request.allocation_id
        self.datagenerator.sr = sr
        self.datagenerator.cf = tuneFreq
        self.datagenerator.keyword_dict['FRONTEND::BANDWIDTH'] = bw
        self.datagenerator.keyword_dict['COL_RF'] = request.center_frequency
        self.datagenerator.keyword_dict['CHAN_RF'] = request.center_frequency
        self.datagenerator.start()
        
        print("Done with deviceSetTuning():")
        return True

    def deviceDeleteTuning(self, fts, tuner_id):
        '''
        ************************************************************
        modify fts, which corresponds to self.frontend_tuner_status[tuner_id]
        return True if the tune deletion succeeded, and False if it failed
        ************************************************************'''
        self.datagenerator.stop()
        return True
  

