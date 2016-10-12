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
/*{% block license %}*/
/*# Allow child templates to include license #*/
/*{% endblock %}*/
//% set className = component.userclass.name 
//% set baseClass = component.reprogclass.name if component is device else component.baseclass.name
//% set includeGuard = className.upper() + '_IMPL_H'
#ifndef ${includeGuard}
#define ${includeGuard}

/*{% if component is device %}*/
#include "${component.reprogclass.header}"
/*{% else %}*/
#include "${component.baseclass.header}"
#include "Device_impl.h"
/*{% endif %}*/

class ${className};

class ${className} : public ${baseClass}
{
    ENABLE_LOGGING
    public:
//% if component is not device
        ${className}(const char *uuid, const char *label);
//% else
        ${className}(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        ${className}(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        ${className}(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        ${className}(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
//% endif

        ~${className}();
        int serviceFunction();
//% if component is device
        CORBA::Boolean allocateCapacity(const CF::Properties& capacities) 
            throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CF::Device::InsufficientCapacity, CORBA::SystemException);
        void deallocateCapacity(const CF::Properties& capacities) 
            throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CORBA::SystemException);

    protected:
        void hwLoadRequest(CF::Properties& request);

//% else 
        virtual void setParentDevice(Device_impl* parentDevice) { _parentDevice = parentDevice; };
        virtual Device_impl* getParentDevice() { return _parentDevice; };
    
    private:
        Device_impl* _parentDevice;
//% endif

//% if component is executabledevice
    private:
        Resource_impl* generateResource(int argc, char* argv[], ConstructorPtr fnptr, const char* libName);
//% endif
};

#endif // ${includeGuard}
