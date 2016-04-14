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
import subprocess
import omniORB

import rawdata

__all__ = ('factory')

class CorbaStream(object):
    def __init__(self, orbargs, orb, format, numa_policy):
        reader_args = numa_policy(['streams/corba/reader'] + orbargs)
        self.reader_proc = subprocess.Popen(reader_args, stdout=subprocess.PIPE)
        ior = self.reader_proc.stdout.readline().rstrip()
        self.reader = orb.string_to_object(ior)

        writer_args = numa_policy(['streams/corba/writer'] + orbargs)
        self.writer_proc = subprocess.Popen(writer_args, stdout=subprocess.PIPE)
        ior = self.writer_proc.stdout.readline().rstrip()
        self.writer = orb.string_to_object(ior)

        self.writer.connect(self.reader, format)

    def start(self):
        self.writer.start()

    def stop(self):
        self.writer.stop()

    def get_reader(self):
        return self.reader_proc.pid

    def get_writer(self):
        return self.writer_proc.pid

    def transfer_size(self, size):
        self.writer.transfer_length(int(size))

    def received(self):
        return self.reader.received()

    def send_time(self):
        return self.writer._get_average_time()

    def recv_time(self):
        return self.reader._get_average_time()

    def terminate(self):
        self.reader_proc.terminate()
        self.writer_proc.terminate()
        self.reader_proc.kill()
        self.writer_proc.kill()
        self.reader_proc.wait()
        self.reader_proc.wait()

class CorbaStreamFactory(object):
    def __init__(self, transport):
        if transport == 'unix':
            self.orbargs = ['-ORBendPoint', 'giop:unix:']
        else:
            self.orbargs = ['-ORBendPoint', 'giop:tcp::']
        self.orbargs += [ '-ORBgiopMaxMsgSize', str(50*1024*1024)]
        self.orb = omniORB.CORBA.ORB_init()

    def create(self, data_format, numa_policy):
        return CorbaStream(self.orbargs, self.orb, data_format, numa_policy)

    def cleanup(self):
        self.orb.destroy()

def factory(transport):
    return CorbaStreamFactory(transport)
