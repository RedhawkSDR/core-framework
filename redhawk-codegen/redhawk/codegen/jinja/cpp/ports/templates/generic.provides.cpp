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
//% set classname = portgen.className()
${classname}::${classname}(std::string port_name, ${component.baseclass.name} *_parent) : 
Port_Provides_base_impl(port_name)
{
    parent = static_cast<${component.userclass.name} *> (_parent);
}

${classname}::~${classname}()
{
}
/*{% for operation in portgen.operations() %}*/

${operation.returns} ${classname}::${operation.name}(${operation.arglist})
{
    boost::mutex::scoped_lock lock(portAccess);
//% set hasreturn = operation.returns != 'void'
/*{% if hasreturn %}*/
    ${operation.temporary} retval${' = %s' % operation.initializer if operation.initializer};
/*{% endif %}*/
    // TODO: Fill in this function
/*{% if hasreturn %}*/
/*{%   if operation.temporary.endswith('_var') %}*/
    return retval._retn();
/*{%   else %}*/
    return retval;
/*{%   endif %}*/
/*{% endif %}*/
}
/*{% endfor %}*/

std::string ${classname}::getRepid() const
{
    return ${portgen.interfaceClass()}::_PD_repoId;
}
