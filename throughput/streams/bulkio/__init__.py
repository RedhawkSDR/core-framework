#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK throughput.
#
# REDHAWK throughput is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK throughput is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import os
try:
    from ossie.utils import sb
    from ossie.utils.sandbox.debugger import Debugger
except ImportError:
    # Fallback for testing raw and CORBA without BulkIO
    sb = None
    Debugger = object

__all__ = ('factory')

PATH = os.path.dirname(__file__)

class NumaLauncher(Debugger):
    def __init__(self, policy):
        self.policy = policy

    def isInteractive(self):
        return False

    def modifiesCommand(self):
        return True

    def canAttach(self):
        return False

    def wrap(self, command, arguments):
        command = self.policy([command] + arguments)
        return command[0], command[1:]

    def name(self):
        return 'numactl'


class BulkioStream(object):
    def __init__(self, format, numa_policy, shared):
        launcher = NumaLauncher(numa_policy)
        self.shared = shared
        self.writer = sb.launch(os.path.join(PATH, 'writer/writer.spd.xml'), debugger=launcher, shared=self.shared)
        self.reader = sb.launch(os.path.join(PATH, 'reader/reader.spd.xml'), debugger=launcher, shared=self.shared)
        self.writer.connect(self.reader)
        self.container = sb.domainless._getSandbox()._getComponentHost()

    def start(self):
        sb.start()

    def stop(self):
        sb.stop()

    def get_reader(self):
        if self.shared:
            return self.container._process.pid()
        else:
            return self.reader._process.pid()

    def get_writer(self):
        if self.shared:
            return self.container._process.pid()
        else:
            return self.writer._process.pid()

    def transfer_size(self, size):
        self.writer.transfer_length = int(size)

    def received(self):
        return int(self.reader.received)

    def send_time(self):
        return float(self.writer.average_time)

    def recv_time(self):
        return float(self.reader.average_time)

    def terminate(self):
        sb.release()

class BulkioCorbaFactory(object):
    def __init__(self, transport):
        configfile = 'config/omniORB-%s.cfg' % transport
        os.environ['OMNIORB_CONFIG'] = os.path.join(PATH, configfile)

    def create(self, format, numa_policy):
        return BulkioStream(format, numa_policy, False)

    def cleanup(self):
        pass

class BulkioLocalFactory(object):
    def create(self, format, numa_policy):
        return BulkioStream(format, numa_policy, True)

def factory(transport, local):
    if sb is None:
        raise ImportError('BulkIO is not available')
    if local:
        return BulkioLocalFactory()
    else:
        return BulkioCorbaFactory(transport)
