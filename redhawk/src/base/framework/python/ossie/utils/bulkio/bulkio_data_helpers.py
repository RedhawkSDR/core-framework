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
from new import classobj
try:
    from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA
except:
    pass

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
        self.sri = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 200, 0.0, 0.001, 1, 1,
                                    "defaultStreamID", True, [])
        
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
                print(msg)
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
                print(msg)
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
        self.sri = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 200, 0.0, 0.001, 1,
                                    1, "defaultStreamID", True, [])
        self.data = []
        self.gotEOS = False
        self.port_lock = threading.Lock()
    
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
            for item in data:
                self.data.append(item)
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
        self.sri = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 200, 0.0, 0.001, 1,
                                    1, "defaultStreamID", True, [])
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
                print "the received packet has the invalid stream ID: "+stream_id+". Valid stream IDs are:"+str(self.valid_streams.keys())
            self.received_data[stream_id] = (self.received_data[stream_id][0] + len(data), self.received_data[stream_id][1] + 1)
            if EOS:
                self.invalid_streams[H.streamID] = self.valid_streams[H.streamID]
                tmp = self.valid_streams.pop(H.streamID)
                self.gotEOS = True
            else:
                self.gotEOS = False
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
        PortClass = classobj('PortClass',
                             (self.port_type,),
                             {'pushPacket':self.pushPacket,
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
        self.port_lock.acquire()
        if EOS:
            self.gotEOS = True
        else:
            self.gotEOS = False
        try:
            self.data.append(data)
        finally:
            self.port_lock.release()

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
                print(msg)
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

class FileSource(object):
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

        self.outPorts = {}
        self.refreshSRI = False
        # Create default SRI
        self.sri = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 200, 0.0, 0.001, 1, 1,
                                        "defaultStreamID", True, [])

        self.port_lock = threading.Lock()
        self.EOS = False

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
                print(msg)
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
                        if self.port_type == BULKIO__POA.dataXML:
                            port.pushPacket(data, EOS, streamID)
                        else:
                            port.pushPacket(data, T, EOS, streamID)
            except Exception, e:
                msg = "The call to pushPacket failed with %s " % e
                msg += "connection %s instance %s" % (connId, port)
                print(msg)
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
            print "FileSource:run() file data has been exhausted!"

        currentSampleTime = self.sri.xstart 
        while not self.EOS and not run_complete:
            T = BULKIO.PrecisionUTCTime(BULKIO.TCM_CPU, BULKIO.TCS_VALID, 0.0, int(currentSampleTime), currentSampleTime - int(currentSampleTime))
            byteData = self.file_d.read(pktsize * self.byte_per_sample)
            if (len(byteData) < pktsize * self.byte_per_sample):
                self.EOS = True
            signalData = byteData
            if self.structFormat not in ('b', 'B', 'c'):
                dataSize = len(byteData)/self.byte_per_sample
                fmt = '<' + str(dataSize) + self.structFormat
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
            print "FileSink:__init__() WARNING - overwriting output file " + str(outputFilename) + " since it already exists "
            os.remove(outputFilename)

        # Make sure if path is provided, it is valid
        path = os.path.dirname(outputFilename)
        if len(path) == 0 or \
            (len(path) > 0 and os.path.isdir(path)):
            self.outFile = open(outputFilename, "wb")
        else:
            print "FileSink:__init__() ERROR - invalid filename path"
            self.outFile = None

        self.port_type = porttype
        self.sri = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 200, 0.0, 0.001, 1,
                                    1, "defaultStreamID", True, [])
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
                for item in data:
                    if self.structFormat == "b" or self.structFormat == "B":
                        outputArray.fromstring(item)
                    else:
                        outputArray.append(item)
                outputArray.tofile(self.outFile) 
            # If end of stream is true, close the output file
            if EOS == True:
                if self.outFile != None:
                    self.outFile.close()
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
