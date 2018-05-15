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
//% set poa = portgenerator.poaClass()
package ${package};

import ${component.package}.${component.baseclass.name};
import org.ossie.component.PortBase;
import org.ossie.component.RHLogger;

/**
 * @generated
 */
public class ${classname} extends ${portgenerator.poaClass()} implements PortBase {

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

    public RHLogger _portLog = null;
    public void setLogger(RHLogger logger)
    {
        this._portLog = logger;
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
        ${operation.returns} retval${' = %s' % operation.defaultval if operation.defaultval};
        return retval;
/*{% endif %}*/
        //end-user-code
    }
/*{% endfor %}*/

    /**
     * @generated
     */
    public String getRepid()
    {
        return ${portgenerator.helperClass()}.id();
    }

    /**
     * @generated
     */
    public String getDirection()
    {
        return "Provides";
    }
}
