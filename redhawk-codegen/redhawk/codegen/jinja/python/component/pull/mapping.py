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

import sys
from redhawk.codegen.model.softwarecomponent import ComponentTypes
from redhawk.codegen.lang.idl import IDLInterface
from redhawk.codegen import libraries

from redhawk.codegen.jinja.mapping import ComponentMapper

class PullComponentMapper(ComponentMapper):
    def _mapComponent(self, softpkg):
        pycomp = {}
        pycomp['baseclass'] = self.baseClass(softpkg)
        pycomp['userclass'] = self.userClass(softpkg)
        pycomp['superclasses'] = self.superClasses(softpkg)
        pycomp['poaclass'] = self.poaClass(softpkg)
        pycomp['interfacedeps'] = self.getInterfaceDependencies(softpkg)
        pycomp['hasmultioutport'] = self.hasMultioutPort(softpkg)
        pycomp['hastunerstatusstructure'] = self.hasTunerStatusStructure(softpkg)
        pycomp['hasfrontendprovides'] = self.hasFrontendProvidesPorts(softpkg)

        # Determine which FRONTEND interfaces this device implements (provides)
        pycomp['implements'] = self.getImplementedInterfaces(softpkg)

        return pycomp

    @staticmethod
    def userClass(softpkg):
        return {'name'  : softpkg.basename()+'_i',
                'file'  : softpkg.basename()+'.py'}

    @staticmethod
    def baseClass(softpkg):
        baseclass = softpkg.basename() + '_base'
        return {'name'  : baseclass,
                'file'  : baseclass+'.py'}

    def hasFrontendProvidesPorts(self, softpkg):
        for port in softpkg.providesPorts():
            if 'FRONTEND' in port.repid():
                return True
        return False

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

    @staticmethod
    def superClasses(softpkg):
        if softpkg.type() == ComponentTypes.RESOURCE:
            name = 'Component'
            package = 'ossie.component'
        elif softpkg.type() == ComponentTypes.DEVICE:
            name = 'Device'
            package = 'ossie.device'
        elif softpkg.type() == ComponentTypes.LOADABLEDEVICE:
            name = 'LoadableDevice'
            package = 'ossie.device'
        elif softpkg.type() == ComponentTypes.EXECUTABLEDEVICE:
            name = 'ExecutableDevice'
            package = 'ossie.device'
        else:
            raise ValueError, 'Unsupported software component type', softpkg.type()
        classes = [{'name': name, 'package': package}]
        if softpkg.descriptor().supports('IDL:CF/AggregateDevice:1.0'):
            classes.append({'name': 'AggregateDevice', 'package': 'ossie.device'})

        deviceinfo = PullComponentMapper.getImplementedInterfaces(softpkg)
        # If this device is any type of tuner, replace the Device_impl base
        # class with the FRONTEND-specific tuner device class
        if 'FrontendTuner' in deviceinfo:
            # Add the most specific tuner delegate interface:
            #   (Digital > Analog > Frontend)
            if 'DigitalScanningTuner' in deviceinfo:
                classes.append({'name': 'digital_scanning_tuner_delegation', 'package': 'frontend'})
            elif 'AnalogScanningTuner' in deviceinfo:
                classes.append({'name': 'analog_scanning_tuner_delegation', 'package': 'frontend'})
            elif 'DigitalTuner' in deviceinfo:
                classes.append({'name': 'digital_tuner_delegation', 'package': 'frontend'})
            elif 'AnalogTuner' in deviceinfo:
                classes.append({'name': 'analog_tuner_delegation', 'package': 'frontend'})
            elif 'FrontendTuner' in additionalinfo:
                classes.append({'name': 'frontend_tuner_delegation', 'package': 'frontend'})

        # Add additonal FRONTEND delegate interfaces
        if 'RFInfo' in deviceinfo:
            classes.append({'name': 'rfinfo_delegation', 'package': 'frontend'})
        if 'RFSource' in deviceinfo:
            classes.append({'name': 'rfsource_delegation', 'package': 'frontend'})
        if 'GPS' in deviceinfo:
            classes.append({'name': 'gps_delegation', 'package': 'frontend'})
        if 'NavData' in deviceinfo:
            classes.append({'name': 'nav_delegation', 'package': 'frontend'})

        return classes

    @staticmethod
    def poaClass(softpkg):
        aggregate = softpkg.descriptor().supports('IDL:CF/AggregateDevice:1.0')
        if softpkg.type() == ComponentTypes.RESOURCE:
            return 'CF__POA.Resource'
        elif softpkg.type() == ComponentTypes.DEVICE:
            if aggregate:
                return 'CF__POA.AggregatePlainDevice'
            else:
                return 'CF__POA.Device'
        elif softpkg.type() == ComponentTypes.LOADABLEDEVICE:
            if aggregate:
                return 'CF__POA.AggregateLoadableDevice'
            else:
                return 'CF__POA.LoadableDevice'
        elif softpkg.type() == ComponentTypes.EXECUTABLEDEVICE:
            if aggregate:
                return 'CF__POA.AggregateExecutableDevice'
            else:
                return 'CF__POA.ExecutableDevice'
        else:
            raise ValueError, 'Unsupported software component type', softpkg.type()

    def getInterfaceDependencies(self, softpkg):
        for namespace in self.getInterfaceNamespaces(softpkg):
            requires = libraries.getPackageRequires(namespace)
            library = libraries.getInterfaceLibrary(namespace)
            yield {'name':library['libname'], 'requires': requires, 'module':  library['pymodule']}
