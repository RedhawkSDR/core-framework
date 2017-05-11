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

from ossie.cf import CF, CF__POA
import struct
import os
import array
import threading
import bulkio_helpers
import time
import logging
from new import classobj
from ossie.utils.redhawk.base import attach
try:
    from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA
except:
    pass
from ossie.utils import _uuid, rhtime

logging.basicConfig()
log = logging.getLogger(__name__)

class ArraySource(object):
    """
    Simple class used to push data into a port from a given array of data.
    """
    def __init__(self, porttype):
        """
        Instantiates a new object and generates a default StreamSRI.  The
        porttype parameter corresponds to the type of data contained in the
        array of data being sent.

        The porttype is also used in the connectPort() method to narrow the
        connection

        """
        self.port_type = porttype
        self.outPorts = {}
        self.refreshSRI = False
        self.sri=bulkio_helpers.defaultSRI
        self.port_lock = threading.Lock()
        self.done = False

    def connectPort(self, connection, connectionId):
        self.port_lock.acquire()
        try:
            port = connection._narrow(self.port_type)
            self.outPorts[str(connectionId)] = port
            self.refreshSRI = True
        finally:
            self.port_lock.release()

    def disconnectPort(self, connectionId):
        self.port_lock.acquire()
        try:
            self.outPorts.pop(str(connectionId), None)
        finally:
            self.port_lock.release()

    def pushSRI(self, H):
        self.port_lock.acquire()
        self.sri = H
        try:
            try:
                for connId, port in self.outPorts.items():
                    if port != None: port.pushSRI(H)
            except Exception, e:
                msg = "The call to pushSRI failed with %s " % e
                msg += "connection %s instance %s" % (connId, port)
                log.warn(msg)
        finally:
            self.port_lock.release()
            self.refreshSRI = False

    def pushPacket(self, data, T, EOS, streamID):
        if self.refreshSRI:
            self.pushSRI(self.sri)
        if EOS: # This deals with subsequent pushes with the same SRI
            self.refreshSRI = True

        self.port_lock.acquire()
        try:
            try:
                for connId, port in self.outPorts.items():
                    if port != None:
                        interface = self.port_type._NP_RepositoryId
                        if interface == 'IDL:BULKIO/dataChar:1.0' or interface == 'IDL:BULKIO/dataOctet:1.0':
                            if len(data) == 0:
                                data = ''
                        port.pushPacket(data, T, EOS, streamID)
            except Exception, e:
                msg = "The call to pushPacket failed with %s " % e
                msg += "connection %s instance %s" % (connId, port)
                log.warn(msg)
        finally:
            self.port_lock.release()

    def getPort(self):
        """
        Returns a Port object of the type CF__POA.Port.
        """
        # The classobj generates a class using the following arguments:
        #
        #    name:        The name of the class to generate
        #    bases:       A tuple containing all the base classes to use
        #    dct:         A dictionary containing all the attributes such as
        #                 functions, and class variables
        PortClass = classobj('PortClass',
                             (CF__POA.Port,),
                             {'connectPort':self.connectPort,
                              'disconnectPort':self.disconnectPort})

        # Create a port using the generate Metaclass and return an instance
        port = PortClass()
        return port._this()

    def run(self, data, sri=None, pktsize=1024, startTime=0.0, sampleRate=1.0, complexData=False, streamID=None):
        """
        Pushes the data through the connected port.  Each packet of data
        contains no more than pktsize elements.  Once all the elements have
        been sent, the method sends an empty list with the EOS set to True to
        indicate the end of the stream.

        Inputs:
            <data>       A list of elements containing the data to push
            <sri>        SRI to send before pushing data
            <pktsize>    The maximum number of elements to send on each push
            <startTime>  The time of the first sample
            <sampleRate> The sample rate of the data used to set xdelta in the SRI
            <complexData>The mode of the data (real=0(False),complex=1(True)) in the SRI
            <streamID>   The streamID of the data
        """
        start = 0           # stores the start of the packet
        end = start         # stores the end of the packet
        sz = len(data)
        self.done = False
        if sri != None:
            self.sri = sri
        else:
            if sampleRate > 0:
                self.sri.xdelta = 1/sampleRate
            if complexData:
                self.sri.mode = 1
            else:
                self.sri.mode = 0
            if streamID != None:
                self.sri.streamID = streamID
            if startTime >= 0.0:
                self.sri.xstart = startTime
        self.pushSRI(self.sri)

        currentSampleTime = self.sri.xstart
        while not self.done:
            chunk = start + pktsize
            # if the next chunk is greater than the file, then grab remaining
            # only, otherwise grab a whole packet size
            if chunk > sz:
                end = sz
                self.done = True
            else:
                end = chunk

            # there are cases when this happens: array has 6 elements and
            # pktsize = 2 (end = length - 1 = 5 and start = 6)
            if start > end:
                self.done = True
                continue

            d = data[start:end]
            start = end

            T = BULKIO.PrecisionUTCTime(BULKIO.TCM_CPU, BULKIO.TCS_VALID, 0.0, int(currentSampleTime), currentSampleTime - int(currentSampleTime))
            self.pushPacket(d, T, False, self.sri.streamID)
            dataSize = len(d)
            if self.sri.mode == 1:
                dataSize = dataSize / 2
            currentSampleTime = currentSampleTime + dataSize/sampleRate
        T = BULKIO.PrecisionUTCTime(BULKIO.TCM_CPU, BULKIO.TCS_VALID, 0.0, int(currentSampleTime), currentSampleTime - int(currentSampleTime))
        self.pushPacket([], T, True, self.sri.streamID)

def addUTCTime(time_1, time_2):
    '''
        Return a UTCTime (or PrecisionUTCTime) value incremented by time_2
    '''
    fulltime = (time_1.twsec+time_1.tfsec) + float(time_2)
    if isinstance(time_1, CF.UTCTime):
        retval = rhtime.now()
    else:
        retval = bulkio_helpers.createCPUTimestamp()
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
        retval = bulkio_helpers.createCPUTimestamp()
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
    def __init__(self, sri, data, tstamps, new_sri):
        self._sri = sri
        self._data = data
        self._new_sri = new_sri
        self._tstamps = tstamps
    def data(self):
        return self._data
    def sri(self):
        return self._sri
    def xdelta(self):
        return self._sri.xdelta
    def sriChanged(self):
        return self._new_sri
    def inputQueueFlushed(self):
        return False
    def getStartTime(self):
        return self._tstamps[0][0]
    def getTimestamps(self):
        return self._tstamps
    def getNetTimeDrift(self):
        if len(self._tstamps) == 1:
            return 0
        diff_time = diffUTCTime(self._tstamps[-1][0], self._tstamps[0][0])
        synth_time = self._sri.xdelta * len(self._data)
        return abs(diff_time - synth_time)
    def getMaxTimeDrift(self):
        max_drift = 0
        for _diff in range(len(self._tstamps)-1):
            diff_time = diffUTCTime(self._tstamps[_diff][0], self._tstamps[_diff-1][0])
            synth_time = self._sri.xdelta * self._tstamps[_diff][1]-self._tstamps[_diff-1][1]
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

class InputStream(BaseStream):
    def __init__(self, parent, sri):
        BaseStream.__init__(self, parent, sri)
        self._enabled = True
        self._data = []
        self._sri_idx = []
        self._eos = False
        self._new_sri = True

    def streamID(self):
        return self._sri.streamID

    def _updateSRI(self, sri):
        # this sri applies to whatever packet is delivered next
        if len(self._data) == 0:
            self._new_sri = True
            self._sri = sri
            return
        self._sri_idx.append(_sriUnit(sri, len(self._data)))
    def _updateData(self, data, T, EOS):
        self._parent.port_cond.acquire()
        try:
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
            count = self.samplesAvailableSingleSRI()
        if not consume:
            consume = count

        while True:
            # is there enough data for this sri?
            # is there enough in the first packet?
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
                    tstamps += [(self._data[data_idx].getTstamp(), curr_idx)]
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
                ret_block = DataBlock(self._sri, ret_data, tstamps, self._new_sri)
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
                        tstamps += [(self._data[data_idx].getTstamp(), curr_idx)]
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
                    ret_block = DataBlock(self._sri, ret_data, tstamps, self._new_sri)
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
                        tstamps += [(self._data[data_idx].getTstamp(), curr_idx)]
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
        return self._eos

class ArraySink(object):
    """
    Simple class used to receive data from a port and store it in a python
    array.
    """
    def __init__(self, porttype):
        """
        Instantiates a new object responsible for writing data from the port
        into an array.

        It is important to notice that the porttype is a BULKIO__POA type and
        not a BULKIO type.  The reason is because it is used to generate a
        Port class that will be returned when the getPort() is invoked.  The
        returned class is the one acting as a server and therefore must be a
        Portable Object Adapter rather and a simple BULKIO object.

        Inputs:
            <porttype>        The BULKIO__POA data type
        """
        self.port_type = porttype
        self.sri=bulkio_helpers.defaultSRI
        self.data = []
        self.timestamps = []
        self._streams = []
        self._livingStreams = {}
        self.gotEOS = False
        self.breakBlock = False
        self.port_lock = threading.Lock()
        self.port_cond = threading.Condition(self.port_lock)

    class estimateStruct():
        len_data=0
        num_timestamps=0
        def __init__(self, data=0, timestamps=0):
            self.len_data = data
            self.num_timestamps = timestamps

    def _isActive(self):
        return not self.gotEOS and not self.breakBlock

    def reset(self):
        if not self._isActive():
            self.gotEOS = False
            self.breakBlock = False

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

    def getCurrentStream(self):
        if len(self._streams) != 0:
            return self._streams[0]
        streams_with_data = []
        for _stream in self._livingStreams:
            if self._livingStreams[_stream].samplesAvailable() != 0:
                streams_with_data.append(_stream)
        if len(streams_with_data) == 0:
            return None
        oldest = (0,'')
        for _stream in streams_with_data:
            if oldest[0] == 0:
                oldest = (_stream, self._livingStreams[_stream].getOldestCreateTime())
                continue
            if self._livingStreams[_stream].getOldestCreateTime() < oldest[1]:
                oldest = (_stream, self._livingStreams[_stream].getOldestCreateTime())
        return self._livingStreams[oldest[0]]

    def start(self):
        self.gotEOS = False
        self.breakBlock = False

    def stop(self):
        self.port_cond.acquire()
        self.breakBlock = True
        self.port_cond.notifyAll()
        self.port_cond.release()

    def eos(self):
        _stream = self.getCurrentStream()
        if _stream:
            return _stream.eos()
        return self.gotEOS

    def waitEOS(self):
        self.port_cond.acquire()
        try:
            while self._isActive():
                if self.eos():
                    break
                self.port_cond.wait()
            return self.gotEOS
        finally:
            self.port_cond.release()

    def _removeStream(self, stream):
        for stream_idx in range(len(self._streams)):
            if self._streams[stream_idx] == stream:
                self._streams.pop(stream_idx)
                break

    def pushSRI(self, H):
        """
        Stores the SteramSRI object regardless that there is no need for it

        Input:
            <H>    The StreamSRI object containing the information required to
                   generate the header file
        """
        self.sri = H
        if not self._livingStreams.has_key(H.streamID):
            self._livingStreams[H.streamID] = InputStream(self, H)
        else:
            self._livingStreams[H.streamID]._updateSRI(H)

    def pushPacket(self, data, ts, EOS, stream_id):
        """
        Appends the data to the end of the array.

        Input:
            <data>        The actual data to append to the array
            <ts>          The timestamp
            <EOS>         Flag indicating if this is the End Of the Stream
            <stream_id>   The unique stream id
        """
        if not self._livingStreams.has_key(stream_id):
            self._livingStreams[stream_id] = InputStream(self, BULKIO.StreamSRI(1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, stream_id, False, []))
        _stream = self._livingStreams[stream_id]
        _stream._updateData(data, ts, EOS)
        if EOS:
            self._streams.append(_stream)
            self._livingStreams.pop(stream_id)
        #self.port_cond.acquire()
        #try:
        #    self.gotEOS = EOS
        #    self.timestamps.append([len(self.data), ts])
        #    self.data += data
        #    self.port_cond.notifyAll()
        #finally:
        #    self.port_cond.release()

        # legacy stuff
        self.data += data

    def estimateData(self):
        self.port_cond.acquire()
        estimate = self.estimateStruct()
        _streams = self.getStreams()
        _total_data = 0
        _total_tstamps = 0
        for _stream in _streams:
            (data_len,tstamp_len) = _stream.dataEstimate()
            _total_data += data_len
            _total_tstamps += tstamp_len
        estimate = self.estimateStruct(_total_data, _total_tstamps)
        self.port_cond.release()
        return estimate

    def __syncAssocData(self, reference):
        retval = []
        length_to_erase = None
        for i,(l,t) in enumerate(reference[::-1]):
            if l > length:
                reference[len(reference)-i-1][0] = l - length
                continue
            if length_to_erase == None:
                length_to_erase = len(reference)-i
            retval.append((l,t))
        if length_to_erase != None:
            del reference[:length_to_erase]
        return retval

    def retrieveData(self, length=None):
        self.port_cond.acquire()
        try:
            retval = []
            rettime = []
            _stream = self.getCurrentStream()
            if not _stream:
                return None
            done = False
            goal = length
            if length == None:
                goal = _stream.samplesAvailable()
            while True:
                _block = _stream.read(count=length)
                if not _block:
                    break
                retval += _block.data()
                rettime += _block.getTimestamps()
                goal_offset = 1
                if _block.sri().subsize != 0:
                    goal_offset = _block.sri().subsize
                if len(retval) == goal/goal_offset:
                    break
        finally:
            self.port_cond.release()
        return (retval, rettime)

    def getPort(self):
        """
        Returns a Port object of the same type as the one specified as the
        porttype argument during the object instantiation.  It uses the
        classobj from the new module to generate a class on runtime.

        The classobj generates a class using the following arguments:

            name:        The name of the class to generate
            bases:       A tuple containing all the base classes to use
            dct:         A dictionary containing all the attributes such as
                         functions, and class variables

        It is important to notice that the porttype is a BULKIO__POA type and
        not a BULKIO type.  The reason is because it is used to generate a
        Port class that will be returned when the getPort() is invoked.  The
        returned class is the one acting as a server and therefore must be a
        Portable Object Adapter rather and a simple BULKIO object.

        """
        # The classobj generates a class using the following arguments:
        #
        #    name:        The name of the class to generate
        #    bases:       A tuple containing all the base classes to use
        #    dct:         A dictionary containing all the attributes such as
        #                 functions, and class variables
        PortClass = classobj('PortClass',
                             (self.port_type,),
                             {'pushPacket':self.pushPacket,
                              'pushSRI':self.pushSRI})

        # Create a port using the generate Metaclass and return an instance
        port = PortClass()
        return port._this()

    def popPacket(self, timeout=1):

        if timeout == -1:
            self.port_lock.acquire()
            try:
                while not self.data:
                    self.port_lock.wait(3)
                else:
                    return self.data.pop(0)
            finally:
                self.port_lock.release()
            raise NoDataException()

        until = time.time() + timeout
        self.port_lock.acquire()
        try:
            while not self.data:
                remain = until - time.time()
                if remain <= 0:
                    break
                self.port_lock.wait(remain)
            else:
                return self.data.pop(0)
        finally:
            self.port_lock.release()
        raise NoDataException()

class ProbeSink(object):
    """
    Simple class used to receive data from a port and store it in a python
    array.
    """
    def __init__(self, porttype):
        """
        Instantiates a new object responsible for writing data from the port
        into an array.

        It is important to notice that the porttype is a BULKIO__POA type and
        not a BULKIO type.  The reason is because it is used to generate a
        Port class that will be returned when the getPort() is invoked.  The
        returned class is the one acting as a server and therefore must be a
        Portable Object Adapter rather and a simple BULKIO object.

        Inputs:
            <porttype>        The BULKIO__POA data type
        """
        self.port_type = porttype
        self.sri=bulkio_helpers.defaultSRI
        self.data = []
        self.gotEOS = False
        self.port_lock = threading.Lock()
        self.valid_streams = {}
        self.invalid_streams = {}
        self.received_data = {}

    def start(self):
        self.gotEOS = False

    def eos(self):
        return self.gotEOS

    def pushSRI(self, H):
        """
        Stores the SteramSRI object regardless that there is no need for it

        Input:
            <H>    The StreamSRI object containing the information required to
                   generate the header file
        """
        self.sri = H
        self.valid_streams[H.streamID] = H
        self.received_data[H.streamID] = (0,0)

    def pushPacket(self, data, ts, EOS, stream_id):
        """
        Appends the data to the end of the array.

        Input:
            <data>        The actual data to append to the array
            <ts>          The timestamp
            <EOS>         Flag indicating if this is the End Of the Stream
            <stream_id>   The unique stream id
        """
        self.port_lock.acquire()
        try:
            if not self.valid_streams.has_key(stream_id):
                log.warn("the received packet has the invalid stream ID: "+stream_id+". Valid stream IDs are:"+str(self.valid_streams.keys()))
            self.received_data[stream_id] = (self.received_data[stream_id][0] + len(data), self.received_data[stream_id][1] + 1)
            if EOS:
                self.invalid_streams[stream_id] = self.valid_streams[stream_id]
                del self.valid_streams[stream_id]
                self.gotEOS = True
            else:
                self.gotEOS = False
        finally:
            self.port_lock.release()

    def pushPacketXML(self, data, EOS, stream_id):
        """
        Adapts dataXML pushPacket to support underlying probe tracking.
        """
        self.pushPacket(data, None, EOS, stream_id)

    def getPort(self):
        """
        Returns a Port object of the same type as the one specified as the
        porttype argument during the object instantiation.  It uses the
        classobj from the new module to generate a class on runtime.

        The classobj generates a class using the following arguments:

            name:        The name of the class to generate
            bases:       A tuple containing all the base classes to use
            dct:         A dictionary containing all the attributes such as
                         functions, and class variables

        It is important to notice that the porttype is a BULKIO__POA type and
        not a BULKIO type.  The reason is because it is used to generate a
        Port class that will be returned when the getPort() is invoked.  The
        returned class is the one acting as a server and therefore must be a
        Portable Object Adapter rather and a simple BULKIO object.

        """
        # The classobj generates a class using the following arguments:
        #
        #    name:        The name of the class to generate
        #    bases:       A tuple containing all the base classes to use
        #    dct:         A dictionary containing all the attributes such as
        #                 functions, and class variables
        if self.port_type == BULKIO__POA.dataXML:
            pushPacket = self.pushPacketXML
        else:
            pushPacket = self.pushPacket
        PortClass = classobj('PortClass',
                             (self.port_type,),
                             {'pushPacket':pushPacket,
                              'pushSRI':self.pushSRI})

        # Create a port using the generate Metaclass and return an instance
        port = PortClass()
        return port._this()

class XmlArraySink(ArraySink):
    "This sub-class exists to override pushPacket for dataXML ports."
    def __init__(self, porttype):
        ArraySink.__init__(self, porttype)

    def pushPacket(self, data, EOS, stream_id):
        """
        Overrides ArraySink.pushPacket(). The difference is that there is no
        <ts> timestamp.

        Appends the data to the end of the array.

        Input:
            <data>        The actual data to append to the array
            <EOS>         Flag indicating if this is the End Of the Stream
            <stream_id>   The unique stream id
        """
        if not self._livingStreams.has_key(stream_id):
            self._livingStreams[stream_id] = InputStream(self, BULKIO.StreamSRI(1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, stream_id, False, []))
        _stream = self._livingStreams[stream_id]
        _stream._updateData([data], None, EOS)
        if EOS:
            self._streams.append(_stream)
            self._livingStreams.pop(stream_id)

class XmlArraySource(ArraySource):
    "This sub-class exists to override pushPacket for dataXML ports."
    def __init__(self, porttype):
        ArraySource.__init__(self, porttype)
        self.eos = False

    def pushPacket(self, data, EOS, streamID):
        """
        Overrides ArraySource.pushPacket(). The difference is that there is no
        <ts> timestamp or sri

        Appends the data to the end of the array.

        Input:
            <data>        The actual data to append to the array
            <EOS>         Flag indicating if this is the End Of the Stream
            <stream_id>   The unique stream id
        """
        self.port_lock.acquire()
        try:
            try:
                for connId, port in self.outPorts.items():
                    if port != None: port.pushPacket(data, EOS, streamID)
            except Exception, e:
                msg = "The call to pushPacket failed with %s " % e
                msg += "connection %s instance %s" % (connId, port)
                log.warn(msg)
        finally:
            self.port_lock.release()

    def run(self, data, sri = None, pktsize = 1024):
        """
        Overrides ArraySource.run() to conform to dataXML. The difference
        is that there is no timestamp or SRI.


        Pushes the data through the connected port.  Each packet of data
        contains no more than pktsize elements.  Once all the elements have
        been sent, the method sends an empty list with the EOS set to True to
        indicate the end of the stream.

        Inputs:
            <data>       A list of elements containing the data to push
            <pktsize>    The maximum number of elements to send on each push
        """
        start = 0           # stores the start of the packet
        end = start         # stores the end of the packet
        sz = len(data)
        self.eos = False

        while not self.eos:
            chunk = start + pktsize
            # if the next chunk is greater than the file, then grab remaining
            # only, otherwise grab a whole packet size
            if chunk > sz:
                end = sz
                self.eos = True
            else:
                end = chunk

            # there are cases when this happens: array has 6 elements and
            # pktsize = 2 (end = length - 1 = 5 and start = 6)
            if start > end:
                self.eos = True
                continue

            d = data[start:end]
            start = end

            self.pushPacket(d, self.eos, self.stream_id)

class SDDSSource(object):
    "This sub-class exists to override pushPacket for dataXML ports."
    def __init__(self):
        self.outPorts = {}
        self.port_lock = threading.Lock()

    def attach(self, streamDef, user_id):
        self.port_lock.acquire()
        retval = None
        try:
            try:
                for connId, port in self.outPorts.items():
                    if port != None:
                        retval_ = port.attach(streamDef, user_id)
                        if retval == None:
                            retval = retval_
                        elif isinstance(retval,str):
                            retval = [retval,retval_]
                        else:
                            retval.append(retval_)

            except Exception, e:
                msg = "The call to attach failed with %s " % e
                msg += "connection %s instance %s" % (connId, port)
                log.warn(msg)
        finally:
            self.port_lock.release()
        return retval

    def detach(self, attachId):
        self.port_lock.acquire()
        try:
            try:
                for connId, port in self.outPorts.items():
                    if port != None:
                        port.detach(attachId)

            except Exception, e:
                msg = "The call to detach failed with %s " % e
                msg += "connection %s instance %s" % (connId, port)
                log.warn(msg)
        finally:
            self.port_lock.release()

    def connectPort(self, connection, connectionId):
        self.port_lock.acquire()
        try:
            port = connection._narrow(BULKIO__POA.dataSDDS)
            if port == None:
                return None
            self.outPorts[str(connectionId)] = port
        except:
            pass
        self.port_lock.release()

    def disconnectPort(self, connectionId):
        self.port_lock.acquire()
        try:
            self.outPorts.pop(str(connectionId), None)
        finally:
            self.port_lock.release()

    def getPort(self):
        """
        Returns a Port object of the type CF__POA.Port.
        """
        # The classobj generates a class using the following arguments:
        #
        #    name:        The name of the class to generate
        #    bases:       A tuple containing all the base classes to use
        #    dct:         A dictionary containing all the attributes such as
        #                 functions, and class variables
        PortClass = classobj('PortClass',
                             (CF__POA.Port,),
                             {'connectPort':self.connectPort,
                              'disconnectPort':self.disconnectPort})

        # Create a port using the generate Metaclass and return an instance
        port = PortClass()
        return port._this()

class FileSource(object):
    """
    Simple class used to push data into a port from a given array of data.
    """
    def __init__(self, porttype, byteswap=False, usesPortTypeDict=None):
        """
        Instantiates a new object and generates a default StreamSRI.  The
        porttype parameter corresponds to the type of data contained in the
        array of data being sent.

        The porttype is also used in the connectPort() method to narrow the
        connection

        """
        self.file_d = None
        self.pkts_sent = 0
        self.port_type = porttype
        self.byte_per_sample = 1
        self.structFormat = "B"
        if(porttype == BULKIO__POA.dataShort):
            self.byte_per_sample = 2
            self.structFormat = "h"
        elif(porttype == BULKIO__POA.dataFloat):
            self.byte_per_sample = 4
            self.structFormat = "f"
        elif(porttype == BULKIO__POA.dataDouble):
            self.byte_per_sample = 8
            self.structFormat = "d"
        elif(porttype == BULKIO__POA.dataChar):
            self.byte_per_sample = 1
            self.structFormat = "b"
        elif(porttype == BULKIO__POA.dataOctet):
            self.byte_per_sample = 1
            self.structFormat = "B"
        elif(porttype == BULKIO__POA.dataUlong):
            self.byte_per_sample = 4
            self.structFormat = "L"
        elif(porttype == BULKIO__POA.dataUshort):
            self.byte_per_sample = 2
            self.structFormat = "H"
        elif(porttype == BULKIO__POA.dataLong):
            self.byte_per_sample = 4
            self.structFormat = "l"
        elif(porttype == BULKIO__POA.dataLongLong):
            self.byte_per_sample = 8
            self.structFormat = "q"
        elif(porttype == BULKIO__POA.dataUlongLong):
            self.byte_per_sample = 8
            self.structFormat = "Q"
        elif(porttype == BULKIO__POA.dataXML):
            self.byte_per_sample = 1
            self.structFormat = "c"
        if byteswap:
            self._byteswap = '>'
        else:
            self._byteswap = '<'

        self.outPorts = {}
        self.connectionNormalization = {}
        self.connectionTranslation = {}
        self.usesPortTypeDict = usesPortTypeDict
        self.refreshSRI = False
        # Create default SRI
        self.sri=bulkio_helpers.defaultSRI

        self.port_lock = threading.Lock()
        self.EOS = False

    def connectPort(self, connection, connectionId):
        self.port_lock.acquire()
        for candidate in self.usesPortTypeDict:
            try:
                port = connection._narrow(self.usesPortTypeDict[candidate]['Port Type'])
                if port == None:
                    continue
                self.outPorts[str(connectionId)] = port
                if self.usesPortTypeDict[candidate]['Port Type']==BULKIO__POA.dataFloat or self.usesPortTypeDict[candidate]['Port Type']==BULKIO__POA.dataDouble:
                    if self.structFormat == 'h':
                        self.connectionNormalization[str(connectionId)] = 1./((2**15)-1)
                    elif self.structFormat == 'b':
                        self.connectionNormalization[str(connectionId)] = 1./((2**7)-1)
                    elif self.structFormat == 'B':
                        self.connectionNormalization[str(connectionId)] = 1./((2**8)-1)
                    elif self.structFormat == 'L':
                        self.connectionNormalization[str(connectionId)] = 1./((2**32)-1)
                    elif self.structFormat == 'H':
                        self.connectionNormalization[str(connectionId)] = 1./((2**16)-1)
                    elif self.structFormat == 'l':
                        self.connectionNormalization[str(connectionId)] = 1./((2**31)-1)
                    elif self.structFormat == 'q':
                        self.connectionNormalization[str(connectionId)] = 1./((2**63)-1)
                    elif self.structFormat == 'Q':
                        self.connectionNormalization[str(connectionId)] = 1./((2**64)-1)
                if self.usesPortTypeDict[candidate]['Port Type']==BULKIO__POA.dataShort:
                    if self.structFormat == 'b' or self.structFormat == 'B':
                        self.connectionTranslation[str(connectionId)] = ('\x00','h')
                elif self.usesPortTypeDict[candidate]['Port Type']==BULKIO__POA.dataUshort:
                    if self.structFormat == 'b' or self.structFormat == 'B':
                        self.connectionTranslation[str(connectionId)] = ('\x00','H')
                elif self.usesPortTypeDict[candidate]['Port Type']==BULKIO__POA.dataLong:
                    if self.structFormat == 'b' or self.structFormat == 'B':
                        self.connectionTranslation[str(connectionId)] = ('\x00'+'\x00'+'\x00','i')
                elif self.usesPortTypeDict[candidate]['Port Type']==BULKIO__POA.dataUlong:
                    if self.structFormat == 'b' or self.structFormat == 'B':
                        self.connectionTranslation[str(connectionId)] = ('\x00'+'\x00'+'\x00','I')
                elif self.usesPortTypeDict[candidate]['Port Type']==BULKIO__POA.dataLongLong:
                    if self.structFormat == 'b' or self.structFormat == 'B':
                        self.connectionTranslation[str(connectionId)] = ('\x00'+'\x00'+'\x00'+'\x00'+'\x00'+'\x00'+'\x00','q')
                elif self.usesPortTypeDict[candidate]['Port Type']==BULKIO__POA.dataUlongLong:
                    if self.structFormat == 'b' or self.structFormat == 'B':
                        self.connectionTranslation[str(connectionId)] = ('\x00'+'\x00'+'\x00'+'\x00'+'\x00'+'\x00'+'\x00','Q')
                self.refreshSRI = True
                break
            except:
                pass
        self.port_lock.release()

    def disconnectPort(self, connectionId):
        self.port_lock.acquire()
        try:
            self.outPorts.pop(str(connectionId), None)
        finally:
            self.port_lock.release()

    def pushSRI(self, H):
        self.port_lock.acquire()
        self.sri = H
        try:
            try:
                for connId, port in self.outPorts.items():
                    if port != None: port.pushSRI(H)
            except Exception, e:
                msg = "The call to pushSRI failed with %s " % e
                msg += "connection %s instance %s" % (connId, port)
                log.warn(msg)
        finally:
            self.port_lock.release()
            self.refreshSRI = False

    def pushPacket(self, data, T, EOS, streamID):
        if self.refreshSRI:
            self.pushSRI(self.sri)
        if EOS: # This deals with subsequent pushes with the same SRI
            self.refreshSRI = True

        self.port_lock.acquire()
        try:
            try:
                for connId, port in self.outPorts.items():
                    if port != None:
                        if self.connectionNormalization.has_key(connId):
                            new_data = range(len(data))
                            for idx in range(len(data)):
                                new_data[idx] = float(data[idx]) * self.connectionNormalization[connId]
                            data = new_data
                        elif self.connectionTranslation.has_key(connId):
                            new_data_str = ''
                            for idx in range(len(data)):
                                new_data_str = new_data_str + data[idx] + self.connectionTranslation[connId][0]
                            fmt = str(len(data))+self.connectionTranslation[connId][1]
                            data = struct.unpack(fmt,new_data_str)

                        if self.port_type == BULKIO__POA.dataXML:
                            port.pushPacket(data, EOS, streamID)
                        else:
                            port.pushPacket(data, T, EOS, streamID)
            except Exception, e:
                msg = "The call to pushPacket failed with %s " % e
                msg += "connection %s instance %s" % (connId, port)
                log.warn(msg)
        finally:
            self.port_lock.release()

    def getPort(self):
        """
        Returns a Port object of the type CF__POA.Port.
        """
        # The classobj generates a class using the following arguments:
        #
        #    name:        The name of the class to generate
        #    bases:       A tuple containing all the base classes to use
        #    dct:         A dictionary containing all the attributes such as
        #                 functions, and class variables
        PortClass = classobj('PortClass',
                             (CF__POA.Port,),
                             {'connectPort':self.connectPort,
                              'disconnectPort':self.disconnectPort})

        # Create a port using the generate Metaclass and return an instance
        port = PortClass()
        return port._this()

    def run(self, filename, sri=None, pktsize=1024, numpkts=0, startTime=0.0, sampleRate=1.0, complexData=False, streamID=None):
        """
        Pushes the data through the connected port.  Each packet of data
        contains no more than pktsize elements.  Once all the elements have
        been sent, the method sends an empty list with the EOS set to True to
        indicate the end of the stream.

        Inputs:
            <data>       A list of elements containing the data to push
            <sri>        SRI to send before pushing data
            <pktsize>    The maximum number of elements to send on each push
            <numpkts>    The maximum number of packets to send
            <startTime>  The time of the first sample
            <sampleRate> The sample rate of the data used to set xdelta in the SRI
            <complexData>The mode of the data (real=0(False),complex=1(True)) in the SRI
            <streamID>   The streamID of the data
        """

        # Update SRI based on arguments passed in
        if sri != None:
            self.sri = sri
        else:
            if sampleRate > 0:
                self.sri.xdelta = 1/sampleRate
            if complexData:
                self.sri.mode = 1
            else:
                self.sri.mode = 0
            if streamID != None:
                self.sri.streamID = streamID
            if startTime >= 0.0:
                self.sri.xstart = startTime

        self.pushSRI(self.sri)

        run_complete = False
        self.pkts_sent = 0
        if self.file_d == None or self.file_d.closed:
            self.file_d = open( filename, 'rb' );
            self.EOS = False

        if self.file_d.closed:
            log.info("file data has been exhausted!")

        currentSampleTime = self.sri.xstart
        while not self.EOS and not run_complete:
            T = BULKIO.PrecisionUTCTime(BULKIO.TCM_CPU, BULKIO.TCS_VALID, 0.0, int(currentSampleTime), currentSampleTime - int(currentSampleTime))
            byteData = self.file_d.read(pktsize * self.byte_per_sample)
            if (len(byteData) < pktsize * self.byte_per_sample):
                self.EOS = True
            signalData = byteData
            if self.structFormat not in ('b', 'B', 'c'):
                dataSize = len(byteData)/self.byte_per_sample
                fmt = self._byteswap + str(dataSize) + self.structFormat
                signalData = struct.unpack(fmt, byteData)
            else:
                dataSize = len(byteData)                                                                     
            if self.sri.mode == 1:                                                                                 
                dataSize = dataSize/2                                                                      

            self.pushPacket(signalData,T, False, self.sri.streamID)
            sampleRate = 1.0/self.sri.xdelta
            currentSampleTime = currentSampleTime + dataSize/sampleRate
            self.pkts_sent += 1
            if numpkts != 0 and numpkts == self.pkts_sent:
                run_complete = True

        if self.EOS:
            # Send EOS flag = true
            T = BULKIO.PrecisionUTCTime(BULKIO.TCM_CPU, BULKIO.TCS_VALID, 0.0, int(currentSampleTime), currentSampleTime - int(currentSampleTime))
            if self.structFormat not in ('b', 'B', 'c'):
                self.pushPacket([], T, True, self.sri.streamID)
            else:
                self.pushPacket('', T, True, self.sri.streamID)
            self.file_d.close()
        else:
            pass

class SDDSSink(object):
    def __init__(self, parent):
        self.attachments = {}
        self.port_lock = threading.Lock()
        self.parent = parent
        self.sri = None

    def __del__(self):
        if self.outFile != None:
            self.outFile.close()

    def attach(self, streamDef, userid):
        """
        Stores the SteramSRI object regardless that there is no need for it

        Input:
            <H>    The StreamSRI object containing the information required to
                   generate the header file
        """
        self.port_lock.acquire()
        attachid = _uuid.uuid1()
        self.attachments[attachid] = (streamDef, userid)
        self.port_lock.release()
        self.parent.attach_cb(streamDef, userid)
        return attachid

    def _get_attachmentIds(self):
        retval = []
        for key in self.attachments:
            retval.append(key)
        return retval

    def _get_attachedStreams(self):
        retval = []
        for key in self.attachments:
            retval.append(self.attachments[key][0])
        return retval

    def _get_usageState(self):
        if len(self.attachments) == 0:
            return BULKIO.dataSDDS.IDLE
        return BULKIO.dataSDDS.ACTIVE

    def getUser(self, attachId):
        if self.attachments.has_key(attachId):
            return self.attachments[attachId][1]
        return ''

    def getStreamDefinition(self, attachId):
        if self.attachments.has_key(attachId):
            return self.attachments[attachId][0]
        return None

    def pushSRI(self, H, T):
        self.sri = H

    def detach(self, attachId):
        self.port_lock.acquire()
        if self.attachments.has_key(attachId):
            self.attachments.pop(attachId)
        self.port_lock.release()
        self.parent.detach_cb(attachId)

    def getPort(self):
        PortClass = classobj('PortClass',
                             (BULKIO__POA.dataSDDS,),
                             {'_get_attachmentIds':self._get_attachmentIds,
                              '_get_attachedStreams':self._get_attachedStreams,
                              '_get_usageState':self._get_usageState,
                              'getUser':self.getUser,
                              'getStreamDefinition':self.getStreamDefinition,
                              'pushSRI':self.pushSRI,
                              'attach':self.attach,
                              'detach':self.detach})

        # Create a port using the generate Metaclass and return an instance
        port = PortClass()
        return port._this()

class FileSink(object):
    """
    Simple class used to receive data from a port and store it in a binary
    file.  It uses the SRI to generate the header.  The file is created the
    first time the SRI is pushed if the file does not exists.  If the file is
    present, then it does not alter the file.
    """
    def __init__(self, outputFilename, porttype):
        """
        Instantiates a new object responsible for writing data from the port
        into an array.

        It is important to notice that the porttype is a BULKIO__POA type and
        not a BULKIO type.  The reason is because it is used to generate a
        Port class that will be returned when the getPort() is invoked.  The
        returned class is the one acting as a server and therefore must be a
        Portable Object Adapter rather and a simple BULKIO object.

        Inputs:
            <outputFilename>  The output filename
            <porttype>        The BULKIO__POA data type
        """
        # If output file already exists, remove it
        if outputFilename != None and os.path.isfile(outputFilename):
            log.warn("overwriting output file " + str(outputFilename) + " since it already exists ")
            os.remove(outputFilename)

        if outputFilename == None:
            self.stream_derived = True
            self.sd_fp = {}
        else:
            self.stream_derived = False
        
        if not self.stream_derived:
            # Make sure if path is provided, it is valid
            path = os.path.dirname(outputFilename)
            if len(path) == 0 or \
                (len(path) > 0 and os.path.isdir(path)):
                self.outFile = open(outputFilename, "wb")
            else:
                log.error("invalid filename path")
                self.outFile = None
        else:
            self.outFile = None

        self.port_type = porttype
        self.sri=bulkio_helpers.defaultSRI
        self.data = []
        self.gotEOS = False
        self.port_lock = threading.Lock()

    def __del__(self):
        if self.outFile != None:
            self.outFile.close()

    def start(self):
        self.gotEOS = False

    def eos(self):
        return self.gotEOS

    def pushSRI(self, H):
        """
        Stores the SteramSRI object regardless that there is no need for it

        Input:
            <H>    The StreamSRI object containing the information required to
                   generate the header file
        """
        self.sri = H

    def pushPacket(self, data, ts, EOS, stream_id):
        """
        Appends the data to the end of the array.

        Input:
            <data>        The actual data to append to the array
            <ts>          The timestamp
            <EOS>         Flag indicating if this is the End Of the Stream
            <stream_id>   The unique stream id
        """
        self.port_lock.acquire()
        if EOS:
            self.gotEOS = True
        else:
            self.gotEOS = False
        try:
            self.byte_per_sample = 1
            self.structFormat = "B"
            if(self.port_type == BULKIO__POA.dataShort):
                self.byte_per_sample = 2
                self.structFormat = "h"
            elif(self.port_type == BULKIO__POA.dataFloat):
                self.byte_per_sample = 4
                self.structFormat = "f"
            elif(self.port_type == BULKIO__POA.dataDouble):
                self.byte_per_sample = 8
                self.structFormat = "d"
            elif(self.port_type == BULKIO__POA.dataChar):
                self.byte_per_sample = 1
                self.structFormat = "b"
            elif(self.port_type == BULKIO__POA.dataOctet):
                self.byte_per_sample = 1
                self.structFormat = "B"
            elif(self.port_type == BULKIO__POA.dataUlong):
                self.byte_per_sample = 4
                self.structFormat = "L"
            elif(self.port_type == BULKIO__POA.dataUshort):
                self.byte_per_sample = 2
                self.structFormat = "H"
            elif(self.port_type == BULKIO__POA.dataLong):
                self.byte_per_sample = 4
                self.structFormat = "l"
            elif(self.port_type == BULKIO__POA.dataLongLong):
                self.byte_per_sample = 8
                self.structFormat = "q"
            elif(self.port_type == BULKIO__POA.dataUlongLong):
                self.byte_per_sample = 8
                self.structFormat = "Q"
            elif(self.port_type == BULKIO__POA.dataXML):
                self.byte_per_sample = 1
                self.structFormat = "c"

            if len(data) > 0:
                outputArray = array.array(self.structFormat)
                if self.structFormat == "b" or self.structFormat == "B":
                    for item in data:
                        outputArray.fromstring(item)
                else:
                    outputArray = outputArray + array.array(self.structFormat, data)
                if self.outFile == None:
                    if self.stream_derived:
                        self.outFile = open(stream_id+'_out', "wb")
                        self.sd_fp[stream_id] = self.outFile
                else:
                    if self.stream_derived:
                        if self.sd_fp.has_key(stream_id):
                            self.outFile = self.sd_fp[stream_id]
                        else:
                            self.outFile = open(stream_id+'_out', "wb")
                            self.sd_fp[stream_id] = self.outFile
                outputArray.tofile(self.outFile)
            # If end of stream is true, close the output file
            if EOS == True:
                if self.outFile != None:
                    self.outFile.close()
                    if self.stream_derived:
                        if self.sd_fp.has_key(stream_id):
                            self.sd_fp.pop(stream_id)
        finally:
            self.port_lock.release()

    def pushPacketXML(self, data, EOS, stream_id):
        """
        Deals with XML data

        Input:
            <data>        xml string
            <EOS>         Flag indicating if this is the End Of the Stream
            <stream_id>   The unique stream id
        """
        self.port_lock.acquire()
        if EOS:
            self.gotEOS = True
        else:
            self.gotEOS = False
        try:
            self.outFile.write(data)
            # If end of stream is true, close the output file
            if EOS == True:
                if self.outFile != None:
                    self.outFile.close()
        finally:
            self.port_lock.release()

    def getPort(self):
        """
        Returns a Port object of the same type as the one specified as the
        porttype argument during the object instantiation.  It uses the
        classobj from the new module to generate a class on runtime.

        The classobj generates a class using the following arguments:

            name:        The name of the class to generate
            bases:       A tuple containing all the base classes to use
            dct:         A dictionary containing all the attributes such as
                         functions, and class variables

        It is important to notice that the porttype is a BULKIO__POA type and
        not a BULKIO type.  The reason is because it is used to generate a
        Port class that will be returned when the getPort() is invoked.  The
        returned class is the one acting as a server and therefore must be a
        Portable Object Adapter rather and a simple BULKIO object.

        """
        # The classobj generates a class using the following arguments:
        #
        #    name:        The name of the class to generate
        #    bases:       A tuple containing all the base classes to use
        #    dct:         A dictionary containing all the attributes such as
        #                 functions, and class variables
        if self.port_type == BULKIO__POA.dataXML:
            pushPacket = self.pushPacketXML
        else:
            pushPacket = self.pushPacket
        PortClass = classobj('PortClass',
                             (self.port_type,),
                             {'pushPacket':pushPacket,
                              'pushSRI':self.pushSRI})

        # Create a port using the generate Metaclass and return an instance
        port = PortClass()
        return port._this()
