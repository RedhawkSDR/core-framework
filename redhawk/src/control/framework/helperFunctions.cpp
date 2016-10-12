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


#include <string>
#include <iostream>

#include <uuid/uuid.h>

#include <boost/filesystem/path.hpp>

#include <ossie/CF/cf.h>
#include <ossie/CorbaUtils.h>
#include <ossie/ossieSupport.h>
#include <ossie/debug.h>

namespace fs = boost::filesystem;

void ossie::createProfileFromFileName(std::string fileName, std::string& profile)
{
    profile = "<profile filename=\"" + fileName + "\" />";

    return;
}

bool ossie::isValidFileName(const char* fileName)
{
    int fsOpSuccessAttempts = 0;
    bool fsOpSuccess = false;
    if (fileName[0] == 0) {
        return fsOpSuccess;
    }
    while (!fsOpSuccess) {
        try {
#if BOOST_FILESYSTEM_VERSION < 3            
	    fs::path testPath(fileName, fs::portable_posix_name);
#else
	    fs::path testPath(fileName);
#endif
            fsOpSuccess = true;
        } catch ( ... ) {
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        }
    }
    return fsOpSuccess;
}

const char* ossie::spd_rel_file(const char* spdFile, const char* name, std::string& fileName)
{
    fs::path spdPath(spdFile);

    fs::path filePath = spdPath.branch_path() / name;

    fileName = filePath.string();

    return fileName.c_str();
}

std::string ossie::generateUUID()
{
    uuid_t id;
    uuid_generate(id);

    // Per the man page, UUID strings are 36 characters plus the '\0' terminator.
    char strbuf[37];
    uuid_unparse(id, strbuf);

    return std::string("DCE:") + strbuf;
}

