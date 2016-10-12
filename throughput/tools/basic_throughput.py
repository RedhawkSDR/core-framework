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
import sys
import time
import getopt

from streams import raw, corba, bulkio
from streams.aggregate import AggregateStream
from benchmark import utils, numa

if __name__ == '__main__':
    transfer_size = 1024
    interface = 'raw'
    transport = 'unix'
    time_period = 10.0
    numa_distance = None
    data_format = 'octet'
    count = 1

    opts, args = getopt.getopt(sys.argv[1:], 'n:s:t:', ['transport=', 'interface=', 'numa-distance=', 'format='])
    for key, value in opts:
        if key == '-n':
            count = int(value)
        elif key == '-s':
            transfer_size = utils.from_binary(value)
        elif key == '-t':
            time_period = utils.time_to_sec(value)
        elif key == '--transport':
            transport = value
        elif key == '--numa-distance':
            numa_distance = int(value)
        elif key == '--interface':
            interface = value
        elif key == '--format':
            data_format = value

    numa_policy = numa.NumaPolicy(numa_distance)

    if interface == 'raw':
        factory = raw.factory(transport)
    elif interface == 'corba':
        factory = corba.factory(transport)
    elif interface == 'bulkio':
        factory = bulkio.factory(transport)
    else:
        raise SystemExit('No interface '+interface)

    streams = AggregateStream(factory, data_format, numa_policy, count)
    streams.transfer_size(transfer_size)

    start = time.time()
    streams.start()
    time.sleep(time_period)
    streams.stop()
    elapsed = time.time() - start

    aggregate_throughput = 0.0
    read_count = streams.received()
    streams.terminate()
    aggregate_throughput += read_count/elapsed

    print 'Elapsed:', elapsed, 'sec'
    print 'Throughput:', aggregate_throughput / (1024**3), 'GBps'

