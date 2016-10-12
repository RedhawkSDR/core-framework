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
# Source: Python_Ports.spd.xml
# Generated on: Mon Jun 03 09:28:54 EDT 2013
# REDHAWK IDE
# Version: 1.9.0
# Build id: ${buildType}201305162033
from ossie.cf import CF, CF__POA
from ossie.utils import uuid

from ossie.resource import Resource
from ossie.properties import simple_property

import Queue, copy, time, threading
from ossie.resource import usesport, providesport
from ossie.cf import ExtendedCF
from omniORB import CORBA
import struct #@UnresolvedImport
#from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA #@UnusedImport 
from ossie.events import PropertyEventSupplier
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
            if (state == NOOP):
                # If there was no data to process sleep to avoid spinning
                time.sleep(self.pause)

class Python_Ports_base(CF__POA.Resource, Resource):
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

            self.port_dataCharIn = bulkio.InCharPort( "dataCharIn")
            self.port_dataOctetIn = bulkio.InOctetPort( "dataOctetIn")
            self.port_dataShortIn = bulkio.InShortPort( "dataShortIn")
            self.port_dataUShortIn = bulkio.InUShortPort( "dataUShortIn")
            self.port_dataLongIn = bulkio.InLongPort( "dataLongIn")
            self.port_dataULongIn = bulkio.InULongPort( "dataULongIn")
            self.port_dataLongLongIn = bulkio.InLongLongPort( "dataLongLongIn")
            self.port_dataULongLongIn = bulkio.InULongLongPort( "dataULongLongIn")
            self.port_dataFloatIn = bulkio.InFloatPort( "dataFloatIn")
            self.port_dataDoubleIn = bulkio.InDoublePort( "dataDoubleIn")
            self.port_dataFileIn = bulkio.InFilePort( "dataFileIn")
            self.port_dataXMLIn = bulkio.InXMLPort( "dataXMLIn")
            self.port_dataSDDSIn = bulkio.InSDDSPort( "dataSDDSIn")


            self.port_dataCharOut = bulkio.OutCharPort( "dataCharOut")
            self.port_dataOctetOut = bulkio.OutOctetPort( "dataOctetOut")
            self.port_dataShortOut = bulkio.OutShortPort( "dataShortOut")
            self.port_dataUShortOut = bulkio.OutUShortPort( "dataUShortOut")
            self.port_dataLongOut = bulkio.OutLongPort( "dataLongOut")
            self.port_dataULongOut = bulkio.OutULongPort( "dataULongOut")
            self.port_dataLongLongOut = bulkio.OutLongLongPort( "dataLongLongOut")
            self.port_dataULongLongOut = bulkio.OutULongLongPort( "dataULongLongOut")
            self.port_dataFloatOut = bulkio.OutFloatPort( "dataFloatOut")
            self.port_dataDoubleOut = bulkio.OutDoublePort( "dataDoubleOut")
            self.port_dataFileOut = bulkio.OutFilePort( "dataFileOut")
            self.port_dataXMLOut = bulkio.OutXMLPort( "dataXMLOut")
            self.port_dataSDDSOut = bulkio.OutSDDSPort( "dataSDDSOut")

            self.port_propEvent = PropertyEventSupplier(self)

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



        port_dataCharIn = providesport(name="dataCharIn",
                                            repid="IDL:BULKIO/dataChar:1.0",
                                            type_="control",)

        port_dataOctetIn = providesport(name="dataOctetIn",
                                            repid="IDL:BULKIO/dataOctet:1.0",
                                            type_="control",)

        port_dataShortIn = providesport(name="dataShortIn",
                                            repid="IDL:BULKIO/dataShort:1.0",
                                            type_="control",)

        port_dataUShortIn = providesport(name="dataUShortIn",
                                            repid="IDL:BULKIO/dataUshort:1.0",
                                            type_="control",)

        port_dataLongIn = providesport(name="dataLongIn",
                                            repid="IDL:BULKIO/dataLong:1.0",
                                            type_="control",)

        port_dataULongIn = providesport(name="dataULongIn",
                                            repid="IDL:BULKIO/dataUlong:1.0",
                                            type_="control",)

        port_dataLongLongIn = providesport(name="dataLongLongIn",
                                            repid="IDL:BULKIO/dataLongLong:1.0",
                                            type_="control",)

        port_dataULongLongIn = providesport(name="dataULongLongIn",
                                            repid="IDL:BULKIO/dataUlongLong:1.0",
                                            type_="control",)

        port_dataFloatIn = providesport(name="dataFloatIn",
                                            repid="IDL:BULKIO/dataFloat:1.0",
                                            type_="control",)

        port_dataDoubleIn = providesport(name="dataDoubleIn",
                                            repid="IDL:BULKIO/dataDouble:1.0",
                                            type_="control",)

        port_dataFileIn = providesport(name="dataFileIn",
                                            repid="IDL:BULKIO/dataFile:1.0",
                                            type_="control",)

        port_dataXMLIn = providesport(name="dataXMLIn",
                                            repid="IDL:BULKIO/dataXML:1.0",
                                            type_="control",)

        port_dataSDDSIn = providesport(name="dataSDDSIn",
                                            repid="IDL:BULKIO/dataSDDS:1.0",
                                            type_="data",)

        port_dataCharOut = usesport(name="dataCharOut",
                                            repid="IDL:BULKIO/dataChar:1.0",
                                            type_="control",)

        port_dataOctetOut = usesport(name="dataOctetOut",
                                            repid="IDL:BULKIO/dataOctet:1.0",
                                            type_="control",)

        port_dataShortOut = usesport(name="dataShortOut",
                                            repid="IDL:BULKIO/dataShort:1.0",
                                            type_="control",)

        port_dataUShortOut = usesport(name="dataUShortOut",
                                            repid="IDL:BULKIO/dataUshort:1.0",
                                            type_="control",)

        port_dataLongOut = usesport(name="dataLongOut",
                                            repid="IDL:BULKIO/dataLong:1.0",
                                            type_="control",)

        port_dataULongOut = usesport(name="dataULongOut",
                                            repid="IDL:BULKIO/dataUlong:1.0",
                                            type_="control",)


        port_dataLongLongOut = usesport(name="dataLongLongOut",
                                            repid="IDL:BULKIO/dataLongLong:1.0",
                                            type_="control",)

        port_dataULongLongOut = usesport(name="dataULongLongOut",
                                            repid="IDL:BULKIO/dataUlongLong:1.0",
                                            type_="control",)

        port_dataFloatOut = usesport(name="dataFloatOut",
                                            repid="IDL:BULKIO/dataFloat:1.0",
                                            type_="control",)

        port_dataDoubleOut = usesport(name="dataDoubleOut",
                                            repid="IDL:BULKIO/dataDouble:1.0",
                                            type_="control",)

        port_dataFileOut = usesport(name="dataFileOut",
                                            repid="IDL:BULKIO/dataFile:1.0",
                                            type_="control",)

        port_dataXMLOut = usesport(name="dataXMLOut",
                                            repid="IDL:BULKIO/dataXML:1.0",
                                            type_="control",)

        port_dataSDDSOut = usesport(name="dataSDDSOut",
                                            repid="IDL:BULKIO/dataSDDS:1.0",
                                            type_="control",)

        port_propEvent = usesport(name="propEvent",
                                            repid="IDL:omg.org/CosEventChannelAdmin/EventChannel:1.0",
                                            type_="responses",)        

