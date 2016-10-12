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


/* SCA */
#ifndef __FILESYSTEM_IMPL__
#define __FILESYSTEM_IMPL__

#include <vector>
#include <string>

#include <boost/filesystem/path.hpp>
#include <boost/thread/mutex.hpp>

#include "ossie/File_impl.h"
#include "ossie/ossieSupport.h"
#include <ossie/prop_helpers.h>

class FileSystem_impl: public virtual POA_CF::FileSystem
{
    ENABLE_LOGGING

public:
    FileSystem_impl ();
    FileSystem_impl (const char* _root);
    ~ FileSystem_impl ();

    void remove (const char* fileName)
        throw (CF::InvalidFileName, CF::FileException, CORBA::SystemException);
    void _local_remove (const char* fileName)
        throw (CF::InvalidFileName, CF::FileException, CORBA::SystemException);

    void copy (const char* sourceFileName, const char* destinationFileName)
        throw (CF::FileException, CF::InvalidFileName, CORBA::SystemException);

    void move (const char* sourceFileName, const char* destinationFileName)
        throw (CF::FileException, CF::InvalidFileName, CORBA::SystemException);

    void mkdir (const char* directoryName)
        throw (CF::InvalidFileName, CF::FileException, CORBA::SystemException);

    void rmdir (const char* directoryName)
        throw (CF::InvalidFileName, CF::FileException, CORBA::SystemException);

    void query (CF::Properties& fileSysProperties)
        throw (CF::FileSystem::UnknownFileSystemProperties, CORBA::SystemException);

    CORBA::Boolean exists (const char* fileName)
        throw (CF::InvalidFileName, CORBA::SystemException);
    CORBA::Boolean _local_exists (const char* fileName)
        throw (CF::InvalidFileName, CORBA::SystemException);

    CF::File_ptr create (const char* fileName)
        throw (CF::FileException, CF::InvalidFileName, CORBA::SystemException);

    CF::File_ptr open (const char* fileName, CORBA::Boolean read_Only)
        throw (CF::FileException, CF::InvalidFileName, CORBA::SystemException);

    CF::FileSystem::FileInformationSequence* list (const char* pattern)
        throw (CF::InvalidFileName, CF::FileException, CORBA::SystemException);

    std::string getLocalPath(const char* fileName);

    void incrementFileIORCount(std::string &fileName, std::string &fileIOR);

    void decrementFileIORCount(std::string &fileName, std::string &fileIOR);

    std::vector< std::string > getFileIOR(std::string &fileName);

private:
    FileSystem_impl (const FileSystem_impl& _fsi);
    FileSystem_impl operator= (FileSystem_impl _fsi);

    void init ();
    void removeDirectory(const boost::filesystem::path& dirPath, bool doRemove);

    std::map< std::string, std::vector<std::string> > fileOpenIOR;

    boost::filesystem::path root;
    boost::mutex interfaceAccess;
    boost::mutex fileIORCountAccess;

};                                                /* END CLASS DEFINITION FileSystem */
#endif                                            /* __FILESYSTEM__ */
