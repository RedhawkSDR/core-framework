#!/usr/bin/env python
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

#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: BasicAC_python_impl1.spd.xml
# Generated on: Wed Jun 15 15:36:05 EDT 2011
from ossie.cf import CF, CF__POA
from ossie.utils import uuid

from ossie.resource import Resource

import Queue, copy, time, threading
from ossie.resource import usesport, providesport

NOOP = -1
NORMAL = 0
FINISH = 1
class ProcessThread(threading.Thread):
    def __init__(self, target, pause=0.0125):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self.target = target
        self.pause = pause
        self.stop_signal = threading.Event()

    def stop(self):
        self.stop_signal.set()

    def updatePause(self, pause):
        self.pause = pause

    def run(self):
        state = NORMAL
        while (state != FINISH) and (not self.stop_signal.isSet()):
            state = self.target()
            if (state == NOOP):
                # If there was no data to process sleep to avoid spinning
                time.sleep(self.pause)

class BasicAC_python_impl1_base(CF__POA.Resource, Resource):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block
        
        def __init__(self, identifier, execparams):
            loggerName = execparams['NAME_BINDING'].replace('/', '.')
            Resource.__init__(self, identifier, execparams, loggerName=loggerName)
            self.process_thread = None
            
        def initialize(self):
            Resource.initialize(self)
            # Instantiate the default implementations for all ports on this component

            self.port_resourceOut = PortCFResourceOut_i(self, "resourceOut")

        def start(self):
            Resource.start(self)
            self.process_thread = ProcessThread(target=self.process, pause=self.PAUSE)
            self.process_thread.start()

        def process(self):
            """The process method should process a single "chunk" of data and then return.  This method will be called
            from the processing thread again, and again, and again until it returns FINISH or stop() is called on the
            component.  If no work is performed, then return NOOP"""
            raise NotImplementedError

        def stop(self):
            # Technically not thread-safe but close enough for now
            process_thread = self.process_thread
            self.process_thread = None

            if process_thread != None:
                process_thread.stop()
                process_thread.join(self.TIMEOUT)
                if process_thread.isAlive():
                    raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")
            Resource.stop(self)

        def releaseObject(self):
            try:
                self.stop()
            except Exception, e:
                self._log.exception("Error stopping: " + str(e))
            Resource.releaseObject(self)

        ######################################################################
        # PORTS
        # 
        # DO NOT ADD NEW PORTS HERE.  You can add ports in your derived class, in the SCD xml file, 
        # or via the IDE.


        # 'CF/Resource' port
        class PortCFResourceOut(CF__POA.Port):
            """This class is a port template for the resourceOut port and
            should not be instantiated nor modified.
            
            The expectation is that the specific port implementation will extend 
            from this class instead of the base CORBA class CF__POA.Port.
            """
            pass

        port_resourceOut = usesport(name="resourceOut",
                                            repid="IDL:CF/Resource:1.0",
                                            type_="control",)        

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.

'''uses port(s)'''


class PortCFResourceOut_i(BasicAC_python_impl1_base.PortCFResourceOut):
    def __init__(self, parent, name):
        self.parent = parent
        self.name = name
        self.outPorts = {}
        self.port_lock = threading.Lock()

    def connectPort(self, connection, connectionId):
        self.port_lock.acquire()
        try:
            port = connection._narrow(CF.Resource)
            self.outPorts[str(connectionId)] = port
        finally:
            self.port_lock.release()

    def disconnectPort(self, connectionId):
        self.port_lock.acquire()
        try:
            self.outPorts.pop(str(connectionId), None)
        finally:
            self.port_lock.release()
        
    def initialize(self):
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outPorts.items():
                    if port != None: port.initialize()
            except Exception, e:
                self.parent._log.exception("The call to initialize failed with %s on port %s connection %s instance %s", e, self.name, connId, port)
        finally:
            self.port_lock.release()

    def releaseObject(self):
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outPorts.items():
                    if port != None: port.releaseObject()
            except Exception, e:
                self.parent._log.exception("The call to releaseObject failed with %s on port %s connection %s instance %s", e, self.name, connId, port)
        finally:
            self.port_lock.release()

    def runTest(self, testid, testValues):
        retVal = None
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outPorts.items():
                    if port != None:retVal = port.runTest(testid, testValues)
            except Exception, e:
                self.parent._log.exception("The call to runTest failed with %s on port %s connection %s instance %s", e, self.name, connId, port)
        finally:
            self.port_lock.release()

        return retVal
 
    def configure(self, configProperties):
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outPorts.items():
                    if port != None: port.configure(configProperties)
            except Exception, e:
                self.parent._log.exception("The call to configure failed with %s on port %s connection %s instance %s", e, self.name, connId, port)
        finally:
            self.port_lock.release()

    def query(self, configProperties):
        retVal = None
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outPorts.items():
                    if port != None:retVal = port.query(configProperties)
            except Exception, e:
                self.parent._log.exception("The call to query failed with %s on port %s connection %s instance %s", e, self.name, connId, port)
        finally:
            self.port_lock.release()

        return retVal
 
    def getPort(self, name):
        retVal = None
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outPorts.items():
                    if port != None:retVal = port.getPort(name)
            except Exception, e:
                self.parent._log.exception("The call to getPort failed with %s on port %s connection %s instance %s", e, self.name, connId, port)
        finally:
            self.port_lock.release()

        return retVal
 
    def start(self):
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outPorts.items():
                    if port != None: port.start()
            except Exception, e:
                self.parent._log.exception("The call to start failed with %s on port %s connection %s instance %s", e, self.name, connId, port)
        finally:
            self.port_lock.release()

    def stop(self):
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outPorts.items():
                    if port != None: port.stop()
            except Exception, e:
                self.parent._log.exception("The call to stop failed with %s on port %s connection %s instance %s", e, self.name, connId, port)
        finally:
            self.port_lock.release()

    def _get_identifier(self):
        retVal = ""
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outPorts.items():
                    if port != None:
                        retVal = port.identifier()
            except Exception, e:
                self.parent._log.exception("The call to identifier failed with %s on port %s connection %s instance %s", e, self.name, connId, port)
        finally:
            self.port_lock.release()

        return retVal
 
    def _get_started(self):
        retVal = None
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outPorts.items():
                    if port != None:
                        retVal = port.started()
            except Exception, e:
                self.parent._log.exception("The call to started failed with %s on port %s connection %s instance %s", e, self.name, connId, port)
        finally:
            self.port_lock.release()

        return retVal
 