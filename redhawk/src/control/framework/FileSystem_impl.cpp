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

#include <iostream>
#include <string>

#include <fnmatch.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/iostreams/detail/ios.hpp>
#ifndef BOOST_VERSION
#include <boost/version.hpp>
#endif

#if BOOST_VERSION < 103499
#  include <boost/filesystem/cerrno.hpp>
#else
#  include <boost/cerrno.hpp>
#endif

namespace fs = boost::filesystem;

#include "ossie/FileSystem_impl.h"
#include "ossie/CorbaUtils.h"
#include "ossie/debug.h"

PREPARE_LOGGING(FileSystem_impl)

FileSystem_impl::FileSystem_impl ()
{
    TRACE_ENTER(FileSystem_impl);

    int fsOpSuccessAttempts = 0;
    bool fsOpSuccess = false;
    while (!fsOpSuccess) {
        try {
#if BOOST_FILESYSTEM_VERSION < 3
            if (fs::path::default_name_check_writable())
                { fs::path::default_name_check(fs::portable_posix_name); }
#endif

            root = fs::initial_path();
            fsOpSuccess = true;
        } catch ( const fs::filesystem_error& ex ) {
            LOG_WARN(FileSystem_impl, "Error in filesystem: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( std::exception& ex ) {
            LOG_WARN(FileSystem_impl, "The following standard exception occurred: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( ... ) {
            LOG_WARN(FileSystem_impl, "Caught an unhandled file system exception. Attempting again")
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        }
    }
    if (!fsOpSuccess) {
        LOG_ERROR(FileSystem_impl, "Unable to get access to initial path");
        throw CF::FileException (CF::CF_EEXIST, "Unable to get access to initial path");
    }

    init ();
    TRACE_EXIT(FileSystem_impl);
}


FileSystem_impl::FileSystem_impl (const char* _root)
{
    TRACE_ENTER(FileSystem_impl);

    root = _root;

    init ();
    TRACE_EXIT(FileSystem_impl);
}


void
FileSystem_impl::init ()
{
    TRACE_ENTER(FileSystem_impl);
    TRACE_EXIT(FileSystem_impl);
}

FileSystem_impl::~FileSystem_impl ()
{
    TRACE_ENTER(FileSystem_impl);
    TRACE_EXIT(FileSystem_impl);
}

void FileSystem_impl::remove (const char* fileName) throw (CORBA::SystemException, CF::FileException, CF::InvalidFileName)
{
    boost::mutex::scoped_lock lock(interfaceAccess);
    _local_remove(fileName);
}

void FileSystem_impl::_local_remove (const char* pattern) throw (CORBA::SystemException, CF::FileException, CF::InvalidFileName)
{
    TRACE_ENTER(FileSystem_impl);

    fs::path filePath(root / pattern);
    fs::path dirPath(filePath.parent_path());
    
    std::string searchPattern;
    if ((filePath.filename() == ".") && (fs::is_directory(filePath))) {
        searchPattern = "*";
    } else {
#if BOOST_FILESYSTEM_VERSION < 3
        searchPattern = filePath.filename();
#else
        searchPattern = filePath.filename().string();
#endif
    }
    
    LOG_TRACE(FileSystem_impl, "[FileSystem::remove] using searchPattern " << searchPattern << " in " << filePath.parent_path());
    
    fs::directory_iterator end_itr; // an end iterator (by boost definition)
    
    int fsOpSuccessAttempts = 0;
    bool fsOpSuccess = false;
    std::string error_msg_out = "filesystem error";
    while (!fsOpSuccess) {
        try {
            for (fs::directory_iterator itr(dirPath); itr != end_itr; ++itr) {
                if (fnmatch(searchPattern.c_str(), itr->path().filename().c_str(), 0) == 0) {
                    //remove the file
                    LOG_TRACE(FileSystem_impl, "About to remove file " << itr->path().string());
                    bool rem_retval = false;
                    try {
                        rem_retval = fs::remove(itr->path());
                    } catch ( std::exception& ex ) {
                        std::ostringstream eout;
                        eout << "The following standard exception occurred: "<<ex.what()<<" While removing file from file system";
                        LOG_ERROR(FileSystem_impl, eout.str())
                        throw (CF::FileException (CF::CF_EEXIST, eout.str().c_str()));
                    } catch (...) {
                        LOG_ERROR(FileSystem_impl, "Error removing file. Permissions may be wrong.");
                        throw (CF::FileException (CF::CF_EEXIST, "[FileSystem_impl::remove] Error removing file from file system"));
                        return;
                    }
   
                    if (!rem_retval) {
                        LOG_ERROR(FileSystem_impl, "Attempt to remove non-existent file.");
                        throw (CF::FileException (CF::CF_EEXIST, "[FileSystem_impl::remove] Error removing file from file system"));
                    }
                }
            }
            fsOpSuccess = true;
        } catch ( const fs::filesystem_error& ex ) {
            LOG_WARN(FileSystem_impl, "Error in filesystem: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("Error in filesystem: ")+ex.what();
            if (fsOpSuccessAttempts == 10)
            { break; }
            usleep(10000);
        } catch ( std::exception& ex ) {
            LOG_WARN(FileSystem_impl, "The following standard exception occurred: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("The following standard exception occurred: ")+ex.what();
            if (fsOpSuccessAttempts == 10)
            { break; }
            usleep(10000);
        } catch ( CORBA::Exception& ex ) {
            LOG_WARN(FileSystem_impl, "The following CORBA exception occurred: "<<ex._name()<<". Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("The following CORBA exception occurred: ")+ex._name();
            if (fsOpSuccessAttempts == 10)
            { break; }
            usleep(10000);
        } catch ( ... ) {
            LOG_WARN(FileSystem_impl, "Caught an unhandled file system exception. Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("Caught an unhandled file system exception.");
            if (fsOpSuccessAttempts == 10)
            { break; }
            usleep(10000);
        }
    }
    
    if (!fsOpSuccess) {
        LOG_ERROR(FileSystem_impl, "caught boost filesystem remove error");
        throw CF::FileException(CF::CF_ENOENT, error_msg_out.c_str());
    }
    TRACE_EXIT(FileSystem_impl);
}

void FileSystem_impl::move (const char* sourceFileName, const char* destinationFileName) throw (CORBA::SystemException, CF::InvalidFileName, CF::FileException)
{
        throw CF::FileException(CF::CF_ENOENT, "move operation not supported");
}

void FileSystem_impl::copy (const char* sourceFileName, const char* destinationFileName) throw (CORBA::SystemException, CF::InvalidFileName, CF::FileException)
{
    TRACE_ENTER(FileSystem_impl);
    boost::mutex::scoped_lock lock(interfaceAccess);

    if (sourceFileName[0] != '/' || destinationFileName[0] != '/' || !ossie::isValidFileName(sourceFileName) || !ossie::isValidFileName(destinationFileName)) {
        LOG_ERROR(FileSystem_impl, "copy passed bad filename, throwing exception.");
        throw CF::InvalidFileName (CF::CF_EINVAL, "[FileSystem::copy] Invalid file name");
    }

    fs::path sFile(root / sourceFileName);
    fs::path dFile(root / destinationFileName);

    int fsOpSuccessAttempts = 0;
    bool fsOpSuccess = false;
    std::string error_msg_out = "filesystem error";

    while (!fsOpSuccess) {
        try {

            if (fs::is_directory(sFile)) {
                return;
            }

            if (this->_local_exists(destinationFileName)) {
                LOG_TRACE(FileSystem_impl, "dest file exists. Removing " << destinationFileName);
                this->_local_remove(destinationFileName);
            }
            fsOpSuccess = true;
        } catch ( const fs::filesystem_error& ex ) {
            LOG_WARN(FileSystem_impl, "Error in filesystem: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("Error in filesystem: ")+ex.what();
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( std::exception& ex ) {
            LOG_WARN(FileSystem_impl, "The following standard exception occurred: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("The following standard exception occurred: ")+ex.what();
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( ... ) {
            LOG_WARN(FileSystem_impl, "Caught an unhandled file system exception. Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("Caught an unhandled file system exception.");
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        }
    }
    if (!fsOpSuccess) {
        LOG_ERROR(FileSystem_impl, "caught boost filesystem_error");
        throw CF::FileException(CF::CF_ENOENT, error_msg_out.c_str());
    }

    error_msg_out = "filesystem error";
    fsOpSuccessAttempts = 0;
    fsOpSuccess = false;
    while (!fsOpSuccess) {
        try {
            fs::copy_file(sFile, dFile, fs::copy_option::overwrite_if_exists);
            fsOpSuccess = true;
        } catch ( const fs::filesystem_error& ex ) {
            LOG_WARN(FileSystem_impl, "Error in filesystem: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("Error in filesystem: ")+ex.what();
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( std::exception& ex ) {
            LOG_WARN(FileSystem_impl, "The following standard exception occurred: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("The following standard exception occurred: ")+ex.what();
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( ... ) {
            LOG_WARN(FileSystem_impl, "Caught an unhandled file system exception. Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("Caught an unhandled file system exception.");
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        }
    }
    if (!fsOpSuccess) {
        LOG_ERROR(FileSystem_impl, "caught boost filesystem_error");
        throw CF::FileException(CF::CF_ENOENT, error_msg_out.c_str());
    }
    TRACE_EXIT(FileSystem_impl);
}

CORBA::Boolean FileSystem_impl::exists (const char* fileName)
throw (CORBA::SystemException, CF::InvalidFileName)
{
    boost::mutex::scoped_lock lock(interfaceAccess);
    return _local_exists(fileName);
}

CORBA::Boolean FileSystem_impl::_local_exists (const char* fileName)
throw (CORBA::SystemException, CF::InvalidFileName)
{
    TRACE_ENTER(FileSystem_impl);
    LOG_TRACE(FileSystem_impl, "Checking for existence of SCA file " << fileName);
    if (fileName[0] != '/' || !ossie::isValidFileName(fileName)) {
        LOG_ERROR(FileSystem_impl, "exists passed bad filename, " << fileName << " throwing exception.");
        throw CF::InvalidFileName (CF::CF_EINVAL, "[FileSystem::exists] Invalid file name");
    }

    fs::path fname(root / fileName);
    LOG_TRACE(FileSystem_impl, "Checking for existence of local file " << fname.string());

    TRACE_EXIT(FileSystem_impl);
    return(fs::exists(fname));
}

CF::FileSystem::FileInformationSequence* FileSystem_impl::list (const char* pattern) throw (CORBA::SystemException, CF::FileException, CF::InvalidFileName)
{
    TRACE_ENTER(FileSystem_impl);

    fs::path filePath(root / pattern);
    fs::path dirPath(filePath.parent_path());

    std::string searchPattern;
    if ((filePath.filename() == ".") && (fs::is_directory(filePath))) {
        searchPattern = "*";
    } else {
        searchPattern = std::string(filePath.filename().c_str());
    }

    LOG_TRACE(FileSystem_impl, "[FileSystem::list] using searchPattern " << searchPattern << " in " << filePath.parent_path());

    CF::FileSystem::FileInformationSequence_var result = new CF::FileSystem::FileInformationSequence;

    fs::directory_iterator end_itr; // an end iterator (by boost definition)

    int fsOpSuccessAttempts = 0;
    bool fsOpSuccess = false;
    std::string error_msg_out = "filesystem error";
    while (!fsOpSuccess) {
        try {
            for (fs::directory_iterator itr(dirPath); itr != end_itr; ++itr) {
                if (fnmatch(searchPattern.c_str(), itr->path().filename().c_str(), 0) == 0) {
                    if ((!itr->path().empty()) && (std::string(itr->path().filename().c_str())[0] == '.') && (itr->path().filename() != searchPattern)) {
                        LOG_TRACE(FileSystem_impl, "[FileSystem::list] found hidden match and ignoring " << itr->path().filename());
                        continue;
                    }
                    LOG_TRACE(FileSystem_impl, "[FileSystem::list] match in list with " << itr->path().filename());
                    CORBA::ULong index = result->length();
                    result->length(index + 1);
                   
                    // We need to specially handle the empty '' pattern
                    if (strlen(pattern) == 0) {
                        result[index].name = CORBA::string_dup("/");
                    } else {
                        result[index].name = CORBA::string_dup(itr->path().filename().c_str());
                    }
                    bool readonly = (access(itr->path().string().c_str(), W_OK));
                    if (fs::is_directory(*itr)) {
                        result[index].kind = CF::FileSystem::DIRECTORY;
                        result[index].size = 0;
                    } else {
                        try {
                            result[index].kind = CF::FileSystem::PLAIN;
                            result[index].size = fs::file_size(*itr);
                        } catch ( ... ) {
                            // this file is not good (i.e.: bad link)
                            result->length(index);
                            LOG_WARN(FileSystem_impl, "[FileSystem::list] found a file that cannot be evaluated: " << itr->path().filename() << ". Not listing it.");
                            continue;
                        }
                    }

                    CF::Properties prop;
                    prop.length(5);
                    prop[0].id = CORBA::string_dup(CF::FileSystem::CREATED_TIME_ID);
                    prop[0].value <<= static_cast<CORBA::ULongLong>(fs::last_write_time(*itr));
                    prop[1].id = CORBA::string_dup(CF::FileSystem::MODIFIED_TIME_ID);
                    prop[1].value <<= static_cast<CORBA::ULongLong>(fs::last_write_time(*itr));
                    prop[2].id = CORBA::string_dup(CF::FileSystem::LAST_ACCESS_TIME_ID);
                    prop[2].value <<= static_cast<CORBA::ULongLong>(fs::last_write_time(*itr));
                    prop[3].id = CORBA::string_dup("READ_ONLY");
                    prop[3].value <<= CORBA::Any::from_boolean(readonly);
                    prop[4].id = CORBA::string_dup("IOR_AVAILABLE");
                    std::string localFilename = itr->path().string();
                    prop[4].value = ossie::strings_to_any(getFileIOR(localFilename), CORBA::tk_string);
                    result[index].fileProperties = prop;
                }
            }
            fsOpSuccess = true;
        } catch ( const fs::filesystem_error& ex ) {
            LOG_WARN(FileSystem_impl, "Error in filesystem: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("Error in filesystem: ")+ex.what();
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( std::exception& ex ) {
            LOG_WARN(FileSystem_impl, "The following standard exception occurred: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("The following standard exception occurred: ")+ex.what();
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( CORBA::Exception& ex ) {
            LOG_WARN(FileSystem_impl, "The following CORBA exception occurred: "<<ex._name()<<". Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("The following CORBA exception occurred: ")+ex._name();
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( ... ) {
            LOG_WARN(FileSystem_impl, "Caught an unhandled file system exception. Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("Caught an unhandled file system exception.");
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        }
    }

    if (!fsOpSuccess) {
        LOG_ERROR(FileSystem_impl, "caught boost filesystem list error");
        throw CF::FileException(CF::CF_ENOENT, error_msg_out.c_str());
    }

    TRACE_EXIT(FileSystem_impl);
    return result._retn();
}


CF::File_ptr FileSystem_impl::create (const char* fileName) throw (CORBA::SystemException, CF::InvalidFileName, CF::FileException)
{
    TRACE_ENTER(FileSystem_impl);

    if (!ossie::isValidFileName(fileName)) {
        LOG_ERROR(FileSystem_impl, "create passed bad filename, throwing exception.");
        throw CF::InvalidFileName (CF::CF_EINVAL, "[FileSystem::create] Invalid file name");
    }

    if (_local_exists(fileName)) {
        LOG_ERROR(FileSystem_impl, "FileName exists in create, throwing exception.");
        throw CF::FileException(CF::CF_EEXIST, "File exists.");
    }

    File_impl* file = new File_impl (fileName, root, this, false, true);
    PortableServer::POA_var poa = ossie::corba::RootPOA()->find_POA("Files", 1);
    PortableServer::ObjectId_var oid = poa->activate_object(file);
    file->_remove_ref();

    CF::File_var fileServant = file->_this();
    std::string fileIOR = ossie::corba::objectToString(fileServant);
    file->setIOR(fileIOR);

    TRACE_EXIT(FileSystem_impl);
    return fileServant._retn();
}

void FileSystem_impl::incrementFileIORCount(std::string &fileName, std::string &fileIOR) {
    boost::mutex::scoped_lock lock(fileIORCountAccess);
    fileOpenIOR[fileName].push_back(fileIOR);
}

void FileSystem_impl::decrementFileIORCount(std::string &fileName, std::string &fileIOR) {
    boost::mutex::scoped_lock lock(fileIORCountAccess);
    if (fileOpenIOR.count(fileName) == 0) {
        return;
    } else {
        std::vector< std::string >::iterator iter = fileOpenIOR[fileName].begin();
        while (iter != fileOpenIOR[fileName].end()) {
            if (*iter == fileIOR) {
                break;
            }
            iter++;
        }
        if (iter == fileOpenIOR[fileName].end()) {
            return;
        } else {
            fileOpenIOR[fileName].erase(iter);
            if (fileOpenIOR[fileName].size() == 0) {
                fileOpenIOR.erase(fileName);
            }
            return;
        }
    }
}

std::vector< std::string > FileSystem_impl::getFileIOR(std::string &fileName) {
    boost::mutex::scoped_lock lock(fileIORCountAccess);
    std::vector< std::string > retVal;
    if (fileOpenIOR.count(fileName) != 0) {
        retVal = fileOpenIOR[fileName];
    }
    return retVal;
}

CF::File_ptr FileSystem_impl::open (const char* fileName, CORBA::Boolean read_Only) throw (CORBA::SystemException, CF::InvalidFileName, CF::FileException)
{
    TRACE_ENTER(FileSystem_impl);
    if (!ossie::isValidFileName(fileName)) {
        LOG_ERROR(FileSystem_impl, "failed to open file; file '" << fileName << "' is invalid");
        throw CF::InvalidFileName (CF::CF_EINVAL, "[FileSystem::open] Invalid file name");
    }


    if (!_local_exists(fileName)) {
        LOG_ERROR(FileSystem_impl, "failed to open file; file '" << fileName << "' does not exist");
        throw CF::FileException(CF::CF_EEXIST, "[FileSystem::open] File does not exist.");
    }

    File_impl* file = new File_impl (fileName, root, this, read_Only, false);
    PortableServer::POA_var poa = ossie::corba::RootPOA()->find_POA("Files", 1);
    PortableServer::ObjectId_var oid = poa->activate_object(file);
    file->_remove_ref();
    
    CF::File_var fileObj = file->_this();
    std::string fileIOR = ossie::corba::objectToString(fileObj);
    std::string strFileName = root.string();
    strFileName += fileName;
    incrementFileIORCount(strFileName, fileIOR);
    file->setIOR(fileIOR);

    TRACE_EXIT(FileSystem_impl);
    return fileObj._retn();
}


void FileSystem_impl::mkdir (const char* directoryName) throw (CORBA::SystemException, CF::FileException, CF::InvalidFileName)
{
    TRACE_ENTER(FileSystem_impl);

    if (!ossie::isValidFileName(directoryName)) {
        LOG_ERROR(FileSystem_impl, "mkdir passed bad filename, throwing exception.");
        throw CF::InvalidFileName (CF::CF_EINVAL, "Invalid file name");
    }

    fs::path dirPath(root / directoryName);

    int fsOpSuccessAttempts = 0;
    bool fsOpSuccess = false;
    bool dirExists = false;
    std::string error_msg_out = "filesystem error";
    while (!fsOpSuccess) {
        try {
            dirExists = fs::exists(dirPath);
            fsOpSuccess = true;
        } catch ( const fs::filesystem_error& ex ) {
            LOG_WARN(FileSystem_impl, "Error in filesystem: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("Error in filesystem: ")+ex.what();
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( std::exception& ex ) {
            LOG_WARN(FileSystem_impl, "The following standard exception occurred: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("The following standard exception occurred: ")+ex.what();
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( ... ) {
            LOG_WARN(FileSystem_impl, "Caught an unhandled file system exception. Attempting again")
            fsOpSuccessAttempts++;
            error_msg_out = std::string("Caught an unhandled file system exception.");
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        }
    }

    if (!fsOpSuccess) {
        LOG_ERROR(FileSystem_impl, "caught boost filesystem list error");
        throw CF::FileException(CF::CF_ENOENT, error_msg_out.c_str());
    }

    try {
        fs::path::iterator walkPath(dirPath.begin());
        fs::path currentPath;
        while (walkPath != dirPath.end()) {
            LOG_TRACE(FileSystem_impl, "Walking path to create directories, current path " << currentPath.string());
            currentPath /= *walkPath;
            if (!fs::exists(currentPath)) {
                LOG_TRACE(FileSystem_impl, "Creating directory " << currentPath.string());
                try {
                    fs::create_directory(currentPath);
                } catch (...) {
                    LOG_ERROR(FileSystem_impl, "Failed to create directory");
                    throw CF::FileException (CF::CF_ENFILE, "Failed to create directory");
                }
            }
            ++walkPath;
        }
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" creating a directory";
        LOG_ERROR(FileSystem_impl, eout.str())
        throw CF::FileException (CF::CF_EEXIST, eout.str().c_str());
    } catch ( ... ) {
        LOG_ERROR(FileSystem_impl, "Directory creation failed");
        throw CF::FileException (CF::CF_EEXIST, "Directory creation failed");
    }

    TRACE_EXIT(FileSystem_impl);
}

void FileSystem_impl::removeDirectory(const fs::path& dirPath, bool doRemove)
{
    TRACE_ENTER(FileSystem_impl);

    try {
        fs::directory_iterator end_itr; // past the end
        for (fs::directory_iterator itr(dirPath); itr != end_itr; ++itr) {
            if (fs::is_directory(*itr))
                { removeDirectory(*itr, doRemove); }
            else {
                LOG_ERROR(FileSystem_impl, "Directory not empty in rmdir.");
                throw CF::FileException();
            }
        }

        if(doRemove)
            { fs::remove(dirPath); }
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while removing a directory";
        LOG_ERROR(FileSystem_impl, eout.str())
        throw CF::FileException (CF::CF_EEXIST, eout.str().c_str());
    } catch ( ... ) {
        LOG_ERROR(FileSystem_impl, "Directory removal failed");
        throw CF::FileException (CF::CF_EEXIST, "Directory removal failed");
    }

    TRACE_EXIT(FileSystem_impl);
}

void FileSystem_impl::rmdir (const char* directoryName) throw (CORBA::SystemException, CF::FileException, CF::InvalidFileName)
{
    TRACE_ENTER(FileSystem_impl);

    if (!ossie::isValidFileName(directoryName)) {
        LOG_ERROR(FileSystem_impl, "rmdir passed bad directory name, throwing exception.");
        throw CF::InvalidFileName (CF::CF_EINVAL, "[FileSystem::rmdir] Invalid directory name");
    }

    try {
        fs::path dirPath(root / directoryName);

        if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
            LOG_ERROR(FileSystem_impl, "rmdir passed non_existant name or name is not a directory, throwing exception.");
            throw CF::InvalidFileName (CF::CF_EINVAL, "[FileSystem::rmdir] Invalid directory name");
        }

        // See the JTAP test for rmdir to understand this
        removeDirectory(dirPath, false); // Test for only empty directories
        removeDirectory(dirPath, true);  // Only empty directories, remove them all
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while calling rmdir";
        LOG_ERROR(FileSystem_impl, eout.str())
        throw CF::InvalidFileName (CF::CF_EINVAL, eout.str().c_str());
    } catch ( ... ) {
        LOG_ERROR(FileSystem_impl, "rmdir throwing exception.");
        throw CF::InvalidFileName (CF::CF_EINVAL, "[FileSystem::rmdir] error");
    }

    TRACE_EXIT(FileSystem_impl);
}


void FileSystem_impl::query (CF::Properties& fileSysProperties) throw (CORBA::SystemException, CF::FileSystem::UnknownFileSystemProperties)
{
    TRACE_ENTER(FileSystem_impl);
#if 0  ///\todo Implement query operations
    bool check;

    for (unsigned int i = 0; i < fileSysProperties.length (); i++) {
        check = false;
        if (strcmp (fileSysProperties[i].id, CF::FileSystem::SIZE) == 0) {
            struct stat fileStat;
            stat (root, &fileStat);
//      fileSysProperties[i].value <<= fileStat.st_size;  /// \bug FIXME
            check = true;
        }
        if (strcmp (fileSysProperties[i].id,
                    CF::FileSystem::AVAILABLE_SIZE) == 0) {
//to complete
        }
        if (!check)
            { throw CF::FileSystem::UnknownFileSystemProperties (); }
    }
#endif
    TRACE_EXIT(FileSystem_impl);
}

std::string FileSystem_impl::getLocalPath (const char* fileName)
{
    TRACE_ENTER(FileSystem_impl);

    fs::path fname(root / fileName);

    LOG_TRACE(FileSystem_impl, "Check for file " << fname.string());

    int fsOpSuccessAttempts = 0;
    bool fsOpSuccess = false;
    std::string retVal = "";
    while (!fsOpSuccess) {
        try {
            if (fs::exists(fname))
                { retVal = fname.string(); }
            else
                { retVal = ""; }
            fsOpSuccess = true;
        } catch ( const fs::filesystem_error& ex ) {
            LOG_WARN(FileSystem_impl, "Error in filesystem: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( std::exception& ex ) {
            LOG_WARN(FileSystem_impl, "The following standard exception occurred: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( ... ) {
            LOG_WARN(FileSystem_impl, "Caught an unhandled file system exception. Attempting again")
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        }
    }
    if (!fsOpSuccess) {
        LOG_ERROR(FileSystem_impl, "getLocalPath failed");
        throw CF::FileException (CF::CF_EEXIST, "getLocalPath failed");
    }

    return retVal;

    TRACE_EXIT(FileSystem_impl);

}

///\todo Implement File object reference clean up.
