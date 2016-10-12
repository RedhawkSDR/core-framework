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

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/
/*{% endblock %}*/
package ${component.package};

import org.ossie.properties.*;
import frontend.*;

/*{% block struct %}*/
/*{% from "frontend_properties.java" import frontendstructdef with context %}*/
 /*{% for prop in component.properties %}*/
/*{%     if prop.name == "frontend_tuner_status" %}*/ 
${frontendstructdef(prop.structdef)}
/*{%     endif %}*/
/*{% endfor %}*/
/*{% endblock %}*/
