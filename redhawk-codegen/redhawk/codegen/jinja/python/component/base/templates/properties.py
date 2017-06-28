#{#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK core.
#
# REDHAWK core is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#}
#{% macro simple(prop) %}
${prop.pyname} = simple_property(id_="${prop.identifier}",
#%    if prop.name
                                 name="${prop.name}",
#%    endif
                                 type_="${prop.type}",
#%    if prop.pyvalue is defined
                                 defvalue=${prop.pyvalue},
#%    endif
#%    if prop.isComplex
                                 complex=True,
#%    endif
                                 mode="${prop.mode}",
                                 action="${prop.action}",
                                 kinds=${prop.kinds|quote|tuple}
#%-   if prop.hasDescription
,
                                 description="""${prop.description}"""
#%-   endif
)
#{% endmacro %}

#{% macro simplesequence(prop) %}
${prop.pyname} = simpleseq_property(id_="${prop.identifier}",
#%    if prop.name
                                    name="${prop.name}",
#%    endif
                                    type_="${prop.type}",
#%    if prop.pyvalue
                                    defvalue=[
#{%-  filter trim|lines|join(', ') %}
#{%   for val in prop.pyvalue %}
${val}
#{%   endfor %}
#{%   endfilter %}
                                             ],
#%    else
                                    defvalue=[],
#%    endif
#%    if prop.units
                                    units="${prop.units}",
#%    endif
#%    if prop.isComplex
                                    complex=True,
#%    endif
                                    mode="${prop.mode}",
                                    action="${prop.action}",
                                    kinds=${prop.kinds|quote|tuple}
#%-   if prop.hasDescription
,
                                    description="""${prop.description}"""
#%-   endif
)
#{% endmacro %}

#{% macro struct(prop) %}
${prop.pyname} = struct_property(id_="${prop.identifier}",
#%    if prop.name
                                 name="${prop.name}",
#%    endif
                                 structdef=${prop.pyclass},
                                 configurationkind=${prop.kinds|quote|tuple},
                                 mode="${prop.mode}"
#%-   if prop.hasDescription
,
                                 description="""${prop.description}"""
#%-   endif
)
#{% endmacro %}

#{% macro structsequence(prop) %}
${prop.pyname} = structseq_property(id_="${prop.identifier}",
#%    if prop.name
                                    name="${prop.name}",
#%    endif
                                    structdef=${prop.structdef.pyclass},
                                    defvalue=[${prop.pyvalues|join(',')}],
                                    configurationkind=${prop.kinds|quote|tuple},
                                    mode="${prop.mode}"
#%-   if prop.hasDescription
,
                                    description="""${prop.description}"""
#%-   endif
)
#{% endmacro %}

#{% macro initializer(fields) %}
#{%  filter trim|lines|join(', ') %}
#{%   for field in fields %}
#{%    if field is simplesequence %}
#{%     if field.pyvalue %}
${field.pyname}=[
#{%-     filter trim|lines|join(', ') %}
#{%       for val in field.pyvalue %}
${val}
#{%       endfor %}
#{%      endfilter %}
]
#{%     else %}
${field.pyname}=[]
#{%     endif %}
#{%    elif field is simple %}
${field.pyname}=${field.pyvalue|default(python.defaultValue(field.type))}
#{%    endif %}
#{%   endfor %}
#{%  endfilter %}
#{% endmacro %}

#{#
# Creates the argument list for calling a base class __init__ method
#}
#{% macro baseinit(fields) %}
#{%   filter trim|lines|join %}
#{%   for field in fields if field.inherited %}
, ${field.pyname}=${field.pyname}
#{%   endfor %}
#{%   endfilter %}
#{% endmacro %}

#{% macro members(fields) %}
#{%   filter trim|lines|join(',') %}
#{%   for field in fields if not field.inherited %}
("${field.pyname}",self.${field.pyname})
#{%   endfor %}
#{%   endfilter %}
#{% endmacro %}

#{% macro structdef(struct, initialize=true) %}
class ${struct.pyclass}(${struct.baseclass|default('object')}):
#{%   for field in struct.fields if not field.inherited %}
#{%   filter codealign %}
#%    if field is simplesequence
    ${field.pyname} = simpleseq_property(
#%    elif field is simple
    ${field.pyname} = simple_property(
#%    endif
                                      id_="${field.identifier}",

#%      if field.name
                                      name="${field.name}",
#%      endif
                                      type_="${field.type}"
#%-     if field.pyvalue is defined
,
#%        if field is simple
                                      defvalue=${field.pyvalue}
#%        elif field is simplesequence
                                      defvalue=[
#{%-        filter trim|lines|join(', ') %}
#{%          for val in field.pyvalue %}
${val}
#{%          endfor %}
#{%         endfilter %}
]
#%        endif
#%      else
#%        if field is simplesequence
,
                                      defvalue=[]
#%        endif
#%-     endif
#%-     if field.isComplex
,
                                      complex=True
#%-     endif
#%-	if field.isOptional
,
                                      optional=True
#%-	endif
)
#{%   endfilter %}

#{%   endfor %}
#{%   if initialize: %}
    def __init__(self, **kw):
        """Construct an initialized instance of this struct definition"""
        for classattr in type(self).__dict__.itervalues():
            if isinstance(classattr, (simple_property, simpleseq_property)):
                classattr.initialize(self)
        for k,v in kw.items():
            setattr(self,k,v)
#{%   else %}
    def __init__(self, ${initializer(struct.fields)}):
#{%     if struct.baseclass %}
        ${struct.baseclass}.__init__(self${baseinit(struct.fields)})
#{%     endif %}
#{%     for field in struct.fields if not field.inherited %}
        self.${field.pyname} = ${field.pyname}
#{%     endfor %}
#{%   endif %}

    def __str__(self):
        """Return a string representation of this structure"""
        d = {}
#{%   for field in struct.fields %}
        d["${field.pyname}"] = self.${field.pyname}
#{%   endfor %}
        return str(d)

    @classmethod
    def getId(cls):
        return "${struct.identifier}"

    @classmethod
    def isStruct(cls):
        return True

    def getMembers(self):
#{%   if struct.baseclass %}
        return ${struct.baseclass}.getMembers(self) + [${members(struct.fields)}]
#{%   else %}
        return [${members(struct.fields)}]
#{%   endif %}
#{% endmacro %}

#{% macro create(prop) %}
#{% if prop is simple %}
${simple(prop)}
#{% elif prop is simplesequence %}
${simplesequence(prop)}
#{% elif prop is struct %}
${struct(prop)}
#{% elif prop is structsequence %}
${structsequence(prop)}
#{% endif %}
#{% endmacro %}

#{% macro enumvalues(prop) %}
#{%   if prop is structsequence %}
#{%     set prop = prop.structdef %}
#{%   endif %}
# Enumerated values for ${prop.identifier}
class ${prop.pyname}:
#{%   if prop is struct %}
#{%     for field in prop.fields if field.enums %}
#{%       if not loop.first %}

#{%       endif %}
    ${enumvalues(field)|indent(4)}
#{%     endfor %}
#{%   else %}
#{%     for enum in prop.enums %}
    ${enum.pylabel} = ${enum.pyvalue}
#{%     endfor %}
#{%   endif %}
#{% endmacro %}
