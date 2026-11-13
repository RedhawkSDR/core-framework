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

#include <string>
#include <list>

#include <boost/thread/shared_mutex.hpp>

#include <ossie/CF/cf.h>
#include <ossie/debug.h>

#include "FileSystem_impl.h"

class FileManager_impl: public virtual POA_CF::FileManager,  public FileSystem_impl
{
    ENABLE_LOGGING

public:
    FileManager_impl (const char* _fsroot);
    ~FileManager_impl ();

    void mount (const char* mountPoint, CF::FileSystem_ptr _fileSystem);

    void unmount (const char* mountPoint);

    void mkdir (const char* directoryName);

    void rmdir (const char* directoryName);

    void query (CF::Properties& fileSysProperties);

    void remove (const char* fileName);

    void copy (const char* sourceFileName, const char* destinationFileName);

    void move (const char* sourceFileName, const char* destinationFileName);

    CORBA::Boolean exists (const char* fileName);

    CF::File_ptr create (const char* fileName);

    CF::File_ptr open (const char* fileName, CORBA::Boolean read_Only);

    CF::FileSystem::FileInformationSequence* list (const char* pattern);

    CF::FileManager::MountSequence* getMounts ();

private:
    struct MountPoint {
        std::string path;
        CF::FileSystem_var fs;
        
        MountPoint (const std::string& path, CF::FileSystem_ptr fs):
            path(path),
            fs(CF::FileSystem::_duplicate(fs))
        {
        }

        bool contains (const std::string& filepath) const;
        std::string getRelativePath (const std::string& filepath) const;
    };

    typedef std::list<MountPoint> MountList;
    MountList mountedFileSystems;
    boost::shared_mutex mountsLock;

    MountList::iterator getMountForPath (const std::string& path);

    CORBA::ULongLong getCombinedProperty (const char* propId);

};                  /* END CLASS DEFINITION FileManager */
#endif              /* __FILEMANAGER__ */
