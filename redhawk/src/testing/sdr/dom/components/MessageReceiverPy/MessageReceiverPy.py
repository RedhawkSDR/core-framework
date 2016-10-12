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
import logging

from ossie.cf import CF, CF__POA
from ossie.resource import Resource, start_component, usesport
from ossie.properties import simple_property, struct_property, simpleseq_property
from ossie import events

class test_message(object):
    item_float = simple_property(id_="item_float",
                                 type_="float")
    item_string = simple_property(id_="item_string",
                                  type_="string")

class TestMessagePort(CF__POA.Resource, Resource):
    """
    Example component to demonstrate REDHAWK receiver-side messaging port.
    """
    port_message_in = usesport(name="message_in",
                         repid="IDL:CosEventComm/PushConsumer:1.0",
                         type_="data")
    received_messages = simpleseq_property(id_="received_messages",
                         name="received_messages",   
                         type_="string",
                         mode="readwrite",
                         action="external",
                         kinds=("configure")
                         )

    def __init__(self, identifier, execparams):
        Resource.__init__(self, identifier, execparams)
        self.received_messages = []

    def initialize(self):
        self.port_message_in = events.MessageConsumerPort(thread_sleep=0.1)
        self.port_message_in.registerMessage("test_message",
                                         test_message, self.messageReceived)

    def messageReceived(self, msgId, msgData):
        self.received_messages.append(msgId+','+repr(msgData.item_float)+','+repr(msgData.item_string))


if __name__ == '__main__':
    logging.getLogger().setLevel(logging.DEBUG)
    start_component(TestMessagePort)
