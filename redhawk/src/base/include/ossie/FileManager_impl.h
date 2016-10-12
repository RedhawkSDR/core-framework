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


#ifndef __FILEMANAGER__IMPL
#define __FILEMANAGER__IMPL

#include "ossie/CF/cf.h"
#include "ossiecf.h"
#include "ossie/debug.h"

#include "FileSystem_impl.h"
class OSSIECF_API FileManager_impl: public virtual POA_CF::FileManager,  public FileSystem_impl
{
    ENABLE_LOGGING

public:
    FileManager_impl (const char* _fsroot);

    ~FileManager_impl ();

    void
    mount (const char* mountPoint, CF::FileSystem_ptr _fileSystem)
    throw (CORBA::SystemException, CF::InvalidFileName,
           CF::FileManager::InvalidFileSystem,
           CF::FileManager::MountPointAlreadyExists);

    void
    unmount (const char* mountPoint)
    throw (CF::FileManager::NonExistentMount, CORBA::SystemException);
    void
    mkdir (const char* directoryName)
    throw (CF::InvalidFileName, CF::FileException, CORBA::SystemException);
    void
    rmdir (const char* directoryName)
    throw (CF::InvalidFileName, CF::FileException, CORBA::SystemException);
    void
    query (CF::Properties& fileSysProperties)
    throw (CF::FileSystem::UnknownFileSystemProperties, CORBA::SystemException);

    void
    remove (const char* fileName)
    throw (CF::InvalidFileName, CF::FileException, CORBA::SystemException);
    void
    copy (const char* sourceFileName, const char* destinationFileName)
    throw (CF::FileException, CF::InvalidFileName, CORBA::SystemException);
    CORBA::Boolean exists (const char* fileName)
    throw (CF::InvalidFileName, CORBA::SystemException);
    CF::File_ptr create (const char* fileName)
    throw (CF::FileException, CF::InvalidFileName, CORBA::SystemException);
    CF::File_ptr open (const char* fileName, CORBA::Boolean read_Only)
    throw (CF::FileException, CF::InvalidFileName, CORBA::SystemException);
    CF::FileSystem::FileInformationSequence* list (const char* pattern)
    throw (CF::InvalidFileName, CF::FileException, CORBA::SystemException);
    CF::FileManager::MountSequence* getMounts ()throw (CORBA::SystemException);

private:
    bool getFSandFSPath(const char* path, long& mountTableIndex, std::string& FSPath);
    unsigned int pathMatches(const char* path, const char* mPoint, std::string& FSPath);
    void regularExpressionMountSearch(std::string new_pattern, CF::FileSystem::FileInformationSequence_var& fis);

    CF::FileManager::MountSequence_var mount_table;

    unsigned int numMounts;

};                  /* END CLASS DEFINITION FileManager */
#endif              /* __FILEMANAGER__ */
