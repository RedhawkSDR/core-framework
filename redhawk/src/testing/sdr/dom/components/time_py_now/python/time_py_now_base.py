#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: time_py_now.spd.xml
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

from ossie.component import Component
from ossie.threadedcomponent import *
from ossie.properties import simple_property
from ossie.properties import simpleseq_property

import Queue, copy, time, threading

class time_py_now_base(CF__POA.Resource, Component, ThreadedComponent):
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

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.
        rightnow = simple_property(id_="rightnow",
                                   type_="utctime",
                                   defvalue="now",
                                   mode="readwrite",
                                   action="external",
                                   kinds=("property",))
        simple1970 = simple_property(id_="simple1970",
                                     type_="utctime",
                                     defvalue="1970:01:01::00:00:00",
                                     mode="readwrite",
                                     action="external",
                                     kinds=("property",))

        simpleSeqDefNow = simpleseq_property(id_="simpleSeqDefNow",
                                             type_="utctime",
                                             defvalue=["now"],
                                             mode="readwrite",
                                             action="external",
                                             kinds=("property",))

        simpleSeqNoDef = simpleseq_property(id_="simpleSeqNoDef",
                                            type_="utctime",
                                            defvalue=[],
                                            mode="readwrite",
                                            action="external",
                                            kinds=("property",))

        simpleSeq1970 = simpleseq_property(id_="simpleSeq1970",
                                           type_="utctime",
                                           defvalue=["1970:01:01::00:00:00"],
                                           mode="readwrite",
                                           action="external",
                                           kinds=("property",))
