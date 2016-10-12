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
/*{% macro frontendstructdef(prop) %}*/
/**
 * The structure for property ${prop.identifier}
 * 
 * @generated
 */
/*{% import "base/properties.java" as properties with context %}*/
public class ${prop.javatype} extends ${prop.baseclass} {
/*{% for field in prop.fields %}*/
/*{%   if not field.inherited %}*/
    ${properties.simple(field)|indent(4)}
//%    endif
/*{% endfor %}*/

/*{% for field in prop.fields if not field.inherited %}*/
/*{%   if loop.first %}*/
    /**
     * @generated
     */
    public ${prop.javatype}(
/*{%- filter trim|lines|join(', ') %}*/
/*{%   for field in prop.fields if not field.inherited %}*/
${field.javatype} ${field.javaname}
/*{%   endfor %}*/
/*{% endfilter -%}*/
) {
        this();
/*{% for field in prop.fields if not field.inherited %}*/
        this.${field.javaname}.setValue(${field.javaname});
/*{% endfor %}*/
    }
//%    endif
/*{% endfor %}*/

    /**
     * @generated
     */
    public ${prop.javatype}() {
/*{% for field in prop.fields if not field.inherited %}*/
        addElement(this.${field.javaname});
/*{% endfor %}*/
    }
};
/*{%- endmacro %}*/
