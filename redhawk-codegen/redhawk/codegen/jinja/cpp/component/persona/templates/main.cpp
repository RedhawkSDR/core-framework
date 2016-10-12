/*#
 * This file is protected by Copyright. Please refer to the COPYRIGHT file 
 * distributed with this source distribution.
 * 
 * This file is part of REDHAWK core.
 * 
 * REDHAWK core is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version.
 * 
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 #*/
/*{% extends "base/main.cpp" %}*/

/*{% block main %}*/
${super()}
/*{% if component is device %}*/
extern "C" {
    Device_impl* construct(int argc, char* argv[], Device_impl* parentDevice) {

        struct sigaction sa;
        sa.sa_handler = signal_catcher;
        sa.sa_flags = 0;
        devicePtr = 0;

        Device_impl::start_device(&devicePtr, sa, argc, argv);

        // Any addition parameters passed into construct can now be
        // set directly onto devicePtr since it is the instantiated
        // Redhawk device
        //      Example:
        //         devicePtr->setSharedAPI(sharedAPI);
        devicePtr->setParentDevice(parentDevice);

        return devicePtr;
    }
}
/*{% else %}*/
/*{% set servant = component.basename + '_servant' %}*/
extern "C" {
    Resource_impl* construct(int argc, char* argv[], Device_impl* parentDevice) {

        ${component.userclass.name}* ${servant};
        Resource_impl::start_component(${servant}, argc, argv);

        // Any addition parameters passed into construct can now be
        // set directly onto component_servant since it is the instantiated
        // Redhawk component
        //      Example:
        //         component_servant->setSharedAPI(sharedAPI);
        ${servant}->setParentDevice(parentDevice);

        return ${servant};
    }
}
/*{% endif %}*/
/*{% endblock %}*/
