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
from ossie.cf import CF, CF__POA
from ossie.resource import Resource, start_component
from ossie.properties import simple_property, simpleseq_property, struct_property
import os, time, signal, copy, sys
import omniORB.any
import logging
import signal

import spdDepLibrary

class CommandWrapperSPDDep_i(CF__POA.Resource, Resource):
    """This component simply ensures that all other components in the waveform get started and
    stopped together."""


    # The signal to send and the number of seconds (max) to wait
    # for the process to actually exit before trying the next signals.
    # None means wait until the process dies (which we have to do to
    # avoid creating zombie processes
    STOP_SIGNALS = ((signal.SIGINT, 1), (signal.SIGTERM, 5), (signal.SIGKILL, None))
    def __init__(self, identifier, execparams):
        loggerName = execparams['NAME_BINDING'].replace('/', '.')
        Resource.__init__(self, identifier, execparams, loggerName=loggerName)
        self._pid = None
        self.execparams = " ".join(["%s %s" % x for x in execparams.items()])

    #####################################
    # Implement the Resource interface
    def start(self):
        self._log.debug("start()")
        if self.get_commandAlive() == False:
            command = self._props["command"]
            args = copy.copy(self._props["args"])
            args.insert(0, command)
            self._log.debug("start %s %r", command, args)
            self._pid = os.spawnv(os.P_NOWAIT, command, args)
            self._log.debug("spawned %s", self._pid)

    def stop(self):
        self._log.debug("stop()")
        if self.get_commandAlive() == True:
            for sig, timeout in self.STOP_SIGNALS:
                try:
                    os.kill(self._pid, sig)
                except OSError:
                    self._pid = None
                    return
                if timeout != None:
                    giveup_time = time.time() + timeout
                    while os.waitpid(self._pid, os.WNOHANG) == (0,0):
                        time.sleep(0.1)
                        if time.time() > giveup_time:
                            break
                else:
                    # Wait until there is a response
                    os.waitpid(self._pid, 0)
            self._pid = None

    ######################################
    # Implement specific property setters/getters 
    def get_commandAlive(self):
        if self._pid != None:
            try:
                os.kill(self._pid, 0)
                if os.waitpid(self._pid, os.WNOHANG) == (0,0):
                    return True
                else:
                    return False
            except OSError:
                pass
        return False

    command = simple_property(\
        id_='DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e',
        type_="string",
        name="command",
        defvalue="/bin/echo")

    commandAlive = simple_property(\
        id_='DCE:95f19cb8-679e-48fb-bece-dc199ef45f20',
        type_="boolean",
        name="commandAlive",
        defvalue=False,
        mode="readonly",
        fget=get_commandAlive)

    someprop = simple_property(\
        id_='DCE:fa8c5924-845c-484a-81df-7941f2c5baa9',
        type_="long",
        name="someprop",
        defvalue=10)

    someprop2 = simple_property(\
        id_='DCE:cf623573-a09d-4fb1-a2ae-24b0b507115d',
        type_="double",
        name="someprop2",
        defvalue=50.0)

    someprop3 = simple_property(\
        id_='DCE:6ad84383-49cf-4017-b7ca-0ec4c4917952',
        type_="double",
        name="someprop3")

    execparams = simple_property(\
        id_='DCE:85d133fd-1658-4e4d-b3ff-1443cd44c0e2',
        type_="string",
        name="execparams")

    execparam1 = simple_property(\
        id_='EXEC_PARAM_1',
        type_="string",
        name="Parameter 1",
        defvalue="Test1",
        kinds=("execparam"))

    execparam2 = simple_property(\
        id_='EXEC_PARAM_2',
        type_="long",
        name="Parameter 2",
        defvalue=2,
        kinds=("execparam"))

    execparam3 = simple_property(\
        id_='EXEC_PARAM_3',
        type_="float",
        name="Parameter 3",
        defvalue=3.125,
        kinds=("execparam"),
        mode="readonly")

    someobjref = simple_property(\
        id_='SOMEOBJREF',
        type_="objref",
        name="Object Ref",
        defvalue=None,
        kinds=("execparam"),
        mode="writeonly")

    someobjref = simpleseq_property(\
        id_='DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53',
        type_="string",
        name="args",
        defvalue=("Hello World",))

    class SomeStruct(object):
        field1 = simple_property(id_="item1",
                                type_="string",
                                defvalue="value1")
        field2 = simple_property(id_="item2",
                                type_="long",
                                defvalue=100)
        field3 = simple_property(id_="item3",
                                type_="double",
                                defvalue=3.14156)

    struct = struct_property(id_="DCE:ffe634c9-096d-425b-86cc-df1cce50612f", 
                             name="struct_test", 
                             structdef=SomeStruct)
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.DEBUG)
    start_component(CommandWrapperSPDDep_i)
