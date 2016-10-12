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
import sys

from omniORB import any
import CosEventChannelAdmin

from ossie.cf import CF, CF__POA
from ossie.cf import ExtendedEvent__POA
import CosEventComm__POA
from ossie.resource import Resource, start_component, usesport
from ossie.properties import simple_property, struct_property, struct_from_any, props_to_any, struct_to_any
from ossie import events

class test_message(object):
    item_float = simple_property(id_="item_float",
                                 type_="float")
    item_string = simple_property(id_="item_string",
                                  type_="string")
    def getId(self):
        return "test_message"


class TestMessagePort(CF__POA.Resource, Resource):
    """
    Example component to demonstrate REDHAWK sender-side messaging port.
    """
    message_out = usesport(name="message_out",
                         repid="IDL:CosEventComm/PushConsumer:1.0",
                         type_="data")

    def __init__(self, identifier, execparams):
        Resource.__init__(self, identifier, execparams)

    def initialize(self):
        self.message_out = events.MessageSupplierPort()
    
    def start(self):
        single_msg = test_message()
        single_msg.item_float = 1.0
        single_msg.item_string = 'some string'
        self.message_out.sendMessage(single_msg)
        msg = test_message()
        msg.item_float = 2.0
        msg.item_string = 'another string'
        msg2 = test_message()
        msg2.item_float = 3.0
        msg2.item_string = 'yet another string'
        self.message_out.sendMessages([msg, msg2])


if __name__ == '__main__':
    logging.getLogger().setLevel(logging.DEBUG)
    start_component(TestMessagePort)
