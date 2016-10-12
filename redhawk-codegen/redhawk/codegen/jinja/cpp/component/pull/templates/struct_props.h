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
/*{% block license %}*/
/*# Allow child templates to include license #*/
/*{% endblock %}*/
/*{% block header %}*/
#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/
/*{% endblock %}*/

/*{% block includes %}*/
#include <ossie/CorbaUtils.h>
#include <CF/cf.h>
#include <ossie/PropertyMap.h>
/*{% set isSet = False %}*/
/*{% for struct in component.structdefs %}*/
/*{%   for field in struct.fields %}*/
/*{%     if field.isOptional and isSet == False %}*/
#include <ossie/OptionalProperty.h>
#include <ossie/AnyUtils.h>
/*{%       set isSet = True %}*/
/*{%     endif %}*/
/*{%   endfor %}*/
/*{% endfor %}*/
/*{% if component['hasmultioutport'] %}*/
#include <bulkio/bulkio.h>
typedef bulkio::connection_descriptor_struct connection_descriptor_struct;
/*{% endif %}*/
/*{% endblock %}*/

/*{% block struct %}*/
/*{% from "properties/properties.cpp" import structdef with context %}*/
/*{% for struct in component.structdefs if not struct.builtin %}*/
${structdef(struct)}

/*{% endfor %}*/
/*{% endblock %}*/
#endif // STRUCTPROPS_H
