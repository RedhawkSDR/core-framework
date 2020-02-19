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
//% set className = component.baseclass.name
//% set includeGuard = className.upper() + '_IMPL_BASE_H'
/*{% block license %}*/
/*# Allow child templates to include license #*/
/*{% endblock %}*/
/*{% block includeGuard %}*/
#ifndef ${includeGuard}
#define ${includeGuard}
/*{% endblock %}*/

/*{% block includes %}*/
#include <boost/thread.hpp>
/*{% for superclass in component.superclasses if superclass.header %}*/
#include ${superclass.header}
/*{% endfor %}*/
#include <ossie/ThreadedComponent.h>
/*{% block includeExtentions %}*/
/*# Allow for child template extensions #*/
/*{% endblock %}*/

/*{% filter lines|unique|join('\n') %}*/
/*{%   for portgen in component.portgenerators if portgen.header() %}*/
#include ${portgen.header()}
/*{%   endfor %}*/
/*{% endfilter %}*/
/*{% if "struct_props.h" in generator.sourceFiles(component) %}*/
#include "struct_props.h"
/*{% endif %}*/
/*{% endblock %}*/
/*{% block defines %}*/
/*# Allow child templates to add #define statements #*/
/*{% endblock %}*/

/*{% from "properties/properties.cpp" import enumvalues %}*/
/*{% for prop in component.properties if prop.enums %}*/
/*{%   if loop.first %}*/
namespace enums {
/*{%   endif %}*/
    ${enumvalues(prop)|indent(4)}
/*{%   if loop.last %}*/
}

/*{%   endif %}*/
/*{% endfor %}*/
/*{% block classPrototype %}*/
class ${className} : public ${component.superclasses|join(', public ', attribute='name')}, protected ThreadedComponent
/*{% endblock %}*/
{
/*{% for portgen in component.portgenerators if portgen.hasDeclaration() %}*/
    friend class ${portgen.className()};
/*{%   if loop.last %}*/

/*{%   endif %}*/
/*{% endfor %}*/
    public:
/*{% block baseConstructor %}*/
/*{% if component is device %}*/
        ${className}(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        ${className}(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        ${className}(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        ${className}(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
/*{% else %}*/
        ${className}(const char *uuid, const char *label);
/*{% endif %}*/
/*{% endblock %}*/
        ~${className}();

/*{% block cfResource %}*/
        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);
/*{% endblock %}*/

/*{% block basePublicFunctions %}*/
        void loadProperties();
/*{% endblock %}*/
/*{% block extendedPublic %}*/
/*{%   if 'FrontendTuner' in component.implements %}*/

        virtual CF::Properties* getTunerStatus(const std::string& allocation_id);
/*{%   endif %}*/
/*{% endblock extendedPublic %}*/

    protected:
/*{% block baseProtectedFunctions %}*/
/*{%   if component.hasmultioutport %}*/
        void connectionTableChanged(const std::vector<connection_descriptor_struct>* oldValue, const std::vector<connection_descriptor_struct>* newValue);

/*{%   endif %}*/
/*{% endblock %}*/
/*{% block baseProtectedMembers %}*/
/*{% for prop in component.properties if not prop.inherited %}*/
/*{%   if loop.first %}*/
        // Member variables exposed as properties
/*{%   endif %}*/
/*{% if prop.kinds|join('') == "message" %}*/
        /// Message structure definition for ${prop.cppname}
/*{% endif %}*/
/*{% if prop.kinds|join('') != "message" %}*/
        /// Property: ${prop.cppname}
/*{% endif %}*/
        ${prop.cpptype} ${prop.cppname};
/*{% endfor %}*/
/*{% for port in component.ports %}*/
/*{%   if loop.first %}*/

        // Ports
/*{%   endif %}*/
        /// Port: ${port.cppname}
        ${port.cpptype} *${port.cppname};
/*{% endfor %}*/
/*{% endblock %}*/
/*{% block extendedProtected%}*/
/*{% endblock %}*/

    private:
/*{% if component is device %}*/
        void construct();
/*{% endif %}*/
/*{% block extendedPrivate%}*/
/*{% endblock %}*/
/*{% block extensions %}*/
/*# Allow for child template extensions #*/
/*{% endblock %}*/
};
#endif // ${includeGuard}
