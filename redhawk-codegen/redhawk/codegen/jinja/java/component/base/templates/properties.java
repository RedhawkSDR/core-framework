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
/*{% macro structdef(prop) %}*/
/**
 * The structure for property ${prop.identifier}
 * 
 * @generated
 */
public static class ${prop.javatype} extends StructDef {
/*{% for field in prop.fields %}*/
/*{%   if field is simple %}*/
    ${simple(field)|indent(4)}
/*{%   elif field is simplesequence %}*/
    ${simplesequence(field)|indent(4)}
/*{%   endif %}*/
/*{% endfor %}*/

    /**
     * @generated
     */
    public ${prop.javatype}(
/*{%- filter trim|lines|join(', ') %}*/
/*{%   for field in prop.fields %}*/
/*{%     if field is simple %}*/
${field.javatype} ${field.javaname}
/*{%     elif field is simplesequence %}*/
List<${field.javatype}> ${field.javaname}
/*{%     endif %}*/
/*{%   endfor %}*/
/*{% endfilter -%}*/
) {
        this();
/*{% for field in prop.fields if not field.inherited %}*/
        this.${field.javaname}.setValue(${field.javaname});
/*{% endfor %}*/
    }

    /**
     * @generated
     */
/*{% for field in prop.fields if not field.inherited %}*/
    public void set_${field.javaname}(/*{% if field is simple %}*/${field.javatype} ${field.javaname}/*{% elif field is simplesequence %}*/List<${field.javatype}> ${field.javaname}/*{% endif%}*/) {
        this.${field.javaname}.setValue(${field.javaname});
    }
    public /*{% if field is simple %}*/${field.javatype}/*{% elif field is simplesequence %}*/List<${field.javatype}>/*{% endif%}*/ get_${field.javaname}() {
        return this.${field.javaname}.getValue();
    }
/*{% endfor %}*/

    /**
     * @generated
     */
    public ${prop.javatype}() {
/*{% for field in prop.fields if not field.inherited %}*/
        addElement(this.${field.javaname});
/*{% endfor %}*/
    }

    public String getId() {
        return ${java.stringLiteral(prop.identifier)};
    }
};
/*{%- endmacro %}*/

/*{% macro simple(prop) %}*/
public final ${prop.javaclass}Property ${prop.javaname} =
    new ${prop.javaclass}Property(
        "${prop.identifier}", //id
        ${java.stringLiteral(prop.name) if prop.name else java.NULL}, //name
        ${prop.javavalue}, //default value
        Mode.${prop.mode|upper}, //mode
        Action.${prop.action|upper}, //action
/*{% if prop.isOptional %}*/
        new Kind[] {${prop.javakinds|join(',')}}, //kind
        true
/*{% else %}*/
        new Kind[] {${prop.javakinds|join(',')}}
/*{% endif %}*/
        );
/*{% endmacro %}*/

/*{% macro simplesequence(prop) %}*/
public final ${prop.javaclass}SequenceProperty ${prop.javaname} =
    new ${prop.javaclass}SequenceProperty(
        "${prop.identifier}", //id
        ${java.stringLiteral(prop.name) if prop.name else java.NULL}, //name
        ${prop.javaclass}SequenceProperty.asList(${prop.javavalues|join(',')}), //default value
        Mode.${prop.mode|upper}, //mode
        Action.${prop.action|upper}, //action
/*{% if prop.isOptional %}*/
        new Kind[] {${prop.javakinds|join(',')}}, //kind
        true
/*{% else %}*/
        new Kind[] {${prop.javakinds|join(',')}}
/*{% endif %}*/
        );
/*{% endmacro %}*/

/*{% macro struct(prop) %}*/
/*{% if not prop.builtin %}*/
${structdef(prop)}

/*{% endif %}*/
public final StructProperty<${prop.javatype}> ${prop.javaname} =
    new StructProperty<${prop.javatype}>(
        "${prop.identifier}", //id
        ${java.stringLiteral(prop.name) if prop.name else java.NULL}, //name
        ${prop.javatype}.class, //type
        new ${prop.javatype}(), //default value
        Mode.${prop.mode|upper}, //mode
        new Kind[] {${prop.javakinds|join(',')}} //kind
        );
/*{% endmacro %}*/

/*{% macro structsequence(prop) %}*/
/*{% if not prop.structdef.builtin %}*/
${structdef(prop.structdef)}

/*{% endif %}*/
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

/*{% macro create(prop) %}*/
/**
 * The property ${prop.identifier}
 * ${prop.description|default("If the meaning of this property isn't clear, a description should be added.", true)}
 *
 * @generated
 */
/*{%   if prop is simple %}*/
${simple(prop)}
/*{%   elif prop is simplesequence %}*/
${simplesequence(prop)}
/*{%   elif prop is struct %}*/
${struct(prop)}
/*{%   elif prop is structsequence %}*/
${structsequence(prop)}
/*{%   endif %}*/
/*{% endmacro %}*/

/*{% macro enumvalues(prop) %}*/
/*{%   if prop is structsequence %}*/
/*{%     set prop = prop.structdef %}*/
/*{%   endif %}*/
/**
 * Enumerated values for ${prop.identifier}
 */
public static class ${prop.javaname} {
/*{%   if prop is struct %}*/
/*{%     for field in prop.fields if field.enums %}*/
/*{%       if not loop.first %}*/

/*{%       endif %}*/
    ${enumvalues(field)|indent(4)}
/*{%     endfor %}*/
/*{%   else %}*/
/*{%     for enum in prop.enums %}*/
    public static final ${enum.javatype} ${enum.javalabel} = ${enum.javavalue};
/*{%     endfor %}*/
/*{%  endif %}*/
}
/*{% endmacro %}*/
