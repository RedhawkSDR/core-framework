#!/usr/bin/env python
#
#
# AUTO-GENERATED
#
# Source: RX_Digitizer_Sim.spd.xml
from ossie.device import start_device
import logging
from frontend.tuner_device import validateRequestSingle, floatingPointCompare, validateRequestVsRFInfo
from redhawk.frontendInterfaces import FRONTEND

from data_generator import DataGenerator

from RX_Digitizer_Sim_base import *

class RX_Digitizer_Sim_i(RX_Digitizer_Sim_base):
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
        The options for devices are: TX, RX, RX_DIGITIZER, CHANNELIZER, DDC, RC_DIGITIZER_CHANNELIZER
     
        For example, if this device has 5 physical
        tuners, each an RX_DIGITIZER, then the code in the construct function should look like this:

        self.setNumChannels(5, "RX_DIGITIZER");
     
        The incoming request for tuning contains a string describing the requested tuner
        type. The string for the request must match the string in the tuner status.
        """
        # TODO add customization here.
        self.setNumChannels(2, "RX_DIGITIZER");
        
        self.datagenerators =[]
        self.datagenerators.append(DataGenerator(self.port_dataShort_out))
        self.datagenerators.append(DataGenerator(self.port_dataShort_out))
        
        self.rfinfo = self._createEmptyRFInfo()
        
        for datagenerator in self.datagenerators:
            datagenerator.keyword_dict['FRONTEND::DEVICE_ID'] = self._get_identifier()
            datagenerator.keyword_dict['FRONTEND::RF_FLOW_ID'] = self.rfinfo.rf_flow_id
            datagenerator.waveform_type = "Sine"
        
    def process(self):
        # TODO fill in your code here
        self._log.debug("process() example log message")
        return NOOP

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
        print "deviceEnable(): Enable the given tuner  *********"
        try:
            self.datagenerators[tuner_id].enableDataFlow()
            fts.enabled = True

        except Exception, e:
            self._log.exception("Got exception % s" %str(e))
            return False

        if fts.center_frequency == 112e6:
            print self.getTunerStatus(fts.allocation_id_csv)

        return True

    def deviceDisable(self,fts, tuner_id):
        '''
        ************************************************************
        modify fts, which corresponds to self.frontend_tuner_status[tuner_id]
        Make sure to reset the 'enabled' member of fts to indicate that tuner as disabled
        ************************************************************'''
        self._log.debug(  "deviceDisable(): Disable the given tuner  *********")
        self.datagenerators[tuner_id].disableDataFlow()
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
        self._log.debug( "deviceSetTuning(): Evaluate whether or not a tuner is added  *********")
        
        self._log.debug(  "allocating tuner_id %s" % tuner_id)
        
        if fts.center_frequency == 111e6:
            raise Exception('bad center frequency')
        
        # Check that allocation can be satisfied
        if self.rfinfo.rf_flow_id !="":
            try:
                validateRequestVsRFInfo(request,self.rfinfo,1)
            except FRONTEND.BadParameterException , e:
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
        print "deviceSetTuning(): 5"
        #Update output multiPort to add this allocation. Make Allocation ID the same as StreamID
        self.matchAllocationIdToStreamId(request.allocation_id, request.allocation_id,"dataShort_out")
                
        # Setup data Generator and start data for that tuner
        self.datagenerators[tuner_id].stream_id = request.allocation_id
        self.datagenerators[tuner_id].sr = sr
        self.datagenerators[tuner_id].cf = tuneFreq
        self.datagenerators[tuner_id].keyword_dict['FRONTEND::BANDWIDTH'] = bw
        self.datagenerators[tuner_id].keyword_dict['COL_RF'] = request.center_frequency
        self.datagenerators[tuner_id].keyword_dict['CHAN_RF'] = request.center_frequency
        self.datagenerators[tuner_id].start()
        
        print "Done with deviceSetTuning():"
        return True
        

    def deviceDeleteTuning(self, fts, tuner_id):
        '''
        ************************************************************
        modify fts, which corresponds to self.frontend_tuner_status[tuner_id]
        return True if the tune deletion succeeded, and False if it failed
        ************************************************************'''
        
        self._log.debug( "deviceDeleteTuning(): Deallocate an allocated tuner  *********")
        self.datagenerators[tuner_id].stop()
        controlAllocationID =  fts.allocation_id_csv.split(',')[0]
        self.removeStreamIdRouting(controlAllocationID, controlAllocationID)
        return True

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
        if freq<0: raise FRONTEND.BadParameterException("Bad CF")
        # set hardware to new value. Raise an exception if it's not possible
        
        #Check Frequency again min/max Range
        #Convert CF based on RFInfo. 
        tuneFreq = self.convert_rf_to_if(freq)
        
        # Check the CF
        if not(validateRequestSingle(self.MINFREQ,self.MAXFREQ,tuneFreq)):
            self._log.debug( "Center Freq Does not fit %s, %s, %s" %(tuneFreq, self.MINFREQ , self.MAXFREQ))
            raise FRONTEND.BadParameterException("Radio Center Freq of %s Does not fit in %s, %s" %(tuneFreq, self.MINFREQ , self.MAXFREQ))
        
        #set tuner new freq
        self.datagenerators[idx].cf = tuneFreq
        self.datagenerators[idx].keyword_dict['COL_RF'] = freq
        self.datagenerators[idx].keyword_dict['CHAN_RF'] = freq
        self.datagenerators[idx].updateandPushSRI()   
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
        if bw<0: raise FRONTEND.BadParameterException("Invalid BW")
        
        newbw,newsr,decimation = self.findBestBWSR(bw,0)
        if not newbw:
            self._log.debug( "Can't Satisfy BW and SR request")
            raise FRONTEND.BadParameterException("Can't Satisfy BW and SR request")
        
        # set hardware to new value. Raise an exception if it's not possible
        self.datagenerators[idx].keyword_dict['FRONTEND::BANDWIDTH'] = newbw
        self.datagenerators[idx].updateandPushSRI()   
        self.frontend_tuner_status[idx].bandwidth = newsr
        self.frontend_tuner_status[idx].bandwidth = newbw
        self.frontend_tuner_status[idx].decimation = decimation

    def getTunerBandwidth(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].bandwidth

    def setTunerAgcEnable(self,allocation_id, enable):
        raise FRONTEND.NotSupportedException("setTunerAgcEnable not supported")

    def getTunerAgcEnable(self,allocation_id):
        raise FRONTEND.NotSupportedException("getTunerAgcEnable not supported")

    def setTunerGain(self,allocation_id, gain):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        if allocation_id != self.getControlAllocationId(idx):
            raise FRONTEND.FrontendException(("ID "+str(allocation_id)+" does not have authorization to modify the tuner"))
        if (gain<0 or gain>10) : raise FRONTEND.BadParameterException("Invalid Gain")
        gain = round(gain,1)
        # magnitude on data generators is 100+gain*10
        self.datagenerators[idx].magnitude = 100+gain*10
        self.frontend_tuner_status[idx].gain = gain

    def getTunerGain(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].gain 
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
        if sr<0: raise FRONTEND.BadParameterException("Invalid SR")

        newbw,newsr,decimation = self.findBestBWSR(0,sr)
        if not newbw:
            self._log.debug( "Can't Satisfy BW and SR request")
            raise FRONTEND.BadParameterException( "Can't Satisfy BW and SR request")
        
        self._log.debug("Setting BW and Sample Rate %s, %s " %(newbw,newsr))
        #set new SR
        self.datagenerators[idx].sr = sr
        self.datagenerators[idx].keyword_dict['FRONTEND::BANDWIDTH'] = newbw     
        self.datagenerators[idx].updateandPushSRI()   
        self.frontend_tuner_status[idx].sample_rate = newsr
        self.frontend_tuner_status[idx].bandwidth = newbw
        self.frontend_tuner_status[idx].decimation = decimation
        
    def getTunerOutputSampleRate(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].sample_rate

    '''
    *************************************************************
    Functions servicing the RFInfo port(s)
    - port_name is the port over which the call was received
    *************************************************************'''
    def get_rf_flow_id(self,port_name):
        return self.rfinfo.rf_flow_id

    def set_rf_flow_id(self,port_name, id):
        pass

    def get_rfinfo_pkt(self,port_name):
        return self.rfinfo

    def _createEmptyRFInfo(self):
        _antennainfo=FRONTEND.AntennaInfo('','','','')
        _freqrange=FRONTEND.FreqRange(0,0,[])
        _feedinfo=FRONTEND.FeedInfo('','',_freqrange)
        _sensorinfo=FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
        _rfcapabilities=FRONTEND.RFCapabilities(_freqrange,_freqrange)
        _rfinfopkt=FRONTEND.RFInfoPkt('',0.0,0.0,0.0,False,_sensorinfo,[],_rfcapabilities,[])
        return _rfinfopkt

    def set_rfinfo_pkt(self,port_name, pkt):
        self.rfinfo = pkt
        for tuner in self.frontend_tuner_status:
            tuner.rf_flow_id=self.rfinfo.rf_flow_id
        for datagenerator in self.datagenerators:
            datagenerator.keyword_dict['FRONTEND::RF_FLOW_ID'] = self.rfinfo.rf_flow_id

    '''
    *************************************************************
     Helper Functions
    *************************************************************'''

    def convert_rf_to_if(self,rf_freq):
        #Convert freq based on RF/IF of Analog Tuner
        
        if self.rfinfo.if_center_freq > 0:
            ifoffset = rf_freq - self.rfinfo.rf_center_freq
            if_freq =self.rfinfo.if_center_freq+ifoffset
        else:
            if_freq = rf_freq 

        self._log.debug("Converted RF Freq of %s to IF Freq %s based on Input RF of %s, IF of %s, and spectral inversion %s" %(rf_freq,if_freq,self.rfinfo.rf_center_freq,self.rfinfo.if_center_freq,self.rfinfo.spectrum_inverted))
        return float(if_freq)
    
    #Start with smallest possible and see if that can satisfy request.
    # Sending 0 for don't care should pass because we are looking for requested to be less than available
    def findBestBWSR(self,requestedBW,requestedSR):
        self._log.debug("findBestBWSR")
        for bw,sr,decimation in self.AVAILABLE_BW_SR:
            self._log.debug("findBestBWSR. Requested: " + str(requestedBW) +" "+ str(requestedSR) + " evaluating: " + str(bw)+" "+ str(sr))
            if bw>= requestedBW and sr>=requestedSR:
                return (bw,sr,decimation)
        return (False,False,False)
  
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.INFO)
    logging.debug("Starting Device")
    start_device(RX_Digitizer_Sim_i)

