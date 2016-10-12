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
class TestMonitor(object):
    def test_started(self, **kw):
        pass

    def test_complete(self, **kw):
        pass

    def pass_started(self, **kw):
        pass

    def sample_added(self, **kw):
        pass

    def pass_complete(self, **kw):
        pass


class BenchmarkTest(object):
    def __init__(self):
        self.monitors = []
        self.__idle_tasks = []

    def add_monitor(self, monitor):
        self.monitors.append(monitor)

    def test_started(self, **kw):
        for monitor in self.monitors:
            monitor.test_started(**kw)

    def test_complete(self, **kw):
        for monitor in self.monitors:
            monitor.test_complete(**kw)

    def pass_started(self, **kw):
        for monitor in self.monitors:
            monitor.pass_started(**kw)

    def pass_complete(self, **kw):
        for monitor in self.monitors:
            monitor.pass_complete(**kw)

    def sample_added(self, **kw):
        for monitor in self.monitors:
            monitor.sample_added(**kw)

    def add_idle_task(self, task):
        self.__idle_tasks.append(task)

    def idle_tasks(self):
        for task in self.__idle_tasks:
            task()
