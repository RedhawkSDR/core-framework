#!/usr/bin/env python
#
# AUTO-GENERATED
#
# Source: svc_startorder.spd.xml

import sys, signal, copy, os
import logging

from ossie.cf import CF, CF__POA #@UnusedImport
from ossie.service import start_service
from omniORB import CORBA, URI, PortableServer, any
import Queue, copy, time, threading
from ossie.properties import simple_property

from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.cf import ExtendedCF
from ossie.cf import ExtendedCF__POA
from ossie.resource import usesport, providesport

class svc_startorder(CF__POA.Resource):

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

    def __init__(self, name="svc_startorder", execparams={}):
        self.name = name
        self._log = logging.getLogger(self.name)
        self.port_output = PortCFResourceOut_i(self, "output")
        self._start_from = ""
        self._stop_from = ""

    def terminateService(self):
        pass

    def initialize(self):
        # TODO
        pass

    def releaseObject(self):
        # TODO
        pass

    def runTest(self, testid, testValues):
        # TODO
        pass

    def configure(self, configProperties):
        for _prop in configProperties:
            if _prop.id == 'start_from':
                self._start_from = _prop.value._v
            if _prop.id == 'stop_from':
                self._stop_from = _prop.value._v

    def query(self, configProperties):
        retval = []
        if len(configProperties) == 0:
            retval.append(CF.DataType(id='start_from',value=any.to_any(self._start_from)))
            retval.append(CF.DataType(id='stop_from',value=any.to_any(self._stop_from)))
        elif len(configProperties) == 1:
            if configProperties[0].id == 'start_from':
                retval.append(CF.DataType(id='start_from',value=any.to_any(self._start_from)))
            if configProperties[0].id == 'stop_from':
                retval.append(CF.DataType(id='stop_from',value=any.to_any(self._stop_from)))
        return retval

    def initializeProperties(self, initialProperties):
        # TODO
        pass

    def registerPropertyListener(self, obj, prop_ids, interval):
        # TODO
        pass

    def unregisterPropertyListener(self, id):
        # TODO
        pass

    def getPort(self, name):
        if name == 'output':
            return self.port_output._this()
        
    def getPortSet(self):
        # TODO
        pass

    def retrieve_records(self, howMany, startingRecord):
        # TODO
        pass

    def retrieve_records_by_date(self, howMany, to_timeStamp):
        # TODO
        pass

    def retrieve_records_from_date(self, howMany, from_timeStamp):
        # TODO
        pass

    def setLogLevel(self, logger_id, newLevel):
        # TODO
        pass

    def getLogConfig(self):
        # TODO
        pass

    def setLogConfig(self, config_contents):
        # TODO
        pass

    def setLogConfigURL(self, config_url):
        # TODO
        pass

    def start(self):
        self._log.info("starting "+self.name)
        devs = self.getDeviceManager().ref.registeredDevices
        for dev in devs:
            if dev._get_label() == 'dev_startorder_1':
                if len(self.port_output._get_connections())==0:
                    self.port_output.connectPort(dev, 'service_connection')
        val = CF.DataType(id='start_from', value=any.to_any(self._start_from+','+self.name))
        self.port_output.configure([val])

    def stop(self):
        self._log.info("stopping "+self.name)

    def _get_log_level(self):
        # TODO
        pass

    def _set_log_level(self, data):
        # TODO
        pass

    def _get_identifier(self):
        # TODO
        pass

    def _get_started(self):
        # TODO
        pass

    def _get_softwareProfile(self):
        # TODO
        pass

class PortCFResourceOut_i(svc_startorder.PortCFResourceOut):
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


if __name__ == '__main__':
    start_service(svc_startorder, thread_policy=PortableServer.SINGLE_THREAD_MODEL)
