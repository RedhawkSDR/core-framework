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
import itertools

def _parse_range(line):
    first, last = line.split('-')
    return range(int(first), int(last)+1)

def _parse_values(line, delim):
    values = []
    for section in line.split(delim):
        if '-' in section:
            values.extend(_parse_range(section))
        else:
            values.append(int(section))
    return values

def get_nodes():
    nodes = '[0]'
    try:
      with open('/sys/devices/system/node/online') as f:
          line = f.readline().strip()
          nodes = _parse_values(line, ',')
    finally:
          return nodes

def get_cpus(node):
    with open('/sys/devices/system/node/node%d/cpulist'%node) as f:
        line = f.readline().strip()
        return _parse_values(line, ',')

def get_distances(node):
    with open('/sys/devices/system/node/node%d/distance'%node) as f:
        line = f.readline().strip()
        return _parse_values(line, ' ')

def get_distance(node, dest):
    return get_distances(node)[dest]

def is_numa_supported():
    try:
        return len(get_nodes()) > 1
    except:
        return False

def is_numactl_available():
    try:
        return os.system('numactl --hardware > /dev/null 2>&1') == 0
    except:
        return False

class NumaWrapper(object):
    def __init__(self, nodes, distance=None):
        if distance is None:
            self.nodes = itertools.repeat(None)
        elif distance == 0:
            self.nodes = itertools.repeat(nodes.next())
        else:
            self.nodes = itertools.islice(nodes, 0, None, distance)

    def __call__(self, command, *args, **kwargs):
        node = self.nodes.next()
        if node is None:
            return command
        else:
            if isinstance(command, basestring):
                command = [command]
            return ['numactl', '--cpunodebind=%d' % node] + command

class NumaPolicy(object):
    def __init__(self, distance=None):
        self.nodes = itertools.cycle(get_nodes())
        self.distance = distance

    def next(self):
        return NumaWrapper(self.nodes, self.distance)
