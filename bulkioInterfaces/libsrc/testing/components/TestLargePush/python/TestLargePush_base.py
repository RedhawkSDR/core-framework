#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
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
# Source: TestLargePush.spd.xml
from ossie.cf import CF, CF__POA
from ossie.utils import uuid

from ossie.resource import Resource
from ossie.properties import simple_property

import Queue, copy, time, threading
from ossie.resource import usesport, providesport
import bulkio

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
            delay = 1e-6
            if (state == NOOP):
                # If there was no data to process sleep to avoid spinning
                delay = self.pause
            time.sleep(delay)

class TestLargePush_base(CF__POA.Resource, Resource):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block

        def __init__(self, identifier, execparams):
            loggerName = (execparams['NAME_BINDING'].replace('/', '.')).rsplit("_", 1)[0]
            Resource.__init__(self, identifier, execparams, loggerName=loggerName)
            self.threadControlLock = threading.RLock()
            self.process_thread = None
            # self.auto_start is deprecated and is only kept for API compatibility
            # with 1.7.X and 1.8.0 components.  This variable may be removed
            # in future releases
            self.auto_start = False

        def initialize(self):
            Resource.initialize(self)
            
            # Instantiate the default implementations for all ports on this component
            self.port_dataChar = bulkio.OutCharPort("dataChar")
            self.port_dataFile = bulkio.OutFilePort("dataFile")
            self.port_dataShort = bulkio.OutShortPort("dataShort")
            self.port_dataUlong = bulkio.OutULongPort("dataUlong")
            self.port_dataUlongLong = bulkio.OutULongLongPort("dataUlongLong")
            self.port_dataUshort = bulkio.OutUShortPort("dataUshort")
            self.port_dataXML = bulkio.OutXMLPort("dataXML")
            self.port_dataLong = bulkio.OutLongPort("dataLong")
            self.port_dataLongLong = bulkio.OutLongLongPort("dataLongLong")
            self.port_dataOctet = bulkio.OutOctetPort("dataOctet")
            self.port_dataFloat = bulkio.OutFloatPort("dataFloat")
            self.port_dataSDDS = bulkio.OutSDDSPort("dataSDDS")
            self.port_dataDouble = bulkio.OutDoublePort("dataDouble")

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

        port_dataChar = usesport(name="dataChar",
                                 repid="IDL:BULKIO/dataChar:1.0",
                                 type_="control")

        port_dataFile = usesport(name="dataFile",
                                 repid="IDL:BULKIO/dataFile:1.0",
                                 type_="control")

        port_dataShort = usesport(name="dataShort",
                                  repid="IDL:BULKIO/dataShort:1.0",
                                  type_="control")

        port_dataUlong = usesport(name="dataUlong",
                                  repid="IDL:BULKIO/dataUlong:1.0",
                                  type_="control")

        port_dataUlongLong = usesport(name="dataUlongLong",
                                      repid="IDL:BULKIO/dataUlongLong:1.0",
                                      type_="control")

        port_dataUshort = usesport(name="dataUshort",
                                   repid="IDL:BULKIO/dataUshort:1.0",
                                   type_="control")

        port_dataXML = usesport(name="dataXML",
                                repid="IDL:BULKIO/dataXML:1.0",
                                type_="control")

        port_dataLong = usesport(name="dataLong",
                                 repid="IDL:BULKIO/dataLong:1.0",
                                 type_="control")

        port_dataLongLong = usesport(name="dataLongLong",
                                     repid="IDL:BULKIO/dataLongLong:1.0",
                                     type_="control")

        port_dataOctet = usesport(name="dataOctet",
                                  repid="IDL:BULKIO/dataOctet:1.0",
                                  type_="control")

        port_dataFloat = usesport(name="dataFloat",
                                  repid="IDL:BULKIO/dataFloat:1.0",
                                  type_="control")

        port_dataSDDS = usesport(name="dataSDDS",
                                 repid="IDL:BULKIO/dataSDDS:1.0",
                                 type_="control")

        port_dataDouble = usesport(name="dataDouble",
                                   repid="IDL:BULKIO/dataDouble:1.0",
                                   type_="control")

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.
        numSamples = simple_property(id_="numSamples",
                                     type_="ulonglong",
                                     defvalue=3000000,
                                     mode="readwrite",
                                     action="external",
                                     kinds=("configure",),
                                     description="""number of samples to send in the push.  should be large enough to exceed the max message size for a CORBA packet, which will force the packet chunking to be exercised."""
                                     )

