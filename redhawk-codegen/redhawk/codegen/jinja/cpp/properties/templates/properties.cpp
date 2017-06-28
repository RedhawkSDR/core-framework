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

/*{% macro initializestructseq(prop) %}*/
/*{% for value in prop.cppvalues %}*/
    {
        ${prop.structdef.cpptype} __tmp;
/*{%   for key in value %}*/
/*{%     for field in prop.structdef.fields %}*/
/*{%       if field.identifier == key %}*/
/*{%         if field is simple %}*/
        __tmp.${field.cppname} = ${value[key]};
/*{%         elif field is simplesequence%}*/
/*{%           for fieldvalue in value[key] %}*/
        __tmp.${field.cppname}.push_back(${fieldvalue});
/*{%           endfor %}*/
/*{%         endif %}*/
/*{%       endif %}*/
/*{%     endfor %}*/
/*{%   endfor %}*/
        ${prop.cppname}.push_back(__tmp);
    }
/*{% endfor %}*/
/*{%- endmacro %}*/

/*{% macro structdef(struct) %}*/
/*{%   for field in struct.fields if field.enums %}*/
/*{%     if loop.first %}*/
namespace enums {
    // Enumerated values for ${struct.identifier}
    namespace ${struct.cppname} {
/*{%     endif %}*/
        ${enumvalues(field)|indent(8)}
/*{%     if loop.last %}*/
    }
}

/*{%     endif %}*/
/*{%   endfor %}*/
struct ${struct.cpptype}${' : public '+struct.baseclass if struct.baseclass} {
    ${struct.cpptype} ()${' : '+struct.baseclass+'()' if struct.baseclass}
    {
/*{% for field in struct.fields %}*/
/*{%   if not field.inherited %}*/
/*{%     if field is simple and field.cppvalue %}*/
        ${field.cppname} = ${field.cppvalue};
/*{%     elif field is simplesequence and field.cppvalues %}*/
/*{%       for value in field.cppvalues %}*/
        ${field.cppname}.push_back(${value});
/*{%       endfor %}*/
/*{%     endif %}*/
/*{%   endif %}*/
/*{% endfor %}*/
    }

    static std::string getId() {
        return std::string("${struct.identifier}");
    }

    static const char* getFormat() {
        return "${struct.format}";
    }
/*{% for field in struct.fields if not field.inherited %}*/
/*{%   if loop.first %}*/

/*{%   endif %}*/
/*{%   if field.isOptional %}*/
/*{%     if field is simplesequence %}*/
    optional_property<${field.cpptype} > ${field.cppname};
/*{%     else %}*/
    optional_property<${field.cpptype}> ${field.cppname};
/*{%     endif %}*/
/*{%   else %}*/
    ${field.cpptype} ${field.cppname};
/*{%   endif %}*/
/*{% endfor %}*/
};

inline bool operator>>= (const CORBA::Any& a, ${struct.cpptype}& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
/*{% for field in struct.fields %}*/
    if (props.contains("${field.identifier}")) {
/*{% if field.isOptional %}*/
        if (!(ossie::any::isNull(props["${field.identifier}"]))) {
/*{%   if field is simple %}*/
/*{%     if field.type == 'char' %}*/
            CORBA::Char tmp;
/*{%     else %}*/
            ${field.cpptype} tmp;
/*{%     endif %}*/
/*{%     set extractName = 'tmp' %}*/
            if (!(props["${field.identifier}"] >>= ${cpp.extract(extractName, field.type, field.iscomplex)})) return false;
/*{%   elif field is simplesequence %}*/
            ${field.cpptype} tmp;
            if (!(props["${field.identifier}"] >>= tmp)) return false;
/*{%   endif %}*/
            s.${field.cppname} = tmp;
        } else {
            s.${field.cppname}.reset();
        }
/*{% else %}*/
/*{%   if field is simple %}*/
/*{%     if field.type == 'char' %}*/
/*{%       set extractName = 'temp_char' %}*/
        CORBA::Char temp_char;
/*{%     else %}*/
/*{%       set extractName = 's.'+field.cppname %}*/
/*{%     endif %}*/
        if (!(props["${field.identifier}"] >>= ${cpp.extract(extractName, field.type, field.iscomplex)})) return false;
/*{%     if field.type == 'char' %}*/
        s.${field.cppname} = temp_char;
/*{%     endif %}*/
/*{%   elif field is simplesequence %}*/
        if (!(props["${field.identifier}"] >>= s.${field.cppname})) return false;
/*{%   endif %}*/
/*{% endif %}*/
    }
/*{% endfor %}*/
    return true;
}

inline void operator<<= (CORBA::Any& a, const ${struct.cpptype}& s) {
    redhawk::PropertyMap props;
/*{% for field in struct.fields %}*/
/*{%   if field.isOptional %}*/
    if (s.${field.cppname}.isSet()) {
/*{%     if field is simple %}*/
        props["${field.identifier}"] = ${cpp.insert('*(s.'+field.cppname+')', field.type, field.iscomplex)};
/*{%     elif field is simplesequence %}*/
        props["${field.identifier}"] = *(s.${field.cppname});
/*{%     endif %}*/
    }
/*{%   else %}*/ 
/*{%     if field is simple %}*/
    props["${field.identifier}"] = ${cpp.insert('s.'+field.cppname, field.type, field.iscomplex)};
/*{%     elif field is simplesequence %}*/
    props["${field.identifier}"] = s.${field.cppname};
/*{%     endif %}*/
/*{%   endif %}*/
/*{% endfor %}*/
    a <<= props;
}

inline bool operator== (const ${struct.cpptype}& s1, const ${struct.cpptype}& s2) {
/*{% for field in struct.fields %}*/
    if (s1.${field.cppname}!=s2.${field.cppname})
        return false;
/*{% endfor %}*/
    return true;
}

inline bool operator!= (const ${struct.cpptype}& s1, const ${struct.cpptype}& s2) {
    return !(s1==s2);
}
/*{%- endmacro %}*/

/*{% macro enumvalues(prop) %}*/
// Enumerated values for ${prop.identifier}
namespace ${prop.cppname} {
/*{% for enum in prop.enums %}*/
    static const ${prop.cpptype} ${enum.cpplabel} = ${enum.cppvalue};
/*{% endfor %}*/
}
/*{%- endmacro %}*/
