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


#ifndef ORBSUPPORT_H
#define ORBSUPPORT_H

#include <string>

/**
The ossieSupport namespace contains useful functions used throughout the
OSSIE framework. The classes in this namespace are also useful for SCA
component developers.
*/

namespace ossie
{

// Miscellaneous helper functions

    void createProfileFromFileName(std::string fileName, std::string& profile);
    std::string getFileNameFromProfile(std::string profile);
    bool isValidFileName(const char* fileName);
    const char* spd_rel_file(const char* spdfile, const char* name, std::string& fileName);

    void configureLogging(const char* logcfgUri, int defaultLevel);

    std::string generateUUID();

}  // Close ossieSupport Namespace
#endif
