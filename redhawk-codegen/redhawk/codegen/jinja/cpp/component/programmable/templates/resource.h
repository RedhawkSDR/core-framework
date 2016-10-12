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
//% set baseClass = component.reprogclass.name
//% set includeGuard = className.upper() + '_IMPL_H'
//% set executesHWComponents = component.executesHWComponents 
//% set executesPersonaDevices = not executesHWComponents
//% set executeType = "resource" if executesHWComponents else "persona"
//% set executeClass = "Resource_impl" if executesHWComponents else "Device_impl"
#ifndef ${includeGuard}
#define ${includeGuard}

#include "${component.reprogclass.header}"

/*{% if component.hasHwLoadRequestProp and component.hasHwLoadStatusProp %}*/
typedef ${baseClass}<hw_load_request_struct, hw_load_status_struct> ${baseClass}_type;
/*{% elif component.hasHwLoadRequestProp %}*/
typedef ${baseClass}<hw_load_request_struct> ${baseClass}_type;
/*{% else %}*/
typedef ${baseClass}<> ${baseClass}_type;
/*{% endif %}*/


class ${className};

class ${className} : public ${baseClass}_type
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
        void initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException);

    protected:
        ${executeClass}* generate${executeType.capitalize()}(int argc, char* argv[], ConstructorPtr fnptr, const char* libName);
        bool loadHardware(HwLoadStatusStruct& requestStatus);
        void unloadHardware(const HwLoadStatusStruct& requestStatus);
        
//% endif
};

#endif // ${includeGuard}
