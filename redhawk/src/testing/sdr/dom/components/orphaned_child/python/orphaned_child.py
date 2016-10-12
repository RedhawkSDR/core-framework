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

import signal

from ossie.cf import CF__POA
from ossie.resource import Resource, start_component
from ossie.utils.log4py import logging

class orphaned_child(CF__POA.Resource, Resource):
    def __init__(self, *args, **kwargs):
        Resource.__init__(self, *args, **kwargs)

        # Ignore normal termination signals to ensure that the process is
        # orphaned when the parent is terminated; start_component() normally
        # establishes handlers for these before creating the component instance
        signal.signal(signal.SIGINT, signal.SIG_IGN)
        signal.signal(signal.SIGTERM, signal.SIG_IGN)
        signal.signal(signal.SIGQUIT, signal.SIG_IGN)

    def releaseObject(self):
        # Overriden to prevent the normal automatic process exit that occurs
        # after releaseObject() in 1.9+
        pass

if __name__ == '__main__':
    logging.getLogger().setLevel(logging.WARN)
    logging.debug("Starting Component")
    start_component(orphaned_child)
