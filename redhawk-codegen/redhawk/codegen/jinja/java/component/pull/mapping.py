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
from redhawk.codegen import libraries

from redhawk.codegen.jinja.java.component.base import BaseComponentMapper

class PullComponentMapper(BaseComponentMapper):
    def _mapComponent(self, softpkg):
        javacomp = {}
        if self.package == '':
            javacomp['package'] = softpkg.basename()+'.java'
        else:
            javacomp['package'] = self.package
        userclass = softpkg.basename()
        baseclass = userclass + '_base'
        javacomp['baseclass'] = {'name': baseclass,
                                 'file': baseclass+'.java'}
        javacomp['userclass'] = {'name': userclass,
                                 'file': userclass+'.java'}
        javacomp['superclass'] = self.superclass(softpkg)
        javacomp['mainclass'] = java.qualifiedName(userclass, javacomp['package'])
        javacomp['jarfile'] = softpkg.basename() + '.jar'
        javacomp['interfacedeps'] = list(self.getInterfaceDependencies(softpkg))
        javacomp['interfacejars'] = self.getInterfaceJars(softpkg)
        javacomp['hasmultioutport'] = self.hasMultioutPort(softpkg)
        javacomp['hasfrontendprovides'] = self.hasFrontendProvidesPorts(softpkg)
        javacomp['hastunerstatusstructure'] = self.hasTunerStatusStructure(softpkg)
        javacomp['implements'] = self.getImplementedInterfaces(softpkg)
        return javacomp

    @staticmethod
    def getImplementedInterfaces(softpkg):
        deviceinfo = set()

        # Ensure that parent interfaces also gets added (so, e.g., a device
        # with a DigitalTuner should also report that it's an AnalogTuner
        # and FrontendTuner)
        inherits = { 'DigitalScanningTuner': ('ScanningTuner', 'DigitalTuner', 'AnalogTuner', 'FrontendTuner'),
                     'AnalogScanningTuner': ('ScanningTuner', 'AnalogTuner', 'FrontendTuner'),
                     'DigitalTuner': ('AnalogTuner', 'FrontendTuner'),
                     'AnalogTuner': ('FrontendTuner',) }

        for port in softpkg.providesPorts():
            idl = IDLInterface(port.repid())
            # Ignore non-FRONTEND intefaces
            if idl.namespace() != 'FRONTEND':
                continue
            interface = idl.interface()
            deviceinfo.add(interface)
            for parent in inherits.get(interface, []):
                deviceinfo.add(parent)

        return deviceinfo

    @staticmethod
    def isTunerStatusStructure(prop):
        if prop.name() != 'frontend_tuner_status':
            return False
        if prop.struct().name() != 'frontend_tuner_status_struct':
            return False
        fields = set(field.name() for field in prop.struct().fields())
        if 'allocation_id_csv' not in fields:
            return False
        if 'bandwidth' not in fields:
            return False
        if 'center_frequency' not in fields:
            return False
        if 'enabled' not in fields:
            return False
        if 'group_id' not in fields:
            return False
        if 'rf_flow_id' not in fields:
            return False
        if 'sample_rate' not in fields:
            return False
        if 'tuner_type' not in fields:
            return False
        return True

    def hasTunerStatusStructure(self, softpkg):
        for prop in softpkg.getStructSequenceProperties():
            if PullComponentMapper.isTunerStatusStructure(prop):
                return True
        return False

    def hasFrontendProvidesPorts(self, softpkg):
        for port in softpkg.providesPorts():
            if 'FRONTEND' in port.repid():
                return True
        return False

    def getInterfaceDependencies(self, softpkg):
        for namespace in self.getInterfaceNamespaces(softpkg):
            yield libraries.getPackageRequires(namespace)

    def getInterfaceJars(self, softpkg):
        jars = []
        for namespace in self.getInterfaceNamespaces(softpkg):
            library = libraries.getInterfaceLibrary(namespace)
            jars.extend(library['jarfiles'])
        return jars

    def superclass(self,softpkg):
        if softpkg.type() == ComponentTypes.RESOURCE:
            name = 'Component'
        elif softpkg.type() == ComponentTypes.DEVICE:
            name = 'ThreadedDevice'
        elif softpkg.type() == ComponentTypes.LOADABLEDEVICE:
            # NOTE: If java gets support for Loadable Devices, this needs to change
            name = 'ThreadedDevice'
        elif softpkg.type() == ComponentTypes.EXECUTABLEDEVICE:
            # NOTE: If java gets support for Executable Devices, this needs to change
            name = 'ThreadedDevice'
        else:
            raise ValueError, 'Unsupported software component type', softpkg.type()
        return {'name': name}
