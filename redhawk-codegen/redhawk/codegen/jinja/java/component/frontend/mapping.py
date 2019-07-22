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
from redhawk.codegen.jinja.java.properties import JavaPropertyMapper

class FrontendComponentMapper(PullComponentMapper):
    def _mapComponent(self, softpkg):
        # Defer most mapping to base class
        javacomp = super(FrontendComponentMapper,self)._mapComponent(softpkg)

        # Determine which FRONTEND interfaces this device implements (provides)
        javacomp['implements'] = self.getImplementedInterfaces(softpkg)
        javacomp['hasfrontendprovides'] = self.hasFrontendProvidesPorts(softpkg)
        javacomp['hastunerstatusstructure'] = self.hasTunerStatusStructure(softpkg)

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

    def superclass(self,softpkg):
        # Start with the superclass from the pull mapping, overriding only
        # what's different for FRONTEND devices
        sc = PullComponentMapper.superclass(self,softpkg)

        # Only plain devices are supported for FRONTEND
        if softpkg.type() == ComponentTypes.DEVICE:
            deviceinfo = FrontendComponentMapper.getImplementedInterfaces(softpkg)
            # If this device is any type of tuner, replace the Device_impl base
            # class with the FRONTEND-specific tuner device class
            if 'FrontendTuner' in deviceinfo:
                if sc['name'] == 'ThreadedDevice':
                    sc['name'] = 'frontend.FrontendTunerDevice<frontend_tuner_status_struct_struct>'
                    sc['header'] = ''
            if 'ScanningTuner' in deviceinfo:
                if sc['name'] == 'frontend.FrontendTunerDevice<frontend_tuner_status_struct_struct>':
                    sc['name'] = 'frontend.FrontendScanningTunerDevice<frontend_tuner_status_struct_struct>'

        return sc

class FrontendPropertyMapper(JavaPropertyMapper):
    TUNER_STATUS_BASE_FIELDS = (
        'tuner_type',
        'allocation_id_csv',
        'center_frequency',
        'bandwidth',
        'sample_rate',
        'enabled',
        'group_id',
        'rf_flow_id'
    )

    FRONTEND_BUILTINS = (
        'FRONTEND::tuner_allocation',
        'FRONTEND::listener_allocation'
    )

    def mapStructProperty(self, prop, fields):
        javaprop = super(FrontendPropertyMapper,self).mapStructProperty(prop, fields)
        if prop.identifier() == 'FRONTEND::tuner_status_struct':
            javaprop['baseclass'] = 'frontend.FETypes.default_frontend_tuner_status_struct_struct'
            for field in fields:
                if field['javaname'] in self.TUNER_STATUS_BASE_FIELDS:
                    field['inherited'] = True
        elif prop.identifier() in self.FRONTEND_BUILTINS:
            javaprop['javatype'] = "frontend.FETypes." + javaprop['javaname'] + "_struct"
            javaprop['javavalue'] = "new " + javaprop['javatype'] + "()"
            javaprop['builtin'] = True
        return javaprop
