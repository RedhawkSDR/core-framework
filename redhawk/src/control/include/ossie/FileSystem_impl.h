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
#include <map>
#include <string>

#include <boost/filesystem/path.hpp>
#include <boost/thread/mutex.hpp>

#include <ossie/CF/cf.h>
#include <ossie/debug.h>

class FileSystem_impl: public virtual POA_CF::FileSystem
{
    ENABLE_LOGGING

public:
    FileSystem_impl (const char* _root);
    ~FileSystem_impl ();

    void remove (const char* fileName);

    void copy (const char* sourceFileName, const char* destinationFileName);

    void move (const char* sourceFileName, const char* destinationFileName);

    void mkdir (const char* directoryName);

    void rmdir (const char* directoryName);

    void query (CF::Properties& fileSysProperties);

    CORBA::Boolean exists (const char* fileName);

    CF::File_ptr create (const char* fileName);

    CF::File_ptr open (const char* fileName, CORBA::Boolean read_Only);

    CF::FileSystem::FileInformationSequence* list (const char* pattern);

    std::string getLocalPath(const char* fileName);
    
    void closeAllFiles();

    void setLogger(rh_logger::LoggerPtr logptr) {
        _fileSysLog = logptr;
    };

protected:
    CORBA::ULongLong getSize () const;
    CORBA::ULongLong getAvailableSpace () const;

    rh_logger::LoggerPtr _fileSysLog;

private:
    FileSystem_impl (const FileSystem_impl& _fsi);
    FileSystem_impl operator= (FileSystem_impl _fsi);

    friend class File_impl;

    typedef std::vector<std::string> IORList;
    typedef std::map<std::string, IORList> IORTable;

    bool _local_exists (const char* fileName);

    void removeDirectory(const boost::filesystem::path& dirPath, bool doRemove);

    void incrementFileIORCount(std::string &fileName, std::string &fileIOR);
    void decrementFileIORCount(std::string &fileName, std::string &fileIOR);
    IORList getFileIOR(const std::string& fileName);

    IORTable fileOpenIOR;

    boost::filesystem::path root;
    boost::mutex interfaceAccess;
    boost::mutex fileIORCountAccess;

};                                                /* END CLASS DEFINITION FileSystem */
#endif                                            /* __FILESYSTEM__ */
