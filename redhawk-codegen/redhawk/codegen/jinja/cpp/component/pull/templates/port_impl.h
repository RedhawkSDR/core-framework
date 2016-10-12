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
#ifndef PORT_H
#define PORT_H

/*{% filter lines|unique|join('\n') %}*/
/*{% for portgen in component.portgenerators %}*/
/*{%   for header in portgen.dependencies() %}*/
#include ${header}
/*{%   endfor %}*/
/*{% endfor %}*/
/*{% endfilter %}*/

class ${component.baseclass.name};
class ${component.userclass.name};

#define CORBA_MAX_TRANSFER_BYTES omniORB::giopMaxMsgSize()

/*{% for portgen in component.portgenerators if portgen.hasDeclaration() %}*/
// ----------------------------------------------------------------------------------------
// ${portgen.className()} declaration
// ----------------------------------------------------------------------------------------
/*{% include portgen.declaration() %}*/

/*{% endfor %}*/
#endif // PORT_H
