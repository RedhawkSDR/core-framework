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

import threading, struct, time, Queue, copy, random
from omniORB import any, CORBA
from new import classobj
from ossie.cf import CF
try:
    from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA
    haveBulkio = True
except:
    haveBulkio = False
    pass


class BadParamException(Exception): pass
class NoDataException(Exception): pass

#A BULKIO.StreamSRI that you can use for the pushPacket
#call if you don't wish to define your own
if haveBulkio:
    defaultSRI = BULKIO.StreamSRI(hversion=1, xstart=0.0, xdelta=1.0,
                              xunits=1, subsize=0, ystart=0.0, ydelta=0.0,
                              yunits=0, mode=0, streamID='defStream', blocking=False, keywords=[])
else:
    defaultSRI = None

def bulkioComplexToPythonComplexList(bulkioList):
    '''
    Convert a BulkIO list of complex data to a list of Python complex
    data.  For example, the BulkIO list:

        [1,2,3,4,5,6]

    is converted as:

        [complex(1,2), complex(3,4), complex(5.6)]

    resulting in:

        [(1+2j), (3+4j), (5+6j)]

    '''

    # The list comprehension below provides efficient behavior, but is
    # somewhat difficult to read; therefore; it necessitates some
    # explaintation.
    #
    # The generator _group() chunks the input list into a series of 2-item
    # lists using indexing; each 2-item list is then unpacked into a real
    # and imaginary value, then used to create a complex value.
    if (len(bulkioList)%2):
        raise BadParamException('BulkIO complex data list must have an even number of entries.')
    def _group(data):
        for ii in xrange(0, len(data), 2):
            yield data[ii:ii+2]
    return [complex(real,imag) for real,imag in _group(bulkioList)]

def pythonComplexListToBulkioComplex(pythonComplexListInput, itemType=float):
    """
    Convert a list of Python complex data to a BulkIO list of complex
    data, with the real and imaginary components interleaved.  For example,
    the Python list:

        [(1+2j), (3+4j), (5+6j)]

    is converted as:

        [(1+2j).real,
         (1+2j).imag,
         (3+4j).real,
         (3+4j).imag,
         (5+6j).real,
         (5+6j).imag]

    resulting in:

        [1.0, 2.0, 3.0, 4.0, 5.0, 6.0]

    By default, the real and imag members of a Python complex value are of
    type float; however, another type object (e.g., 'int') can be given with
    the 'itemType' argument to convert the output items:

        pythonComplexListToBulkioComplex([(1+2j),(3+4j)], int)

    returns:

        [1, 2, 3, 4]
    """
    def _collapse(values):
        for val in values:
            try:
                yield val.real
                yield val.imag
            except:
                # Python 2.4 does not support .real or .imag for simple types,
                # which may occur in the list if mixed input is given. As a
                # workaround, return the value and 0 converted to the same type
                # as val.
                yield val
                yield type(val)(0)
    gen = (x for x in _collapse(pythonComplexListInput))
    if itemType == float:
        return list(gen)
    else:
        return [itemType(x) for x in gen]

def createCPUTimestamp():
    """
    Generates a BULKIO.PrecisionUTCTime object using the current
    CPU time that you can use in the pushPacket call
    """
    ts = time.time()
    return BULKIO.PrecisionUTCTime(BULKIO.TCM_CPU,
                                   BULKIO.TCS_VALID, 0.0,
                                   int(ts), ts - int(ts))
def compareSRI(sriA, sriB):
    """
    Will compare two BULKIO.StreamSRI objects and return True
    if they are both equal, and false otherwise
    """
    if not sriA or not sriB:
        return False

    return (sriA.hversion == sriB.hversion) and \
           (sriA.xstart == sriB.xstart) and \
           (sriA.xdelta == sriB.xdelta) and \
           (sriA.xunits == sriB.xunits) and \
           (sriA.subsize == sriB.subsize) and \
           (sriA.ystart == sriB.ystart) and \
           (sriA.ydelta == sriB.ydelta) and \
           (sriA.yunits == sriB.yunits) and \
           (sriA.mode == sriB.mode) and \
           (sriA.streamID == sriB.streamID) and \
           (sriA.blocking == sriB.blocking) and \
           (sriA.keywords == sriB.keywords)

def formatData(dataSet, portRef=None, BULKIOtype=None):
    """
    Takes in a data set as a list, and either, a reference to a BULKIO
    input port, or a BULKIO data type (i.e. BULKIO.dataFloat) and returns
    a properly formatted dataSet that is able to be sent into that
    port via the pushPacket call.  For character data you can send
    in a a list of 8 bit signed numbers, a list of strings all of
    which are single characters, or a single string.  For octet
    data you can send in a list of 8 bit unsigned numbers, a list of
    strings all of which are single characters, or a single string.
    """
    if portRef and not BULKIOtype:
        if '_NP_RepositoryId' not in dir(portRef):
            raise BadParamException('portRef parameter is not a valid BULKIO port reference')
        if portRef._is_a('IDL:BULKIO/UsesPortStatisticsProvider:1.0'):
            raise BadParamException("portRef parameter is a BULKIO output port, not an input port")
        else:
            portIDL = portRef._NP_RepositoryId
    elif BULKIOtype and not portRef:
        portIDL = BULKIOtype._NP_RepositoryId
    else:
        raise BadParamException("Must specify either a portRef or a dataType, but not both")

    dataType = portIDL.split('/')[1].split(':')[0]

    dataSetType = type(dataSet)
    validSetTypes = [str, list, tuple]
    if dataSetType not in validSetTypes:
        msg = 'dataSet is not a valid type (' + type(dataSet) + '). Valid types: '
        for x in validSetTypes:
            msg = msg + str(x) + ', '
        raise BadParamException(msg)
    if len(dataSet) == 0:
            raise BadParamException('dataSet parameter has length 0')

    if dataSetType == str and not (dataType == 'dataChar' or dataType == 'dataOctet'
                                           or dataType == 'dataFile' or dataType == 'dataXML'):
                raise BadParamException('dataSet parameter was of type string but the port is not of type character, octet, file, or xml')

    if dataType == 'dataChar':
        if dataSetType == str:
            return dataSet
        else:
            for elem in dataSet:
                if type(elem) == str and len(elem) != 1:
                    raise BadParamException('dataSet contains a string of length > 1 but port type is character')
                if type(elem) == int and (elem > 127 or elem < -128):
                    raise BadParamException('dataSet contains values which are out of range for type character(signed 8bit):', elem)
                if type(elem) != type(dataSet[0]):
                    raise BadParamException('dataSet contains multiple types of data:', type(elem), ' -- ', type(dataSet[0]))
                if type(elem) != str and type(elem) != int:
                    raise BadParamException('dataSet contains values which can not be interpreted as type character:', elem, ' -- ', type(elem))
            if type(dataSet[0]) == str:
                fmt = str(len(dataSet)) + 'c'
            else:
                fmt = str(len(dataSet)) + 'b'
            return struct.pack(fmt, *[i for i in dataSet])
    elif dataType == 'dataDouble':
        validDataTypes = [int, long, float]
        for elem in dataSet:
            if type(elem) not in validDataTypes:
                raise BadParamException('dataSet contains invalid data types for port type double:', elem, ' -- ', type(elem))
        packData = struct.pack(str(len(dataSet))+'d', *[x for x in dataSet])
        unpackData = struct.unpack(str(len(dataSet))+'d', packData)
        return (list)(unpackData)
    elif dataType == 'dataFloat':
        validDataTypes = [int, long, float]
        for elem in dataSet:
            if type(elem) not in validDataTypes:
                raise BadParamException('dataSet contains invalid data types for port type float:', elem, ' -- ', type(elem))
        packData = struct.pack(str(len(dataSet))+'f', *[x for x in dataSet])
        unpackData = struct.unpack(str(len(dataSet))+'f', packData)
        return (list)(unpackData)
    elif dataType == 'dataLong':
        validDataTypes = [int, long]
        for elem in dataSet:
            if type(elem) not in validDataTypes:
                raise BadParamException('dataSet contains invalid data types for port type long:', elem, ' -- ', type(elem))
        return (list)(dataSet)
    elif dataType == 'dataLongLong':
        validDataTypes = [int, long]
        for elem in dataSet:
            if type(elem) not in validDataTypes:
                raise BadParamException('dataSet contains invalid data types for port type longlong:', elem, ' -- ', type(elem))
        return (list)(dataSet)
    elif dataType == 'dataOctet':
        if dataSetType == str:
            return dataSet
        else:
            for elem in dataSet:
                if type(elem) == str and len(elem) != 1:
                    raise BadParamException('dataSet contains a string of length > 1 but port type is octet')
                if type(elem) == int and (elem > 255 or elem < 0):
                    raise BadParamException('dataSet contains values which are out of range for type octet(unsigned 8bit):', elem)
                if type(elem) != type(dataSet[0]):
                    raise BadParamException('dataSet contains multiple types of data:', type(elem), ' -- ', type(dataSet[0]))
                if type(elem) != str and type(elem) != int:
                    raise BadParamException('dataSet contains values which can not be interpreted as type octet:', elem, ' -- ', type(elem))
            if type(dataSet[0]) == str:
                fmt = str(len(dataSet)) + 'c'
            else:
                fmt = str(len(dataSet)) + 'B'
            return struct.pack(fmt, *[i for i in dataSet])
    elif dataType == 'dataShort':
        validDataTypes = [int, long]
        for elem in dataSet:
            if type(elem) not in validDataTypes:
                raise BadParamException('dataSet contains invalid data types for port type short:', elem, ' -- ', type(elem))
            if elem > 32767 or elem < -32768:
                raise BadParamException('dataSet contains values which are out of range for type short(signed 16bit):', elem)
        return (list)(dataSet)
    elif dataType == 'dataUlong':
        validDataTypes = [int, long]
        for elem in dataSet:
            if type(elem) not in validDataTypes:
                raise BadParamException('dataSet contains invalid data types for port type ulong:', elem, ' -- ', type(elem))
            if elem < 0:
                raise BadParamException('dataSet contains values which are out of range for type ulong:', elem)
        return (list)(dataSet)
    elif dataType == 'dataUlongLong':
        validDataTypes = [int, long]
        for elem in dataSet:
            if type(elem) not in validDataTypes:
                raise BadParamException('dataSet contains invalid data types for port type ulonglong:', elem, ' -- ', type(elem))
            if elem < 0:
                raise BadParamException('dataSet contains values which are out of range for type ulonglong:', elem)
        return (list)(dataSet)
    elif dataType == 'dataUshort':
        validDataTypes = [int, long]
        for elem in dataSet:
            if type(elem) not in validDataTypes:
                raise BadParamException('dataSet contains invalid data types for port type ushort:', elem, ' -- ', type(elem))
            if elem > 65536 or elem < 0:
                raise BadParamException('dataSet contains values which are out of range for type ushort(unsigned 16bit):', elem)
        return (list)(dataSet)
    elif dataType == 'dataFile':
        validDataTypes = [str]
        if type(dataSet) not in validDataTypes:
            raise BadParamException('dataSet contains invalid data types for port type file:', validDataTypes[0], ' -- ', type(validDataTypes[0]))
        return dataSet
    elif dataType == 'dataXML':
        validDataTypes = [str]
        if type(dataSet) not in validDataTypes:
            raise BadParamException('dataSet contains invalid data types for port type xml:', validDataTypes[0], ' -- ', type(validDataTypes[0]))
        return dataSet

def restoreData(dataSet, dataType):
    """
    Function for reformatting data received over an octet or
    character port which is not in the desired format. Supported
    dataTypes are '8bit'(signed 8 bit), 'u8bit'(unsigned 8 bit)
    or 'char'.  This function will attempt to return a list of numbers
    or a list of characters depending on the dataType that you choose
    """
    valid_types = ['char', '8bit', 'u8bit']
    if dataType not in valid_types:
        msg = 'dataType not supported, supported types: '
        for x in valid_types:
                msg = msg + str(x) + ', '
        raise BadParamException(msg)

    fmt = str(len(dataSet))
    if dataType == 'char':
        fmt = fmt + 'c'
    elif dataType == '8bit':
        fmt = fmt + 'b'
    elif dataType == 'u8bit':
        fmt = fmt + 'B'

    try:
        return [x for x in struct.unpack(fmt, dataSet)]
    except:
        raise Exception("Was not able to restore data to type: " + dataType)

def genRandomDataSet(sampleSize, signed=True, numSamples=1000):
    """
    Generates a random data set of the dataType
    that is specified.  Supported sampleSizes are:
       8, 16, 32, 64

    The number of samples in the dataset is specified
    by numSamples, and the signed flag defaults to True
    for signed data
    """
    min = 0
    max = 1
    if sampleSize == 8:
        if signed:
            min = (-1)*(2**7)
            max = 2**7
        else:
            min = 0
            max = 2**8
    elif sampleSize == 16:
        if signed:
            min = (-1)*(2**15)
            max = 2**15
        else:
            min = 0
            max = 2**16
    elif sampleSize == 32:
        if signed:
            min = (-1)*(2**31)
            max = 2**31
        else:
            min = 0
            max = 2**32
    elif sampleSize == 64:
        if signed:
            min = (-1)*(2**63)
            max = 2**63
        else:
            min = 0
            max = 2**64
    else:
        raise BadParamException('sampleSize: ' + str(sampleSize) + ' not supported')

    #Build random data set
    dataSet = []
    random.seed()
    for x in range(numSamples):
        dataSet.append(random.randrange(min, max))
    return dataSet


def createBULKIOInputPort(portType):
    """
    Function used to create as a BULKIOInput Port. The single argument
    you must pass is the port type that it is supposed to be from the
    bulkio.bulkioInterfaces BULKIO__POA module i.e. (BULKIO__POA.dataShort).
    You can then make the connectPort call on the output port using this port
    as the argument for the input port side. It will be able to receive data
    from that port and provide access to the data through the normal BULKIO Port API.
    """

    def pushSRI(self, H):
        """
        This function is called whenever the pushSRI call is
        made on the connected output port.  This function will
        store the SRI in a dictionary with the streamID as the
        key and the SRI and whether or not the SRI changed as
        the values
        """
        self.port_lock.acquire()
        if H.streamID not in self.sriDict:
            self.sriDict[H.streamID] = (copy.deepcopy(H), True)
            if H.blocking:
                self.blocking = H.blocking
        else:
            sri, sriChanged = self.sriDict[H.streamID]
            if not compareSRI(sri, H):
                self.sriDict[H.streamID] = (copy.deepcopy(H), True)
                if H.blocking:
                    self.blocking = H.blocking
        self.port_lock.release()

    def SDDS_pushSRI(self, H, T):
        """
        This function is called whenever the pushSRI call is
        made on the connected output port.  This function will
        store the SRI in a dictionary with the streamID as the
        key and the SRI and whether or not the SRI changed as
        the values
        """
        self.port_lock.acquire()
        if H.streamID not in self.sriDict:
            self.sriDict[H.streamID] = (copy.deepcopy(H), copy.deepcopy(T))
            if H.blocking:
                self.blocking = H.blocking
        else:
            sri, cur_T = self.sriDict[H.streamID]
            timediff = not ((cur_T.twsec == T.twsec) and (cur_T.tfsec == T.tfsec))
            if (not compareSRI(sri, H)) or timediff:
                self.sriDict[H.streamID] = (copy.deepcopy(H), copy.deepcopy(T))
                if H.blocking:
                    self.blocking = H.blocking
        self.port_lock.release()

    def pushPacket(self, *args):
        """
        This function is called whenever the pushPacket call is
        made on the connected output port.  This attempts to place
        a new CORBA packet on the date queue.  To pull packets off
        of the queue use the getPacket call
        """
        data = args[0]
        #the dataXML port and dataFile port don't
        #have time tags so check the number of args
        if len(args) == 4:
            T = args[1]
            EOS = args[2]
            streamID = args[3]
        else:
            T = None
            EOS = args[1]
            streamID = args[2]

        self.port_lock.acquire()
        packet = None
        try:
            sri = BULKIO.StreamSRI(1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, streamID, False, [])
            sriChanged = False
            if self.sriDict.has_key(streamID):
                sri, sriChanged = self.sriDict[streamID]
                self.sriDict[streamID] = (sri, False)
            else:
                self.sriDict[streamID] = (sri, False)
                sriChanged = True
            if self.blocking:
                packet = (data, T, EOS, streamID, copy.deepcopy(sri), sriChanged, False)
                self.queue.put(packet)
            else:
                if self.queue.full():
                    try:
                        self.queue.mutex.acquire()
                        self.queue.queue.clear()
                        self.queue.mutex.release()
                    except Queue.Empty:
                        pass
                    packet = (data, T, EOS, streamID, copy.deepcopy(sri), sriChanged, True)
                else:
                    packet = (data, T, EOS, streamID, copy.deepcopy(sri), sriChanged, False)
                self.queue.put(packet)
        finally:
            self.port_lock.release()


    def attach(self, stream, userid):
        """
        stub to be filled in for SDDS port functionality
        """
        pass

    def detach(self, id=None):
        """
        stub to be filled in for SDDS port functionality
        """
        pass

    def getCurrentQueueDepth(self):
        self.port_lock.acquire()
        depth = self.queue.qsize()
        self.port_lock.release()
        return depth

    def getMaxQueueDepth(self):
        self.port_lock.acquire()
        depth = self.queue.maxsize
        self.port_lock.release()
        return depth

    #set to -1 for infinite queue
    def setMaxQueueDepth(self, newDepth):
        self.port_lock.acquire()
        self.queue.maxsize = int(newDepth)
        self.port_lock.release()

    def getPacket(self, timetowait=0.0):
        """
        Returns the packet at the front of the queue if the
        queue is not empty.  Other wise will return None unless
        the block or timeout arguments are utilized.  These
        arguments are not currently part of the normal port
        API but are very useful for unit testing.  If the
        flags are not used then the function behaves the same
        way the code generators set it up to.
        """
        try:
            if timetowait == -1:
                data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed = self.queue.get(block=True)
            else:
                data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed = self.queue.get(timeout=timetowait)

            if EOS:
                if self.sriDict.has_key(streamID):
                    sri, sriChanged = self.sriDict.pop(streamID)
                    if sri.blocking:
                        stillBlock = False
                        for _sri, _sriChanged in self.sriDict.values():
                            if _sri.blocking:
                                stillBlock = True
                                break
                        if not stillBlock:
                            self.blocking = False
            return (data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed)
        except Queue.Empty:
            return None, None, None, None, None, None, None

    #######################################
    # These are used if your port attempts
    # to mimic an SDDS input port
    if portType == BULKIO__POA.dataSDDS:
        PortClass = classobj('Port',
                             (object,portType),
                             {'pushSRI':SDDS_pushSRI,
                              'attach':attach,
                              'detach':detach,
                              'sriDict':{},
                              'port_lock':threading.Lock()})
        tmp = PortClass()
        retval = tmp._this()
        retval.attach = tmp.attach
        retval.detach = tmp.detach
        return retval
    else:
        PortClass = classobj('PortClass',
                             (object,portType),
                             {'pushPacket':pushPacket,
                              'getPacket':getPacket,
                              'pushSRI':pushSRI,
                              'sriDict':{},
                              'queue':Queue.Queue(1000),
                              'blocking':False,
                              'getMaxQueueDepth':getMaxQueueDepth,
                              'setMaxQueueDepth':setMaxQueueDepth,
                              'getCurrentQueueDepth':getCurrentQueueDepth,
                              'port_lock':threading.Lock()})
        tmp = PortClass()
        retval = tmp._this()
        retval.getMaxQueueDepth = tmp.getMaxQueueDepth
        retval.setMaxQueueDepth = tmp.setMaxQueueDepth
        retval.getCurrentQueueDepth = tmp.getCurrentQueueDepth
        retval.blocking = tmp.blocking
        retval.getPacket = tmp.getPacket
        return retval
