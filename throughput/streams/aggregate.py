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
class AggregateStream(object):
    def __init__(self, factory, data_format, numa_policy, count):
        self.streams = [factory.create(data_format, numa_policy.next()) for ii in xrange(count)]

    def start(self):
        for stream in self.streams:
            stream.start()

    def stop(self):
        for stream in self.streams:
            stream.stop()

    def received(self):
        return sum(stream.received() for stream in self.streams)

    def transfer_size(self, length):
        for stream in self.streams:
            stream.transfer_size(length)

    def terminate(self):
        for stream in self.streams:
            stream.terminate()
