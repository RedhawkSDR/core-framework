/*
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
 */

#include <list>
#include <cstdlib>

#include <ossie/type_traits.h>

const std::string ossie::internal::demangle (const std::string& name)
{
    if (name.empty()) {
	return name;
    }

    std::string::const_iterator pos = name.begin();
    if (*pos == 'N') {
	// Namespaced name, skip first character; technically, we should only
	// read one name segment unless we see this, but in practice it doesn't
	// matter
	++pos;
    }

    // Read '<length>name' segments until the end of the name, or the 'E' that
    // marks the end of a namespaced name
    std::list<std::string> parts;
    while ((pos != name.end()) && (*pos != 'E')) {
	// Find the end of the length
	std::string::const_iterator digit_end = pos;
	while ((digit_end != name.end()) && isdigit(*digit_end)) {
	    ++digit_end;
	}

	// Read the name length 
	std::string length(pos, digit_end);
	int name_length = std::atoi(length.c_str());

	// Pull out the length-delimited name
	std::string::const_iterator name_end = digit_end + name_length;
	parts.push_back(std::string(digit_end, name_end));

	// Advance past end of segment
	pos = name_end;
    }

    // Build a proper namespaced name from the parts
    std::string name_out = parts.front();
    parts.erase(parts.begin());
    for (std::list<std::string>::iterator ii = parts.begin(); ii != parts.end(); ++ii) {
	name_out += "::" + *ii;
    }
    return name_out;
}
