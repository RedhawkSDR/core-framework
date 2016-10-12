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
//% set classname = portgenerator.className()
package ${package};

import ${component.package}.${component.baseclass.name};

/**
 * @generated
 */
public class ${classname} extends ${portgenerator.poaClass()} {

    /**
     * @generated
     */
    protected ${component.baseclass.name} parent;

    /**
     * @generated
     */
    protected String name;

    /**
     * @generated
     */
    public ${classname}(${component.baseclass.name} parent, String portName)
    {
        this.parent = parent;
        this.name = portName;

        //begin-user-code
        //end-user-code
    }
/*{% for operation in portgenerator.operations() %}*/

    /**
     * @generated
     */
    public ${operation.returns} ${operation.name}(${operation.arglist})${" throws " + operation.throws if operation.throws}
    {
        //begin-user-code
        // TODO you must provide an implementation for this port.
/*{% if operation.returns != 'void' %}*/
    /*{%   if operation.returns == 'org.omg.CORBA.Object' %}*/
        return null;
/*{%   else %}*/
        return ${java.defaultValue(operation.returns)};
/*{%   endif %}*/
/*{% endif %}*/
        //end-user-code
    }
/*{% endfor %}*/
}
