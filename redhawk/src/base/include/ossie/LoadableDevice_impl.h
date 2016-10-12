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

#ifndef LOADABLE_DEVICE_IMPL_H
#define LOADABLE_DEVICE_IMPL_H

#include <vector>
#include <map>
#include "Resource_impl.h"
#include "Device_impl.h"
#include "FileSystem_impl.h"
#include "CF/cf.h"
#include <boost/filesystem/path.hpp>

typedef std::multimap<std::string, std::string, std::less<std::string>, std::allocator<std::pair<std::string, std::string> > >
copiedFiles_type;


/* CLASS DEFINITION *******************************************************************************
 ************************************************************************************************ */
class OSSIECF_API LoadableDevice_impl: public virtual
    POA_CF::LoadableDevice,
    public
    Device_impl
{
    ENABLE_LOGGING

protected:
    void incrementFile (std::string);
    void decrementFile (std::string);
    std::map<std::string, int> loadedFiles;
    std::map<std::string, CF::FileSystem::FileType> fileTypeTable;
    copiedFiles_type copiedFiles;

public:
    LoadableDevice_impl (char*, char*, char*, char*);
    LoadableDevice_impl (char*, char*, char*, char*, CF::Properties capacities);
    LoadableDevice_impl (char*, char*, char*, char*, char*);
    LoadableDevice_impl (char*, char*, char*, char*, CF::Properties capacities, char*);
    virtual ~LoadableDevice_impl ();
    void
    load (CF::FileSystem_ptr fs, const char* fileName,
          CF::LoadableDevice::LoadType loadKind)
    throw (CF::LoadableDevice::LoadFail, CF::InvalidFileName,
           CF::LoadableDevice::InvalidLoadKind, CF::Device::InvalidState,
           CORBA::SystemException);
    void
    unload (const char* fileName)
    throw (CF::InvalidFileName, CF::Device::InvalidState,
           CORBA::SystemException);
    bool
    isFileLoaded (const char* fileName);

 protected:

    void _loadTree(CF::FileSystem_ptr fs, std::string remotePath, boost::filesystem::path& localPath, std::string fileKey);
    void _deleteTree(const std::string &fileKey);
    void _copyFile(CF::FileSystem_ptr fs, const std::string &remotePath, const std::string &localPath, const std::string &fileKey);

 public:

    void configure (const CF::Properties& configProperties)
    throw (CF::PropertySet::PartialConfiguration,
           CF::PropertySet::InvalidConfiguration, CORBA::SystemException);
    LoadableDevice_impl(); // No default constructor
    LoadableDevice_impl(LoadableDevice_impl&); // No copying

private:
};

#endif

