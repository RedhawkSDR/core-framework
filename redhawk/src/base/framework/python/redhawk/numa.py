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

def parseRange(line):
    first, last = line.split('-')
    return range(int(first), int(last)+1)

def parseValues(line, delim=','):
    values = []
    for section in line.split(delim):
        if '-' in section:
            values.extend(parseRange(section))
        else:
            values.append(int(section))
    return values

class NumaNode(object):
    def __init__(self, node):
        self.node = node
        self.cpus = self._getCpuList()

    def _getCpuList(self):
        filename = '/sys/devices/system/node/node%d/cpulist' % self.node
        with open(filename) as f:
            line = f.readline().strip()
            return parseValues(line, ',')

class NumaTopology(object):
    def __init__(self):
        self.nodes = [NumaNode(node) for node in self._getNodes()]
        self.cpus = sum((node.cpus for node in self.nodes), [])

    def _getNodes(self):
        with open('/sys/devices/system/node/online') as f:
            line = f.readline().strip()
            return parseValues(line, ',')

    def getNodeForCpu(self, cpu):
        for node in self.nodes:
            if cpu in node.cpus:
                return node
        return None
