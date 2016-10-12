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
#include <list>

#include <fnmatch.h>

#include "ossie/FileManager_impl.h"
#include "ossie/debug.h"
#include "ossie/CorbaUtils.h"
#include "ossie/ossieSupport.h"


namespace {

    static std::string normalizeMountPath (const std::string& path)
    {
        // Remove any trailing / in the mount point.
        if (!path.empty() && (path[path.size()-1] == '/')) {
            return path.substr(0, path.size()-1);
        }
        return path;
    }

}


bool FileManager_impl::MountPoint::contains (const std::string& filepath) const
{
    if (filepath.find(path) != 0) {
        // File path is not on mount path
        return false;
    }

    if (filepath.length() > path.length()) {
        // File path does not explicitly match mount path; ensure that the last path components
        // match exactly (e.g. "/a/bc" should not match "/a/b").
        if (filepath[path.length()] != '/') {
            return false;
        }
    }
    
    return true;
}

std::string FileManager_impl::MountPoint::getRelativePath (const std::string& filepath) const
{
    // Assuming that root is a prefix of path, remove it from path.
    return filepath.substr(path.length());
}


PREPARE_LOGGING(FileManager_impl);

FileManager_impl::FileManager_impl (const char* _fsroot):
    FileSystem_impl(_fsroot),
    mountedFileSystems(),
    mountsLock()
{
    TRACE_ENTER(FileManager_impl);
    TRACE_EXIT(FileManager_impl);
}

FileManager_impl::~FileManager_impl()
{
    TRACE_ENTER(FileManager_impl)
    TRACE_EXIT(FileManager_impl);
}

void FileManager_impl::mount (const char* mountPoint, CF::FileSystem_ptr fileSystem)
    throw (CORBA::SystemException, CF::InvalidFileName,
           CF::FileManager::InvalidFileSystem, CF::FileManager::MountPointAlreadyExists)
{
    TRACE_ENTER(FileManager_impl);

    if (CORBA::is_nil(fileSystem)) {
        throw CF::FileManager::InvalidFileSystem();
    }

    std::string mountPath = normalizeMountPath(mountPoint);
    if (!ossie::isValidFileName(mountPath.c_str())) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid mount point");
    }

    // Exclusive access to the mount table is required.
    boost::unique_lock<boost::shared_mutex> lock(mountsLock);

    // Ensure that the mount point is not already in use or inside another
    // mount point.
    for (MountList::iterator mount = mountedFileSystems.begin(); mount != mountedFileSystems.end(); ++mount) {
        if (mount->path == mountPath) {
            throw CF::FileManager::MountPointAlreadyExists();
        } else if (mount->contains(mountPath)) {
            throw CF::InvalidFileName(CF::CF_EINVAL, "Cannot mount file system inside another mounted file system");
        }
    }

    LOG_TRACE(FileManager_impl, "Mounting remote file system on " << mountPath);
    mountedFileSystems.push_back(MountPoint(mountPath, fileSystem));

    TRACE_EXIT(FileManager_impl)
}


void FileManager_impl::unmount (const char* mountPoint)
    throw (CORBA::SystemException, CF::FileManager::NonExistentMount)
{
    TRACE_ENTER(FileManager_impl);

    std::string mountPath = normalizeMountPath(mountPoint);

    // Exclusive access to the mount table is required.
    boost::unique_lock<boost::shared_mutex> lock(mountsLock);

    // Find the mount and remove it.
    for (MountList::iterator mount = mountedFileSystems.begin(); mount != mountedFileSystems.end(); ++mount) {
        if (mount->path == mountPath) {
            LOG_TRACE(FileManager_impl, "Unmounting remote file system on " << mountPath);
            mountedFileSystems.erase(mount);

            TRACE_EXIT(FileManager_impl);
            return;
        }
    }

    // Mount did not exist.
    throw CF::FileManager::NonExistentMount();
}


void FileManager_impl::remove (const char* fileName)
    throw (CORBA::SystemException, CF::FileException, CF::InvalidFileName)
{
    TRACE_ENTER(FileManager_impl);

    if (!ossie::isValidFileName(fileName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid file name");
    }

    LOG_TRACE(FileManager_impl, "Removing file " << fileName);

    // Lock the mount table shared to allow others to access the file system,
    // but prevent changes to the mount table itself.
    boost::shared_lock<boost::shared_mutex> lock(mountsLock);

    // Check if file is on one of the mounted file systems
    MountList::iterator mount = getMountForPath(fileName);
    if (mount == mountedFileSystems.end()) {
        LOG_TRACE(FileManager_impl, "Removing local file");
        FileSystem_impl::remove(fileName);
    } else {
        std::string filePath = mount->getRelativePath(fileName);
        LOG_TRACE(FileManager_impl, "Removing " << filePath << " on remote file system mounted at " << mount->path);
        mount->fs->remove(filePath.c_str());
    }

    TRACE_EXIT(FileManager_impl)
}


void FileManager_impl::copy (const char* sourceFileName, const char* destinationFileName)
    throw (CORBA::SystemException, CF::InvalidFileName, CF::FileException)
{
    TRACE_ENTER(FileManager_impl);

    // Validate absolute file names
    if (sourceFileName[0] != '/' || !ossie::isValidFileName(sourceFileName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid source file name");
    } else if (destinationFileName[0] != '/' || !ossie::isValidFileName(destinationFileName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid destination file name");
    } else if (strcmp(sourceFileName, destinationFileName) == 0) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Destination file name is identical to source file name");
    }

    LOG_TRACE(FileManager_impl, "Copy " << sourceFileName << " to " << destinationFileName);

    // Lock the mount table shared to allow others to access the file system,
    // but prevent changes to the mount table itself.
    boost::shared_lock<boost::shared_mutex> lock(mountsLock);

    MountList::iterator sourceMount = getMountForPath(sourceFileName);
    MountList::iterator destMount = getMountForPath(destinationFileName);

    if (sourceMount == destMount) {
        // Source and destination are on the same file system...
        if (sourceMount == mountedFileSystems.end()) {
            // ...which is also the local file system.
            LOG_TRACE(FileManager_impl, "Copying locally");
            FileSystem_impl::copy(sourceFileName, destinationFileName);
        } else {
            // ...which is a remote file system.
            LOG_TRACE(FileManager_impl, "Copying locally on remote file system");
            const std::string srcPath = sourceMount->getRelativePath(sourceFileName);
            const std::string dstPath = destMount->getRelativePath(destinationFileName);
            sourceMount->fs->copy(srcPath.c_str(), dstPath.c_str());
        }
        return;
    }

    LOG_TRACE(FileManager_impl, "Copying between filesystems");

    // Open the source file (may be local).
    CF::File_var srcFile;
    if (sourceMount == mountedFileSystems.end()) {
        srcFile = FileSystem_impl::open(sourceFileName, true);
    } else {
        const std::string srcPath = sourceMount->getRelativePath(sourceFileName);
        srcFile = sourceMount->fs->open(srcPath.c_str(), true);
    }

    // Open the destination file (may be local).
    CF::File_var dstFile;
    if (destMount == mountedFileSystems.end()) {
        if (FileSystem_impl::exists(destinationFileName)) {
            FileSystem_impl::remove(destinationFileName);
        }
        dstFile = FileSystem_impl::create(destinationFileName);
    } else {
        const std::string dstPath = destMount->getRelativePath(destinationFileName);
        CF::FileSystem_ptr destFS = destMount->fs;
        if (destFS->exists(dstPath.c_str())) {
            destFS->remove(dstPath.c_str());
        }
        dstFile = destFS->create(dstPath.c_str());
    }

    std::ostringstream eout;
    bool fe=false;
    try {
      // Read the data
      CF::OctetSequence_var data;
      CORBA::ULong bytes = srcFile->sizeOf();
      const CORBA::ULong DEFAULT_CHUNK_SIZE =  ossie::corba::giopMaxMsgSize() * 0.95;
      while (bytes > 0) {
        // Read the file data in 1MB chunks. If omniORB uses the GIOP protocol to talk to the source filesystem
        // (i.e. it is in a different ORB), reads of more than about 2MB will exceed the maximum GIOP packet
        // size and raise a MARSHAL exception.
        CORBA::ULong chunkSize = std::min(bytes, DEFAULT_CHUNK_SIZE);
        bytes -= chunkSize;
        
        try {
          srcFile->read(data, chunkSize);
        } catch ( std::exception& ex ) {
          eout << "The following standard exception occurred: "<<ex.what()<<" While \"srcFile->read\"";
          throw(CF::FileException());
        } catch ( CF::FileException& ex ) {
          eout << "File Exception occured,  While \"srcFile->read\"";
          throw;
        } catch ( CF::File::IOException& ex ) {
          eout << "File IOException occured,  While \"srcFile->read\"";
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
          std::ostringstream eout;
          eout << "The following standard exception occurred: "<<ex.what()<<" While \"dstFile->write\"";
          throw(CF::FileException());
        } catch ( CF::FileException& ex ) {
          eout << "File Exception occurred, during \"dstFile->write\"";
          throw;
        } catch ( CF::File::IOException& ex ) {
          eout << "File IOException occurred, during \"dstFile->write\"";
          throw;
        } catch ( CORBA::Exception& ex ) {
          eout << "The following CORBA exception occurred: "<<ex._name()<<" While \"dstFile->write\"";
          throw(CF::FileException());
        } catch( ... ) {
          eout << "[FileManager::copy] \"dstFile->write\" failed with Unknown Exception\n";
          throw(CF::FileException());
        }

      }
    }
    catch(...) {
      LOG_ERROR(FileManager_impl, eout.str());
      fe = true;
    }

   // close the files
    try {
      try {
        srcFile->close();
      } catch ( std::exception& ex ) {
        eout << "The following standard exception occurred: "<<ex.what()<<" While \"srcFile->close\"";
        throw(CF::FileException());
      } catch ( CF::FileException& ex ) {
        eout << "File Exception occured, during \"srcFile->close\"";
        throw;
      } catch ( CORBA::Exception& ex ) {
        eout << "The following CORBA exception occurred: "<<ex._name()<<" While \"srcFile->close\"";
        throw(CF::FileException());
      } catch( ... ) {
        eout << "[FileManager::copy] \"srcFile->close\" failed with Unknown Exception\n";
        throw(CF::FileException());
      }
    } catch(...) {
      LOG_ERROR(FileManager_impl, eout.str());
      fe = true;
    }


    try {
      try {
        dstFile->close();
      } catch ( std::exception& ex ) {
        eout << "The following standard exception occurred: "<<ex.what()<<" While \"dstFile->close\"";
        throw(CF::FileException());
      } catch ( CF::FileException& ex ) {
        eout << "File Exception occured, during \"srcFile->close\"";
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

    TRACE_EXIT(FileManager_impl);
}


void FileManager_impl::move (const char* sourceFileName, const char* destinationFileName)
    throw (CORBA::SystemException, CF::InvalidFileName, CF::FileException)
{
    TRACE_ENTER(FileManager_impl);

    // Validate absolute file names
    if (sourceFileName[0] != '/' || !ossie::isValidFileName(sourceFileName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid source file name");
    } else if (destinationFileName[0] != '/' || !ossie::isValidFileName(destinationFileName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid destination file name");
    } else if (strcmp(sourceFileName, destinationFileName) == 0) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Destination file name is identical to source file name");
    }

    LOG_TRACE(FileManager_impl, "Move " << sourceFileName << " to " << destinationFileName);

    // Lock the mount table shared to allow others to access the file system,
    // but prevent changes to the mount table itself.
    boost::shared_lock<boost::shared_mutex> lock(mountsLock);

    MountList::iterator sourceMount = getMountForPath(sourceFileName);
    MountList::iterator destMount = getMountForPath(destinationFileName);

    if (sourceMount == destMount) {
        // Source and destination are on the same file system...
        if (sourceMount == mountedFileSystems.end()) {
            // ...which is also the local file system.
            LOG_TRACE(FileManager_impl, "Moving locally");
            FileSystem_impl::move(sourceFileName, destinationFileName);
        } else {
            // ...which is a remote file system.
            LOG_TRACE(FileManager_impl, "Moving locally on remote file system");
            const std::string srcPath = sourceMount->getRelativePath(sourceFileName);
            const std::string dstPath = destMount->getRelativePath(destinationFileName);
            sourceMount->fs->move(srcPath.c_str(), dstPath.c_str());
        }
        return;
    }

    LOG_TRACE(FileManager_impl, "Moving between filesystems");

    // Perform a copy followed by a remove, which is the only way we can move
    // across file systems. This operation is not atomic, and making it atomic
    // across a distributed file system is difficult. If necessary, we can
    // acquire an exclusive lock which at least prevents anyone going through
    // the domain's FileManager from interfering with this operation.
    this->copy(sourceFileName, destinationFileName);
    this->remove(sourceFileName);

    TRACE_EXIT(FileManager_impl);
}


CORBA::Boolean FileManager_impl::exists (const char* fileName)
    throw (CORBA::SystemException, CF::InvalidFileName)
{
    TRACE_ENTER(FileManager_impl);

    if (!ossie::isValidFileName(fileName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid file name");
    }

    LOG_TRACE(FileManager_impl, "Checking for existence of " << fileName);

    // Lock the mount table shared to allow others to access the file system,
    // but prevent changes to the mount table itself.
    boost::shared_lock<boost::shared_mutex> lock(mountsLock);

    // Check if path is on one of the mounted file systems
    MountList::iterator mount = getMountForPath(fileName);
    CORBA::Boolean status;
    if (mount == mountedFileSystems.end()) {
        LOG_TRACE(FileManager_impl, "Checking local file system");
        status = FileSystem_impl::exists(fileName);
    } else {
        std::string filePath = mount->getRelativePath(fileName);
        LOG_TRACE(FileManager_impl, "Checking for " << filePath << " on remote file system mounted at " << mount->path);
        status = mount->fs->exists(filePath.c_str());
    }

    TRACE_EXIT(FileManager_impl);
    return status;
}


CF::FileSystem::FileInformationSequence* FileManager_impl::list (const char* pattern)
    throw (CORBA::SystemException, CF::FileException, CF::InvalidFileName)
{
    TRACE_ENTER(FileManager_impl);
    
    LOG_TRACE(FileManager_impl, "List files with pattern " << pattern);

    CF::FileSystem::FileInformationSequence_var result;

    // Lock the mount table shared to allow others to access the file system,
    // but prevent changes to the mount table itself.
    boost::shared_lock<boost::shared_mutex> lock(mountsLock);

    MountList::iterator mount = getMountForPath(pattern);
    if (mount == mountedFileSystems.end()) { 
        LOG_TRACE(FileManager_impl, "Listing local file system");
        result = FileSystem_impl::list(pattern);

        // Check for any mount points that match the pattern.
        std::string searchPattern = pattern;
        if (!searchPattern.empty() && (searchPattern[searchPattern.size()-1] == '/')) {
            searchPattern += "*";
        }

        for (MountList::iterator ii = mountedFileSystems.begin(); ii != mountedFileSystems.end(); ++ii) {
            if (fnmatch(searchPattern.c_str(), ii->path.c_str(), 0) == 0) {
                CORBA::ULong index = result->length();
                result->length(index+1);
                result[index].name = CORBA::string_dup(ii->path.substr(1).c_str());
                result[index].kind = CF::FileSystem::FILE_SYSTEM;
                result[index].size = 0;
                result[index].fileProperties.length(0);
            }
        }
    } else {
        const std::string searchPath = mount->getRelativePath(pattern);
        if (searchPath.empty()) {
            // Exact match for mount point
            LOG_TRACE(FileManager_impl, "List mount point " << mount->path);
            result = new CF::FileSystem::FileInformationSequence();
            result->length(1);
            result[0].name = CORBA::string_dup(mount->path.substr(1).c_str());
            result[0].kind = CF::FileSystem::FILE_SYSTEM;
            result[0].size = 0;
            result[0].fileProperties.length(0);
        } else {
            // List contents of mount point
            LOG_TRACE(FileManager_impl, "Listing " << searchPath << " on remote file system mounted at " << mount->path);
            result = mount->fs->list(searchPath.c_str());
        }
    }

    TRACE_EXIT(FileManager_impl);
    return result._retn();
}


CF::File_ptr FileManager_impl::create (const char* fileName)
    throw (CORBA::SystemException, CF::InvalidFileName, CF::FileException)
{
    TRACE_ENTER(FileManager_impl);

    if (!ossie::isValidFileName(fileName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid file name");
    }

    LOG_TRACE(FileManager_impl, "Creating file " << fileName)

    // Lock the mount table shared to allow others to access the file system,
    // but prevent changes to the mount table itself.
    boost::shared_lock<boost::shared_mutex> lock(mountsLock);

    MountList::iterator mount = getMountForPath(fileName);
    CF::File_var file;
    if (mount == mountedFileSystems.end()) {
        LOG_TRACE(FileManager_impl, "Creating local file");
        file = FileSystem_impl::create(fileName);
    } else {
        const std::string filePath = mount->getRelativePath(fileName);
        LOG_TRACE(FileManager_impl, "Creating " << filePath << " on remote file system mounted at " << mount->path);
        file = mount->fs->create(filePath.c_str());
    }

    TRACE_EXIT(FileManager_impl);
    return file._retn();
}


CF::File_ptr FileManager_impl::open (const char* fileName, CORBA::Boolean read_Only)
    throw (CORBA::SystemException, CF::InvalidFileName, CF::FileException)
{
    TRACE_ENTER(FileManager_impl);

    if (!ossie::isValidFileName(fileName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid file name");
    }

    LOG_TRACE(FileManager_impl, "Opening file " << fileName << std::string((read_Only)?" readonly":" readwrite"));

    // Lock the mount table shared to allow others to access the file system,
    // but prevent changes to the mount table itself.
    boost::shared_lock<boost::shared_mutex> lock(mountsLock);

    MountList::iterator mount = getMountForPath(fileName);
    CF::File_var file;
    if (mount == mountedFileSystems.end()) {
        LOG_TRACE(FileManager_impl, "Opening local file");
        file = FileSystem_impl::open(fileName, read_Only);
    } else {
        const std::string filePath = mount->getRelativePath(fileName);
        LOG_TRACE(FileManager_impl, "Opening " << filePath << " on remote file system mounted at " << mount->path);
        file = mount->fs->open(filePath.c_str(), read_Only);
    }

    TRACE_EXIT(FileManager_impl);
    return file._retn();
}


void FileManager_impl::mkdir (const char* directoryName)
    throw (CORBA::SystemException, CF::FileException, CF::InvalidFileName)
{
    TRACE_ENTER(FileManager_impl);

    if (!ossie::isValidFileName(directoryName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid directory name");
    }

    LOG_TRACE(FileManager_impl, "Making directory " << directoryName)

    // Lock the mount table shared to allow others to access the file system,
    // but prevent changes to the mount table itself.
    boost::shared_lock<boost::shared_mutex> lock(mountsLock);

    MountList::iterator mount = getMountForPath(directoryName);
    if (mount == mountedFileSystems.end()) {
        LOG_TRACE(FileManager_impl, "Making local directory");
        FileSystem_impl::mkdir(directoryName);
    } else {
        const std::string dirPath = mount->getRelativePath(directoryName);
        LOG_TRACE(FileManager_impl, "Making directory " << dirPath << " on remote file system mounted at " << mount->path);
        mount->fs->mkdir(dirPath.c_str());
    }

    TRACE_EXIT(FileManager_impl);
}


void FileManager_impl::rmdir (const char* directoryName)
    throw (CORBA::SystemException, CF::FileException, CF::InvalidFileName)
{
    TRACE_ENTER(FileManager_impl);

    if (!ossie::isValidFileName(directoryName)) {
        throw CF::InvalidFileName(CF::CF_EINVAL, "Invalid directory name");
    }

    LOG_TRACE(FileManager_impl, "Removing directory " << directoryName)

    // Lock the mount table shared to allow others to access the file system,
    // but prevent changes to the mount table itself.
    boost::shared_lock<boost::shared_mutex> lock(mountsLock);

    MountList::iterator mount = getMountForPath(directoryName);
    if (mount == mountedFileSystems.end()) {
        LOG_TRACE(FileManager_impl, "Removing local directory");
        FileSystem_impl::rmdir(directoryName);
    } else {
        const std::string dirPath = mount->getRelativePath(directoryName);
        LOG_TRACE(FileManager_impl, "Removing directory " << dirPath << " on remote file system mounted at " << mount->path);
        mount->fs->rmdir(dirPath.c_str());
    }

    TRACE_EXIT(FileManager_impl);
}


void FileManager_impl::query (CF::Properties& fileSysProperties)
    throw (CORBA::SystemException, CF::FileSystem::UnknownFileSystemProperties)
{
    TRACE_ENTER(FileManager_impl);

    CF::Properties unknownProps;

    // Lock the mount table shared to allow others to access the file system,
    // but prevent changes to the mount table itself.
    boost::shared_lock<boost::shared_mutex> lock(mountsLock);
    if (fileSysProperties.length () == 0) {
        LOG_TRACE(FileManager_impl, "Query all properties (SIZE, AVAILABLE_SPACE)");
        fileSysProperties.length(2);
        fileSysProperties[0].id = CORBA::string_dup("SIZE");
        CORBA::ULongLong size = getSize();
        size += getCombinedProperty(CF::FileSystem::SIZE);
        fileSysProperties[0].value <<= size;
        fileSysProperties[1].id = CORBA::string_dup("AVAILABLE_SPACE");
        CORBA::ULongLong available = getAvailableSpace();
        available += getCombinedProperty(CF::FileSystem::AVAILABLE_SPACE);
        fileSysProperties[1].value <<= available;
    } else {
        for (CORBA::ULong index = 0; index < fileSysProperties.length(); ++index) {
            if (strcmp (fileSysProperties[index].id, CF::FileSystem::SIZE) == 0) {
                CORBA::ULongLong size = getSize();
                size += getCombinedProperty(CF::FileSystem::SIZE);
                fileSysProperties[index].value <<= size;
            } else if (strcmp(fileSysProperties[index].id, CF::FileSystem::AVAILABLE_SPACE) == 0) {
                CORBA::ULongLong available = getAvailableSpace();
                available += getCombinedProperty(CF::FileSystem::AVAILABLE_SPACE);
                fileSysProperties[index].value <<= available;
            } else {
                CORBA::ULong count = unknownProps.length();
                unknownProps.length(count+1);
                unknownProps[count] = fileSysProperties[index];
            }
        }
    }

    if (unknownProps.length() > 0) {
        throw CF::FileSystem::UnknownFileSystemProperties(unknownProps);
    }

    TRACE_EXIT(FileManager_impl)
}

CORBA::ULongLong FileManager_impl::getCombinedProperty (const char* propId)
{
    CORBA::ULongLong totalSize = 0;
    for (MountList::iterator mount = mountedFileSystems.begin(); mount != mountedFileSystems.end(); ++mount) {
        CF::Properties props;
        props.length(1);
        props[0].id = propId;
        mount->fs->query(props);
        CORBA::ULongLong fsSize;
        props[0].value >>= fsSize;
        totalSize += fsSize;
    }

    return totalSize;
}


CF::FileManager::MountSequence* FileManager_impl::getMounts ()
    throw (CORBA::SystemException)
{
    TRACE_ENTER(FileManager_impl);

    // Lock the mount table shared to allow others to access the file system,
    // but prevent changes to the mount table itself.
    boost::shared_lock<boost::shared_mutex> lock(mountsLock);

    CF::FileManager::MountSequence_var result = new CF::FileManager::MountSequence();
    result->length(mountedFileSystems.size());
    CORBA::ULong index = 0;
    for (MountList::iterator ii = mountedFileSystems.begin(); ii != mountedFileSystems.end(); ++ii, ++index) {
        result[index].mountPoint = ii->path.c_str();
        result[index].fs = CF::FileSystem::_duplicate(ii->fs);
    }

    TRACE_EXIT(FileManager_impl);
    return result._retn();
}
 
 
FileManager_impl::MountList::iterator FileManager_impl::getMountForPath (const std::string& path)
{
    MountList::iterator mount;
    for (mount = mountedFileSystems.begin(); mount != mountedFileSystems.end(); ++mount) {
        if (mount->contains(path)) {
            break;
        }
    }
    return mount;
}
