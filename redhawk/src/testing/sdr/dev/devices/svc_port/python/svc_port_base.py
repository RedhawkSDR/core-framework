#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: svc_port.spd.xml
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

from ossie.device import Device
from ossie.threadedcomponent import *
from ossie.properties import simple_property

import Queue, copy, time, threading
from ossie.resource import usesport, providesport, PortCallError

class svc_port_base(CF__POA.Device, Device, ThreadedComponent):
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
            self.port_svc1_in = PortCFLogConfigurationIn_i(self, "svc1_in")
            self.port_svc1_in._portLog = self._baseLog.getChildLogger('svc1_in', 'ports')
            self.port_svc2_in = PortCFLogConfigurationIn_i(self, "svc2_in")
            self.port_svc2_in._portLog = self._baseLog.getChildLogger('svc2_in', 'ports')

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
                self._baseLog.exception("Error stopping")
            Device.releaseObject(self)

        ######################################################################
        # PORTS
        # 
        # DO NOT ADD NEW PORTS HERE.  You can add ports in your derived class, in the SCD xml file, 
        # or via the IDE.

        # 'CF/LogConfiguration' port
        class PortCFLogConfigurationIn(CF__POA.LogConfiguration):
            """This class is a port template for the PortCFLogConfigurationIn_i port and
            should not be instantiated nor modified.
            
            The expectation is that the specific port implementation will extend
            from this class instead of the base CORBA class CF__POA.LogConfiguration.
            """
            pass

        class PortCFDeviceIn(CF__POA.Device):
            """This class is a port template for the PortCFLogConfigurationIn_i port and
            should not be instantiated nor modified.
            
            The expectation is that the specific port implementation will extend
            from this class instead of the base CORBA class CF__POA.LogConfiguration.
            """
            pass

        port_svc1_in = providesport(name="svc1_in",
                                    repid="IDL:CF/LogConfiguration:1.0",
                                    type_="control")

        port_svc2_in = providesport(name="svc2_in",
                                    repid="IDL:CF/LogConfiguration:1.0",
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


        svc_1 = simple_property(id_="svc_1",
                                name="svc_1",
                                type_="string",
                                mode="readwrite",
                                action="external",
                                kinds=("property",))


        svc_2 = simple_property(id_="svc_2",
                                name="svc_2",
                                type_="string",
                                mode="readwrite",
                                action="external",
                                kinds=("property",))


'''provides port(s). Send logging to _portLog '''

class PortCFLogConfigurationIn_i(svc_port_base.PortCFLogConfigurationIn):
    def __init__(self, parent, name):
        self.parent = parent
        self.name = name
        self.sri = None
        self.queue = Queue.Queue()
        self.port_lock = threading.Lock()

    def getLogLevel(self, logger_id):
        # TODO:
        pass

    def setLogLevel(self, logger_id, newLevel):
        # TODO:
        pass

    def getNamedLoggers(self):
        # TODO:
        pass

    def resetLog(self):
        # TODO:
        pass

    def getLogConfig(self):
        # TODO:
        pass

    def setLogConfig(self, config_contents):
        # TODO:
        pass

    def setLogConfigURL(self, config_url):
        # TODO:
        pass

    def _get_log_level(self):
        # TODO:
        pass

    def _set_log_level(self, data):
        # TODO:
        pass
