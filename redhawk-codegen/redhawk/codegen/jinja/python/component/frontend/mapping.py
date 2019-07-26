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

from redhawk.codegen.jinja.python.component.pull.mapping import PullComponentMapper
from redhawk.codegen.jinja.python.properties import PythonPropertyMapper

class FrontendComponentMapper(PullComponentMapper):
    def _mapComponent(self, softpkg):
        # Defer most mapping to base class
        pycomp = super(FrontendComponentMapper,self)._mapComponent(softpkg)

        return pycomp

    @staticmethod
    def superClasses(softpkg):
        # Start with the superclasses from the pull mapping, overriding only
        # what's different for FRONTEND devices
        classes = PullComponentMapper.superClasses(softpkg)

        # Only plain devices are supported for FRONTEND
        if softpkg.type() == ComponentTypes.DEVICE:
            deviceinfo = PullComponentMapper.getImplementedInterfaces(softpkg)
            # If this device is any type of tuner, replace the Device_impl base
            # class with the FRONTEND-specific tuner device class
            if 'FrontendTuner' in deviceinfo:
                for parent in classes:
                    if parent['name'] == 'Device':
                        parent['name'] = 'FrontendTunerDevice'
                        parent['package'] = 'frontend'

                    # Set the parent to scanner if needed
                    if 'DigitalScanningTuner' in deviceinfo or 'AnalogScanningTuner' in deviceinfo:
                        if parent['name'] == 'FrontendTunerDevice':
                            parent['name'] = 'FrontendScannerDevice'

        return classes

class FrontendPropertyMapper(PythonPropertyMapper):
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
        'FRONTEND::scanner_allocation',
        'FRONTEND::listener_allocation'
    )

    def mapStructProperty(self, prop, fields):
        pyprop = super(FrontendPropertyMapper,self).mapStructProperty(prop, fields)
        if prop.identifier() == 'FRONTEND::tuner_status_struct':
            pyprop['baseclass'] = 'frontend.default_frontend_tuner_status_struct_struct'
            for field in fields:
                if field['pyname'] in self.TUNER_STATUS_BASE_FIELDS:
                    field['inherited'] = True
        elif prop.identifier() in self.FRONTEND_BUILTINS:
            pyprop['pytype'] = "frontend.fe_types." + pyprop['pyname']
            pyprop['pyvalue'] = pyprop['pytype'] + "()"
            pyprop['builtin'] = True
        return pyprop

    def getStructPropertyType(self, prop):
        if prop.identifier() in self.FRONTEND_BUILTINS:
            return 'frontend.%s' % prop.name()
        else:
            return super(FrontendPropertyMapper,self).getStructPropertyType(prop)
