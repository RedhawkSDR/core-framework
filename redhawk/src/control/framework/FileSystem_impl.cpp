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

#include "ossie/FileSystem_impl.h"
#include "ossie/File_impl.h"
#include "ossie/CorbaUtils.h"
#include "ossie/ossieSupport.h"
#include "ossie/prop_helpers.h"
#include <ossie/PropertyMap.h>
#include <ossie/boost_compat.h>

namespace fs = boost::filesystem;


namespace {

#define RETRY_START \
    try { \
        int _RETRIES_ = 0; \
        while (true) { \
            try {

#define RETRY_END \
                break; \
            } catch (const fs::filesystem_error& ex) { \
                if (++_RETRIES_ >= retry_max) throw ex; \
                usleep(retry_delay); \
            } \
        } \
    } catch (const fs::filesystem_error& ex) { \
        throw CF::FileException(CF::CF_NOTSET, ex.what()); \
    } catch (const std::bad_alloc& ex) { \
        throw CF::FileException(CF::CF_ENOMEM, "Bad allocation"); \
    }

    class UnreliableFS {
    public:
        UnreliableFS (int max, int delay):
            retry_max(max),
            retry_delay(delay)
        {
        }

        UnreliableFS ():
            retry_max(10),
            retry_delay(10000)
        {
        }

        fs::directory_iterator begin (const fs::path& path)
        {
            RETRY_START;
            return fs::directory_iterator(path);
            RETRY_END;
        }

        fs::directory_iterator increment (fs::directory_iterator& itr)
        {
            RETRY_START;
            return ++itr;
            RETRY_END;
        }

        bool exists (const fs::path& path)
        {
            RETRY_START;
            return fs::exists(path);
            RETRY_END;
        }
    
        bool is_directory (const fs::path& path)
        {
            RETRY_START;
            return fs::is_directory(path);
            RETRY_END;
        }

        bool remove (const fs::path& path)
        {
            RETRY_START;
            return fs::remove(path);
            RETRY_END;
        }

        void copy_file (const fs::path& source, const fs::path& dest, BOOST_SCOPED_ENUM(fs::copy_option) option)
        {
            if (option == fs::copy_option::overwrite_if_exists) {
                // In older versions of boost, copy_file overwrites but does not truncate
                // the file. To work around this bug, remove the destination file (this
                // is a no-op if it doesn't exist).
                remove(dest);
            }
            RETRY_START;
            fs::copy_file(source, dest, option);
            RETRY_END;
        }

        bool create_directory (const fs::path& dirpath)
        {
            RETRY_START;
            return fs::create_directory(dirpath);
            RETRY_END;
        }

    private:
        int retry_max;
        int retry_delay;
    };

#undef RETRY_START
#undef RETRY_END

}


PREPARE_CF_LOGGING(FileSystem_impl)


FileSystem_impl::FileSystem_impl (const char* _root):
    root(_root)
{
}

FileSystem_impl::~FileSystem_impl ()
{
}

void FileSystem_impl::remove (const char* fileName) throw (CF::FileException, CF::InvalidFileName, CORBA::SystemException)
{
    boost::mutex::scoped_lock lock(interfaceAccess);
    
    fs::path fname(root / fileName);
    fs::path dirPath(fname.parent_path());
    UnreliableFS fsops;
    
    if (fileName[0] != '/') {
        throw CF::FileException(CF::CF_EEXIST, "Filename must be absolute");
    }
    
    if ((fname.string().find('?') != std::string::npos) or (fname.string().find('*') != std::string::npos)) {
        if ((dirPath.string().find('?') != std::string::npos) or (dirPath.string().find('*') != std::string::npos)) {
            throw CF::InvalidFileName(CF::CF_EINVAL, "Wildcards can only be applied after the rightmost path separator");
        }
    } else if (!fsops.exists(fname)) {
        throw CF::FileException(CF::CF_EEXIST, "File does not exist");
    }

    std::string searchPattern = BOOST_PATH_STRING(fname.filename());
    RH_TRACE(_fileSysLog, "Remove using search pattern " << searchPattern << " in " << dirPath);
    
    const fs::directory_iterator end_itr; // an end iterator (by boost definition)
    for (fs::directory_iterator itr = fsops.begin(dirPath); itr != end_itr; fsops.increment(itr)) {
        const std::string& filename = BOOST_PATH_STRING(itr->path().filename());
        if (fnmatch(searchPattern.c_str(), filename.c_str(), 0) == 0) {
            RH_TRACE(_fileSysLog, "Removing file " << itr->path().string());  
            if (!fsops.remove(itr->path())) {
                throw CF::FileException(CF::CF_EEXIST, "File does not exist");
            }
        }
    }
}

void FileSystem_impl::move (const char* sourceFileName, const char* destinationFileName) throw (CORBA::SystemException, CF::InvalidFileName, CF::FileException)
{
    boost::mutex::scoped_lock lock(interfaceAccess);

    // Validate file names
    if (sourceFileName[0] != '/' || !ossie::isValidFileName(sourceFileName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid source file name");
    } else if (destinationFileName[0] != '/' || !ossie::isValidFileName(destinationFileName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid destination file name");
    } else if (strcmp(sourceFileName, destinationFileName) == 0) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Destination file name is identical to source file name");
    }

    fs::path sourcePath(root / sourceFileName);
    fs::path destPath(root / destinationFileName);
    UnreliableFS fsops;

    // Ensure the source exists; if it's a directory, do nothing
    if (!fsops.exists(sourcePath)) {
        throw CF::FileException(CF::CF_ENOENT, "Source file does not exist");
    } else if (fsops.is_directory(sourcePath)) {
        return;
    }
    
    // Ensure that the destination directory exists
    if (!fsops.is_directory(destPath.parent_path())) {
        throw CF::FileException(CF::CF_ENOTDIR, "Destination directory does not exist");
    }

    // Perform the actual move; this works for directories as well as files.
    RH_TRACE(_fileSysLog, "Moving local file " << sourcePath << " to " << destPath);
    if (rename(sourcePath.string().c_str(), destPath.string().c_str())) {
        throw CF::FileException(CF::CF_EINVAL, "Unexpected failure in move");
    }
}

void FileSystem_impl::copy (const char* sourceFileName, const char* destinationFileName) throw (CORBA::SystemException, CF::InvalidFileName, CF::FileException)
{
    boost::mutex::scoped_lock lock(interfaceAccess);

    // Validate file names
    if (sourceFileName[0] != '/' || !ossie::isValidFileName(sourceFileName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid source file name");
    } else if (destinationFileName[0] != '/' || !ossie::isValidFileName(destinationFileName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid destination file name");
    } else if (strcmp(sourceFileName, destinationFileName) == 0) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Destination file name is identical to source file name");
    }

    fs::path sourcePath(root / sourceFileName);
    fs::path destPath(root / destinationFileName);
    UnreliableFS fsops;

    // Ensure the source exists; if it's a directory, do nothing
    if (!fsops.exists(sourcePath)) {
        throw CF::FileException(CF::CF_ENOENT, "Source file does not exist");
    } else if (fsops.is_directory(sourcePath)) {
        return;
    }
    
    // Ensure that the destination directory exists
    if (!fsops.is_directory(destPath.parent_path())) {
        throw CF::FileException(CF::CF_ENOTDIR, "Destination directory does not exist");
    }

    // Perform the copy.
    RH_TRACE(_fileSysLog, "Copying local file " << sourcePath << " to " << destPath);
    fsops.copy_file(sourcePath, destPath, fs::copy_option::overwrite_if_exists);
}

CORBA::Boolean FileSystem_impl::exists (const char* fileName)
throw (CORBA::SystemException, CF::InvalidFileName)
{
    boost::mutex::scoped_lock lock(interfaceAccess);

    RH_TRACE(_fileSysLog, "Checking for existence of SCA file " << fileName);
    if (fileName[0] != '/' || !ossie::isValidFileName(fileName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid file name");
    }
    bool status = _local_exists(fileName);

    return status;
}

bool FileSystem_impl::_local_exists (const char* fileName)
{
    fs::path fname(root / fileName);
    UnreliableFS fsops;
    RH_TRACE(_fileSysLog, "Checking for existence of local file " << fname.string());
    try {
        return fsops.exists(fname);
    } catch (const CF::FileException& exc) {
        // Convert the default CF::FileException to CF::InvalidFileName to
        // match the declared signature of FileSystem::exists()
        throw CF::InvalidFileName(exc.errorNumber, exc.msg);
    }
}

CF::FileSystem::FileInformationSequence* FileSystem_impl::list (const char* pattern) throw (CORBA::SystemException, CF::FileException, CF::InvalidFileName)
{
    fs::path filePath(root / pattern);
    fs::path dirPath(filePath.parent_path());
    UnreliableFS fsops;

    // Validate the input pattern and its path.
    if ((dirPath.string().find('?') != std::string::npos) || (dirPath.string().find('*') != std::string::npos)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Wildcards can only be applied after the rightmost path separator");
    } else if (!fsops.exists(dirPath)) {
        throw CF::FileException(CF::CF_EEXIST, "Path does not exist");
    } else if (!fsops.is_directory(dirPath)) {
        throw CF::FileException(CF::CF_ENOTDIR, "Path is not a directory");
    }

    std::string searchPattern = BOOST_PATH_STRING(filePath.filename());
    if ((searchPattern == ".") && (fsops.is_directory(filePath))) {
        searchPattern = "*";
    }
    RH_TRACE(_fileSysLog, "List using search pattern " << searchPattern << " in " << dirPath);

    CF::FileSystem::FileInformationSequence_var result = new CF::FileSystem::FileInformationSequence;

    const fs::directory_iterator end_itr; // an end iterator (by boost definition)
    for (fs::directory_iterator itr = fsops.begin(dirPath); itr != end_itr; fsops.increment(itr)) {
        const std::string filename = BOOST_PATH_STRING(itr->path().filename());
        if (fnmatch(searchPattern.c_str(), filename.c_str(), 0) == 0) {
            if ((filename.length() > 0) && (filename[0] == '.') && (filename != searchPattern)) {
                RH_TRACE(_fileSysLog, "Ignoring hidden match " << filename);
                continue;
            }
            RH_TRACE(_fileSysLog, "Match in list with " << filename);
            CORBA::ULong index = result->length();
            result->length(index + 1);
                
            // We need to specially handle the empty '' pattern
            if (strlen(pattern) == 0) {
                result[index].name = CORBA::string_dup("/");
            } else {
                result[index].name = CORBA::string_dup(filename.c_str());
            }
            if (fsops.is_directory(*itr)) {
                result[index].kind = CF::FileSystem::DIRECTORY;
                result[index].size = 0;
            } else {
                try {
                    result[index].kind = CF::FileSystem::PLAIN;
                    result[index].size = fs::file_size(*itr);
                } catch ( ... ) {
                    // this file is not good (i.e.: bad link)
                    result->length(index);
                    RH_WARN(_fileSysLog, "File cannot be evaluated, excluding from list: " << filename);
                    continue;
                }
            }

            const std::string localFilename = itr->path().string();
            bool readonly = access(localFilename.c_str(), W_OK);
            bool executable = !access(localFilename.c_str(), X_OK);
            CORBA::ULongLong modtime = fs::last_write_time(*itr);

            redhawk::PropertyMap& props = redhawk::PropertyMap::cast(result[index].fileProperties);
            props[CF::FileSystem::CREATED_TIME_ID] = modtime;
            props[CF::FileSystem::MODIFIED_TIME_ID] = modtime;
            props[CF::FileSystem::LAST_ACCESS_TIME_ID] = modtime;
            props["READ_ONLY"] = readonly;
            props["EXECUTABLE"] = executable;
            props["IOR_AVAILABLE"] = getFileIOR(localFilename);
        }
    }

    return result._retn();
}


CF::File_ptr FileSystem_impl::create (const char* fileName) throw (CORBA::SystemException, CF::InvalidFileName, CF::FileException)
{
    if (!ossie::isValidFileName(fileName)) {
        throw CF::InvalidFileName (CF::CF_EINVAL, "Invalid file name");
    } else if (_local_exists(fileName)) {
        throw CF::FileException(CF::CF_EEXIST, "File exists");
    }

    File_impl* file = File_impl::Create(fileName, this);
    PortableServer::POA_var poa = ossie::corba::RootPOA()->find_POA("Files", 1);
    PortableServer::ObjectId_var oid = poa->activate_object(file);
    file->_remove_ref();

    CF::File_var fileServant = file->_this();
    std::string fileIOR = ossie::corba::objectToString(fileServant);
    file->setIOR(fileIOR);

    return fileServant._retn();
}

void FileSystem_impl::closeAllFiles() {
    RH_TRACE(_fileSysLog, "Closing all open file handles");
    std::vector<std::string> iors;
    {
        boost::mutex::scoped_lock lock(fileIORCountAccess);
        for (IORTable::iterator iortable_iter = fileOpenIOR.begin();iortable_iter != fileOpenIOR.end();iortable_iter++) {
            iors.push_back(iortable_iter->second[0]); 
        }
    }
    std::vector<std::string>::iterator iors_iter = iors.begin();
    while (iors_iter != iors.end()) {
        try {
            CORBA::Object_var obj = ossie::corba::stringToObject(iors_iter->c_str());
            CF::File_var open_file = CF::File::_narrow(obj);
            std::cout<<"(1)"<<std::endl;
            open_file->close();
            std::cout<<"(2s)"<<std::endl;
        } catch ( ... ) {
            // the file may not be there anymore
        }
        iors_iter++;
    }
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

std::vector< std::string > FileSystem_impl::getFileIOR(const std::string& fileName) {
    boost::mutex::scoped_lock lock(fileIORCountAccess);
    std::vector< std::string > retVal;
    if (fileOpenIOR.count(fileName) != 0) {
        retVal = fileOpenIOR[fileName];
    }
    return retVal;
}

CF::File_ptr FileSystem_impl::open (const char* fileName, CORBA::Boolean read_Only) throw (CORBA::SystemException, CF::InvalidFileName, CF::FileException)
{
    if (!ossie::isValidFileName(fileName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid file name");
    } else if (!_local_exists(fileName)) {
        throw CF::FileException(CF::CF_EEXIST, "File does not exist");
    }

    File_impl* file = File_impl::Open(fileName, this, read_Only);
    PortableServer::POA_var poa = ossie::corba::RootPOA()->find_POA("Files", 1);
    PortableServer::ObjectId_var oid = poa->activate_object(file);
    file->_remove_ref();
    
    CF::File_var fileObj = file->_this();
    std::string fileIOR = ossie::corba::objectToString(fileObj);
    std::string strFileName = root.string();
    strFileName += fileName;
    incrementFileIORCount(strFileName, fileIOR);
    file->setIOR(fileIOR);

    return fileObj._retn();
}


void FileSystem_impl::mkdir (const char* directoryName) throw (CORBA::SystemException, CF::FileException, CF::InvalidFileName)
{
    if (!ossie::isValidFileName(directoryName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid file name");
    }

    fs::path dirPath(root / directoryName);
    fs::path currentPath;
    UnreliableFS fsops;

    for (fs::path::iterator walkPath = dirPath.begin(); walkPath != dirPath.end(); ++walkPath) {
        RH_TRACE(_fileSysLog, "Walking path to create directories, current path " << currentPath.string());
        currentPath /= *walkPath;
        if (!fsops.exists(currentPath)) {
            RH_TRACE(_fileSysLog, "Creating directory " << currentPath.string());
            fsops.create_directory(currentPath);
        } else if (!fsops.is_directory(currentPath)) {
            std::string msg = currentPath.string() + " is not a directory";
            throw CF::FileException(CF::CF_ENOTDIR, msg.c_str());
        }
    }
}

void FileSystem_impl::removeDirectory(const fs::path& dirPath, bool doRemove)
{
    UnreliableFS fsops;
    const fs::directory_iterator end_itr; // past the end
    for (fs::directory_iterator itr = fsops.begin(dirPath); itr != end_itr; fsops.increment(itr)) {
        if (fsops.is_directory(*itr)) {
            removeDirectory(*itr, doRemove);
        } else {
            throw CF::FileException(CF::CF_ENOTEMPTY, "Directory is not empty");
        }
    }

    if(doRemove) {
        fsops.remove(dirPath);
    }
}

void FileSystem_impl::rmdir (const char* directoryName) throw (CORBA::SystemException, CF::FileException, CF::InvalidFileName)
{
    if (!ossie::isValidFileName(directoryName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid directory name");
    }

    fs::path dirPath(root / directoryName);
    UnreliableFS fsops;

    if (!fsops.exists(dirPath)) {
        throw CF::FileException(CF::CF_EEXIST, "Path does not exist");
    } else if (!fsops.is_directory(dirPath)) {
        throw CF::FileException(CF::CF_ENOTDIR, "Path is not a directory");
    }

    // See the JTAP test for rmdir to understand this
    removeDirectory(dirPath, false); // Test for only empty directories
    removeDirectory(dirPath, true);  // Only empty directories, remove them all
}


void FileSystem_impl::query (CF::Properties& fileSysProperties) throw (CORBA::SystemException, CF::FileSystem::UnknownFileSystemProperties)
{
    if (fileSysProperties.length () == 0) {
        RH_TRACE(_fileSysLog, "Query all properties (SIZE, AVAILABLE_SPACE)");
        fileSysProperties.length(2);
        fileSysProperties[0].id = CORBA::string_dup("SIZE");
        fileSysProperties[0].value <<= getSize();
        fileSysProperties[1].id = CORBA::string_dup("AVAILABLE_SPACE");
        fileSysProperties[1].value <<= getAvailableSpace();
    } else {
        for (CORBA::ULong index = 0; index < fileSysProperties.length(); ++index) {
            if (strcmp (fileSysProperties[index].id, CF::FileSystem::SIZE) == 0) {
                fileSysProperties[index].value <<= getSize();
            } else if (strcmp(fileSysProperties[index].id, CF::FileSystem::AVAILABLE_SPACE) == 0) {
                fileSysProperties[index].value <<= getAvailableSpace();
            } else {
                throw CF::FileSystem::UnknownFileSystemProperties();
            }
        }
    }
}

std::string FileSystem_impl::getLocalPath (const char* fileName)
{
    return (root / fileName).string();
}

CORBA::ULongLong FileSystem_impl::getSize () const
{
    try {
        fs::space_info space = fs::space(root);
        return space.capacity;
    } catch (const fs::filesystem_error& ex) {
        RH_INFO(_fileSysLog, "Unexpected error querying file system size: " << ex.what());
        return 0;
    }
}

CORBA::ULongLong FileSystem_impl::getAvailableSpace () const
{
    try {
        fs::space_info space = fs::space(root);
        return space.available;
    } catch (const fs::filesystem_error& ex) {
        RH_INFO(_fileSysLog, "Unexpected error querying file system available space: " << ex.what());
        return 0;
    }
}
