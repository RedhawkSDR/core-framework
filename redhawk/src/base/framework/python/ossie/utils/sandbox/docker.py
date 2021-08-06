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
import logging
import socket
import os
from   ossie.cf import CF
import urlparse
import time
import subprocess
import threading
import urllib
import ossie.utils.log4py.config
from   ossie.utils.log4py import RedhawkLogger
from  omniORB import CORBA
from process import LocalProcess as LocalProcess
from clusterCfgParser import ClusterCfgParser
import shlex

class DockerProcess(LocalProcess):
    def __init__(self, command, arguments, image, environment=None, stdout=None):
        if (os.popen('docker images -q '+image+' 2> /dev/null').read() == ''):
            raise RuntimeError("No docker image exists for entry point '%s'" % image)

        
        cfgParser = ClusterCfgParser("Docker")

        #self.local_dir = cfgParser.info["local_dir"]
        #self.mount_dir = cfgParser.info["mount_dir"]

        mountCmd = ""
        #if not "None" in self.mount_dir:
        #    mountCmd = " --mount type=bind,source=" + self.local_dir + ",target=" + self.mount_dir

        if 'python' in command or 'java' in command:
            for arg in arguments:
                command = command + " " + arg
            dockerCmd = "docker run --rm -d --network host --name " + arguments[-1].replace(":", "") + mountCmd + " " + image
            dockerArgs = shlex.split(dockerCmd) + [command]
        else:
            dockerCmd = "docker run --rm -d --network host -P --name " + arguments[-1].replace(":", "") + mountCmd + " --entrypoint " + command + " " + image
            dockerArgs = shlex.split(dockerCmd) + arguments

        print(dockerArgs)

        super(DockerProcess, self).__init__(dockerArgs[0], dockerArgs[1:], environment=None, stdout=None)
        self.__pod_name = arguments[7]
        self.__tracker = None
        self.__callback = None
        self.__children = []
        self.__status = "Genesis"
        self.__timeout = 30  #worst case scenario
        self.__sleepIncrement = 1

    def setTerminationCallback(self, callback):
        if not self.__tracker:
            # Nothing is currently waiting for notification, start monitor.
            name = self.__pod_name  # set the name of your container
            print("setTerminateCallback " + name)
            self.__tracker = threading.Thread(name=name, target=self._monitorProcess)
            self.__tracker.daemon = False
            self.__tracker.start()
        self.__callback = callback

    # Set up a callback to notify when the component exits abnormally.
    def terminate_callback(pid, status):
        self._cleanHeap(pid)
        if status > 0:
            print('Component %s (pid=%d) exited with status %d' % (name, pid, status))
        elif status < 0:
            print('Component %s (pid=%d) terminated with signal %d' % (name, pid, -status))
            
    def timeout(self):
        return self.__timeout
            
    def sleepIncrement(self):
        return self.__sleepIncrement

    def isAlive(self):
        arguments = ["docker", "container", "inspect", "-f", "'{{.State.Running}}'", self.__pod_name]
        try:
            self.__status = subprocess.check_output(arguments)
        except:
            self.__status = "Not Running"
        print("Status: " + self.__status.replace("\n", ""))

        if "true" in self.__status:
            return True
        else:
            return False

    def createYaml(self, name, entry_point, code, execparams):
        return ""
