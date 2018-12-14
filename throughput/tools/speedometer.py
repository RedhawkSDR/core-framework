#!/usr/bin/python
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
import numpy
import itertools

from streams import raw, corba, bulkio
from benchmark import utils, numa
from benchmark.procinfo import CpuInfo, ProcessInfo
from benchmark.tests import TestMonitor, BenchmarkTest
from benchmark.csv import CSVOutput

class Speedometer(TestMonitor):
    def __init__(self, period):
        # Quiet the warning about GTK Tooltip deprecation
        import warnings
        with warnings.catch_warnings():
            warnings.filterwarnings('ignore', category=DeprecationWarning)
            self.figure = pyplot.figure()

        self.figure.canvas.set_window_title('REDHAWK Speedometer')

        # Create a line graph of throughput over time
        self.axes = self.figure.add_subplot(111)
        self.axes.set_xlabel('Time')
        self.axes.set_ylabel('Throughput (Bps)')

        self.x = []
        self.y = []
        self.line, = self.axes.plot(self.x, self.y)
        self.axes.set_xlim(xmax=period)

        self.figure.subplots_adjust(bottom=0.2)

        sliderax = self.figure.add_axes([0.2, 0.05, 0.6, 0.05])
        min_size = 1024
        max_size = 8*1024*1024
        default_size = 1*1024*1024
        self.slider = matplotlib.widgets.Slider(sliderax, 'Transfer size', min_size, max_size,
                                                default_size, '%.0f')

        self.figure.show()

    def sample_added(self, **stats):
        self.x.append(stats['time'])
        self.y.append(stats['rate'])
        self.line.set_xdata(self.x)
        self.line.set_ydata(self.y)
        self.axes.relim()
        self.axes.autoscale_view(scalex=False)
        self.figure.canvas.draw()

    def wait(self):
        pyplot.show()

    def update(self):
        self.figure.canvas.flush_events()


class ThroughputTest(BenchmarkTest):
    def __init__(self, poll_time, run_time):
        BenchmarkTest.__init__(self)
        self.poll_time = poll_time
        self.run_time = run_time
        self.num_cpus = sum(len(numa.get_cpus(n)) for n in numa.get_nodes())

    def run(self, name, stream):
        reader_stats = ProcessInfo(stream.get_reader())
        writer_stats = ProcessInfo(stream.get_writer())

        cpu_info = CpuInfo()

        self.test_started(name=name)

        stream.start()

        start = time.time()
        next = start + self.poll_time
        end = start + self.run_time

        now = start
        last_time = start
        last_total = 0

        while time.time() < end:
            self.pass_started()

            # Allow UI to update, etc.
            self.idle_tasks()

            # Wait until next scheduled poll time
            sleep_time = next - time.time()
            if sleep_time > 0.0:
                time.sleep(sleep_time)

            # Measure time elapsed since last sample
            now = time.time()
            elapsed = now - last_time
            last_time = now

            # Set next expected sample time based on the current time, in
            # case there was an unusually long sample period
            next = now + self.poll_time

            # Calculate average throughput over the sample period
            current_total = stream.received()
            delta = current_total - last_total
            last_total = current_total
            current_rate = delta / elapsed

            # Aggregate CPU usage
            reader = reader_stats.poll()
            writer = writer_stats.poll()

            system = cpu_info.poll()
            sys_cpu = self.num_cpus * 100.0 / sum(system.values())

            sample = {'time': now-start,
                      'rate': current_rate,
                      'write_cpu': writer['cpu'] * sys_cpu,
                      'write_rss': writer['rss'],
                      'write_majflt': writer['majflt'],
                      'write_minflt': writer['minflt'],
                      'write_threads': writer['threads'],
                      'read_cpu': reader['cpu'] * sys_cpu,
                      'read_rss': reader['rss'],
                      'read_majflt': reader['majflt'],
                      'read_minflt': reader['minflt'],
                      'read_threads': reader['threads'],
                      'cpu_user': system['user'] * sys_cpu,
                      'cpu_system': system['system'] * sys_cpu,
                      'cpu_idle': system['idle'] * sys_cpu,
                      'cpu_iowait': system['iowait'] * sys_cpu,
                      'cpu_irq': system['irq'] * sys_cpu,
                      'cpu_softirq': system['softirq'] * sys_cpu,
                      }
            self.sample_added(**sample)
            self.pass_complete()

        stream.stop()

        self.test_complete()


if __name__ == '__main__':
    transport = 'unix'
    numa_distance = None
    poll_time = 0.25
    run_time = 30.0
    nogui = False
    interface = 'bulkio'
    transfer_size = 1*1024*1024

    opts, args = getopt.getopt(sys.argv[1:], 's:t:p:', ['interface=', 'transport=', 'numa-distance=', 'no-gui'])
    for key, value in opts:
        if key == '-s':
            transfer_size = utils.from_binary(value)
        elif key == '-t':
            run_time = utils.time_to_sec(value)
        elif key == '-p':
            poll_time = float(value)
        elif key == '--transport':
            transport = value
        elif key == '--numa-distance':
            numa_distance = int(value)
        elif key == '--no-gui':
            nogui = True
        elif key == '--interface':
            interface = value

    test = ThroughputTest(poll_time, run_time)

    import matplotlib
    from matplotlib import pyplot
    display = Speedometer(run_time)
    test.add_idle_task(display.update)
    test.add_monitor(display)

    if interface == 'raw':
        factory = raw.factory(transport)
    elif interface == 'corba':
        factory = corba.factory(transport)
    elif interface == 'bulkio':
        factory = bulkio.factory(transport)

    numa_policy = numa.NumaPolicy(numa_distance)

    stream = factory.create('octet', numa_policy.next())
    display.slider.on_changed(stream.transfer_size)
    try:
        stream.transfer_size(transfer_size)
        test.run(interface, stream)
    finally:
        stream.terminate()

    display.wait()
