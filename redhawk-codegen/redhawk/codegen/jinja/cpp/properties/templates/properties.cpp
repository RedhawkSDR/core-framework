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
/*{% macro initsequence(prop) %}*/
//%   if prop is structsequence
${prop.cppname}.resize(${prop.cppvalues|length});
//%     for value in prop.cppvalues
//%       set index = loop.index0
//%       for field in prop.structdef.fields
${prop.cppname}[${index}].${field.cppname} = ${value[field.identifier]};
//%       endfor
//%     endfor
//%   else
// Set the sequence with its initial values
//%     for value in prop.cppvalues
${prop.cppname}.push_back(${value});
//%     endfor
//%   endif
/*{%- endmacro %}*/

/*{% macro addproperty(prop) %}*/
addProperty(${prop.cppname},
//% if prop.cppvalues
            ${prop.cppname},
//% elif prop.cppvalue
            ${prop.cppvalue},
//% endif
            "${prop.identifier}",
            "${prop.name}",
            "${prop.mode}",
            "${prop.units}",
            "${prop.action}",
            "${prop.kinds|join(',')}");
/*{%- endmacro %}*/

/*{% macro structdef(struct) %}*/
struct ${struct.cpptype}${' : public '+struct.baseclass if struct.baseclass} {
    ${struct.cpptype} ()${' : '+struct.baseclass+'()' if struct.baseclass}
    {
/*{% for field in struct.fields if not field.inherited and field.cppvalue %}*/
        ${field.cppname} = ${field.cppvalue};
/*{% endfor %}*/
    };

    static std::string getId() {
        return std::string("${struct.identifier}");
    };
/*{% for field in struct.fields if not field.inherited %}*/
/*{%   if loop.first %}*/

/*{%   endif %}*/
    ${field.cpptype} ${field.cppname};
/*{% endfor %}*/
};

inline bool operator>>= (const CORBA::Any& a, ${struct.cpptype}& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
/*{% set ifelse = joiner('else ') %}*/
/*{% for field in struct.fields %}*/
        ${ifelse()}if (!strcmp("${field.identifier}", props[idx].id)) {
/*{% if field.type == 'char' %}*/
/*{%   set extractName = 'temp_char' %}*/
            CORBA::Char temp_char;
/*{% else %}*/
/*{%   set extractName = 's.'+field.cppname %}*/
/*{% endif %}*/
            if (!(props[idx].value >>= ${cpp.extract(extractName, field.type, field.iscomplex)})) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
/*{% if field.type == 'char' %}*/
            s.${field.cppname} = temp_char;
/*{% endif %}*/
        }
/*{% endfor %}*/
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const ${struct.cpptype}& s) {
    CF::Properties props;
    props.length(${struct.fields|length});
/*{% for field in struct.fields %}*/
    props[${loop.index0}].id = CORBA::string_dup("${field.identifier}");
    props[${loop.index0}].value <<= ${cpp.insert('s.'+field.cppname, field.type, field.iscomplex)};
/*{% endfor %}*/
    a <<= props;
};

inline bool operator== (const ${struct.cpptype}& s1, const ${struct.cpptype}& s2) {
/*{% for field in struct.fields %}*/
    if (s1.${field.cppname}!=s2.${field.cppname})
        return false;
/*{% endfor %}*/
    return true;
};

inline bool operator!= (const ${struct.cpptype}& s1, const ${struct.cpptype}& s2) {
    return !(s1==s2);
};
/*{%- endmacro %}*/
