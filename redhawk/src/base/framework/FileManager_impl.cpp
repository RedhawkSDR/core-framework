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


#include <iostream>
#include <string>
#include <algorithm>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/exception.hpp>

#ifndef BOOST_VERSION
#include <boost/version.hpp>
#endif

#if BOOST_VERSION < 103499
#  include <boost/filesystem/cerrno.hpp>
#else
#  include <boost/cerrno.hpp>
#endif

namespace fs = boost::filesystem;

#if BOOST_FILESYSTEM_VERSION >= 3
#include <boost/filesystem/operations.hpp>
#endif

#include "ossie/FileManager_impl.h"
#include "ossie/debug.h"
#include "ossie/CorbaUtils.h"

#include <boost/regex.hpp>

//#include <sys/types.h>
//#include <regex.h>

PREPARE_LOGGING(FileManager_impl);

FileManager_impl::FileManager_impl (const char* _fsroot): FileSystem_impl (_fsroot)
{
    TRACE_ENTER(FileManager_impl)

    int fsOpSuccessAttempts = 0;
    bool fsOpSuccess = false;
    while (!fsOpSuccess) {
        try {
#if BOOST_FILESYSTEM_VERSION < 3
            if (fs::path::default_name_check_writable())
                { fs::path::default_name_check(boost::filesystem::portable_posix_name); }
#endif
            numMounts = 0;
            mount_table = new CF::FileManager::MountSequence(5);
            fsOpSuccess = true;
        } catch ( const fs::filesystem_error& ex ) {
            LOG_WARN(FileManager_impl, "Error in filesystem: "<<ex.what()<<". Attempting again")
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( std::exception& ex ) {
            std::ostringstream eout;
            eout << "The following standard exception occurred: "<<ex.what()<<" when creating the File Manager. Attempting again";
            LOG_WARN(FileManager_impl, eout.str())
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( CORBA::Exception& ex ) {
            std::ostringstream eout;
            eout << "The following CORBA exception occurred: "<<ex._name()<<" when creating the File Manager. Attempting again";
            LOG_WARN(FileManager_impl, eout.str())
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        } catch ( ... ) {
            LOG_WARN(FileManager_impl, "Caught an unhandled file system exception. Attempting again")
            fsOpSuccessAttempts++;
            if (fsOpSuccessAttempts == 10)
                { break; }
            usleep(10000);
        }
    }
    if (!fsOpSuccess) {
        LOG_ERROR(FileManager_impl, "Unable to get access to initial path");
        throw CF::FileException (CF::CF_EEXIST, "Unable to get access to initial path");
    }

    TRACE_EXIT(FileManager_impl)
}

FileManager_impl::~FileManager_impl()
{
    TRACE_ENTER(FileManager_impl)
}

void
FileManager_impl::mount (const char* mountPoint,
                         CF::FileSystem_ptr _fileSystem)
throw (CORBA::SystemException, CF::InvalidFileName,
       CF::FileManager::InvalidFileSystem,
       CF::FileManager::MountPointAlreadyExists)
{
    TRACE_ENTER(FileManager_impl)
    LOG_TRACE(FileManager_impl, "Entering mount with " << mountPoint)

    if (CORBA::is_nil (_fileSystem))
        { throw CF::FileManager::InvalidFileSystem (); }

    CF::FileManager::MountType _mt;

    for (unsigned int i = 0; i < mount_table->length(); i++) {
        if (strcmp(mountPoint, mount_table[i].mountPoint) == 0)
            { throw CF::FileManager::MountPointAlreadyExists (); }
    }

    numMounts++;
    mount_table->length(numMounts);

    mount_table[numMounts-1].mountPoint = CORBA::string_dup(mountPoint);
    mount_table[numMounts-1].fs = CF::FileSystem::_duplicate(_fileSystem);


    TRACE_EXIT(FileManager_impl)
}


void
FileManager_impl::unmount (const char* mountPoint)
throw (CORBA::SystemException, CF::FileManager::NonExistentMount)
{
    TRACE_ENTER(FileManager_impl)
    LOG_TRACE(FileManager_impl, "Entering unmount with " << mountPoint)

    for (unsigned int i = 0; i < numMounts; i++) {
        if (strcmp (mount_table[i].mountPoint, mountPoint) == 0) {
            LOG_TRACE(FileManager_impl, "Found mount point to delete.")
            for (unsigned int j = i; j < mount_table->length() - 1; ++j) ///\todo this leaks FileSystems etc (check)
                { mount_table[j] = mount_table[j+1]; }

            mount_table->length(mount_table->length() - 1);
            numMounts--;
            TRACE_EXIT(FileManager_impl)
            return;
        }
    }

    LOG_ERROR(FileManager_impl, "Throwing CF::FileManager::NonExistentMount from FileManger unmount.")
    throw CF::FileManager::NonExistentMount ();
}


void
FileManager_impl::remove (const char* fileName)
throw (CORBA::SystemException, CF::FileException, CF::InvalidFileName)
{
    TRACE_ENTER(FileManager_impl)
    LOG_TRACE(FileManager_impl, "Entering remove with " << fileName)

    if (fileName[0] != '/' || !ossie::isValidFileName(fileName)) {
        LOG_ERROR(FileManager_impl, "remove passed bad filename, throwing exception.");
        throw CF::InvalidFileName (CF::CF_EINVAL, "[FileManager::remove] Invalid file name");
    }

    long fileFS(0);
    std::string filePath;
    CF::File_var file_var;

    // see if file is on one of the mounted file systems
    if (getFSandFSPath(fileName, fileFS, filePath)) {
        LOG_TRACE(FileManager_impl, "[FileManager::remove] found mountPoint " << mount_table[fileFS].mountPoint << " and localPath " << filePath);

        if (!mount_table[fileFS].fs->exists(filePath.c_str())) {
            throw CF::FileException(CF::CF_NOTSET, "[FileManager::remove] File does not exist on requested File System");
        }

        try {
            mount_table[fileFS].fs->remove(filePath.c_str());
        } catch ( std::exception& ex ) {
            std::ostringstream eout;
            eout << "The following standard exception occurred: "<<ex.what()<<" While opening file " << fileName;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch ( CF::FileException& ex ) {
            throw;
        } catch ( CORBA::Exception& ex ) {
            std::ostringstream eout;
            eout << "The following CORBA exception occurred: "<<ex._name()<<" While opening file " << fileName;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch( ... ) {
            LOG_ERROR(FileManager_impl, "[FileManager::remove] While opening file " << fileName << ": Unknown Exception\n");
            throw(CF::FileException());
        }
    }
    // if not on a mounted file system, see if it's in the local file system
    else if (FileSystem_impl::exists(fileName)) {
        LOG_TRACE(FileManager_impl, "[FileManager::remove] Couldn't find file on mountPoint - removing " << fileName << " from local filesystem");
        FileSystem_impl::remove(fileName);
    }
    // file can't be found
    else {
        throw CF::FileException(CF::CF_NOTSET, "[FileManager::remove] File does not exist on any mounted File System");
    }

    TRACE_EXIT(FileManager_impl)
}


void
FileManager_impl::copy (const char* sourceFileName,
                        const char* destinationFileName)
throw (CORBA::SystemException, CF::InvalidFileName, CF::FileException)
{
    LOG_TRACE(FileManager_impl, "[FileManager::copy] entering with " << sourceFileName << " to " << destinationFileName)

    if (!ossie::isValidFileName(sourceFileName) || !ossie::isValidFileName(destinationFileName)) {
        LOG_ERROR(FileManager_impl, "[FileManager::copy] passed bad filename, throwing exception.");
        throw CF::InvalidFileName (CF::CF_EINVAL, "[FileManager::copy] Invalid file name");
    }

    long srcFS(0), dstFS(0);
    bool srcMountedFlag, dstMountedFlag;
    std::string srcPath;
    std::string dstPath;

    srcMountedFlag = getFSandFSPath(sourceFileName, srcFS, srcPath);
    dstMountedFlag = getFSandFSPath(destinationFileName, dstFS, dstPath);

    // make sure source file exists
    if (srcMountedFlag) {
        if (!mount_table[srcFS].fs->exists (srcPath.c_str())) {
            LOG_ERROR(FileManager_impl, "[FileManager::copy] Throwing exception from copy because source file does not exist.");
            throw CF::InvalidFileName (CF::CF_EINVAL, "[FileSystem::Copy] Invalid file name");
        }
    } else {
        if (!FileSystem_impl::exists(sourceFileName)) {
            LOG_ERROR(FileManager_impl, "[FileManager::copy] Throwing exception from copy because source file does not exist.");
            throw CF::InvalidFileName (CF::CF_EINVAL, "[FileSystem::Copy] Invalid file name");
        }
    }


    //-----------------------------
    // Check within one FileSystem
    //-----------------------------
    if (srcMountedFlag == dstMountedFlag) {
        // both files are on local file system
        if (srcMountedFlag == false) {
            LOG_TRACE(FileManager_impl, "[FileManager::copy] \"fs->copy\" " << sourceFileName << " to " << destinationFileName << " locally on FileManager's filesystem");
            try {
                FileSystem_impl::copy(sourceFileName, destinationFileName);
            } catch ( std::exception& ex ) {
                std::ostringstream eout;
                eout << "The following standard exception occurred: "<<ex.what()<<" While \"fs->copy\" " << sourceFileName << " to " << destinationFileName;
                LOG_ERROR(FileManager_impl, eout.str())
                throw(CF::FileException());
            } catch ( CF::FileException& ex ) {
                throw;
            } catch ( CORBA::Exception& ex ) {
                std::ostringstream eout;
                eout << "The following CORBA exception occurred: "<<ex._name()<<" While \"fs->copy\" " << sourceFileName << " to " << destinationFileName;
                LOG_ERROR(FileManager_impl, eout.str())
                throw(CF::FileException());
            } catch( ... ) {
                LOG_ERROR(FileManager_impl, "[FileManager::copy] \"fs->copy\" " << sourceFileName << " to " << destinationFileName << " failed with Unknown Exception\n");
                throw(CF::FileException());
            }
            return;
        }
        // if both files are on the same mounted filesystem
        else if (srcFS == dstFS) {
            LOG_TRACE(FileManager_impl, "[FileManager::copy] \"fs->copy\" " << sourceFileName << " to " << destinationFileName << "  locally on mounted filesystem");
            try {
                mount_table[srcFS].fs->copy(srcPath.c_str(), dstPath.c_str());
            } catch ( std::exception& ex ) {
                std::ostringstream eout;
                eout << "The following standard exception occurred: "<<ex.what()<<" While \"fs->copy\" " << sourceFileName << " to " << destinationFileName;
                LOG_ERROR(FileManager_impl, eout.str())
                throw(CF::FileException());
            } catch ( CF::FileException& ex ) {
                throw;
            } catch ( CORBA::Exception& ex ) {
                std::ostringstream eout;
                eout << "The following CORBA exception occurred: "<<ex._name()<<" While \"fs->copy\" " << sourceFileName << " to " << destinationFileName;
                LOG_ERROR(FileManager_impl, eout.str())
                throw(CF::FileException());
            } catch( ... ) {
                LOG_ERROR(FileManager_impl, "[FileManager::copy] \"fs->copy\" " << srcPath << " to " << dstPath << " failed with Unknown Exception\n");
                throw(CF::FileException());
            }
            return;
        }
    }

    //-----------------------------
    // Copy file across FileSystems
    //-----------------------------

    CF::File_var srcFile;
    CF::File_var dstFile;

    //open source file
    if (srcMountedFlag) {
        try {
            srcFile = mount_table[srcFS].fs->open(srcPath.c_str(), true);
        } catch ( std::exception& ex ) {
            std::ostringstream eout;
            eout << "The following standard exception occurred: "<<ex.what()<<" While opening file " << srcPath;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch ( CF::FileException& ex ) {
            throw;
        } catch ( CORBA::Exception& ex ) {
            std::ostringstream eout;
            eout << "The following CORBA exception occurred: "<<ex._name()<<" While opening file " << srcPath;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch( ... ) {
            LOG_ERROR(FileManager_impl, "[FileManager::copy] While opening file " << srcPath << ": Unknown Exception\n");
            throw(CF::FileException());
        }
    } else {
        try {
            srcFile = FileSystem_impl::open(sourceFileName, true);
        } catch ( std::exception& ex ) {
            std::ostringstream eout;
            eout << "The following standard exception occurred: "<<ex.what()<<" While opening file " << srcPath;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch ( CF::FileException& ex ) {
            throw;
        } catch ( CORBA::Exception& ex ) {
            std::ostringstream eout;
            eout << "The following CORBA exception occurred: "<<ex._name()<<" While opening file " << srcPath;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch( ... ) {
            LOG_ERROR(FileManager_impl, "[FileManager::copy] While opening file " << sourceFileName << ": Unknown Exception\n");
            throw(CF::FileException());
        }
    }

    // ensure destination file is available and ready for writing
    if (dstMountedFlag) {   // destination is a mounted filesystem
        //remove file if it already exists
        if (mount_table[dstFS].fs->exists(dstPath.c_str())) {
            try {
                mount_table[dstFS].fs->remove(dstPath.c_str());
            } catch ( std::exception& ex ) {
                std::ostringstream eout;
                eout << "The following standard exception occurred: "<<ex.what()<<" While \"fs->remove\" " << dstPath;
                LOG_ERROR(FileManager_impl, eout.str())
                throw(CF::FileException());
            } catch ( CF::FileException& ex ) {
                throw;
            } catch ( CORBA::Exception& ex ) {
                std::ostringstream eout;
                eout << "The following CORBA exception occurred: "<<ex._name()<<" While \"fs->remove\" " << dstPath;
                LOG_ERROR(FileManager_impl, eout.str())
                throw(CF::FileException());
            } catch( ... ) {
                LOG_ERROR(FileManager_impl, "[FileManager::copy] \"fs->remove\" " << dstPath << " failed with Unknown Exception\n");
                throw(CF::FileException());
            }
        }
        // create new file
        try {
            dstFile = mount_table[dstFS].fs->create(dstPath.c_str());
        } catch ( std::exception& ex ) {
            std::ostringstream eout;
            eout << "The following standard exception occurred: "<<ex.what()<<" While \"fs->create\" " << dstPath;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch ( CF::FileException& ex ) {
            throw;
        } catch ( CORBA::Exception& ex ) {
            std::ostringstream eout;
            eout << "The following CORBA exception occurred: "<<ex._name()<<" While \"fs->create\" " << dstPath;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch( ... ) {
            LOG_ERROR(FileManager_impl, "[FileManager::copy] \"fs->create\" " << dstPath << " failed with Unknown Exception\n");
            throw(CF::FileException());
        }
    } else { // destination is on the local filesystem
        // remove the file if it already exists
        if (FileSystem_impl::exists(destinationFileName)) {
            try {
                FileSystem_impl::remove(destinationFileName);
            } catch ( std::exception& ex ) {
                std::ostringstream eout;
                eout << "The following standard exception occurred: "<<ex.what()<<" While \"fs->remove\" " << destinationFileName;
                LOG_ERROR(FileManager_impl, eout.str())
                throw(CF::FileException());
            } catch ( CF::FileException& ex ) {
                throw;
            } catch ( CORBA::Exception& ex ) {
                std::ostringstream eout;
                eout << "The following CORBA exception occurred: "<<ex._name()<<" While \"fs->remove\" " << destinationFileName;
                LOG_ERROR(FileManager_impl, eout.str())
                throw(CF::FileException());
            } catch( ... ) {
                LOG_ERROR(FileManager_impl, "[FileManager::copy] \"fs->remove\" " << destinationFileName << " failed with Unknown Exception\n");
                throw(CF::FileException());
            }
        }
        // create the file
        try {
            dstFile = FileSystem_impl::create(destinationFileName);
        } catch ( std::exception& ex ) {
            std::ostringstream eout;
            eout << "The following standard exception occurred: "<<ex.what()<<" While \"fs->create\" " << destinationFileName;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch ( CF::FileException& ex ) {
            throw;
        } catch ( CORBA::Exception& ex ) {
            std::ostringstream eout;
            eout << "The following CORBA exception occurred: "<<ex._name()<<" While \"fs->create\" " << destinationFileName;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch( ... ) {
            LOG_ERROR(FileManager_impl, "[FileManager::copy] \"fs->create\" " << destinationFileName << " failed with Unknown Exception\n");
            throw(CF::FileException());
        }
    }

    LOG_TRACE(FileManager_impl, "[FileManager::copy] \"fs->copy\" " << sourceFileName << " to " << destinationFileName << "  remotely between filesystems");

    // read the data
    CF::OctetSequence_var data;
    CORBA::ULong bytes = srcFile->sizeOf();
    bool  fe = false;
    std::ostringstream eout;
    try {
      while (bytes > 0) {
        // Read the file data in 1MB chunks. If omniORB uses the GIOP protocol to talk to the source filesystem
        // (i.e. it is in a different ORB), reads of more than about 2MB will exceed the maximum GIOP packet
        // size and raise a MARSHAL exception.
        const CORBA::ULong DEFAULT_CHUNK_SIZE = 1*1024*1024;
        CORBA::ULong chunkSize = std::min(bytes, DEFAULT_CHUNK_SIZE);
        bytes -= chunkSize;
        try {
            srcFile->read(data, chunkSize);
        } catch ( std::exception& ex ) {
          eout << "The following standard exception occurred: "<<ex.what()<<" While \"srcFile->read\"";
          throw(CF::FileException());
        } catch ( CF::File::IOException &ex ) {
          eout << "File Exception occured,  While \"srcFile->read\"";
          throw;
        } catch ( CF::FileException &ex ) {
          eout << "File Exception occured,  While \"srcFile->read\"";
          throw;
        } catch ( CORBA::Exception& ex ) {
          eout << "The following CORBA exception occurred: "<<ex._name()<<" While \"srcFile->read\"";
          throw(CF::FileException());
        } catch( ... ) {
          eout << "[FileManager::copy] \"srcFile->read\" failed with Unknown Exception\n";
          throw(CF::FileException());
        }


        // write the data
        try {
            dstFile->write(data);
        } catch ( std::exception& ex ) {
            eout << "The following standard exception occurred: "<<ex.what()<<" While \"dstFile->write\"";
            throw(CF::FileException());
        } catch ( CF::File::IOException &ex ) {
          eout << "File Exception occured,  While \"dstFile->write\"";
          throw;
        } catch ( CF::FileException &ex ) {
          eout << "File Exception occured,  While \"dstFile->write\"";
          throw;
        } catch ( CORBA::Exception& ex ) {
            eout << "The following CORBA exception occurred: "<<ex._name()<<" While \"dstFile->write\"";
            throw(CF::FileException());
        } catch( ... ) {
          eout << "[FileManager::copy] \"dstFile->write\" failed with Unknown Exception\n";
          throw(CF::FileException());
        }
      }
    } catch(...) {
      LOG_ERROR(FileManager_impl, eout.str());
      fe = true;
    }

    
    try {
      // close the files
      try {
        srcFile->close();
      } catch ( std::exception& ex ) {
        eout << "The following standard exception occurred: "<<ex.what()<<" While \"srcFile->close\"";
        throw(CF::FileException());
      } catch ( CF::FileException& ex ) {
        throw;
      } catch ( CORBA::Exception& ex ) {
        eout << "The following CORBA exception occurred: "<<ex._name()<<" While \"srcFile->close\"";
        throw(CF::FileException());
      } catch( ... ) {
        eout << "[FileManager::copy] \"srcFile->close\" failed with Unknown Exception\n";
        throw(CF::FileException());
      }
    }catch(...) {
      LOG_ERROR(FileManager_impl, eout.str());
      fe = true;
    }
    

    try {
      try {
        dstFile->close();
      } catch ( std::exception& ex ) {
        eout << "The following standard exception occurred: "<<ex.what()<<" While \"dstFile->close\"";
      } catch ( CF::FileException& ex ) {
        throw;
      } catch ( CORBA::Exception& ex ) {
        eout << "The following CORBA exception occurred: "<<ex._name()<<" While \"dstFile->close\"";
        throw(CF::FileException());
      } catch( ... ) {
        eout << "[FileManager::copy] \"dstFile->close\" failed with Unknown Exception\n";
        throw(CF::FileException());
      }
    } catch(...) {
      LOG_ERROR(FileManager_impl, eout.str());
      fe = true;
    }

    if ( fe ) {
      throw(CF::FileException());
    }
    
}


CORBA::Boolean FileManager_impl::exists (const char* fileName)
throw (CORBA::SystemException, CF::InvalidFileName)
{
    LOG_TRACE(FileManager_impl, "[FileManager::exists] Entering exists with " << fileName)

    if (!ossie::isValidFileName(fileName)) {
        LOG_ERROR(FileManager_impl, "[FileManager::exists] passed bad filename, throwing exception.");
        throw CF::InvalidFileName (CF::CF_EINVAL, "[FileManager::exists] Invalid file name");
    }

    LOG_TRACE(FileManager_impl, "[FileManager::exists] running FS.exists for file " << fileName)

    //test code
    long fileFS(0);
    std::string filePath;

    // see if file is on one of the mounted file systems
    if (getFSandFSPath(fileName, fileFS, filePath)) {
        LOG_TRACE(FileManager_impl, "[FileManager::exists] found mountPoint " << mount_table[fileFS].mountPoint << " and localPath " << filePath);

        if (mount_table[fileFS].fs->exists(filePath.c_str())) {
            return true;
        } else {
            return false;
        }
    } else {
        return FileSystem_impl::exists(fileName);
    }

}


CF::FileSystem::FileInformationSequence *
FileManager_impl::list (const char* pattern) throw (CORBA::SystemException,
                                                    CF::FileException,
                                                    CF::InvalidFileName)
{
    LOG_TRACE(FileManager_impl, "[FileManager::list] entering with " << pattern);
    
    std::string new_pattern;

    if ((pattern[0] != '/') && (strlen(pattern) > 0)) {
        if (pattern[0] == '?') {
            new_pattern="/";
            new_pattern.append(&pattern[1]);
        } else if (pattern[0] == '*') {
            new_pattern="/";
            new_pattern.append(pattern);
        } else {
            throw CF::InvalidFileName(CF::CF_EINVAL, "[FileManager::list] Relative path given.");
        }
    } else {
        new_pattern = pattern;
    }

    long fileFS(0);
    std::string filePath;
    std::string searchPattern = new_pattern.c_str();

    CF::FileSystem::FileInformationSequence_var fis;

    // see if file is on one of the mounted file systems
    if (getFSandFSPath(new_pattern.c_str(), fileFS, filePath)) {
        LOG_TRACE(FileManager_impl, "[FileManager::list] found mountPoint " << ossie::corba::returnString(mount_table[fileFS].mountPoint) << " and localPath " << filePath);

        std::string::size_type nextSlash = searchPattern.find("/", 1);   // see if there are any slashes after the initial one
        if (nextSlash == std::string::npos) {   // there's no additional slashes
            fis = new CF::FileSystem::FileInformationSequence;
            fis->length(0);
            regularExpressionMountSearch(new_pattern, fis);
        } else {  // look into the contents of the mount point only if it has a trailing slash
            try {
                fis = mount_table[fileFS].fs->list(filePath.c_str());
            } catch ( std::exception& ex ) {
                std::ostringstream eout;
                eout << "The following standard exception occurred: "<<ex.what()<<" While listing " << new_pattern;
                LOG_ERROR(FileManager_impl, eout.str())
                throw(CF::FileException());
            } catch ( CF::FileException& ex ) {
                throw;
            } catch ( CORBA::Exception& ex ) {
                std::ostringstream eout;
                eout << "The following CORBA exception occurred: "<<ex._name()<<" While listing " << new_pattern;
                LOG_ERROR(FileManager_impl, eout.str())
                throw(CF::FileException());
            } catch( ... ) {
                LOG_ERROR(FileManager_impl, "[FileManager::list] While listing " << new_pattern.c_str() << ": Unknown Exception\n");
                throw(CF::FileException());
            }
        }
    }
    // if not on a mounted file system, see if it's in the local file system
    else {
        LOG_TRACE(FileManager_impl, "[FileManager::list] couldn't find file on mountPoint - listing local filesystem");
        try {
            fis = FileSystem_impl::list(new_pattern.c_str());
        } catch ( std::exception& ex ) {
            std::ostringstream eout;
            eout << "The following standard exception occurred: "<<ex.what()<<" While listing " << new_pattern;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch ( CF::FileException& ex ) {
            throw;
        } catch ( CORBA::Exception& ex ) {
            std::ostringstream eout;
            eout << "The following CORBA exception occurred: "<<ex._name()<<" While listing " << new_pattern;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch( ... ) {
            LOG_ERROR(FileManager_impl, "[FileManager::list] While listing " << new_pattern.c_str() << ": Unknown Exception\n");
            throw(CF::FileException());
        }
        // only the FileManager can mount file systems, so the search for mount points applies only here
        //  also assume that a single level of mounting is possible
        if (searchPattern.size() > 1) {
            std::string::size_type nextSlash = searchPattern.find("/", 1);   // see if there are any slashes after the initial one
            if (nextSlash == std::string::npos) {   // there's no additional slashes
                regularExpressionMountSearch(new_pattern, fis);
            } else if ((nextSlash == (searchPattern.size()-1))) {  // it's a trailing slash (list the mount's contents)
                // this was done in getFSandFSPath
            }
        } else if (strncmp(pattern, "/", 1) == 0) { // the search string is /
            for (unsigned int mount = 0; mount<mount_table->length(); mount++) {
                std::string tmpMount = ossie::corba::returnString(mount_table[mount].mountPoint);
                fis->length(fis->length()+1);
                fis[fis->length()-1].name = CORBA::string_dup(&(tmpMount.c_str()[1])); // eliminate the preceding slash
                fis[fis->length()-1].kind = CF::FileSystem::FILE_SYSTEM;
                fis[fis->length()-1].size = 0;
                CF::Properties prop;
                prop.length(0);
                fis[fis->length()-1].fileProperties = prop;
            }
        }
    }

    return fis._retn();

}

void FileManager_impl::regularExpressionMountSearch(std::string new_pattern, CF::FileSystem::FileInformationSequence_var& fis)
{
    std::string initial_patternString = new_pattern.c_str();
    std::string::iterator pattern_iter = initial_patternString.begin();
    std::string patternString = "";
    while (pattern_iter != initial_patternString.end()) {
        const char value = *pattern_iter;
        if (value == '*') {
            patternString.append(".*");
        } else if (value == '?') {
            patternString.append(1, '.');
        } else {
            patternString.append(1, value);
        }
        pattern_iter++;
    }
    boost::regex expression(patternString);
    for (unsigned int mount = 0; mount<mount_table->length(); mount++) {
        std::string tmpMount = ossie::corba::returnString(mount_table[mount].mountPoint);
        bool match = boost::regex_match(tmpMount, expression);
        if (match) {
            fis->length(fis->length()+1);
            fis[fis->length()-1].name = CORBA::string_dup(&(tmpMount.c_str()[1])); // eliminate the preceding slash
            fis[fis->length()-1].kind = CF::FileSystem::FILE_SYSTEM;
            fis[fis->length()-1].size = 0;
            CF::Properties prop;
            prop.length(0);
            fis[fis->length()-1].fileProperties = prop;
        }
    }
}


CF::File_ptr FileManager_impl::create (const char* fileName) throw (CORBA::
                                                                    SystemException,
                                                                    CF::
                                                                    InvalidFileName,
                                                                    CF::
                                                                    FileException)
{
    LOG_TRACE(FileManager_impl, "Entering create with " << fileName)

    if (!ossie::isValidFileName(fileName)) {
        LOG_ERROR(FileManager_impl, "create passed bad filename, throwing exception.");
        throw CF::InvalidFileName (CF::CF_EINVAL, "[FileManager::create] Invalid file name");
    }

    long fileFS(0);
    std::string filePath;
    CF::File_var file_var;

    // see if location is on one of the mounted file systems
    if (getFSandFSPath(fileName, fileFS, filePath)) {
        LOG_TRACE(FileManager_impl, "Found mountPoint " << mount_table[fileFS].mountPoint << " and localPath " << filePath);

        // File should not already exist
        if (mount_table[fileFS].fs->exists(filePath.c_str())) {
            throw CF::FileException(CF::CF_NOTSET, "[FileManager::create] File already exists on requested File System");
        }

        try {
            file_var = mount_table[fileFS].fs->create(filePath.c_str());
        } catch ( std::exception& ex ) {
            std::ostringstream eout;
            eout << "The following standard exception occurred: "<<ex.what()<<" While creating file " << fileName;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch ( CF::FileException& ex ) {
            throw;
        } catch ( CORBA::Exception& ex ) {
            std::ostringstream eout;
            eout << "The following CORBA exception occurred: "<<ex._name()<<" While creating file " << fileName;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch( ... ) {
            LOG_ERROR(FileManager_impl, "[FileManager::open] While creating file " << fileName << ": Unknown Exception\n");
            throw(CF::FileException());
        }
    }
    // if location is not on a mounted file system, see if it's in the local file system
    else if (FileSystem_impl::exists(fileName)) {
        throw CF::FileException(CF::CF_NOTSET, "[FileManager::create] File already exists on the local File System");
    }
    // file can't be found
    else {
        LOG_TRACE(FileManager_impl, "couldn't find location on mountPoint - creating with local filesystem");
        return FileSystem_impl::create(fileName);
    }

    return file_var._retn();
}


CF::File_ptr
FileManager_impl::open (const char* fileName, CORBA::Boolean read_Only)
throw (CORBA::SystemException, CF::InvalidFileName, CF::FileException)
{
    LOG_TRACE(FileManager_impl, "Entering open with " << fileName)

    if (!ossie::isValidFileName(fileName)) {
        LOG_ERROR(FileManager_impl, "open passed bad filename, throwing exception.");
        throw CF::InvalidFileName (CF::CF_EINVAL, "[FileManager::open] Invalid file name");
    }

    long fileFS(0);
    std::string filePath;
    CF::File_var file_var;

    // see if file is on one of the mounted file systems
    if (getFSandFSPath(fileName, fileFS, filePath)) {
        LOG_TRACE(FileManager_impl, "Found mountPoint " << mount_table[fileFS].mountPoint << " and localPath " << filePath);

        if (!mount_table[fileFS].fs->exists(filePath.c_str())) {
            throw CF::FileException(CF::CF_NOTSET, "[FileManager::open] File does not exist on requested File System");
        }

        try {
            file_var = mount_table[fileFS].fs->open (filePath.c_str(), read_Only);
        } catch ( std::exception& ex ) {
            std::ostringstream eout;
            eout << "The following standard exception occurred: "<<ex.what()<<" While opening file " << fileName;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch ( CF::FileException& ex ) {
            throw;
        } catch ( CORBA::Exception& ex ) {
            std::ostringstream eout;
            eout << "The following CORBA exception occurred: "<<ex._name()<<" While opening file " << fileName;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch( ... ) {
            LOG_ERROR(FileManager_impl, "[FileManager::open] While opening file " << fileName << ": Unknown Exception\n");
            throw(CF::FileException());
        }
    }
    // if not on a mounted file system, see if it's in the local file system
    else if (FileSystem_impl::exists(fileName)) {
        LOG_TRACE(FileManager_impl, "couldn't find file on mountPoint - opening with local filesystem");
        file_var = FileSystem_impl::open(fileName, read_Only);
    }
    // file can't be found
    else {
        LOG_ERROR(FileManager_impl, "File does not exist on any mounted File System");
        throw CF::FileException(CF::CF_NOTSET, "[FileManager::open] File does not exist on any mounted File System");
    }

    return file_var._retn();
}


void
FileManager_impl::mkdir (const char* directoryName)
throw (CORBA::SystemException, CF::FileException, CF::InvalidFileName)
{
    LOG_TRACE(FileManager_impl, "[FileManager::mkdir] Entering with " << directoryName)

    if (!ossie::isValidFileName(directoryName)) {
        LOG_ERROR(FileManager_impl, "[FileManager::mkdir] passed bad filename, throwing exception.");
        throw CF::InvalidFileName (CF::CF_EINVAL, "[FileManager::mkdir] Invalid directory name");
    }

    long fileFS(0);
    std::string filePath;

    // see if location is on one of the mounted file systems
    if (getFSandFSPath(directoryName, fileFS, filePath)) {
        LOG_TRACE(FileManager_impl, "[FileManager::mkdir] Found mountPoint " << mount_table[fileFS].mountPoint << " and localPath " << filePath);

        // Directory should not already exist
        if (mount_table[fileFS].fs->exists(filePath.c_str())) {
            throw CF::FileException(CF::CF_NOTSET, "[FileManager::mkdir] Directory already exists on requested File System");
        }

        try {
            mount_table[fileFS].fs->mkdir(filePath.c_str());
        } catch ( std::exception& ex ) {
            std::ostringstream eout;
            eout << "The following standard exception occurred: "<<ex.what()<<" While making directory " << directoryName;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch ( CF::FileException& ex ) {
            throw;
        } catch ( CORBA::Exception& ex ) {
            std::ostringstream eout;
            eout << "The following CORBA exception occurred: "<<ex._name()<<" While making directory " << directoryName;
            LOG_ERROR(FileManager_impl, eout.str())
            throw(CF::FileException());
        } catch( ... ) {
            LOG_ERROR(FileManager_impl, "[FileManager::mkdir] While making directory " << directoryName << ": Unknown Exception\n");
            throw(CF::FileException());
        }
    }
    // if location is not on a mounted file system, see if it's in the local file system
    else if (FileSystem_impl::exists(directoryName)) {
        LOG_ERROR(FileManager_impl, "Directory already exists on the local File System")
        throw CF::FileException(CF::CF_NOTSET, "[FileManager::mkdir] Directory already exists on the local File System");
    }
    // directory doesn't exist (that's a good thing)
    else {
        LOG_TRACE(FileManager_impl, "[FileManager::mkdir] couldn't find location on mountPoint - making on local filesystem");
        FileSystem_impl::mkdir(directoryName);
    }

    TRACE_EXIT(FileManager_impl)
}


void
FileManager_impl::rmdir (const char* directoryName)
throw (CORBA::SystemException, CF::FileException, CF::InvalidFileName)
{
    LOG_TRACE(FileManager_impl, "Entering rmdir with " << directoryName)

    if (!ossie::isValidFileName(directoryName)) {
        LOG_ERROR(FileManager_impl, "rmdir passed bad filename, throwing exception.");
        throw CF::InvalidFileName (CF::CF_EINVAL, "[FileManager::rmdir] Invalid directory name");
    }

    long fileFS(0);
    std::string filePath;

    // see if location is on one of the mounted file systems
    if (getFSandFSPath(directoryName, fileFS, filePath)) {
        LOG_TRACE(FileManager_impl, "[FileManager::rmdir] Found mountPoint " << mount_table[fileFS].mountPoint << " and localPath " << filePath);

        // Directory should already exist
        if (!mount_table[fileFS].fs->exists(filePath.c_str())) {
            throw CF::FileException(CF::CF_NOTSET, "[FileManager::rmdir] Directory does not exist on requested File System");
        }

        // Ask the mounted file system handle the call, letting any exceptions propagate through
        // (FileManager is a subclass of FileSystem, and inherits the behavior of rmdir, including
        // the exceptions).
        mount_table[fileFS].fs->rmdir(filePath.c_str());
    }
    // if location is not on a mounted file system, see if it's in the local file system
    else if (FileSystem_impl::exists(directoryName)) {
        FileSystem_impl::rmdir(directoryName);
    }
    // directory doesn't exist
    else {
        throw CF::FileException(CF::CF_NOTSET, "[FileManager::rmdir] Directory does not exist on the local File System");
    }

    TRACE_EXIT(FileManager_impl)
}


void
FileManager_impl::query (CF::Properties& fileSysProperties)
throw (CORBA::SystemException, CF::FileSystem::UnknownFileSystemProperties)
{
    TRACE_ENTER(FileManager_impl)

    bool check;

    for (unsigned int i = 0; i < fileSysProperties.length (); i++) {
        check = false;

        if (strcmp (fileSysProperties[i].id, CF::FileSystem::SIZE) == 0) {
            CORBA::Long totalSize, temp;
            totalSize = 0;

            for (unsigned int j = 0; j < mount_table->length(); j++) {
                CF::DataType dt;
                dt.id = CORBA::string_dup ("SIZE");
                CF::Properties pr (2, 1, &dt, 0);

                try {
                    mount_table[j].fs->query (pr);
                } catch( CORBA::SystemException& se ) {
                    LOG_ERROR(FileManager_impl, "fs->query failed with CORBA::SystemException");
                    throw;
                } catch ( std::exception& ex ) {
                    std::ostringstream eout;
                    eout << "The following standard exception occurred: "<<ex.what()<<" While fs->query";
                    LOG_ERROR(FileManager_impl, eout.str())
                    throw(CF::FileException());
                } catch ( CF::FileException& ex ) {
                    throw;
                } catch ( CORBA::Exception& ex ) {
                    std::ostringstream eout;
                    eout << "The following CORBA exception occurred: "<<ex._name()<<" While fs->query";
                    LOG_ERROR(FileManager_impl, eout.str())
                    throw(CF::FileException());
                } catch( ... ) {
                    LOG_ERROR(FileManager_impl, "fs->query failed with Unknown Exception");
                    throw;
                }

                CF::DataType* _dt = pr.get_buffer ();

                for (unsigned int k = 0; k < pr.length (); k++) {
                    _dt->value >>= temp;
                    totalSize = totalSize + temp;
                    _dt++;
                }

                fileSysProperties[i].value >>= temp;
                fileSysProperties[i].value <<= totalSize + temp;

                check = true;
            }
        }

        if (strcmp (fileSysProperties[i].id,
                    CF::FileSystem::AVAILABLE_SPACE) == 0) {
            CORBA::Long totalSize;
            totalSize = 0;

            for (unsigned int i = 0; i < mount_table->length(); i++) {
            }

            check = true;
        }

        if (!check)
            { throw CF::FileSystem::UnknownFileSystemProperties (); }

        ///\todo Add functionality to query ALL FileManager properties
    }
    TRACE_EXIT(FileManager_impl)
}


CF::FileManager::MountSequence*
FileManager_impl::getMounts ()throw (CORBA::SystemException)
{
    TRACE_ENTER(FileManager_impl)
    CF::FileManager::MountSequence_var result = new CF::FileManager::MountSequence(mount_table);
    return result._retn();
}

bool FileManager_impl::getFSandFSPath(const char* path, long& mountTableIndex, std::string& FSPath)
{
    LOG_TRACE(FileManager_impl, "Entering getFSandFSPath with path " << path)

    fs::path fullPath(path);
    fs::path::iterator fullItr(fullPath.begin());
    std::string tmpFSPath;

    unsigned int lastMatchLength(0);
    mountTableIndex = -1;

    for(unsigned int i(0); i < numMounts; ++i) {
        unsigned int matchLength = pathMatches(path, mount_table[i].mountPoint, tmpFSPath);
        if ( matchLength > lastMatchLength) {
            mountTableIndex = i;
            lastMatchLength = matchLength;
            FSPath = tmpFSPath;
        }
    }
    LOG_TRACE(FileManager_impl, "Found mountIndex " << mountTableIndex << " and local path " << FSPath << " lastMatchLength " << lastMatchLength)
    if (mountTableIndex < 0 ) {
        return false;
    }
    return true;

}

unsigned int FileManager_impl::pathMatches(const char* path, const char* mPoint, std::string& FSPath)
{
    LOG_TRACE(FileManager_impl, "Entering pathMatches with path " << path << " mount point " << mPoint)

    fs::path fullPath(path);
    fs::path::iterator fullItr(fullPath.begin());

    fs::path mPath(mPoint);
    fs::path::iterator mItr(mPath.begin());

    unsigned int commonElements(0);

    while (fullItr != fullPath.end()) {
        if (*fullItr != *mItr)
            { break; }

        // don't want to match "/" to "/"
        if ((commonElements > 0) || (*fullItr != "/")) {
            ++commonElements;
        }
        ++fullItr;
        ++mItr;
    }


    fs::path localPath("/");

    while (fullItr != fullPath.end()) {
        localPath /= *fullItr;
        ++fullItr;
    }
    FSPath = localPath.string();

    return commonElements;
}

