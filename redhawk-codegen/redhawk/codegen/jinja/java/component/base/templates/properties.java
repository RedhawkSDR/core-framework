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
    ${simple(field)|indent(4)}
/*{% endfor %}*/

    /**
     * @generated
     */
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
     * @generated
     */
    public ${prop.javatype}() {
/*{% for field in prop.fields %}*/
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
        new Kind[] {${prop.javakinds|join(',')}} //kind
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
        new Kind[] {${prop.javakinds|join(',')}} //kind
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
