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
    while (!fsOpSuccess) {
        try {
#if BOOST_FILESYSTEM_VERSION == 2
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


static std::string getLogConfig(std::string uri)
{
    std::string localPath;

    std::string::size_type fsPos = uri.find("?fs=");
    if (std::string::npos == fsPos) {
        return localPath;
    }

    std::string IOR = uri.substr(fsPos + 4);
    CORBA::Object_var obj = ossie::corba::stringToObject(IOR);
    if (CORBA::is_nil(obj)) {
        return localPath;
    }

    CF::FileSystem_var fileSystem = CF::FileSystem::_narrow(obj);
    if (CORBA::is_nil(fileSystem)) {
        return localPath;
    }

    std::string remotePath = uri.substr(0, fsPos);
    CF::OctetSequence_var data;
    try {
        CF::File_var remoteFile = fileSystem->open(remotePath.c_str(), true);
        CORBA::ULong size = remoteFile->sizeOf();
        remoteFile->read(data, size);
    } catch (...) {
        return localPath;
    }

    std::string tempPath = remotePath;
    std::string::size_type slashPos = remotePath.find_last_of('/');
    if (std::string::npos != slashPos) {
        tempPath.erase(0, slashPos + 1);
    }
    std::fstream localFile(tempPath.c_str(), std::ios::out|std::ios::trunc);
    if (!localFile) {
        return localPath;
    }

    if (localFile.write((const char*)data->get_buffer(), data->length())) {
        localPath = tempPath;
    }
    localFile.close();

    return localPath;
}


void ossie::configureLogging(const char* logcfgUri, int defaultLevel)
{
    if (logcfgUri) {
        if (strncmp("file://", logcfgUri, 7) == 0) {
            // File URI, just remove the scheme.
            LoggingConfigurator::configure(logcfgUri + 7);
            return;
        } else if (strncmp("sca:", logcfgUri, 4) == 0) {
            // SCA URI; "?fs=" must have been given, or the file will not be located.
            std::string localFile = getLogConfig(std::string(logcfgUri + 4));
            if (!localFile.empty()) {
                LoggingConfigurator::configure(localFile.c_str());
                return;
            }
        }
    }

    LoggingConfigurator::configure(defaultLevel);
}
