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
# Source: ac_with_properties.spd.xml
# Generated on: Mon Apr 15 15:46:35 EDT 2013
# REDHAWK IDE
# Version: R.1.8.3
# Build id: v201303122306
from ossie.cf import CF, CF__POA
from ossie.utils import uuid

from ossie.resource import Resource
from ossie.properties import simple_property

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

class ac_with_properties_base(CF__POA.Resource, Resource):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block
        
        def __init__(self, identifier, execparams):
            loggerName = (execparams['NAME_BINDING'].replace('/', '.')).rsplit("_", 1)[0]
            Resource.__init__(self, identifier, execparams, loggerName=loggerName)
            self.threadControlLock = threading.RLock()
            self.process_thread = None
            # self.auto_start is deprecated and is only kept for API compatability
            # with 1.7.X and 1.8.0 components.  This variable may be removed
            # in future releases
            self.auto_start = False
            
        def initialize(self):
            Resource.initialize(self)
            
            # Instantiate the default implementations for all ports on this component

            self.port_components = PortCFResourceOut_i(self, "components")

        def start(self):
            self.threadControlLock.acquire()
            try:
                Resource.start(self)
                if self.process_thread == None:
                    self.process_thread = ProcessThread(target=self.process, pause=self.PAUSE)
                    self.process_thread.start()
            finally:
                self.threadControlLock.release()

        def process(self):
            """The process method should process a single "chunk" of data and then return.  This method will be called
            from the processing thread again, and again, and again until it returns FINISH or stop() is called on the
            component.  If no work is performed, then return NOOP"""
            raise NotImplementedError

        def stop(self):
            self.threadControlLock.acquire()
            try:
                process_thread = self.process_thread
                self.process_thread = None

                if process_thread != None:
                    process_thread.stop()
                    process_thread.join(self.TIMEOUT)
                    if process_thread.isAlive():
                        raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")
                Resource.stop(self)
            finally:
                self.threadControlLock.release()

        def releaseObject(self):
            try:
                self.stop()
            except Exception:
                self._log.exception("Error stopping")
            self.threadControlLock.acquire()
            try:
                Resource.releaseObject(self)
            finally:
                self.threadControlLock.release()

        ######################################################################
        # PORTS
        # 
        # DO NOT ADD NEW PORTS HERE.  You can add ports in your derived class, in the SCD xml file, 
        # or via the IDE.
        
        def compareSRI(self, a, b):
            if a.hversion != b.hversion:
                return False
            if a.xstart != b.xstart:
                return False
            if a.xdelta != b.xdelta:
                return False
            if a.xunits != b.xunits:
                return False
            if a.subsize != b.subsize:
                return False
            if a.ystart != b.ystart:
                return False
            if a.ydelta != b.ydelta:
                return False
            if a.yunits != b.yunits:
                return False
            if a.mode != b.mode:
                return False
            if a.streamID != b.streamID:
                return False
            if a.blocking != b.blocking:
                return False
            if len(a.keywords) != len(b.keywords):
                return False
            for keyA, keyB in zip(a.keywords, b.keywords):
                if keyA.value._t != keyB.value._t:
                    return False
                if keyA.value._v != keyB.value._v:
                    return False
            return True


        # 'CF/Resource' port
        class PortCFResourceOut(CF__POA.Port):
            """This class is a port template for the components port and
            should not be instantiated nor modified.
            
            The expectation is that the specific port implementation will extend 
            from this class instead of the base CORBA class CF__POA.Port.
            """
            pass

        port_components = usesport(name="components",
                                            repid="IDL:CF/Resource:1.0",
                                            type_="control",)        

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.       
        other_component = simple_property(id_="other_component",
                                          type_="float",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        trigger_property = simple_property(id_="trigger_property",
                                          type_="string",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )

'''uses port(s)'''


class PortCFResourceOut_i(ac_with_properties_base.PortCFResourceOut):
    def __init__(self, parent, name):
        self.parent = parent
        self.name = name
        self.outConnections = {}
        self.port_lock = threading.Lock()

    def connectPort(self, connection, connectionId):
        self.port_lock.acquire()
        try:
            port = connection._narrow(CF.Resource)
            self.outConnections[str(connectionId)] = port
        finally:
            self.port_lock.release()

    def disconnectPort(self, connectionId):
        self.port_lock.acquire()
        try:
            self.outConnections.pop(str(connectionId), None)
        finally:
            self.port_lock.release()
        
    def initialize(self):
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outConnections.items():
                    if port != None: port.initialize()
            except Exception:
                self.parent._log.exception("The call to initialize failed on port %s connection %s instance %s", self.name, connId, port)
        finally:
            self.port_lock.release()

    def releaseObject(self):
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outConnections.items():
                    if port != None: port.releaseObject()
            except Exception:
                self.parent._log.exception("The call to releaseObject failed on port %s connection %s instance %s", self.name, connId, port)
        finally:
            self.port_lock.release()

    def runTest(self, testid, testValues):
        retVal = None
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outConnections.items():
                    if port != None:retVal = port.runTest(testid, testValues)
            except Exception:
                self.parent._log.exception("The call to runTest failed on port %s connection %s instance %s", self.name, connId, port)
        finally:
            self.port_lock.release()

        return retVal
 
    def configure(self, configProperties):
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outConnections.items():
                    if port != None: port.configure(configProperties)
            except Exception:
                self.parent._log.exception("The call to configure failed on port %s connection %s instance %s", self.name, connId, port)
        finally:
            self.port_lock.release()

    def query(self, configProperties):
        retVal = None
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outConnections.items():
                    if port != None:retVal = port.query(configProperties)
            except Exception:
                self.parent._log.exception("The call to query failed on port %s connection %s instance %s", self.name, connId, port)
        finally:
            self.port_lock.release()

        return retVal
 
    def getPort(self, name):
        retVal = None
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outConnections.items():
                    if port != None:retVal = port.getPort(name)
            except Exception:
                self.parent._log.exception("The call to getPort failed on port %s connection %s instance %s", self.name, connId, port)
        finally:
            self.port_lock.release()

        return retVal
 
    def start(self):
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outConnections.items():
                    if port != None: port.start()
            except Exception:
                self.parent._log.exception("The call to start failed on port %s connection %s instance %s", self.name, connId, port)
        finally:
            self.port_lock.release()

    def stop(self):
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outConnections.items():
                    if port != None: port.stop()
            except Exception:
                self.parent._log.exception("The call to stop failed on port %s connection %s instance %s", self.name, connId, port)
        finally:
            self.port_lock.release()

    def _get_identifier(self):
        retVal = ""
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outConnections.items():
                    if port != None:
                        retVal = port._get_identifier()
            except Exception:
                self.parent._log.exception("The call to _get_identifier failed on port %s connection %s instance %s", self.name, connId, port)
        finally:
            self.port_lock.release()

        return retVal
 
    def _get_started(self):
        retVal = None
        self.port_lock.acquire()

        try:    
            try:
                for connId, port in self.outConnections.items():
                    if port != None:
                        retVal = port._get_started()
            except Exception:
                self.parent._log.exception("The call to _get_started failed on port %s connection %s instance %s", self.name, connId, port)
        finally:
            self.port_lock.release()

        return retVal
  
