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
# AUTO-GENERATED
#
# Source: start_event_service.spd.xml

import logging

from ossie.cf import CF, CF__POA
from ossie.service import start_service
from omniORB import PortableServer

from ossie.properties import simple_property, simpleseq_property
from ossie.events import MessageSupplierPort

class start_event_service(CF__POA.Resource):
    class StateChange(object):
        identifier = simple_property(
                                     id_="state_change::identifier",
                                     name="identifier",
                                     type_="string")
    
        event = simple_property(
                                id_="state_change::event",
                                name="event",
                                type_="string")
    
        def __init__(self, **kw):
            """Construct an initialized instance of this struct definition"""
            for classattr in type(self).__dict__.itervalues():
                if isinstance(classattr, (simple_property, simpleseq_property)):
                    classattr.initialize(self)
            for k,v in kw.items():
                setattr(self,k,v)
    
        def __str__(self):
            """Return a string representation of this structure"""
            d = {}
            d["identifier"] = self.identifier
            d["event"] = self.event
            return str(d)
    
        @classmethod
        def getId(cls):
            return "state_change"
    
        @classmethod
        def isStruct(cls):
            return True
    
        def getMembers(self):
            return [("identifier",self.identifier),("event",self.event)]
            
    def __init__(self, name="start_event_service", execparams={}):
        self.name = name
        self._log = logging.getLogger(self.name)
        self._started = False
        self.port_message_out = MessageSupplierPort()

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
        # TODO
        pass

    def query(self, configProperties):
        # TODO
        pass

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
        if name != 'message_out':
            raise CF.PortSupplier.UnknownPort()
        return self.port_message_out._this()

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
        if self._started:
            return
        self._log.info('starting %s', self.name)
        self._started = True
        message = start_event_service.StateChange()
        message.identifier = self.name
        message.event = "start"
        self.port_message_out.sendMessage(message)

    def stop(self):
        if not self._started:
            return
        self._log.info('stopping %s', self.name)
        self._started = False
        message = start_event_service.StateChange()
        message.identifier = self.name
        message.event = "stop"
        self.port_message_out.sendMessage(message)

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
        return self._started

    def _get_softwareProfile(self):
        # TODO
        pass


if __name__ == '__main__':
    start_service(start_event_service, thread_policy=PortableServer.SINGLE_THREAD_MODEL)
