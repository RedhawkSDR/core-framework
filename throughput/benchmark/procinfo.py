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
import itertools

__all__ = ('CpuInfo', 'ProcessInfo')

class ProcFile(object):
    def __init__(self, filename):
        self.__filename = filename
        self.__last = self.scan()

    def scan(self):
        with open(self.__filename) as f:
            fields = f.readline().strip().split()
        result = {}
        for (name, format), value in itertools.izip(self.FIELDS, fields):
            result[name] = format(value)
        return result

    def poll(self):
        current = self.scan()
        last = self.__last
        self.__last = current
        return self.format(current, last)

    def format(self, current, last):
        return current


class CpuInfo(ProcFile):
    FIELDS = [
        ('name', str),
        ('user', int),
        ('nice', int),
        ('system', int),
        ('idle', int),
        ('iowait', int),
        ('irq', int),
        ('softirq', int),
        ('steal', int),
        ('guest', int)
    ]

    def __init__(self):
        ProcFile.__init__(self, '/proc/stat')

    def format(self, current, last):
        results = {}
        for (name, _) in self.FIELDS:
            # Skip first column (cpu name)
            if name == 'name':
                continue
            results[name] = current[name] - last[name]
        return results


class ProcessInfo(ProcFile):
    FIELDS = [
        ('pid', int),
        ('comm', str),
        ('state', str),
        ('ppid', int),
        ('pgrp', int),
        ('session', int),
        ('tty_nr', str),
        ('tpgid', str),
        ('flags', str),
        ('minflt', int),
        ('cminflt', int),
        ('majflt', int),
        ('cmajflt', int),
        ('utime', int),
        ('stime', int),
        ('cutime', int),
        ('cstime', int),
        ('priority', int),
        ('nice', int),
        ('num_threads', int),
        ('itrealvalue', str),
        ('starttime', int),
        ('vsize', int),
        ('rss', int),
        ('rsslim', int),
        ('startcode', int),
        ('endcode', int),
        ('startstack', int),
        ('kstkesp', str),
        ('kstkeip', str),
        ('signal', int),
        ('blocked', int),
        ('sigignore', str),
        ('sigcatch', str),
        ('wchan', str),
        ('nswap', str),
        ('cnswap', str),
        ('exit_signal', str),
        ('processor', int),
        ('rt_priority', int),
        ('policy', str),
        ('delayacct_blkio_ticks', str),
        ('guest_time', str),
        ('cguest_time', str)
    ]

    def __init__(self, pid):
        filename = '/proc/%d/stat' % (pid,)
        ProcFile.__init__(self, filename)

    def format(self, current, last):
        # Calculate deltas
        d_utime = current['utime'] - last['utime']
        d_stime = current['stime'] - last['stime']
        d_majflt = current['majflt'] - last['majflt']
        d_minflt = current['minflt'] - last['minflt']

        results = {
            'utime': d_utime,
            'stime': d_stime,
            'cpu': d_utime+d_stime,
            'rss' : current['rss'],
            'majflt': d_majflt,
            'minflt': d_minflt,
            'threads': current['num_threads']
        }

        return results
