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
import shlex
import ossie.utils.log4py.config
from   ossie.utils.log4py import RedhawkLogger
from  omniORB import CORBA
from process import LocalProcess as LocalProcess
import tempfile

import copy
import yaml

from ossie import parsers
from ossie.utils.sca import findSdrRoot
import os
from clusterCfgParser import ClusterCfgParser

class EksKubeProcess(LocalProcess):
    def __init__(self, command, arguments, image, environment=None, stdout=None):
        print(image)
        self.namespace = 'redhawk-sandbox'
        self.tmp = tempfile.NamedTemporaryFile(prefix="k8s_component_config_", suffix=".yaml")
        
        cfgParser = ClusterCfgParser("EksKube")

        self.REGISTRY = cfgParser.info["registry"]
        self.DOCKER_CONFIG_JSON = cfgParser.info["dockerconfigjson"]
        self.TAG = cfgParser.info["tag"]

        fileName = self.createYaml(command, image, arguments)


        command_str = "kubectl apply -f " + fileName
        pod_name = arguments[-1].replace(":", "").lower()
        self.__pod_name = pod_name + "-pod"

        kubectlArgs = shlex.split(command_str)

        super(EksKubeProcess, self).__init__(kubectlArgs[0], kubectlArgs[1:], environment=None, stdout=None)
        
        self.badStatus = ["Terminating", "Completed", "CrashLoopBackOff", "InvalidImageName", "RunContainerError"]
        self.__file_name = fileName
        self.__tracker = None
        self.__callback = None
        self.__children = []
        self.__status = "Genesis"
        self.__timeout = 600  #worst case scenario
        self.__sleepIncrement = 1

        print "EksKubeProcess Constructor called"
        print "Pod created: " + self.__file_name + "\n"

    def setTerminationCallback(self, callback):
        if not self.__tracker:
            # Nothing is currently waiting for notification, start monitor.
            name = 'pod-%s-tracker' % self.__pod_name
            print "setTerminateCallback " + name
            self.__tracker = threading.Thread(name=name, target=self._monitorProcess)
            self.__tracker.daemon = False
            self.__tracker.start()
        self.__callback = callback

    def terminate_callback(pod_name, status):
        print "Hello from terminate_callback!"
        command = ['kubectl', 'delete', '-f', self.__file_name]
        print pod_name + "is, indeed, in a Running state\n"

        try:
            output = subprocess.check_output(command)
            print "Attempted to delete pod: " + pod_name + ":"
            print output
        except:
            print "oh no crash and burn from terminate_callback\n"

    def _monitorProcess(self):
        try:
            print "call to _monitorProcess to poll pod status..."
            #Retry status poll 10 times before giving up on
            self.poll(10)
        except:
            # If kubectl poll fails, don't bother with notification.
            print "_monitorProcess attempt to poll for pod status failed!"
            return
    
    def terminate(self):
        for child in self.__children:
            child.terminate()
        self.__children = []

        if self.__callback:
            print "Calling terminate on pod"
            # For SOME REASON, calling this function does NOT call the terminate_callback function that is supposed to delete the pod
            #self.__callback(self.__pod_name, status)
            # So I'm doing it here
            print self.__status 
            print self.__file_name
            
            command = ['kubectl', 'delete', '-f', self.__file_name]

            try:
                output = subprocess.check_output(command)
                print "Attempted to delete pod: " + self.__file_name + ":"
                print output
            except:
                print "Failed to delete pod " + self.__file_name
        self.tmp.close()
            
    def timeout(self):
        return self.__timeout
            
    def sleepIncrement(self):
        return self.__sleepIncrement

    def isAlive(self):
        arguments = ["kubectl", "get", "pod", self.__pod_name, "-n", self.namespace, "-o=jsonpath={.status.containerStatuses[0].state.waiting.reason}"]
        self.__status = subprocess.check_output(arguments)
        print "isAlive Status: " + self.__status

        
        if self.__status in self.badStatus:
            return False
        else:
            return True

    def poll(self, numRetries):
        arguments = ["kubectl", "get", "pod", self.__pod_name, "-n", self.namespace, "-o=jsonpath={.status.containerStatuses[0].state.waiting.reason}"]
        i = 0
        print("ATTEMPTING POLL ", arguments)
        if numRetries < 0:
            numRetries = 1
        while i < numRetries:
            # Poll for pod status
            time.sleep(self.__sleepIncrement)
            self.__status = subprocess.check_output(arguments)
            print "Poll Status: " + self.__status

            if self.__status in self.badStatus:
                break
            elif self.__status == "":
                print "Status is now " + self.__status
                break
            i = i + 1


    def createYaml(self, command, image, arguments):
        """spd is of type ossie.parsers.spd.softPkg"""
        if not os.path.exists(command):
            raise RuntimeError, "Entry point '%s' does not exist" % command
        elif not os.access(command, os.X_OK|os.R_OK):
            raise RuntimeError, "Entry point '%s' is not executable" % command

        full_image = str(self.REGISTRY) + "/" + str(image) + ":" + str(self.TAG)
    
        print("TEST: \""+command + "\" " + full_image)
        namespace_cfg = {'apiVersion': 'v1',
                         'kind': 'Namespace',
                         'metadata': {'name': self.namespace, 'labels': {'name': self.namespace}}}
        k8s_cfg = {'apiVersion': 'v1',
                   'kind': 'Secret',
                   'metadata': {'name': 'regcred', 'namespace': self.namespace},
                   'data': {'.dockerconfigjson': self.DOCKER_CONFIG_JSON},
                   'type': 'kubernetes.io/dockerconfigjson'}
        configs = [namespace_cfg, k8s_cfg]
    
    
        # if code.get_type().lower() == 'container':
        if True:  # TODO: for development/debuggin purposes
            # This will be the lone entry in 'containers'
            print(arguments)
    
            output_file = '/tmp/k8s_component_config_' + arguments[-1].lower().replace(":", "") + '.yaml'
    
            if "python" in command:
                exec_value = command
                for key in arguments:
                    exec_value = exec_value + " " + key
                exec_key = "args"
                exec_value = [exec_value]
            else:
                exec_key = "command"
                exec_value = [command]+arguments
    
            pod_cfg = {
                'apiVersion': 'v1',
                'kind': 'Pod',
                'metadata': {'name': arguments[-1].lower().replace(":", "") + '-pod', 'namespace': self.namespace},
                'spec': {
                    'containers': [{'image': full_image,
                                    'name': arguments[5].lower().replace("_", "") + '-container',
                                    exec_key: exec_value}],
                    'imagePullSecrets': [{'name': 'regcred'}]}}
    
            configs.append(pod_cfg)

        output_file = self.tmp.name
        with open(output_file, 'w') as f:
            yaml.dump_all(configs, f, default_flow_style=False)
    
        return output_file
