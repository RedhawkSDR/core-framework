#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: dev_startorder.spd.xml
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

from ossie.device import Device
from ossie.threadedcomponent import *
from ossie.properties import simple_property

import Queue, copy, time, threading
from ossie.resource import usesport, providesport
from ossie.cf import ExtendedCF
from ossie.cf import ExtendedCF__POA

class dev_startorder_base(CF__POA.Device, Device, ThreadedComponent):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block

        def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
            Device.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)
            ThreadedComponent.__init__(self)

            # self.auto_start is deprecated and is only kept for API compatibility
            # with 1.7.X and 1.8.0 devices.  This variable may be removed
            # in future releases
            self.auto_start = False
            # Instantiate the default implementations for all ports on this device
            self.port_output = PortCFResourceOut_i(self, "output")

        def start(self):
            Device.start(self)
            ThreadedComponent.startThread(self, pause=self.PAUSE)

        def stop(self):
            Device.stop(self)
            if not ThreadedComponent.stopThread(self, self.TIMEOUT):
                raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")

        def releaseObject(self):
            try:
                self.stop()
            except Exception:
                self._log.exception("Error stopping")
            Device.releaseObject(self)

        ######################################################################
        # PORTS
        # 
        # DO NOT ADD NEW PORTS HERE.  You can add ports in your derived class, in the SCD xml file, 
        # or via the IDE.

        # 'CF/Resource' port
        class PortCFResourceOut(ExtendedCF__POA.QueryablePort):
            """This class is a port template for the PortCFResourceOut_i port and
            should not be instantiated nor modified.
            
            The expectation is that the specific port implementation will extend
            from this class instead of the base CORBA class ExtendedCF__POA.QueryablePort.
            """
            pass

        port_output = usesport(name="output",
                               repid="IDL:CF/Resource:1.0",
                               type_="control")

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.
        device_kind = simple_property(id_="DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d",
                                      name="device_kind",
                                      type_="string",
                                      mode="readonly",
                                      action="eq",
                                      kinds=("allocation",),
                                      description="""This specifies the device kind""")


        device_model = simple_property(id_="DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb",
                                       name="device_model",
                                       type_="string",
                                       mode="readonly",
                                       action="eq",
                                       kinds=("allocation",),
                                       description=""" This specifies the specific device""")


        start_from = simple_property(id_="start_from",
                                     type_="string",
                                     defvalue="",
                                     mode="readwrite",
                                     action="external",
                                     kinds=("property",))


        stop_from = simple_property(id_="stop_from",
                                    type_="string",
                                    defvalue="",
                                    mode="readwrite",
                                    action="external",
                                    kinds=("property",))



'''uses port(s)'''

class PortCFResourceOut_i(dev_startorder_base.PortCFResourceOut):
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
                        raise
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
                        raise
        finally:
            self.port_lock.release()

    def runTest(self, testid, testValues):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.runTest(testid, testValues)
                    except Exception:
                        self.parent._log.exception("The call to runTest failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        return retVal

    def configure(self, configProperties):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.configure(configProperties)
                    except Exception:
                        self.parent._log.exception("The call to configure failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

    def query(self, configProperties):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.query(configProperties)
                    except Exception:
                        self.parent._log.exception("The call to query failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        return retVal

    def initializeProperties(self, initialProperties):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.initializeProperties(initialProperties)
                    except Exception:
                        self.parent._log.exception("The call to initializeProperties failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

    def registerPropertyListener(self, obj, prop_ids, interval):
        retVal = ""
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.registerPropertyListener(obj, prop_ids, interval)
                    except Exception:
                        self.parent._log.exception("The call to registerPropertyListener failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        return retVal

    def unregisterPropertyListener(self, id):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.unregisterPropertyListener(id)
                    except Exception:
                        self.parent._log.exception("The call to unregisterPropertyListener failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

    def getPort(self, name):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getPort(name)
                    except Exception:
                        self.parent._log.exception("The call to getPort failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        return retVal

    def getPortSet(self):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getPortSet()
                    except Exception:
                        self.parent._log.exception("The call to getPortSet failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        return retVal

    def retrieve_records(self, howMany, startingRecord):
        retVal = []
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.retrieve_records(howMany, startingRecord)
                    except Exception:
                        self.parent._log.exception("The call to retrieve_records failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        return retVal

    def retrieve_records_by_date(self, howMany, to_timeStamp):
        retVal = []
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.retrieve_records_by_date(howMany, to_timeStamp)
                    except Exception:
                        self.parent._log.exception("The call to retrieve_records_by_date failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        return retVal

    def retrieve_records_from_date(self, howMany, from_timeStamp):
        retVal = []
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.retrieve_records_from_date(howMany, from_timeStamp)
                    except Exception:
                        self.parent._log.exception("The call to retrieve_records_from_date failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        return retVal

    def setLogLevel(self, logger_id, newLevel):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.setLogLevel(logger_id, newLevel)
                    except Exception:
                        self.parent._log.exception("The call to setLogLevel failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

    def getLogConfig(self):
        retVal = ""
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getLogConfig()
                    except Exception:
                        self.parent._log.exception("The call to getLogConfig failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        return retVal

    def setLogConfig(self, config_contents):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.setLogConfig(config_contents)
                    except Exception:
                        self.parent._log.exception("The call to setLogConfig failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

    def setLogConfigURL(self, config_url):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.setLogConfigURL(config_url)
                    except Exception:
                        self.parent._log.exception("The call to setLogConfigURL failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

    def start(self):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.start()
                    except Exception:
                        self.parent._log.exception("The call to start failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

    def stop(self):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.stop()
                    except Exception:
                        self.parent._log.exception("The call to stop failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

    def _get_log_level(self):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port._get_log_level()
                    except Exception:
                        self.parent._log.exception("The call to _get_log_level failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        return retVal

    def _set_log_level(self, data):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port._set_log_level(data)
                    except Exception:
                        self.parent._log.exception("The call to _set_log_level failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

    def _get_identifier(self):
        retVal = ""
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port._get_identifier()
                    except Exception:
                        self.parent._log.exception("The call to _get_identifier failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        return retVal

    def _get_started(self):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port._get_started()
                    except Exception:
                        self.parent._log.exception("The call to _get_started failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        return retVal

    def _get_softwareProfile(self):
        retVal = ""
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port._get_softwareProfile()
                    except Exception:
                        self.parent._log.exception("The call to _get_softwareProfile failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        return retVal


