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
import os, time, signal, copy, sys
import omniORB.any
import logging
import signal

PROPERTIES = (
              (
               u'DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e', # ID
               u'command', # NAME
               u'string', # TYPE
               u'readwrite', # MODE
               '/bin/echo', # DEFAULT
               None, # UNITS
               'external', # ACTION
               (u'configure',), # KINDS
              ),
              (
               u'DCE:95f19cb8-679e-48fb-bece-dc199ef45f20', # ID
               u'commandAlive', # NAME
               u'boolean', # TYPE
               u'readonly', # MODE
               False, # DEFAULT
               None, # UNITS
               'external', # ACTION
               (u'configure',), # KINDS
              ),
              (
               u'DCE:fa8c5924-845c-484a-81df-7941f2c5baa9', # ID
               u'someprop', # NAME
               u'long', # TYPE
               u'readwrite', # MODE
               10, # DEFAULT
               None, # UNITS
               'external', # ACTION
               (u'configure',), # KINDS
              ),
              (
               u'DCE:85d133fd-1658-4e4d-b3ff-1443cd44c0e2', # ID
               u'execparams', # NAME
               u'string', # TYPE
               u'readonly', # MODE
               None, # DEFAULT
               None, # UNITS
               'external', # ACTION
               (u'configure',), # KINDS
              ),
              (
               u'EXEC_PARAM_1', # ID
               u'Parameter 1', # NAME
               u'string', # TYPE
               u'readonly', # MODE
               'Test1', # DEFAULT
               None, # UNITS
               'external', # ACTION
               (u'execparam',), # KINDS
              ),
              (
               u'EXEC_PARAM_2', # ID
               u'Parameter 2', # NAME
               u'string', # TYPE
               u'readonly', # MODE
               '2', # DEFAULT
               None, # UNITS
               'external', # ACTION
               (u'execparam',), # KINDS
              ),
              (
               u'EXEC_PARAM_3', # ID
               u'Parameter 3', # NAME
               u'string', # TYPE
               u'readonly', # MODE
               '3.3333', # DEFAULT
               None, # UNITS
               'external', # ACTION
               (u'execparam',), # KINDS
              ),
              (
               u'DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53', # ID
               u'args', # NAME
               u'string', # TYPE
               u'readwrite', # MODE
               ('Hello World',), # DEFAULT
               None, # UNITS
               'external', # ACTION
               (u'configure',), # KINDS
              ),
             )

class CommandWrapper_i(CF__POA.Resource, Resource):
    """This component simply ensures that all other components in the waveform get started and
    stopped together."""

    # The signal to send and the number of seconds (max) to wait
    # for the process to actually exit before trying the next signals.
    # None means wait until the process dies (which we have to do to
    # avoid creating zombie processes
    STOP_SIGNALS = ((signal.SIGINT, 1), (signal.SIGTERM, 5), (signal.SIGKILL, None))
    def __init__(self, identifier, execparams):
        loggerName = execparams['NAME_BINDING'].replace('/', '.')
        Resource.__init__(self, identifier, execparams, propertydefs=PROPERTIES, loggerName=loggerName)
        self._props["execparams"] = " ".join(["%s %s" % x for x in execparams.items()])
        self._pid = None

    #####################################
    # Implement the Resource interface
    def start(self):
        self._log.debug("start()")
        if self.query_commandAlive() == False:
            command = self._props["command"].value.value()
            args = copy.copy(self._props["args"])
            args.insert(0, command)
            self._log.debug("start %s %r", command, args)
            self._pid = os.spawnv(os.P_NOWAIT, command, args)
            self._log.debug("spawned %s", self._pid)

    def stop(self):
        self._log.debug("stop()")
        if self.query_commandAlive() == True:
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
    def query_commandAlive(self):
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

if __name__ == '__main__':
    logging.getLogger().setLevel(logging.DEBUG)
    start_component(CommandWrapper_i)
