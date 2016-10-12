#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: through.spd.xml
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

from ossie.component import Component
from ossie.threadedcomponent import *

import Queue, copy, time, threading
from ossie.resource import usesport, providesport
from ossie.cf import ExtendedCF
from ossie.cf import ExtendedCF__POA

class through_base(CF__POA.Resource, Component, ThreadedComponent):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block

        def __init__(self, identifier, execparams):
            loggerName = (execparams['NAME_BINDING'].replace('/', '.')).rsplit("_", 1)[0]
            Component.__init__(self, identifier, execparams, loggerName=loggerName)
            ThreadedComponent.__init__(self)

            # self.auto_start is deprecated and is only kept for API compatibility
            # with 1.7.X and 1.8.0 components.  This variable may be removed
            # in future releases
            self.auto_start = False
            # Instantiate the default implementations for all ports on this component
            self.port_input = PortCFLifeCycleIn_i(self, "input")
            self.port_output = PortCFLifeCycleOut_i(self, "output")

        def start(self):
            Component.start(self)
            ThreadedComponent.startThread(self, pause=self.PAUSE)

        def stop(self):
            Component.stop(self)
            if not ThreadedComponent.stopThread(self, self.TIMEOUT):
                raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")

        def releaseObject(self):
            try:
                self.stop()
            except Exception:
                self._log.exception("Error stopping")
            Component.releaseObject(self)

        ######################################################################
        # PORTS
        # 
        # DO NOT ADD NEW PORTS HERE.  You can add ports in your derived class, in the SCD xml file, 
        # or via the IDE.

        # 'CF/LifeCycle' port
        class PortCFLifeCycleIn(CF__POA.LifeCycle):
            """This class is a port template for the PortCFLifeCycleIn_i port and
            should not be instantiated nor modified.
            
            The expectation is that the specific port implementation will extend
            from this class instead of the base CORBA class CF__POA.LifeCycle.
            """
            pass

        # 'CF/LifeCycle' port
        class PortCFLifeCycleOut(ExtendedCF__POA.QueryablePort):
            """This class is a port template for the PortCFLifeCycleOut_i port and
            should not be instantiated nor modified.
            
            The expectation is that the specific port implementation will extend
            from this class instead of the base CORBA class ExtendedCF__POA.QueryablePort.
            """
            pass

        port_input = providesport(name="input",
                                  repid="IDL:CF/LifeCycle:1.0",
                                  type_="control")

        port_output = usesport(name="output",
                               repid="IDL:CF/LifeCycle:1.0",
                               type_="control")

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.

'''provides port(s)'''

class PortCFLifeCycleIn_i(through_base.PortCFLifeCycleIn):
    def __init__(self, parent, name):
        self.parent = parent
        self.name = name
        self.sri = None
        self.queue = Queue.Queue()
        self.port_lock = threading.Lock()

    def initialize(self):
        # TODO:
        pass

    def releaseObject(self):
        # TODO:
        pass

'''uses port(s)'''

class PortCFLifeCycleOut_i(through_base.PortCFLifeCycleOut):
    def __init__(self, parent, name):
        self.parent = parent
        self.name = name
        self.outConnections = {}
        self.port_lock = threading.Lock()

    def connectPort(self, connection, connectionId):
        self.port_lock.acquire()
        try:
            port = connection._narrow(CF.LifeCycle)
            self.outConnections[str(connectionId)] = port
        finally:
            self.port_lock.release()

    def disconnectPort(self, connectionId):
        self.port_lock.acquire()
        try:
            self.outConnections.pop(str(connectionId), None)
        finally:
            self.port_lock.release()

    def _get_connections(self):
        self.port_lock.acquire()
        try:
            return [ExtendedCF.UsesConnection(name, port) for name, port in self.outConnections.iteritems()]
        finally:
            self.port_lock.release()

    def initialize(self):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.initialize()
                    except Exception:
                        self.parent._log.exception("The call to initialize failed on port %s connection %s instance %s", self.name, connId, port)
        finally:
            self.port_lock.release()

    def releaseObject(self):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.releaseObject()
                    except Exception:
                        self.parent._log.exception("The call to releaseObject failed on port %s connection %s instance %s", self.name, connId, port)
        finally:
            self.port_lock.release()


