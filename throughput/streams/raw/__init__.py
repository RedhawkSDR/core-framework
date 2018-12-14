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
import signal
import subprocess
import mmap
import tempfile
import ctypes

__all__ = ('factory')

class control(object):
    def __init__(self, transfer_size):
        fd, self.filename = tempfile.mkstemp()
        os.ftruncate(fd, 20)
        self.buf = mmap.mmap(fd, 20, mmap.MAP_SHARED, mmap.PROT_WRITE)
        os.close(fd)
        self.total_bytes = ctypes.c_uint64.from_buffer(self.buf)
        self.total_bytes.value = 0
        self.average_time = ctypes.c_double.from_buffer(self.buf, 8)
        self.average_time.value = 0.0
        self.transfer_size = ctypes.c_uint32.from_buffer(self.buf, 16)
        self.transfer_size.value = transfer_size

    def __del__(self):
        os.unlink(self.filename)


class RawStream(object):
    def __init__(self, transport, numa_policy):
        self.writer_control = control(16384)
        writer_args = numa_policy(['streams/raw/writer', transport, self.writer_control.filename])
        self.writer_proc = subprocess.Popen(writer_args, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        writer_addr = self.writer_proc.stdout.readline().rstrip()

        self.reader_control = control(16384)
        reader_args = numa_policy(['streams/raw/reader', transport, writer_addr, self.reader_control.filename])
        self.reader_proc = subprocess.Popen(reader_args)

    def start(self):
        self.writer_proc.stdin.write('\n')

    def stop(self):
        os.kill(self.writer_proc.pid, signal.SIGINT)

    def get_reader(self):
        return self.reader_proc.pid

    def get_writer(self):
        return self.writer_proc.pid

    def transfer_size(self, size):
        self.writer_control.transfer_size.value = int(size)
        self.reader_control.transfer_size.value = int(size)

    def received(self):
        return self.reader_control.total_bytes.value

    def send_time(self):
        return self.writer_control.average_time.value

    def recv_time(self):
        return self.reader_control.average_time.value

    def terminate(self):
        # Assuming stop() was already called, the reader and writer should have
        # already exited
        self.writer_proc.kill()
        self.reader_proc.kill()
        self.writer_proc.wait()
        self.reader_proc.wait()

class RawStreamFactory(object):
    def __init__(self, transport):
        self.transport = transport

    def create(self, format, numa_policy):
        return RawStream(self.transport, numa_policy)

    def cleanup(self):
        pass

def factory(transport):
    return RawStreamFactory(transport)
