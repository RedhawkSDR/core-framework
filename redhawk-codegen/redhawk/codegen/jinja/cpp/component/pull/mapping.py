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

from redhawk.codegen.model.softwarecomponent import ComponentTypes
from redhawk.codegen.lang.idl import IDLInterface

from redhawk.codegen.jinja.cpp.component.base import BaseComponentMapper

class PullComponentMapper(BaseComponentMapper):
    def _mapComponent(self, softpkg):
        cppcomp = {}
        cppcomp['baseclass'] = self.baseClass(softpkg)
        cppcomp['userclass'] = self.userClass(softpkg)
        cppcomp['superclasses'] = self.superClasses(softpkg)
        cppcomp['interfacedeps'] = tuple(self.getInterfaceDependencies(softpkg))
        cppcomp['hasmultioutport'] = self.hasMultioutPort(softpkg)
        cppcomp['implements'] = self.getImplementedInterfaces(softpkg)
        return cppcomp

    @staticmethod
    def getImplementedInterfaces(softpkg):
        additionalinfo = set()
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
            additionalinfo.add(interface)
            for parent in inherits.get(interface, []):
                additionalinfo.add(parent)

        return additionalinfo

    @staticmethod
    def userClass(softpkg):
        return {'name'  : softpkg.basename()+'_i',
                'header': softpkg.basename()+'.h',
                'file'  : softpkg.basename()+'.cpp'}

    @staticmethod
    def baseClass(softpkg):
        baseclass = softpkg.basename() + '_base'
        return {'name'  : baseclass,
                'header': baseclass+'.h',
                'file'  : baseclass+'.cpp'}

    @staticmethod
    def superClasses(softpkg):
        if softpkg.type() == ComponentTypes.RESOURCE:
            name = 'Component'
        elif softpkg.type() == ComponentTypes.DEVICE:
            name = 'Device_impl'
            aggregate = 'virtual POA_CF::AggregatePlainDevice'
        elif softpkg.type() == ComponentTypes.LOADABLEDEVICE:
            name = 'LoadableDevice_impl'
            aggregate = 'virtual POA_CF::AggregateLoadableDevice'
        elif softpkg.type() == ComponentTypes.EXECUTABLEDEVICE:
            name = 'ExecutableDevice_impl'
            aggregate = 'virtual POA_CF::AggregateExecutableDevice'
        else:
            raise ValueError, 'Unsupported software component type', softpkg.type()
        classes = [{'name': name, 'header': '<ossie/'+name+'.h>'}]
        if softpkg.descriptor().supports('IDL:CF/AggregateDevice:1.0'):
            classes.append({'name': aggregate, 'header': '<CF/AggregateDevices.h>'})
            classes.append({'name': 'AggregateDevice_impl', 'header': '<ossie/AggregateDevice_impl.h>'})

        additionalinfo = PullComponentMapper.getImplementedInterfaces(softpkg)

        if 'DigitalScanningTuner' in additionalinfo:
            classes.append({'name': 'virtual frontend::digital_scanning_tuner_delegation', 'header': ''})
        elif 'AnalogScanningTuner' in additionalinfo:
            classes.append({'name': 'virtual frontend::analog_scanning_tuner_delegation', 'header': ''})
        elif 'DigitalTuner' in additionalinfo:
            classes.append({'name': 'virtual frontend::digital_tuner_delegation', 'header': ''})
        elif 'AnalogTuner' in additionalinfo:
            classes.append({'name': 'virtual frontend::analog_tuner_delegation', 'header': ''})
        elif 'FrontendTuner' in additionalinfo:
            classes.append({'name': 'virtual frontend::frontend_tuner_delegation', 'header': ''})

        # Add additonal FRONTEND delegate interfaces
        if 'RFInfo' in additionalinfo:
            classes.append({'name': 'virtual frontend::rfinfo_delegation', 'header': ''})
        if 'RFSource' in additionalinfo:
            classes.append({'name': 'virtual frontend::rfsource_delegation', 'header': ''})
        if 'GPS' in additionalinfo:
            classes.append({'name': 'virtual frontend::gps_delegation', 'header': ''})
        if 'NavData' in additionalinfo:
            classes.append({'name': 'virtual frontend::nav_delegation', 'header': ''})

        return classes
