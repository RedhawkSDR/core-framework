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
import threading
import time
import os
import subprocess
import shlex
from ossie.utils.sandbox.process import LocalProcess as LocalProcess

import copy
import uuid
import yaml
import tempfile

from ossie import parsers
from ossie.utils.sca import findSdrRoot
from .clusterCfgParser import ClusterCfgParser

class DockerSwarmProcess(LocalProcess):
    def __init__(self, command, arguments, image, environment=None, stdout=None):
        print(image)
        cfgParser = ClusterCfgParser("DockerSwarm")
        self.tmp = tempfile.NamedTemporaryFile(prefix="swarm_component_config_", suffix=".yaml")

        self.REGISTRY = cfgParser.info["registry"]
        self.TAG = cfgParser.info["tag"]
        self.KEY = cfgParser.info["key"]
        self.USER = cfgParser.info["user"]
        self.IP = cfgParser.info["ip"]
        self.SSH_CMD = "ssh -i "+self.KEY+" "+self.USER+"@"+self.IP
        self.DOCKER_LOGIN = "aws ecr get-login-password --region us-gov-west-1 | docker login --username AWS --password-stdin " + cfgParser.info["registry"]

        fileName = self.createYaml(command, image, arguments)
    
        cmd = self.SSH_CMD + " '" + self.DOCKER_LOGIN + "'"
        print(cmd)
        os.popen(cmd)

        print("cat "+str(fileName))

        os.popen("scp -i " + self.KEY + " " + fileName + " " + self.USER + "@" + self.IP + ":/tmp/" )
        print(arguments)

        i = 0
        for arg in arguments:
            if arg == "NAME_BINDING":
                name = arguments[i+1]
                break
            i = i + 1
        command_str = self.SSH_CMD + " \"docker stack deploy --compose-file "+fileName+" "+arguments[-1].replace(":", "")+"  --with-registry-auth\""

        dockerArgs = shlex.split(command_str)

        super(DockerSwarmProcess, self).__init__(dockerArgs[0], dockerArgs[1:], environment=None, stdout=None)
        print(command)
        print(arguments)
        self.__stack = arguments[-1].replace(":", "")
        self.__container_name = self.__stack+"_"+name.lower()
        self.__file_name = fileName
        self.__tracker = None
        self.__callback = None
        self.__children = []
        self.__status = "Genesis"
        self.__timeout = 120  #worst case scenario
        self.__sleepIncrement = 1

    def setTerminationCallback(self, callback):
        if not self.__tracker:
            # Nothing is currently waiting for notification, start monitor.
            name = self.__container_name  # set the name of your container
            print("setTerminateCallback " + name)
            self.__tracker = threading.Thread(name=name, target=self._monitorProcess)
            self.__tracker.daemon = False
            self.__tracker.start()
        self.__callback = callback

    # Set up a callback to notify when the component exits abnormally.
    def terminate_callback(self, pod_name, status):
        print("Hello from terminate_callback!")
        print(pod_name)
        print(status)
        #if "complete" in status.lower() or "running" in status.lower():
        command = self.SSH_CMD + " \"docker stack rm "+self.__stack+"\""
        print("command: "+command)
        print(pod_name + "is in a " + status + " State\n")
        #else:
        #    print "pod " + pod_name + "is not in a running state, current state is: " + status + "\n"
        try:
            output = subprocess.check_output(shlex.split(command))
            print("Attempted to delete pod: " + pod_name + ":")
            print(output)
        except:
            print("oh no crash and burn from terminate_callback\n")

    def terminate(self):
        super(DockerSwarmProcess, self).terminate()

        if self.__callback:
            self.terminate_callback(self.__container_name, self.__status)
        self.tmp.close()
            
    def timeout(self):
        return self.__timeout
            
    def sleepIncrement(self):
        return self.__sleepIncrement

    def isAlive(self):
        arguments =  self.SSH_CMD + ' "docker service ps ' + self.__container_name + ' --format \'{{.CurrentState}}\'" '
        print(arguments)
        try:
            self.__status = subprocess.check_output(shlex.split(arguments))
        except:
            # no need to print but just ignore if this happens
            pass
        print("Status: " + self.__status.replace("\n", ""))

        if "failed" in self.__status.lower() or "rejected" in self.__status.lower():
            return False
        else:
            return True


    def createYaml(self, command, image, arguments):
        """spd is of type ossie.parsers.spd.softPkg"""
     
        full_image = str(self.REGISTRY) + "/" + str(image) + ":" + str(self.TAG)
    
        swarm_cfg = {'version': '3'}
    

        # if code.get_type().lower() == 'container':
        if True:  # TODO: for development/debuggin purposes
            # This will be the lone entry in 'containers'
            network_name = 'host' #name.lower() + '-network-' + str(uuid.uuid4()).lower()
    
            if "python" in command:
                exec_value = command
                for key in arguments:
                    exec_value = exec_value + " " + key
                command = []
                key = 'command'
                args = [exec_value]
            else:
                #exec_value = ""
                #for key in arguments:
                #    exec_value = exec_value + " " + key
                key = 'entrypoint'
                command = [command]
                args = arguments
    
            srv_cfg = {
                'image': full_image,
                key: command + args,
                'networks': ['outside'],
                'deploy': {'placement': {'constraints': ['node.role == worker']}}}
    
            swarm_cfg.update({'services': {arguments[5].lower(): srv_cfg}})
            swarm_cfg.update({'networks': { 'outside': { 'external' : {'name': 'host'}}}})

        output_file = self.tmp.name
        with open(output_file, 'w') as f:
            yaml.dump(swarm_cfg, f, default_flow_style=False)
    
        return output_file
