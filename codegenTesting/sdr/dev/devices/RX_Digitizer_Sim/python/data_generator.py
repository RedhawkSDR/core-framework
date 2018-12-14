
import Waveform
import math
import time
from bulkio.bulkioInterfaces import BULKIO
from omniORB import any
from ossie.cf import CF
import threading
import bulkio


class DataGenerator(object):
    
    def __init__(self,outputPort,stream_id="stream_id",cf=100e6,sr=10e6):
        self.outputPort = outputPort
        self.stream_id = stream_id
        self.cf = cf
        self.sr = sr
        self._waveform = Waveform.Waveform()
        self.enable = False
        self.phase = 0
        self.chirp = 0
        self.sample_time_delta = 0.0
        self.delta_phase = 0.0
        self.delta_phase_offset = 0.0
        self.magnitude = 100
        self.xfer_len  = 100000 
        self.last_xfer_len = self.xfer_len
        self.next_time = None
        self.throttle = True
        self.terminate = False
        self.thread = None
        self.firstTime = True
        self.keyword_dict = {}
        self.sri = BULKIO.StreamSRI(1, 0.0, 0.0, BULKIO.UNITS_TIME, 0, 0.0, 0.0, BULKIO.UNITS_NONE, 0, self.stream_id, False, [])

    
    def start(self):

        if not self.thread:
            print "Starting Thread"
            self.thread = threading.Thread(target = self._run)
            self.thread.start()
            
        
        
    def stop(self):
        if self.thread:
            print "Stopping Thread"
            self.terminate = True
            self.thread.join(2)
        else:
            print "No Thread to Stop"

    
    def _run(self):
        while True:
            if self.terminate:
                return
            self._push_data()
    
    def enableDataFlow(self):
        self.next_time = bulkio.timestamp.now()
        self.updateandPushSRI()
        self.enable = True 
        
    def disableDataFlow(self):
        print "Disable Data Flow"
        self.enable = False 
        self.outputPort.pushPacket([], self.next_time, True, self.stream_id)
    
    def _push_data(self):
        if self.enable:
  
            self.sample_time_delta = 1.0/self.sr
            
            self.delta_phase = self.cf * self.sample_time_delta
            self.delta_phase_offset = self.chirp * self.sample_time_delta * self.sample_time_delta
            
            data = self._waveform.sincos(self.magnitude, self.phase, self.delta_phase, self.last_xfer_len, 1)
            data = [int(i) for i in data]
            self.phase += self.delta_phase*self.last_xfer_len # increment phase
            self.phase -= math.floor(self.phase) # module 1.0
            
            self.outputPort.pushPacket(data, self.next_time, False, self.stream_id)
            
            # Advance time
            self.next_time.tfsec += self.last_xfer_len * self.sample_time_delta
            if self.next_time.tfsec > 1.0:
                self.next_time.tfsec -= 1.0
                self.next_time.twsec += 1.0
            
            # If we are throttling, wait...otherwise run at full speed
            if self.throttle:
                wait_amt = self.last_xfer_len * self.sample_time_delta
                try:
                    time.sleep(wait_amt)
                finally:
                    return 
        return
    def updateandPushSRI(self):
        self.sri.xdelta =1.0/self.sr
        self.sri.streamID = self.stream_id
        keywords = []
        for keyword in self.keyword_dict.keys():
            keywords.append(CF.DataType(keyword, any.to_any(self.keyword_dict[keyword])))
        self.sri.keywords = keywords
        self.sri.mode = 1
        try:
            self.outputPort.pushSRI(self.sri)
        except Exception, e:
            print "Exception on pushSRI" , str(e)

class TestPort(object):
    
    def pushPacket(self,data,sometime,EOS,stream_id):
        print "Pusing Data of length" , len(data)

if __name__ == "__main__":
    
    aport = TestPort()
    generator = DataGenerator(aport)
    generator.start()
    time.sleep(2)
    generator.enableDataFlow()
    time.sleep(2)
    generator.stop()
    print "complete"
    
    
    
            