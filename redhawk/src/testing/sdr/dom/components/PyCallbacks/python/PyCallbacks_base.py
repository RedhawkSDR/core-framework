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
# Source: PyCallbacks.spd.xml
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

from ossie.resource import Resource
from ossie.threadedcomponent import *
from ossie.properties import simple_property
from ossie.properties import simpleseq_property
from ossie.properties import struct_property
from ossie.properties import structseq_property

import Queue, copy, time, threading

class PyCallbacks_base(CF__POA.Resource, Resource, ThreadedComponent):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block

        def __init__(self, identifier, execparams):
            loggerName = (execparams['NAME_BINDING'].replace('/', '.')).rsplit("_", 1)[0]
            Resource.__init__(self, identifier, execparams, loggerName=loggerName)
            ThreadedComponent.__init__(self)

            # self.auto_start is deprecated and is only kept for API compatibility
            # with 1.7.X and 1.8.0 components.  This variable may be removed
            # in future releases
            self.auto_start = False
            # Instantiate the default implementations for all ports on this component

        def start(self):
            Resource.start(self)
            ThreadedComponent.startThread(self, pause=self.PAUSE)

        def stop(self):
            if not ThreadedComponent.stopThread(self, self.TIMEOUT):
                raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")
            Resource.stop(self)

        def releaseObject(self):
            try:
                self.stop()
            except Exception:
                self._log.exception("Error stopping")
            Resource.releaseObject(self)

        ######################################################################
        # PORTS
        # 
        # DO NOT ADD NEW PORTS HERE.  You can add ports in your derived class, in the SCD xml file, 
        # or via the IDE.

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.
        count = simple_property(id_="count",
                                type_="ulong",
                                defvalue=0,
                                mode="readwrite",
                                action="external",
                                kinds=("configure",))
        
        constellation = simpleseq_property(id_="constellation",
                                           type_="float",
                                           defvalue=[
                                                      complex(1.0,0.0),
                                                      complex(0.0,1.0),
                                                      complex(-1.0,0.0),
                                                      complex(0.0,-1.0),
                                                      ],
                                           complex=True,
                                           mode="readwrite",
                                           action="external",
                                           kinds=("configure",))
        
        callbacks_run = simpleseq_property(id_="callbacks_run",
                                           type_="string",
                                           defvalue=None,
                                           mode="readwrite",
                                           action="external",
                                           kinds=("configure",))
        
        class Station(object):
            name = simple_property(id_="name",
                                   type_="string",
                                   defvalue="WBJC")
        
            frequency = simple_property(id_="frequency",
                                        type_="float",
                                        defvalue=91.7)
        
            def __init__(self, **kw):
                """Construct an initialized instance of this struct definition"""
                for attrname, classattr in type(self).__dict__.items():
                    if type(classattr) == simple_property:
                        classattr.initialize(self)
                for k,v in kw.items():
                    setattr(self,k,v)
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["name"] = self.name
                d["frequency"] = self.frequency
                return str(d)
        
            def getId(self):
                return "station"
        
            def isStruct(self):
                return True
        
            def getMembers(self):
                return [("name",self.name),("frequency",self.frequency)]
        
        station = struct_property(id_="station",
                                  structdef=Station,
                                  configurationkind=("configure",),
                                  mode="readwrite")
        
        class Endpoint(object):
            host = simple_property(id_="host",
                                   type_="string")
        
            port = simple_property(id_="port",
                                   type_="short")
        
            def __init__(self, host="", port=0):
                self.host = host
                self.port = port
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["host"] = self.host
                d["port"] = self.port
                return str(d)
        
            def getId(self):
                return "endpoint"
        
            def isStruct(self):
                return True
        
            def getMembers(self):
                return [("host",self.host),("port",self.port)]
        
        servers = structseq_property(id_="servers",
                                     structdef=Endpoint,
                                     defvalue=[],
                                     configurationkind=("configure",),
                                     mode="readwrite")
        

