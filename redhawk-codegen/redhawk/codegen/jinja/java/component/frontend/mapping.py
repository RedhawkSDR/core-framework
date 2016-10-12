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

from redhawk.codegen.lang import java
from redhawk.codegen.model.softwarecomponent import ComponentTypes
from redhawk.codegen.lang.idl import IDLInterface

from redhawk.codegen.jinja.java.component.pull.mapping import PullComponentMapper

class FrontendComponentMapper(PullComponentMapper):
    def _mapComponent(self, softpkg):
        javacomp = {}
        javacomp['package'] = self.package
        userclass = softpkg.name()
        baseclass = userclass + '_base'
        javacomp['baseclass'] = {'name': baseclass,
                                 'file': baseclass+'.java'}
        javacomp['userclass'] = {'name': userclass,
                                 'file': userclass+'.java'}
        javacomp['superclass'] = self.superclass(softpkg)
        javacomp['mainclass'] = java.qualifiedName(userclass, self.package)
        javacomp['jarfile'] = softpkg.name() + '.jar'
        javacomp['interfacedeps'] = list(self.getInterfaceDependencies(softpkg))
        javacomp['interfacejars'] = self.getInterfaceJars(softpkg)
        javacomp['hasbulkio'] = self.hasBulkioPorts(softpkg)
        javacomp['hasfrontend'] = self.hasFrontendPorts(softpkg)
        javacomp['hasfrontendprovides'] = self.hasFrontendProvidesPorts(softpkg)
        javacomp['isafrontendtuner'] = self.isAFrontendTuner(softpkg)
        javacomp['hasfrontendtunerprovides'] = self.hasFrontendTunerProvidesPorts(softpkg)
        javacomp['hasdigitaltunerprovides'] = self.hasDigitalTunerProvidesPorts(softpkg)
        javacomp['hasanalogtunerprovides'] = self.hasAnalogTunerProvidesPorts(softpkg)
        javacomp['softpkgcp'] = self.softPkgDeps(softpkg, format='cp')
        return javacomp

    def superclass(self, softpkg):
        if softpkg.type() == ComponentTypes.RESOURCE:
            name = 'Resource'
        elif softpkg.type() == ComponentTypes.DEVICE:
            name = 'Device'
            # If device contains FrontendInterfaces 
            #  FrontendTuner, DigitalTuner or AnalogTuner, have
            #  device inherit from FrontendTunerDevice
            #  instead of Device_impl
            name = 'Device'
            for port in softpkg.providesPorts():
                idl = IDLInterface(port.repid())
                if idl.namespace() == 'FRONTEND':
                    if idl.interface().find('DigitalTuner') != -1 or \
                       idl.interface().find('AnalogTuner') != -1 or \
                       idl.interface().find('FrontendTuner') != -1:
                        name = 'frontend.FrontendTunerDevice<frontend_tuner_status_struct_struct>'
        elif softpkg.type() == ComponentTypes.LOADABLEDEVICE:
            # NOTE: If java gets support for Loadable Devices, this needs to change
            name = 'Device'
        elif softpkg.type() == ComponentTypes.EXECUTABLEDEVICE:
            # NOTE: If java gets support for Executable Devices, this needs to change
            name = 'Device'
        else:
            raise ValueError, 'Unsupported software component type', softpkg.type()
        return {'name': name}

    def hasFrontendPorts(self, softpkg):
        for port in softpkg.ports():
            if 'FRONTEND' in port.repid():
                return True
        return False

    def hasFrontendProvidesPorts(self, softpkg):
        for port in softpkg.providesPorts():
            if 'FRONTEND' in port.repid():
                return True
        return False

    def isAFrontendTuner(self,softpkg):
        return self.hasFrontendTunerProvidesPorts(softpkg) or \
               self.hasAnalogTunerProvidesPorts(softpkg) or \
               self.hasDigitalTunerProvidesPorts(softpkg)

    def hasFrontendTunerProvidesPorts(self, softpkg):
        for port in softpkg.providesPorts():
            idl = IDLInterface(port.repid())
            if idl.namespace() == 'FRONTEND':
                if idl.interface().find('FrontendTuner') != -1:
                    return True
        return False

    def hasDigitalTunerProvidesPorts(self, softpkg):
        for port in softpkg.providesPorts():
            idl = IDLInterface(port.repid())
            if idl.namespace() == 'FRONTEND':
                if idl.interface().find('DigitalTuner') != -1:
                    return True
        return False

    def hasAnalogTunerProvidesPorts(self, softpkg):
        for port in softpkg.providesPorts():
            idl = IDLInterface(port.repid())
            if idl.namespace() == 'FRONTEND':
                if idl.interface().find('AnalogTuner') != -1:
                    return True
        return False
