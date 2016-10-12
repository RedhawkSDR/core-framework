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
/*{% macro frontendtunerstatusstructdef(prop) %}*/
/**
 * The structure for property ${prop.identifier}
 * 
 * @generated
 */
/*{% import "base/properties.java" as properties with context %}*/
public class ${prop.javatype} extends frontend.FrontendTunerStructProps.default_frontend_tuner_status_struct_struct {
/*{% for field in prop.fields %}*/
    ${properties.simple(field)|indent(4)}
/*{% endfor %}*/

    /**
 *      * @generated
 *           */
    public ${prop.javatype}(
/*{%- filter trim|lines|join(', ') %}*/
/*{%   for field in prop.fields %}*/
${field.javatype} ${field.javaname}
/*{%   endfor %}*/
/*{% endfilter -%}*/
) {
        this();
/*{% for field in prop.fields %}*/
        this.${field.javaname}.setValue(${field.javaname});
/*{% endfor %}*/
    }

    /**
 *      * @generated
 *           */
    public ${prop.javatype}() {
/*{% for field in prop.fields %}*/
        addElement(this.${field.javaname});
/*{% endfor %}*/
    }
};
/*{%- endmacro %}*/

/*{% macro frontendstructsequence(prop) %}*/
public final StructSequenceProperty<${prop.structdef.javatype}> ${prop.javaname} =
    new StructSequenceProperty<${prop.structdef.javatype}> (
        "${prop.identifier}", //id
        ${java.stringLiteral(prop.name) if prop.name else java.NULL}, //name
        ${prop.structdef.javatype}.class, //type
/*{% if prop.javavalues %}*/
        StructSequenceProperty.asList(
/*{%   filter trim|lines|join(',\n')|indent(12, true) %}*/
/*{%     for values in prop.javavalues %}*/
            new ${prop.structdef.javatype}(${values|join(', ')})
/*{%     endfor %}*/
/*{%   endfilter %}*/

            ), //defaultValue
/*{% else %}*/
        StructSequenceProperty.<${prop.structdef.javatype}>asList(), //defaultValue
/*{% endif %}*/
        Mode.${prop.mode|upper}, //mode
        new Kind[] { ${prop.javakinds|join(',')} } //kind
    );
/*{% endmacro %}*/
