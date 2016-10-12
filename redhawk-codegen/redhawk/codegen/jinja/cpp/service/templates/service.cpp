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
//% set className = component.userclass.name
//% set baseClass = component.baseclass.name
//% set artifactType = component.artifacttype
//% set operations = component.operations
/**************************************************************************

    This is the ${artifactType} code. This file contains the child class where
    custom functionality can be added to the ${artifactType}. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "${component.userclass.header}"

PREPARE_LOGGING(${className})
${className}::${className}(char *devMgr_ior, char *name) :
    ${baseClass}(devMgr_ior, name)
{
}

${className}::~${className}()
{
}

/*{% for operation in operations %}*/
${operation.returns} ${className}::${operation.name}(${operation.arglist})
{
/*{% if operation.returns != 'void' %}*/
/*{%   if operation.returns == 'CORBA::Object_ptr' %}*/
    ${operation.returns} tmpVal = CORBA::Object::_nil();
/*{%   else %}*/
    ${operation.returns} tmpVal;
/*{%   endif %}*/
/*{% endif %}*/
    // TODO: Fill in this function
/*{% if operation.returns != 'void' %}*/
    
/*{%   if operation.returns == 'char*' %}*/
    return CORBA::string_dup(tmpVal);
/*{%   else %}*/
    return tmpVal;
/*{%   endif %}*/
/*{% endif %}*/
}

/*{% endfor %}*/
