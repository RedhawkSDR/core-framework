
from ossie.cf import CF, CF__POA
from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA
import time

def createCPUTimestamp():
    """
    Generates a BULKIO.PrecisionUTCTime object using the current
    CPU time that you can use in the pushPacket call
    """
    ts = time.time()
    return BULKIO.PrecisionUTCTime(BULKIO.TCM_CPU,
                                   BULKIO.TCS_VALID, 0.0,
                                   int(ts), ts - int(ts))

def addUTCTime(time_1, time_2):
    '''
        Return a UTCTime (or PrecisionUTCTime) value incremented by time_2
    '''
    fulltime = (time_1.twsec+time_1.tfsec) + float(time_2)
    if isinstance(time_1, CF.UTCTime):
        retval = rhtime.now()
    else:
        retval = createCPUTimestamp()
    retval.twsec = int(fulltime)
    retval.tfsec = fulltime - retval.twsec
    return retval

def subUTCTime(time_1, time_2):
    '''
        Return a UTCTime (or PrecisionUTCTime) value reduced by time_2
    '''
    fulltime = (time_1.twsec+time_1.tfsec) - float(time_2)
    if isinstance(time_1, CF.UTCTime):
        retval = rhtime.now()
    else:
        retval = createCPUTimestamp()
    retval.twsec = int(fulltime)
    retval.tfsec = fulltime - retval.twsec
    return retval

def diffUTCTime(time_1, time_2):
    '''
        Return the difference in time between 2 UTCTime (or PrecisionUTCTime) values
    '''
    retval = (time_1.twsec+time_1.tfsec) - (time_2.twsec+time_2.tfsec)
    return retval

class BaseStream(object):
    def __init__(self, parent, sri):
        self._sri = sri
        self._parent = parent
    def sri(self):
        return self._sri

class DataBlock(object):
    def __init__(self, sri, data, tstamps, new_sri, flushed=False):
        self._sri = sri
        self._data = data
        self._new_sri = new_sri
        self._tstamps = tstamps
        self._flushed = flushed
    def data(self):
        return self._data
    def sri(self):
        return self._sri
    def xdelta(self):
        return self._sri.xdelta
    def sriChanged(self):
        return self._new_sri
    def inputQueueFlushed(self):
        return self._flushed
    def getStartTime(self):
        return self._tstamps[0][1]
    def getTimestamps(self):
        return self._tstamps
    def getNetTimeDrift(self):
        if len(self._tstamps) == 1:
            return 0
        diff_time = diffUTCTime(self._tstamps[-1][1], self._tstamps[0][1])
        synth_time = self._sri.xdelta * len(self._data)
        return abs(diff_time - synth_time)
    def getMaxTimeDrift(self):
        max_drift = 0
        for _diff in range(len(self._tstamps)-1):
            diff_time = diffUTCTime(self._tstamps[_diff][1], self._tstamps[_diff-1][1])
            synth_time = self._sri.xdelta * self._tstamps[_diff][0]-self._tstamps[_diff-1][0]
            drift = abs(diff_time - synth_time)
            if drift > max_drift:
                max_drift = drift
        return max_drift

class _dataUnit(object):
    def __init__(self, data, T, valid):
        self._data = data
        self._T = T
        self._valid = valid
        self._creation = time.time()
    def getData(self):
        return self._data
    def delData(self, begin=0, end=-1):
        if type(self._data) == str:
            if end == -1:
                end = len(_data)
            self._data = self._data[:begin]+self._data[end:]
        else:
            del self._data[begin:end]
        self._valid = False
    def getCreateTime(self):
        return self._creation
    def getTstamp(self):
        return self._T
    def updateTstamp(self, offset):
        if self._T:
            self._T = addUTCTime(self._T, offset)
    def cleanup(self, count, xdelta):
        self.delData(end=count)
        self.updateTstamp(count * xdelta)
    def getValidTstamp(self):
        return self._valid

class _sriUnit(object):
    def __init__(self, sri, offset):
        self._offset = offset
        self._sri = sri
    def getOffset(self):
        return self._offset
    def changeOffset(self, delta):
        self._offset = self._offset - delta
    def getSri(self):
        return self._sri
    def getStreamID(self):
        return self._sri.streamID

class StreamMgr(object):
    def __init__(self):
        self._streams = []
        self._livingStreams = {}

    def getStream(self, streamID):
        for _stream in self._streams:
            if _stream.getStreamID() == streamID:
                return _stream
        for _stream in self._livingStreams:
            if self._livingStreams[_stream].getStreamID() == streamID:
                return self._livingStreams[_stream]
        return None

    def getStreams(self):
        retval = []
        for _stream in self._streams:
            retval.append(_stream)
        for _stream in self._livingStreams:
            retval.append(self._livingStreams[_stream])
        return retval

    def getCurrentStream(self, timeout=-1):
        if len(self._streams) != 0:
            return self._streams[0]
        streams_with_data = []
        start_time = time.time()
        while True:
            for _stream in self._livingStreams:
                if self._livingStreams[_stream].samplesAvailable() != 0:
                    streams_with_data.append(_stream)
            if len(streams_with_data) == 0:
                if timeout != -1:
                    if time.time() - start_time >= timeout:
                        return None
                time.sleep(0.1)
                continue
            else:
                break
        oldest = (0,'')
        for _stream in streams_with_data:
            if oldest[0] == 0:
                oldest = (_stream, self._livingStreams[_stream].getOldestCreateTime())
                continue
            if self._livingStreams[_stream].getOldestCreateTime() < oldest[1]:
                oldest = (_stream, self._livingStreams[_stream].getOldestCreateTime())
        return self._livingStreams[oldest[0]]

    def _removeStream(self, stream):
        for stream_idx in range(len(self._streams)):
            if self._streams[stream_idx] == stream:
                self._streams.pop(stream_idx)
                break

class _notificationList(list):
    def __init__(self, *args):
        list.__init__(self, *args)
        self._popCallback = self.defaultCallback
    def defaultCallback(self):
        pass
    def addPopCallback(self, fn):
        self._popCallback = fn
    def pop(self, arg):
        retval = list.pop(self, arg)
        self._popCallback()
        return retval

class InputStream(BaseStream):
    def __init__(self, parent, sri):
        BaseStream.__init__(self, parent, sri)
        self._enabled = True
        self._data = _notificationList()
        self._sri_idx = []
        self._eos = False
        self._new_sri = True
        self._flushed = False

    def streamID(self):
        return self._sri.streamID

    def _updateSRI(self, sri):
        # this sri applies to whatever packet is delivered next
        if len(self._data) == 0:
            self._new_sri = True
            self._sri = sri
            return
        self._sri_idx.append(_sriUnit(sri, len(self._data)))

    def _flushData(self):
        self._parent.port_cond.acquire()
        try:
            self._data = []
            self._flushed = True
        finally:
            self._parent.port_cond.release()

    def setDataPopCallback(self, fn):
        self._data.addPopCallback(fn)

    def _updateData(self, data, T, EOS):
        self._parent.port_cond.acquire()
        try:
            if (T == None) or ((len(data) != 0) and (T.tcstatus != BULKIO.TCS_INVALID)):
                self._data.append(_dataUnit(data, T, True))
            if EOS:
                self._eos = True
                self._parent.port_cond.notifyAll()
        finally:
            self._parent.port_cond.release()

    def getOldestCreateTime(self):
        if len(self._data) != 0:
            return self._data[0].getCreateTime()
        else:
            return None

    def _dataCurrSri(self):
        upper_end = len(self._data)
        if len(self._sri_idx) != 0:
            upper_end = self._sri_idx[0].getOffset()
        total_data = 0
        for total_data_idx in range(upper_end):
            total_data += len(self._data[total_data_idx].getData())
        return total_data

    def _packetRemoved(self):
        '''
        this function exists to inform any class that overloads this class that the self._data queue has been decremented by 1
        '''
        pass

    def read(self, count=None, consume=None, blocking=True):
        '''
            Blocking read from the data buffer.
            count: number of items read. All available if None
            consumer: how far to move the read pointer. Move by count if None

            Note: if a new SRI was received, read all data until the new SRI
        '''
        # if the amount of data left is 0, erase this from parent._streams
        if len(self._data) == 0 and self._eos:
            self._parent._removeStream(self)
            return None

        if not count:
            self._parent.port_cond.acquire()
            count = self.samplesAvailableSinglePush()
            self._parent.port_cond.release()
        if not consume:
            consume = count

        while True:
            # is there enough data for this sri?
            # is there enough in the first packet?
            try:
             self._parent.port_cond.acquire()
             if len(self._data) != 0:
              if len(self._sri_idx) == 0:
                if self._dataCurrSri() < count:
                    continue
                ret_data = []
                tstamps = []
                total_read = actual_read = data_idx = curr_idx = consume_count = 0
                consume_count = consume
                while total_read < count:
                    actual_read = len(self._data[data_idx].getData())
                    if total_read + actual_read > count:
                        actual_read = count - total_read
                    ret_data += self._data[data_idx].getData()[:actual_read]
                    tstamps += [(curr_idx, self._data[data_idx].getTstamp())]
                    curr_idx += len(self._data[data_idx].getData())
                    total_read += actual_read
                    consume_now = actual_read
                    if consume_now > consume_count:
                        consume_now = consume_count
                    consume_count -= consume_now
                    self._data[data_idx].cleanup(consume_now, self._sri.xdelta)
                    data_idx += 1
                if self._sri.subsize != 0:
                    framelength = self._sri.subsize if not self._sri.mode else 2 * self._sri.subsize
                    if float(len(ret_data))/framelength != len(ret_data)/framelength:
                        print 'The data length ('+str(len(ret_data))+') divided by subsize ('+str(self._sri.subsize)+')is not a whole number'
                        return None
                    _ret_data = []
                    for idx in range(len(ret_data)/framelength):
                        _ret_data.append(ret_data[idx*framelength:(idx+1)*framelength])
                    ret_data = _ret_data
                ret_block = DataBlock(self._sri, ret_data, tstamps, self._new_sri, self._flushed)
                if self._flushed:
                    self._flushed = False
                self._new_sri = False
                while True:
                    if len(self._data) == 0:
                        break
                    if len(self._data[0].getData()) == 0:
                        self._data.pop(0)
                    else:
                        break
                return ret_block
              # are there other sri in the queue?
              else:
                ret_data = []
                tstamps = []
                curr_idx = 0
                number_data = self._sri_idx[0].getOffset()
                total_data = self._dataCurrSri()
                if total_data <= count:
                    # return it all for the current sri and queue up the next sri
                    for data_idx in range(number_data):
                        ret_data += self._data[data_idx].getData()
                        tstamps += [(curr_idx, self._data[data_idx].getTstamp())]
                        curr_idx += len(self._data[data_idx].getData())
                    number_pop = 0
                    consume_count = consume
                    while consume_count != 0:
                        consume_now = len(self._data[0].getData())
                        if consume_now <= consume_count:
                            self._data.pop(0)
                            consume_count -= consume_now
                            number_pop += 1
                            continue
                        self._data[0].cleanup(consume_count, self._sri.xdelta)
                        consume_count = 0
                    for _item_sri_idx in self._sri_idx:
                        _item_sri_idx.changeOffset(number_pop)
                    if self._sri.subsize != 0:
                        framelength = self._sri.subsize if not self._sri.mode else 2 * self._sri.subsize
                        if float(len(ret_data))/framelength != len(ret_data)/framelength:
                            print 'The data length ('+str(len(ret_data))+') divided by subsize ('+str(self._sri.subsize)+')is not a whole number'
                            return None
                        _ret_data = []
                        for idx in range(len(ret_data)/framelength):
                            _ret_data.append(ret_data[idx*framelength:(idx+1)*framelength])
                        ret_data = _ret_data
                    ret_block = DataBlock(self._sri, ret_data, tstamps, self._new_sri, self._flushed)
                    if self._flushed:
                        self._flushed = False
                    self._new_sri = True
                    self._sri = self._sri_idx.pop(0).getSri()
                    return ret_block
                else:
                    # find a subset
                    total_read = actual_read = 0
                    for data_idx in range(number_data):
                        actual_read = len(self._data[data_idx].getData())
                        if total_read + actual_read > count:
                            actual_read = count - total_read
                        ret_data += self._data[data_idx].getData()[:actual_read]
                        tstamps += [(curr_idx, self._data[data_idx].getTstamp())]
                        curr_idx += len(self._data[data_idx].getData())
                        total_read += actual_read
                        if total_read == count:
                            break
                    number_pop = 0
                    consume_count = consume
                    while consume_count != 0:
                        consume_now = len(self._data[0].getData())
                        if consume_now <= consume_count:
                            self._data.pop(0)
                            consume_count -= consume_now
                            number_pop += 1
                            continue
                        self._data[0].cleanup(consume_count, self._sri.xdelta)
                        consume_count = 0
                    for _item_sri_idx in self._sri_idx:
                        _item_sri_idx.changeOffset(number_pop)
                    if self._sri.subsize != 0:
                        framelength = self._sri.subsize if not self._sri.mode else 2 * self._sri.subsize
                        if float(len(ret_data))/framelength != len(ret_data)/framelength:
                            print 'The data length ('+str(len(ret_data))+') divided by subsize ('+str(self._sri.subsize)+')is not a whole number'
                            return None
                        _ret_data = []
                        for idx in range(len(ret_data)/framelength):
                            _ret_data.append(ret_data[idx*framelength:(idx+1)*framelength])
                        ret_data = _ret_data
                    ret_block = DataBlock(self._sri, ret_data, tstamps, self._new_sri)
                    self._new_sri = False
                    return ret_block
            # this sleep happens if len(self._sri_idx) == 0 and total_data < count
            finally:
             self._parent.port_cond.release()
            if not blocking:
                break
            time.sleep(0.1)
        return None

    def tryread(self, count=None, consume=None):
        '''
            Non-blocking read from the data buffer.
            count: number of items read. All available if None
            consumer: how far to move the read pointer. Move by count if None

            Note: if a new SRI was received, read all data until the new SRI
        '''
        return self.read(count, consume, False)

    def skip(self, count=None):
        '''
            Move forward the read pointer by count or the next SRI, whichever comes first.
            Move to next SRI if None
            Returns the number of skipped elements
        '''
        number_pop = 0
        consume_count = count
        total_consumed = 0
        while consume_count != 0:
            consume_now = len(self._data[0].getData())
            if consume_now <= consume_count:
                self._data.pop(0)
                consume_count -= consume_now
                total_consumed += consume_now
                for _item_sri_idx in self._sri_idx:
                    _item_sri_idx.changeOffset(1)
                if len(self._sri_idx) != 0:
                    if self._sri_idx[0].getOffset() <= 0:
                        self._sri = self._sri_idx[0].getSri()
                        self._new_sri = True
                        self._sri_idx.pop(0)
                        break
                continue
            total_consumed += consume_count
            self._data[0].cleanup(consume_count, self._sri.xdelta)
            consume_count = 0
        return total_consumed
    def ready(self):
        return self.samplesAvailable() > 0
    def dataEstimate(self):
        len_data = tstamps = 0
        for _data in self._data:
            len_data += len(_data.getData())
            tstamps += 1
        return (len_data,tstamps)
    def samplesAvailable(self):
        len_data = 0
        for _data in self._data:
            len_data += len(_data.getData())
        return len_data
    def samplesAvailableSinglePush(self):
        return len(self._data[0].getData())
    def samplesAvailableSingleSRI(self):
        len_data = 0
        data_block_reads = len(self._data)
        if len(self._sri_idx) != 0:
            data_block_reads = self._sri_idx[0].getOffset()
        for _data_idx in range(data_block_reads):
            len_data += len(self._data[_data_idx].getData())
        return len_data
    def enable(self):
        '''
            Do not drop incoming data
        '''
        self._enabled = True
    def disable(self):
        '''
            Drop all incoming data
        '''
        self._enabled = False
    def enabled(self):
        return self._enabled
    def eos(self):
        '''
            Has EOS been received?
        '''
        if len(self._data) == 0 and self._eos:
            self._parent._removeStream(self)
        return self._eos
