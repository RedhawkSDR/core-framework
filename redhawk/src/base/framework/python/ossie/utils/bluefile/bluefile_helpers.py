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

import os
import threading
import time
import numpy
from new import classobj

from ossie.cf import CF, CF__POA
import ossie.properties
from ossie.utils.bulkio import bulkio_helpers
from ossie.utils.log4py import logging

import bluefile
try:
    import bulkio
    from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA
except:
    pass

logging.basicConfig()
log = logging.getLogger(__name__)

# BLUE files use an epoch of Jan 1, 1950, while REDHAWK uses the Unix epoch
# (Jan 1, 1970); REDHAWK_EPOCH_J1950 is the Unix epoch as a J1950 time
REDHAWK_EPOCH_J1950 = 631152000.0

def unix_to_j1950(ts):
    """
    Converts seconds since the Unix epoch (Jan 1, 1970) to a J1950 time.
    """
    return ts + REDHAWK_EPOCH_J1950

def j1950_to_unix(ts):
    """
    Converts a J1950 time to seconds since the Unix epoch (Jan 1, 1970).
    """
    return ts - REDHAWK_EPOCH_J1950

def hdr_to_sri(hdr, stream_id):
    """
    Generates a StreamSRI object based on the information contained in the
    X-Midas header file.
    
    Inputs:
        <hdr>    The X-Midas header file
        <stream_id>    The stream id
    
    Output:
        Returns a BULKIO.StreamSRI object 
    """
    hversion = 1  
    xstart = hdr['xstart']
    xdelta = hdr['xdelta']
    xunits = hdr['xunits']
    data_type = hdr['type']
    data_format = hdr['format']

    
    # The subsize needs to be 0 if one-dimensional or > 0 otherwise so 
    # using the data type to find out
    if data_type == 1000:
        subsize = 0
        ystart = 0
        ydelta = 0
        yunits = BULKIO.UNITS_NONE
    else:
        subsize = hdr['subsize']
        ystart = hdr['ystart']  
        ydelta = hdr['ydelta']  
        yunits = hdr['yunits']
    
    # The mode is based on the data type: 0 if is Scalar or 1 if it is 
    # Complex.  Setting it to -1 for any other type
    if data_format.startswith('S'):
        mode = 0
    elif data_format.startswith('C'):
        mode = 1
    else:
        mode = -1            
    
    kwds = []        

    # Getting all the items in the extended header
    if hdr.has_key('ext_header'):
        ext_hdr = hdr['ext_header']
        if isinstance(ext_hdr, dict):            
            for key, value in ext_hdr.iteritems():
                try:
                    data=item[1]
                    if isinstance(item[1], numpy.generic):
                        data=item[1].item()
                    dt = CF.DataType(key, ossie.properties.numpy_to_tc_value(data, type(item[1]).__name__))
                    #print "DEBUG (dict) AFTER dt.value:", dt.value, dt.value.value(), type(dt.value.value())
                    kwds.append(dt)
                except:
                    continue
        elif isinstance(ext_hdr, list):
            for item in ext_hdr:
                try:
                    data=item[1]
                    if isinstance(item[1], numpy.generic):
                        data=item[1].item()
                    dtv=ossie.properties.numpy_to_tc_value(data, type(item[1]).__name__)
                    dt = CF.DataType(item[0], dtv )
                    #print "DEBUG (list) AFTER dt.value:", dt.value, dt.value.value(), type(dt.value.value())
                    kwds.append(dt)
                except Exception, e:
                    continue
            
    return BULKIO.StreamSRI(hversion, xstart, xdelta, xunits, 
                            subsize, ystart, ydelta, yunits,
                            mode, stream_id, True, kwds)
        

def sri_to_hdr(sri, data_type, data_format):
    """
    Generates an X-Midas header file from the SRI information.
    
    Inputs:
        <sri>          The BULKIO.StreamSRI object
        <data_type>    The X-Midas file type (1000, 2000, etc)
        <data_format>  The X-Midas data format (SD, SF, CF, etc)
    
    Output:
        Returns an X-Midas header file 
    """
    kwds = {}
    
    kwds['xstart'] = sri.xstart
    kwds['xdelta'] = sri.xdelta
    kwds['xunits'] = sri.xunits
    
    kwds['subsize'] = sri.subsize
    kwds['ystart'] = sri.ystart
    kwds['ydelta'] = sri.ydelta
    kwds['yunits'] = sri.yunits
    
    kwds['format'] = data_format
    kwds['type'] = data_type

    # Default to REDHAWK epoch
    kwds['timecode'] = REDHAWK_EPOCH_J1950
    
    ext_hdr = sri.keywords
    if len(ext_hdr) > 0:
        items = []
        for item in ext_hdr:
            items.append((item.id, item.value.value()))
        
        kwds['ext_header'] = items
        
    return bluefile.header(**kwds)

def compare_bluefiles(file1=None, file2=None):
    """
    Compares 2 X-midas bluefiles.  It first checks to make sure the number of 
    elements are the same.  If they are not the same it returns false.  If they 
    are the same then it compares item by item.  If at least one of the items
    does not match it prints the index number and the values and returns False
    AFTER iterating through the whole data set otherwise it returns True.
    
    Inputs:
            <file1>    The first X-midas file to compare
            <file2>    The second X-midas file to compare
    Output:
        True if each element in the data in both files matches. 
    """
    msg = None
    # making sure the files exists
    if file1 == None:
        msg = '\nThe first file cannot be None\n'
    if file2 == None:
        msg = '\nThe second file cannot be None\n'
    if not os.path.isfile(file1):
        msg = '\nThe file %s is either invalid or it does not exists\n' % file1
    if not os.path.isfile(file2):
        msg = '\nThe file %s is either invalid or it does not exists\n' % file2

    if msg != None:
        log.error(msg)
        return False
    
    hdr1, data1 = bluefile.read(file1)
    hdr2, data2 = bluefile.read(file2)      

    if hdr1['format'].startswith('C'):
        data1 = data1.flatten()

    if hdr2['format'].startswith('C'):
        data2 = data2.flatten()

    sz1 = len(data1)
    sz2 = len(data2)
      
    # check the number of elements
    if sz1 != sz2:
        log.error("Files does not contain the same amount of items")
        return False
    
    are_the_same = True
    # check each element in the data
    for i, item1, item2 in zip(range(0, sz1), data1, data2):
        if item1 != item2:
            log.error("Item[%d]:\t%s  <==>   %s are not the same" % 
                                            (i, str(item1), str(item2)))
            are_the_same = False
    
    return are_the_same

class BlueFileReader(object):
    """
    Simple class used to send data to a port from an X-Midas file.  It uses 
    the header to generate a SRI.
    """
    def __init__(self, porttype, throttle=False):
        """
        Instantiates a new object and generates a default StreamSRI.  The 
        porttype parameter corresponds to the type of data contained in the 
        array of data being sent.  
        
        The porttype is also used in the connectPort() method to narrow the 
        connection
        
        """        
        self.porttype = porttype
        
        self.outPorts = {}
        self.refreshSRI = False
        self.defaultStreamSRI = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 0, 0.0, 
                                                 0.001, 1, 0, "sampleStream", 
                                                 True, [])
        self.port_lock = threading.Lock()
        self._throttle=throttle
        self.done = False

    def connectPort(self, connection, connectionId):
        self.port_lock.acquire()
        try:
            port = connection._narrow(self.porttype)
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
        self.defaultStreamSRI = H
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

    def pushPacket(self, data, T, EOS, streamID):        
        if self.refreshSRI:
            self.pushSRI(self.defaultStreamSRI)

        self.port_lock.acquire()

        if self._throttle:
            time.sleep(len(data)*self.defaultStreamSRI.xdelta/2.0)

        try:
            try:
                for connId, port in self.outPorts.items():
                    if port != None: port.pushPacket(data, T, EOS, streamID)
            except Exception, e:
                msg = "The call to pushPacket failed with %s " % e
                msg += "connection %s instance %s" % (connId, port)
                log.warn(msg)
        finally:
            self.port_lock.release()

        if self._throttle:
            time.sleep(len(data)*self.defaultStreamSRI.xdelta/2.0)

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
    
    def run(self, infile, pktsize=1024, streamID=None):
        """
        Pushes the data through the connected port.  Each packet of data 
        contains no more than pktsize elements.  Once all the elements have 
        been sent, the method sends an empty list with the EOS set to True to 
        indicate the end of the stream.
        
        Inputs:
            <infile>     The name of the X-Midas file containing the data 
                         to push
            <pktsize>    The maximum number of elements to send on each push
            <streamID>   The stream ID to be used, if None, then it defaults to filename 
        """        
        hdr, data = bluefile.read(infile, list)
        # generates a new SRI based on the header of the file
        path, stream_id = os.path.split(infile)
        if streamID == None:
            sri = hdr_to_sri(hdr, stream_id)
        else:
            sri = hdr_to_sri(hdr, streamID)
        self.pushSRI(sri)
        
        start = 0           # stores the start of the packet
        end = start         # stores the end of the packet

        # Flatten framed (type 2000) and/or complex data (where each element
        # may be a 2-tuple)
        if hdr['format'].startswith('C') or hdr['class'] == 2:
            data = numpy.reshape(data,(-1,))

        # For complex float/double, get a view of the data as the scalar type
        # instead of the complex type
        if hdr['format'] == 'CF':
            data = data.view(numpy.float32)
        elif hdr['format'] == 'CD':
            data = data.view(numpy.float64)

        sz = len(data)      
        self.done = False
        
        # Use midas header timecode to set time of first sample
        # NOTE: midas time is seconds since Jan. 1 1950
        #       Redhawk time is seconds since Jan. 1 1970
        currentSampleTime = 0.0
        if hdr.has_key('timecode'):
            # Set sample time to seconds since Jan. 1 1970 
            currentSampleTime = j1950_to_unix(hdr['timecode'])
            if currentSampleTime < 0:
                currentSampleTime = 0.0

        # Quantize packet size to match complex ("spa" is scalars per atom) and
        # framing ("ape" is atoms per element), with a minimum of one frame per
        # packet
        spe = hdr['spa'] * hdr['ape']
        pktsize = max(spe, int(pktsize/spe) * spe)

        # Create a normalized timestamp from the initial sample time
        T = bulkio.timestamp.create(currentSampleTime, 0.0)
        bulkio.timestamp.normalize(T)

        while not self.done:
            chunk = start + pktsize
            # if the next chunk is greater than the file, then grab remaining
            # only, otherwise grab a whole packet size
            if chunk >= sz:
                end = sz
                self.done = True
            else:
                end = chunk
            
            dataset = data[start:end]
            
            # X-Midas returns an array, so we need to generate a list
            if hdr['format'].endswith('B'):
                d = dataset.tostring()
            else:
                d = dataset.tolist()
            start = end

            self.pushPacket(d, T, False, sri.streamID)

            # Update the sample time to account for the packet; the data size
            # should always be an integral number of elements (taking subsize
            # and complex into account), but use a float just in case there's
            # a partial packet at the end
            dataSize = len(d) / float(spe)
            # Make a best guess at the time axis, based on the whether it's
            # framed data (and the Y-axis is in units of time), defaulting to
            # the X-axis
            if sri.subsize > 0 and sri.yunits == BULKIO.UNITS_TIME:
                delta = sri.ydelta
            else:
                delta = sri.xdelta
            T += dataSize * delta

        if hdr['format'].endswith('B'):
            self.pushPacket('', T, True, sri.streamID)
        else: 
            self.pushPacket([], T, True, sri.streamID)
    
        
class BlueFileWriter(object):
    """
    Simple class used to receive data from a port and store it in an X-Midas
    file.  It uses the SRI to generate the header.  The file is created the 
    first time the SRI is pushed if the file does not exists.  If the file is 
    present, then it does not alter the file.
    """
    def __init__(self, input_stream=None, porttype=None, rem_file=False):
        """
        Instantiates a new object responsible for writing data from the port 
        into a file.  The file name is given by the input_stream variable.
        
        It is important to notice that the porttype is a BULKIO__POA type and
        not a BULKIO type.  The reason is because it is used to generate a 
        Port class that will be returned when the getPort() is invoked.  The
        returned class is the one acting as a server and therefore must be a
        Portable Object Adapter rather and a simple BULKIO object.
        
        Inputs:
            <input_stream>    The X-Midas file to generate
            <porttype>        The BULKIO__POA data type
            <rem_file>        Removes the input_stream if present 
        """
        
        if input_stream != None and os.path.isfile(input_stream):
            os.remove(input_stream)
        
        self.port_type = porttype
        self.outFile = input_stream
        self.port_lock = threading.Lock()
        self.eos_cond = threading.Condition(self.port_lock)
        self.gotEOS = False
        self.header = None
        self.done = False
        self._firstPacket = True
    
    def start(self):
        self.done = False
        self.gotEOS = False
    
    def eos(self):
        return self.gotEOS
    
    def pushSRI(self, H):
        """
        Generate an X-Midas header from the StreamSRI if the input_stream file
        does not exist.  If the file already exists, then it does not perform 
        any operation
        
        Input:
            <H>    The StreamSRI object containing the information required to
                   generate the header file
        """
        self.port_lock.acquire()
        try:
            # The header can be created only once as the size changes
            # when data is pushed
            if self.header == None:
                # the file is either type 1000 or 2000
                if H.subsize == 0:
                    data_type = 1000
                else:
                    data_type = 2000

                # scalar 
                if H.mode == 0:
                    data_format = 'S'
                # complex 
                else:
                    data_format = 'C'

                if self.port_type == BULKIO__POA.dataShort:
                    data_format = data_format + 'I'
                elif self.port_type == BULKIO__POA.dataUshort:
                    data_format = data_format + 'I'
                elif self.port_type == BULKIO__POA.dataFloat:
                    data_format = data_format + 'F'
                elif self.port_type == BULKIO__POA.dataDouble:
                    data_format = data_format + 'D'
                elif self.port_type == BULKIO__POA.dataChar:
                    data_format = data_format + 'B'
                elif self.port_type == BULKIO__POA.dataOctet:
                    data_format = data_format + 'B'
                elif self.port_type == BULKIO__POA.dataUlong:
                    data_format = data_format + 'L'
                elif self.port_type == BULKIO__POA.dataLong:
                    data_format = data_format + 'L'
                # default to double precision data
                else:
                    data_format = data_format + 'D'
                
                # generate the header and then create the file
                self.header = sri_to_hdr(H, data_type, data_format)
                bluefile.writeheader(self.outFile, self.header, 
                                     keepopen=0, ext_header_type=list)
        finally:
            self.port_lock.release()

        
    def pushPacket(self, data, ts, EOS, stream_id):
        """
        Pushes data to the file.
        
        Input:
            <data>        The actual data to write to the file
            <ts>          The timestamp
            <EOS>         Flag indicating if this is the End Of the Stream
            <stream_id>   The name of the file
        """
        self.port_lock.acquire()
        if EOS:
            self.gotEOS = True
            self.eos_cond.notifyAll()
        else:
            self.gotEOS = False
        try:
            # If the header doesn't already have meaningful timecode, update it
            # with the packet time and write it out to disk
            if self._firstPacket and self.header and ts.tcstatus == BULKIO.TCS_VALID:
                self.header['timecode'] = unix_to_j1950(ts.twsec + ts.tfsec)
                bluefile.writeheader(self.outFile, self.header)
            self._firstPacket = False

            if self.header and self.header['format'][1] == 'B':
                # convert back from string to array of 8-bit integers
                try:
                    data = numpy.fromstring(data, numpy.int8)
                except TypeError:
                    data = numpy.array(data, numpy.int8)

            # If complex data, need to convert data back from array of scalar values
            # to some form of complex values
            if self.header and self.header['format'][0] == 'C':
                # float and double are handled by numpy
                # each array element is a single complex value
                if self.header['format'][1] in ('F', 'D'):
                    data = bulkio_helpers.bulkioComplexToPythonComplexList(data)
                # other data types are not handled by numpy
                # each element is two value array representing real and imaginary values
                # if data is also framed, wait to reshape everything at once
                elif self.header['subsize'] == 0:
                    # Need to rehape the data into complex value pairs
                    data = numpy.reshape(data,(-1,2))

            # If framed data, need to frame the data according to subsize
            if self.header and self.header['subsize'] != 0:
                # If scalar or single complex values, just frame it
                if self.header['format'][0] != 'C' or  self.header['format'][1] in ('F', 'D'):
                    data = numpy.reshape(data,(-1, int(self.header['subsize'])))
                # otherwise, frame and pair as complex values
                else:
                    data = numpy.reshape(data,(-1, int(self.header['subsize']), 2))

            bluefile.write(self.outFile, hdr=None, data=data, 
                       append=1)     
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

    def waitEOS(self, timeout):
        """
        Waits for an end-of-stream notification from the source.

        Input:
            <timeout>     Maximum number of seconds to wait for EOS
        """
        end = time.time() + timeout
        self.eos_cond.acquire()
        try:
            while not self.gotEOS:
                timeout = end - time.time()
                if timeout <= 0:
                    break
                self.eos_cond.wait(timeout)
        finally:
            self.eos_cond.release()
