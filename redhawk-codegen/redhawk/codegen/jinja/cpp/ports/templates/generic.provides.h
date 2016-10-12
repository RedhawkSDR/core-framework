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
class ${classname} : public POA_${portgen.interfaceClass()}, public Port_Provides_base_impl
{
    public:
        ${classname}(std::string port_name, ${component.baseclass.name} *_parent);
        ~${classname}();

/*{% for op in portgen.operations() %}*/
        ${op.returns} ${op.name}(${op.arglist});
/*{% endfor %}*/
        std::string getRepid() const;

    protected:
        ${component.userclass.name} *parent;
        boost::mutex portAccess;
};
