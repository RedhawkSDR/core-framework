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
import time
import logging

class SlowComponent_i(CF__POA.Resource, Resource):
    """This component simply ensures that all other components in the waveform get started and
    stopped together."""

    def __init__(self, identifier, execparams):
        Resource.__init__(self, identifier, execparams)
        delay = int(execparams["CREATE_DELAY"])
        self._log.debug('Delaying create completion by %d seconds', delay)
        time.sleep(delay)


if __name__ == '__main__':
    logging.getLogger().setLevel(logging.DEBUG)
    start_component(SlowComponent_i)
