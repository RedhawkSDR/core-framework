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

from ossie.utils.sandbox import LocalSandbox as _LocalSandbox
from ossie.utils.sandbox import IDESandbox as _IDESandbox
from ossie.utils.model import NoMatchingPorts as _NoMatchingPorts
from io_helpers import FileSource as _FileSource
from io_helpers import FileSink as _FileSink
import os as _os
import logging as _logging
import time as _time
import threading as _threading
import signal

log = _logging.getLogger(__name__)

__all__ = ('proc',)

class ProcessingTimeout(Exception):
    def __init__(self):
        pass
    def __str__(self):
        return "Processing timed out before completion"

def findInDataCoverterPortName(dataFormat):
    portName = ''
    if '8' in dataFormat:
        if 'u' in dataFormat:
            portName = 'dataOctet'
        elif 't' in dataFormat:
            portName = 'dataChar'
    elif '16' in dataFormat:
        if 'u' in dataFormat:
            portName = 'dataUshort'
        elif 't' in dataFormat:
            portName = 'dataShort'
    elif '32' in dataFormat:
        if 'u' in dataFormat:
            portName = 'dataUlong'
        elif 't' in dataFormat:
            portName = 'dataLong'
        elif 'f' in dataFormat:
            portName = 'dataFloat'
    elif '64' in dataFormat:
        if 'u' in dataFormat:
            portName = 'dataUlongLong'
        elif 't' in dataFormat:
            portName = 'dataLongLong'
        elif 'f' in dataFormat:
            portName = 'dataDouble'
    return portName
    
def findOutDataCoverterPortName(portType):
    if portType == 'IDL:BULKIO/dataOctet:1.0':
        portName = 'dataOctet_out'
    elif portType == 'IDL:BULKIO/dataChar:1.0':
        portName = 'dataChar_out'
    elif portType == 'IDL:BULKIO/dataUshort:1.0':
        portName = 'dataUshort_out'
    elif portType == 'IDL:BULKIO/dataShort:1.0':
        portName = 'dataShort_out'
    elif portType == 'IDL:BULKIO/dataUlong:1.0':
        portName = 'dataUlong_out'
    elif portType == 'IDL:BULKIO/dataLong:1.0':
        portName = 'dataLong_out'
    elif portType == 'IDL:BULKIO/dataUlongLong:1.0':
        portName = 'dataUlongLong_out'
    elif portType == 'IDL:BULKIO/dataLongLong:1.0':
        portName = 'dataLongLong_out'
    elif portType == 'IDL:BULKIO/dataFloat:1.0':
        portName = 'dataFloat_out'
    elif portType == 'IDL:BULKIO/dataDouble:1.0':
        portName = 'dataDouble_out'
    return portName

def interfaceFromFormat(dataFormat):
    portName = ''
    if '8' in dataFormat:
        if 'u' in dataFormat:
            portName = 'IDL:BULKIO/dataOctet:1.0'
        elif 't' in dataFormat:
            portName = 'IDL:BULKIO/dataChar:1.0'
    elif '16' in dataFormat:
        if 'u' in dataFormat:
            portName = 'IDL:BULKIO/dataUshort:1.0'
        elif 't' in dataFormat:
            portName = 'IDL:BULKIO/dataShort:1.0'
    elif '32' in dataFormat:
        if 'u' in dataFormat:
            portName = 'IDL:BULKIO/dataUlong:1.0'
        elif 't' in dataFormat:
            portName = 'IDL:BULKIO/dataLong:1.0'
        elif 'f' in dataFormat:
            portName = 'IDL:BULKIO/dataFloat:1.0'
    elif '64' in dataFormat:
        if 'u' in dataFormat:
            portName = 'IDL:BULKIO/dataUlongLong:1.0'
        elif 't' in dataFormat:
            portName = 'IDL:BULKIO/dataLongLong:1.0'
        elif 'f' in dataFormat:
            portName = 'IDL:BULKIO/dataDouble:1.0'
    return portName

def matchPortFormat(dataFormat, portType):
    if dataFormat == 'BLUE':
        return True
    if '8' in dataFormat:
        if 'u' in dataFormat:
            if portType == 'IDL:BULKIO/dataOctet:1.0':
                return True
        elif 't' in dataFormat:
            if portType == 'IDL:BULKIO/dataChar:1.0':
                return True
    elif '16' in dataFormat:
        if 'u' in dataFormat:
            if portType == 'IDL:BULKIO/dataUshort:1.0':
                return True
        elif 't' in dataFormat:
            if portType == 'IDL:BULKIO/dataShort:1.0':
                return True
    elif '32' in dataFormat:
        if 'u' in dataFormat:
            if portType == 'IDL:BULKIO/dataUlong:1.0':
                return True
        elif 't' in dataFormat:
            if portType == 'IDL:BULKIO/dataLong:1.0':
                return True
        elif 'f' in dataFormat:
            if portType == 'IDL:BULKIO/dataFloat:1.0':
                return True
    elif '64' in dataFormat:
        if 'u' in dataFormat:
            if portType == 'IDL:BULKIO/dataUlongLong:1.0':
                return True
        elif 't' in dataFormat:
            if portType == 'IDL:BULKIO/dataLongLong:1.0':
                return True
        elif 'f' in dataFormat:
            if portType == 'IDL:BULKIO/dataDouble:1.0':
                return True
    return False

def proc(comp,source,sink=None,sourceFmt=None,sinkFmt=None,sampleRate=1.0,execparams={},configure={},providesPortName=None,usesPortName=None,timeout=None,objType="component"):
    '''
    This function launches a component, passes a file through the component, saves the result
    in another file, and then terminates the component. The output file format is determined
    by the type of the component's output (uses) port

    comp              The name of the component to launch
    source            The input filename
    sink              The output filename
                       Set to False for no output
                       Set to None for the output filename to have the format: stream_id+'_out'
    sourceFmt         The format for the input file content
                       Two valid values:
                         - None         Attempt to read as a 16t
                         - Formatted string:
                            8/16/32/64  bit precision (mandatory)
                            u/t/f       unsigned fixed, signed fixed, floating point (mandatory)
                            c           complex (optional)
                            r           byte-swap (optional)
    sinkFmt           The desired format for the output file. Complex or real is driven by the data
                       Three valid values:
                         - None         Match the format of comp's out connection type
                         - BLUE         blue file format
                         - Formatted string:
                            8/16/32/64  bit precision (mandatory)
                            u/t/f       unsigned fixed, signed fixed, floating point (mandatory)
                            r           byte-swap (optional)
    execparams        Dictionary containing overloaded exec properties
    configure         Dictionary containing overloaded configurable properties
    timeout           Length of time (in seconds) before function forcibly exits
    providesPortName  Input port to be used on the component to launch
                        - This argument should only be used to resolve ambiguities
    usesPortName      Output port to be used on the component to launch
                        - This argument should only be used to resolve ambiguities
    '''
    global exit_condition
    global timeout_condition
    global crashed_condition
    global old_signal_SIGINT
    global break_condition
    instanceName = None
    refid=None
    impl=None
    debugger=None
    window=None
    initialize=True
    sinkBlue = False
    if sinkFmt == 'BLUE':
        sinkBlue = True

    def undo_all(undos):
        keys = undos.keys()
        keys.sort(reverse=True)
        for undo in keys:
            try:
                undoCall = getattr(undos[undo][0], undo.split(':')[-1])
                if len(undos[undo]) == 1:
                    undoCall()
                else:
                    undoCall(undos[undo][1])
            except Exception, e:
                pass
    
    def signalHandler(sig, frame):
        global break_condition
        global old_signal_SIGINT
        signal.signal(signal.SIGINT,old_signal_SIGINT)
        break_condition = True
    
    old_signal_SIGINT = signal.signal(signal.SIGINT, signalHandler)
                
    def checkTimeout(timeout, undos):
        begin_time = _time.time()
        global timeout_condition
        global break_condition
        global exit_condition
        while True:
            if timeout < (_time.time() - begin_time):
                log.warn("Timeout reached. File '"+sink+"' is incomplete")
                timeout_condition = True
                undo_all(undos)
            _time.sleep(0.1)
            if exit_condition:
                break
            if break_condition:
                break
        # exiting timeout thread

    def checkCrashed(_comp,comp, undos):
        global crashed_condition
        global exit_condition
        while True:
            if not _comp._process.isAlive():
                log.warn('******************************************\n\n\nThe component '+comp+' crashed\nThis indicates a bug in the component code\n\n\n******************************************')
                crashed_condition = True
                undo_all(undos)
            _time.sleep(0.1)
            if exit_condition:
                break
        # exiting timeout thread
        
    begin_time = _time.time()
    sdrRoot = _os.environ.get('SDRROOT', None)
    log.trace("Creating local sandbox with SDRROOT '%s'", sdrRoot)
    if sdrRoot is None:
        sdrRoot = _os.getcwd()
    _sandbox = _LocalSandbox(sdrRoot)
    order = 1
    undos = {str(order)+':shutdown':(_sandbox,)}
    order += 1
            
    if comp!=None:
        try:
            _comp=_sandbox.launch(comp,objType="component",instanceName=instanceName, refid=refid, impl=impl, debugger=debugger, window=window, execparams=execparams, configure=configure, initialize=initialize, timeout=timeout)
        except Exception, e:
            undo_all(undos)
            raise
    else:
        _comp = comp
    if _comp==None:
        undo_all(undos)
        raise Exception('Unable to deploy the component')
        
    try:
        _read=_FileSource(filename=source,dataFormat=sourceFmt,midasFile=sinkBlue,sampleRate=sampleRate)
        undos[str(order)+':releaseObject']=(_read,)
        order += 1
    except Exception, e:
        undo_all(undos)
        raise

    _write = None
    outputDataConverter = False
    if sink != False:
        try:
            _write=_FileSink(filename=sink,midasFile=sinkBlue)
            undos[str(order)+':releaseObject']=(_write,)
            order += 1
        except Exception, e:
            undo_all(undos)
            raise
         
        outputDataConverter = False
        matchingInterface = None
        if sinkFmt != None:
            matchedOutput = False
            matchingInterface = interfaceFromFormat(sinkFmt)
            if usesPortName != None:
                for port in _comp.ports:
                    if port._direction != 'Uses':
                        continue
                    if port._name == usesPortName:
                        interface = 'IDL:'+port._using.nameSpace+'/'+port._using.name+':1.0'
                        matchedOutput = matchPortFormat(sinkFmt, interface)
                        break
            else:
                for port in _comp.ports:
                    if port._direction != 'Uses':
                        continue
                    interface = 'IDL:'+port._using.nameSpace+'/'+port._using.name+':1.0'
                    matchedOutput = matchPortFormat(sinkFmt, interface)
                    if matchedOutput:
                        matchingInterface = None
                        break
            if not matchedOutput:
                outputDataConverter = True
    
        if outputDataConverter:
            try:
                _dataConverter_out=_sandbox.launch('DataConverter', timeout=timeout)
            except Exception, e:
                undo_all(undos)
                if type(e).__name__ == "ValueError" and \
                   str(e) == "'DataConverter' is not a valid softpkg name or SPD file":
                    raise Exception("DataConverter component is not installed in SDRROOT.")
                else:
                    raise
            try:
                _comp.connect(_dataConverter_out, usesPortName=usesPortName)
                undos[str(order)+':disconnect']=(_comp,_dataConverter_out)
                order += 1
            except Exception, e:
                undo_all(undos)
                raise
            try:
                portName = findOutDataCoverterPortName(matchingInterface)
                _dataConverter_out.connect(_write, usesPortName=portName)
                undos[str(order)+':disconnect']=(_dataConverter_out,_write)
                order += 1
            except Exception, e:
                undo_all(undos)
                raise
        else:
            try:
                _comp.connect(_write, usesPortName=usesPortName)
                undos[str(order)+':disconnect']=(_comp,_write)
                order += 1
            except Exception, e:
                undo_all(undos)
                raise

    inputDataConverter = False
    try:
        _read.connect(_comp, providesPortName=providesPortName)
        undos[str(order)+':disconnect']=(_read,_comp)
        order += 1
    except _NoMatchingPorts, e:
        if sourceFmt != None:
            inputDataConverter = True
        else:
            undo_all(undos)
            raise
    except Exception, e:
        undo_all(undos)
        raise
    if inputDataConverter:
        try:
            _dataConverter=_sandbox.launch('DataConverter', timeout=timeout)
        except Exception, e:
            undo_all(undos)
            if type(e).__name__ == "ValueError" and \
               str(e) == "'DataConverter' is not a valid softpkg name or SPD file":
                raise Exception("DataConverter component is not installed in SDRROOT.")
            else:
                raise
        try:
            portName = findInDataCoverterPortName(sourceFmt)
            _read.connect(_dataConverter, providesPortName=portName)
            undos[str(order)+':disconnect']=(_read,_dataConverter)
            order += 1
        except Exception, e:
            undo_all(undos)
            raise
        try:
            _dataConverter.connect(_comp)
            undos[str(order)+':disconnect']=(_dataConverter,_comp)
            order += 1
        except Exception, e:
            undo_all(undos)
            raise
    
    if inputDataConverter:
        try:
            _dataConverter.start()
            undos[str(order)+':stop']=(_comp,)
            order += 1
        except Exception, e:
            undo_all(undos)
            raise

    if outputDataConverter:
        try:
            _dataConverter_out.start()
            undos[str(order)+':stop']=(_comp,)
            order += 1
        except Exception, e:
            undo_all(undos)
            raise

    try:
        _comp.start()
        undos[str(order)+':stop']=(_comp,)
        order += 1
    except Exception, e:
        undo_all(undos)
        raise
    
    try:
        _read.start()
        undos[str(order)+':stop']=(_read,)
        order += 1
    except Exception, e:
        undo_all(undos)
        raise
    
    if sink!=False:
        try:
            _write.start()
            undos[str(order)+':stop']=(_write,)
            order += 1
        except Exception, e:
            undo_all(undos)
            raise
    
    exit_condition = False
    timeout_condition = False
    crashed_condition = False
    break_condition = False
    
    if timeout:
        timeout = timeout - (_time.time() - begin_time)
        _runThread = _threading.Thread(target=checkTimeout,args=(timeout,undos))
        _runThread.setDaemon(True)
        _runThread.start()
    
    
    if sink!=False:
        try:
            done = False
            while not done:
                eos = _write.eos()
                if timeout_condition:
                    break
                if break_condition:
                    break
                if eos:
                    done = True
                    continue
                if not _comp._process.isAlive():
                    print '******************************************\n\n\nThe component '+comp+' crashed\nThis indicates a bug in the component code\n\n\n******************************************'
                    break
                _time.sleep(0.1)
        except Exception, e:
            if (not timeout_condition) and (not break_condition):
                undo_all(undos)
                raise
    else:
        try:
            _c_runThread = _threading.Thread(target=checkCrashed,args=(_comp, comp, undos))
            _c_runThread.setDaemon(True)
            _c_runThread.start()
            raw_input('enter a new line to exit processing\n')
            done = True
            #if not _comp._process.isAlive():
            #    print '******************************************\n\n\nThe component '+comp+' crashed\nThis indicates a bug in the component code\n\n\n******************************************'
            #else:
            #    done=True
        except Exception, e:
            undo_all(undos)
            raise
            

    exit_condition = True
    
    if (not done) and (not break_condition):
        undo_all(undos)
        raise Exception('Component died abnormally')
    
    if break_condition:
        undo_all(undos)
        return
    
    if (not timeout_condition) and (not break_condition):
        undo_all(undos)
        return
        
    return Exception('An undetermined error ocurred in the flow')
