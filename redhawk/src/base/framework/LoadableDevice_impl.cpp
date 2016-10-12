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

#include "ossie/LoadableDevice_impl.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <boost/filesystem.hpp>
#include <iostream>

namespace fs = boost::filesystem;

static time_t getModTime (const CF::Properties& properties)
{
    CORBA::ULongLong modTime = 0;
    for (CORBA::ULong ii = 0; ii < properties.length(); ++ii) {
        if (strcmp(properties[ii].id, "MODIFIED_TIME") == 0) {
            properties[ii].value >>= modTime;
        }
    }
    return static_cast<time_t>(modTime);
}

static bool checkPath(const std::string& envpath, const std::string& pattern, char delim=':')
{
    // First, check if the pattern is even in the input path
    std::string::size_type start = envpath.find(pattern);
    if (start == std::string::npos) {
        return false;
    }
    // Next, make sure that the pattern starts at a boundary--either at the
    // beginning, or immediately following a delimiter
    if ((start != 0) && (envpath[start-1] != delim)) {
        return false;
    }
    // Finally, make sure that the pattern ends at a boundary as well
    std::string::size_type end = start + pattern.size();
    return ((end == envpath.size()) || (envpath[end] == delim));
}

PREPARE_LOGGING(LoadableDevice_impl)


/* LoadableDevice_impl ****************************************************************************
    - constructor 1: no capacities defined
************************************************************************************************ */
LoadableDevice_impl::LoadableDevice_impl (char* devMgr_ior, char* id, char* lbl, char* sftwrPrfl):
    Device_impl (devMgr_ior, id, lbl, sftwrPrfl)
{
}


/* LoadableDevice_impl ****************************************************************************
    - constructor 2: capacities defined
************************************************************************************************ */
LoadableDevice_impl::LoadableDevice_impl (char* devMgr_ior, char* id, char* lbl, char* sftwrPrfl,
                                          CF::Properties capacities):
    Device_impl (devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
}

/* LoadableDevice_impl ****************************************************************************
    - constructor 1: no capacities defined
************************************************************************************************ */
LoadableDevice_impl::LoadableDevice_impl (char* devMgr_ior, char* id, char* lbl, char* sftwrPrfl, 
                                          char* composite_ior):
    Device_impl (devMgr_ior, id, lbl, sftwrPrfl, composite_ior)
{
}


/* LoadableDevice_impl ****************************************************************************
    - constructor 2: capacities defined
************************************************************************************************ */
LoadableDevice_impl::LoadableDevice_impl (char* devMgr_ior, char* id, char* lbl, char* sftwrPrfl,
                                          CF::Properties capacities, char* composite_ior):
    Device_impl (devMgr_ior, id, lbl, sftwrPrfl, capacities, composite_ior)
{
}


/* LoadableDevice_impl ****************************************************************************
    - destructor
************************************************************************************************ */
LoadableDevice_impl::~LoadableDevice_impl ()
{

#ifdef DEBUG_ON
  // use std::out incase log4cxx is shutdown
  std::cout <<  "LoadableDevice, DTOR........ START " << std::endl;
  std::cout <<  " copiedFiles....size:" << copiedFiles.size()  << std::endl;
  std::cout <<  " loadedFiles....size:" << loadedFiles.size()  << std::endl;
  std::cout <<  "LoadableDevice, DTOR........ END " << std::endl;
#endif

}


/* load *******************************************************************************************
    - loads a file into the device
************************************************************************************************ */
void
LoadableDevice_impl::load (CF::FileSystem_ptr fs, const char* fileName,
                           CF::LoadableDevice::LoadType loadKind)
throw (CORBA::SystemException, CF::Device::InvalidState,
       CF::LoadableDevice::InvalidLoadKind, CF::InvalidFileName,
       CF::LoadableDevice::LoadFail)
{
    LOG_DEBUG(LoadableDevice_impl, "load " << fileName)

// verify that the device is in a valid state for loading
    if (!isUnlocked () || isDisabled ()) {
        LOG_ERROR(LoadableDevice_impl, "Cannot load. System is either LOCKED, SHUTTING DOWN or DISABLED.")
        LOG_DEBUG(LoadableDevice_impl, "Unlocked: " << isUnlocked ())
        LOG_DEBUG(LoadableDevice_impl, "isDisabled: " << isDisabled ())
        throw (CF::Device::
               InvalidState
               ("Cannot load. System is either LOCKED, SHUTTING DOWN or DISABLED."));
    }

    LOG_DEBUG(LoadableDevice_impl, "It's not locked and not disabled")

// verify that the loadKind is supported (only executable is supported by this version)
    if ((loadKind != CF::LoadableDevice::EXECUTABLE) && (loadKind != CF::LoadableDevice::SHARED_LIBRARY)) {
        LOG_ERROR(LoadableDevice_impl, "It's not CF::LoadableDevice::EXECUTABLE or CF::LoadableDevice::SHARED_LIBRARY")
        throw CF::LoadableDevice::InvalidLoadKind ();
    }

    std::string workingFileName = fileName;

// verify the file name exists in the file system and get a pointer to it
// NOTE: in this context, this step is redundant; the 'installApplication' method
// already performs this existence check
    try {
        if (!fs->exists (workingFileName.c_str())) {
            LOG_ERROR(LoadableDevice_impl, "File " << workingFileName << " does not exist")
            throw (CF::InvalidFileName (CF::CF_ENOENT, "Cannot load. File name is invalid."));
        }
    } catch ( ... ) {
        LOG_ERROR(LoadableDevice_impl, "Exception raised when calling the file system")
        throw;
    }

    LOG_DEBUG(LoadableDevice_impl, "Cleaning name " << fileName)
    // Get rid of all the directories in the given name (if any)
    CF::FileSystem::FileInformationSequence_var contents = fs->list(workingFileName.c_str());
    std::string simpleName;
    std::string::size_type pos = workingFileName.find_last_of("/");
    if (pos == std::string::npos) {
        simpleName = workingFileName;
    } else {
        simpleName = workingFileName.substr(pos + 1);
    }

    LOG_DEBUG(LoadableDevice_impl, "Is " << fileName << " a directory?")
    CF::FileSystem::FileInformationType* fileInfo = 0;
    for (unsigned int i = 0; i < contents->length(); i++) {
        LOG_DEBUG(LoadableDevice_impl, "comparing " << simpleName << " and " << contents[i].name)
        if (!simpleName.compare(contents[i].name)) {
            fileInfo = &contents[i];
            break;
        }
    }
    if (!fileInfo) {
        LOG_ERROR(LoadableDevice_impl, "The file system couldn't find " << fileName)
        throw (CF::InvalidFileName (CF::CF_ENOENT, "Cannot load. File name is invalid."));
    }

    std::string relativeFileName = "";
    if (loadedFiles.count(workingFileName)) {
        relativeFileName = workingFileName;
        if (workingFileName[0] == '/') {
            relativeFileName = workingFileName.substr(1);
        }
        if (fs::exists(relativeFileName)) {
            // Check if the remote file is newer than the local file, and if so, update the file
            // in the cache. No consideration is given to clock sync differences between systems.
            time_t remoteModifiedTime = getModTime(fileInfo->fileProperties);
            time_t cacheModifiedTime = fs::last_write_time(relativeFileName);
            LOG_TRACE(LoadableDevice_impl, "Remote modified: " << remoteModifiedTime << " Local modified: " << cacheModifiedTime);
            if (remoteModifiedTime > cacheModifiedTime) {
                LOG_DEBUG(LoadableDevice_impl, "Remote file is newer than local file");
            } else {
                LOG_DEBUG(LoadableDevice_impl, "File exists in cache");
                incrementFile(workingFileName);
                return;
            }
        }
    }

    if (fileInfo->kind != CF::FileSystem::DIRECTORY) {
        // create a local directory to copy the file to
        // The target file is a file
        LOG_DEBUG(LoadableDevice_impl, "Loading the file " << fileName)

        // Create parent directories
        bool done = false;
        std::string initialDir("");
        std::string::size_type begin_pos = 0;
        std::string::size_type last_slash = workingFileName.find_last_of("/");
        if (last_slash != std::string::npos) {
            while (!done) {
                std::string::size_type pos = workingFileName.find_first_of("/", begin_pos);
                if (pos == begin_pos) {
                    begin_pos++;
                    continue;
                }
                if (pos == std::string::npos)
                    { break; }
                initialDir += workingFileName.substr(begin_pos, (pos - begin_pos)) + std::string("/");
                mkdir(initialDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                LOG_DEBUG(LoadableDevice_impl, "Creating directory (from " << workingFileName << ") " << initialDir)
                begin_pos = pos + 1;
            }
        }

        // Create the output file
        std::fstream fileStream;
        std::ios_base::openmode mode;
        mode = std::ios::out;
        relativeFileName = workingFileName;
        if (workingFileName[0] == '/') {
            relativeFileName = workingFileName.substr(1);
        }
        _copyFile( fs, workingFileName, relativeFileName, workingFileName );
        chmod(relativeFileName.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        fileTypeTable[workingFileName] = CF::FileSystem::PLAIN;
    } else {
        // The target file is a directory
        LOG_DEBUG(LoadableDevice_impl, "Copying the file " << fileName << " as a directory to the cache as " << workingFileName)
        fileTypeTable[workingFileName] = CF::FileSystem::DIRECTORY;
        fs::path localPath = fs::path(workingFileName).branch_path().relative_path();
        copiedFiles.insert(copiedFiles_type::value_type(workingFileName, localPath.string()));
        _loadTree(fs, workingFileName, localPath, std::string(fileName));
        relativeFileName = workingFileName;
        if (workingFileName[0] == '/') {
            relativeFileName = workingFileName.substr(1);
        }
    }

// add filename to loadedfiles. If it's been already loaded, then increment its counter
    LOG_DEBUG(LoadableDevice_impl, "Incrementing " << workingFileName << " vs " << fileName)
    incrementFile (workingFileName);

    // Update environment to use newly-loaded library
    if (loadKind == CF::LoadableDevice::SHARED_LIBRARY) {
        LOG_DEBUG(LoadableDevice_impl, "Configuring shared library");
        bool CLibrary = false;
        bool PythonPackage = false;
        bool JavaJar = false;
        // Check to see if it's a C library
        std::string command = "readelf -h ";
        command += relativeFileName;
        command += std::string(" 2>&1"); // redirect stdout to /dev/null
        FILE *fileCheck = popen(command.c_str(), "r");
        int status = pclose(fileCheck);
        std::string currentPath = ossie::getCurrentDirName();
        if (!status) { // this file is a C library
            std::string ld_library_path;
            if (getenv("LD_LIBRARY_PATH")) {
                ld_library_path = getenv("LD_LIBRARY_PATH");
            }
            // Determine the full directory path of the file; there is always
            // at least one slash, so it's safe to erase from that point on
            std::string additionalPath = currentPath+std::string("/")+relativeFileName;
            additionalPath.erase(additionalPath.rfind('/'));

            // Make sure that the current path is not already in LD_LIBRARY_PATH
            if (!checkPath(ld_library_path, additionalPath)) {
                LOG_DEBUG(LoadableDevice_impl, "Adding " << additionalPath << " to LD_LIBRARY_PATH");
                if (!ld_library_path.empty()) {
                    ld_library_path += ':';
                }
                ld_library_path += additionalPath;
                setenv("LD_LIBRARY_PATH", ld_library_path.c_str(), 1);
            }
            CLibrary = true;
        }
        // Check to see if it's a Python module
        if (!CLibrary) {
          currentPath = ossie::getCurrentDirName();
            std::string::size_type lastSlash = relativeFileName.find_last_of("/");
            if (lastSlash == std::string::npos) { // there are no slashes in the name
                std::string fileOrDirectoryName = relativeFileName;
                std::string relativePath = "";
                unsigned int fileNameSize = fileOrDirectoryName.size();
                if (!fileOrDirectoryName.compare(fileNameSize-4, 4, ".pyc")) {
                    fileOrDirectoryName.erase(fileNameSize-4, 4);
                } else if (!fileOrDirectoryName.compare(fileNameSize-3, 3, ".py")) {
                    fileOrDirectoryName.erase(fileNameSize-3, 3);
                }
                std::string command = "python -c \"import ";
                command += fileOrDirectoryName;
                command += std::string("\" 2>&1"); // redirect stdout and stderr to /dev/null
                FILE *fileCheck = popen(command.c_str(), "r");
                int status = pclose(fileCheck);
                if (!status) {
                    // The import worked
                    std::string additionalPath = "";
                    if (fileInfo->kind == CF::FileSystem::DIRECTORY) {
                        additionalPath = currentPath+std::string("/")+relativeFileName;
                    } else {
                        additionalPath = currentPath+std::string("/")+relativePath;
                    }
                    std::string pythonpath = "";
                    if (getenv("PYTHONPATH")) {
                        pythonpath = getenv("PYTHONPATH");
                    }
                    std::string::size_type pathLocation = pythonpath.find(additionalPath);
                    if (pathLocation == std::string::npos) {
                        unsigned int lastSeparator = pythonpath.find_last_of(":");
                        if (lastSeparator == pythonpath.size()-1) {
                            pythonpath += additionalPath;
                        } else {
                            pythonpath += std::string(":")+additionalPath;
                        }
                        LOG_DEBUG(LoadableDevice_impl, "Adding " << additionalPath << " to PYTHONPATH");
                        setenv("PYTHONPATH", pythonpath.c_str(), 1);
                    }
                    PythonPackage = true;
                }
                chdir(currentPath.c_str());
            } else {
                // grab the last string after the slash
                std::string fileOrDirectoryName = "";
                std::string relativePath = "";
                fileOrDirectoryName.assign(relativeFileName, lastSlash+1, relativeFileName.size()-lastSlash);
                unsigned int fileNameSize = fileOrDirectoryName.size();
                if (!fileOrDirectoryName.compare(fileNameSize-4, 4, ".pyc")) {
                    fileOrDirectoryName.erase(fileNameSize-4, 4);
                } else if (!fileOrDirectoryName.compare(fileNameSize-3, 3, ".py")) {
                    fileOrDirectoryName.erase(fileNameSize-3, 3);
                }
                relativePath.assign(relativeFileName, 0, lastSlash);
                if (chdir(relativePath.c_str())) {
                        // this is an invalid path
                } else {
                    std::string command = "python -c \"import ";
                    command += fileOrDirectoryName;
                    command += std::string("\" 2>&1"); // redirect stdout and stderr to /dev/null
                    FILE *fileCheck = popen(command.c_str(), "r");
                    int status = pclose(fileCheck);
                    if (!status) {
                        // The import worked
                        std::string additionalPath = "";
                        if (fileInfo->kind == CF::FileSystem::DIRECTORY) {
                            additionalPath = currentPath+std::string("/")+relativeFileName;
                        } else {
                            additionalPath = currentPath+std::string("/")+relativePath;
                        }
                        std::string pythonpath = "";
                        if (getenv("PYTHONPATH")) {
                            pythonpath = getenv("PYTHONPATH");
                        }
                        std::string::size_type pathLocation = pythonpath.find(additionalPath);
                        if (pathLocation == std::string::npos) {
                            unsigned int lastSeparator = pythonpath.find_last_of(":");
                            if (lastSeparator == pythonpath.size()-1) {
                                pythonpath += additionalPath;
                            } else {
                                pythonpath += std::string(":")+additionalPath;
                            }
                            LOG_DEBUG(LoadableDevice_impl, "Adding " << additionalPath << " to PYTHONPATH");
                            setenv("PYTHONPATH", pythonpath.c_str(), 1);
                        }
                        PythonPackage = true;
                    }
                }
                chdir(currentPath.c_str());
            }
        }
        // Check to see if it's a Java package
        if (!CLibrary and !PythonPackage) {
            std::string command = "file ";
            command += relativeFileName;
            FILE *fileCheck = popen(command.c_str(), "r");
            char retval1[1024], retval2[1024];
            fscanf(fileCheck, "%s %s", &(retval1[0]), &(retval2[0]));
            std::string fileType = retval2;
            pclose(fileCheck);
            unsigned int fileNameSize = relativeFileName.size();
            unsigned int fileTypeSize = fileType.size();
            if ((fileNameSize>=4)and(fileTypeSize>=3)) {
                if (!relativeFileName.compare(fileNameSize-4, 4, ".jar") and !fileType.compare(0, 3, "Zip")) {
                  currentPath = ossie::getCurrentDirName();
                    std::string classpath = "";
                    if (getenv("CLASSPATH")) {
                        classpath = getenv("CLASSPATH");
                    }
                    std::string additionalPath = currentPath+std::string("/")+relativeFileName;
                    std::string::size_type pathLocation = classpath.find(additionalPath);
                    if (pathLocation == std::string::npos) {
                        unsigned int lastSeparator = classpath.find_last_of(":");
                        if (lastSeparator == classpath.size()-1) {
                            classpath += additionalPath;
                        } else {
                            classpath += std::string(":")+additionalPath;
                        }
                        LOG_DEBUG(LoadableDevice_impl, "Adding " << additionalPath << " to CLASSPATH");
                        setenv("CLASSPATH", classpath.c_str(), 1);
                    }
                    JavaJar = true;
                }
            }
        }
        // It doesn't match anything, assume that it's a set of libraries
        if (!(CLibrary || PythonPackage || JavaJar)) {
            std::string ld_library_path;
            if (getenv("LD_LIBRARY_PATH")) {
                ld_library_path = getenv("LD_LIBRARY_PATH");
            }
            // Make sure that the current path is not already in LD_LIBRARY_PATH
            const std::string additionalPath = currentPath+std::string("/")+relativeFileName;
            if (!checkPath(ld_library_path, additionalPath)) {
                LOG_DEBUG(LoadableDevice_impl, "Adding " << additionalPath << " to LD_LIBRARY_PATH");
                if (!ld_library_path.empty()) {
                    ld_library_path += ':';
                }
                ld_library_path += additionalPath;
                setenv("LD_LIBRARY_PATH", ld_library_path.c_str(), 1);
            }
        }
    }

}


void LoadableDevice_impl::_loadTree(CF::FileSystem_ptr fs, std::string remotePath, fs::path& localPath, std::string fileKey)
{

    LOG_DEBUG(LoadableDevice_impl, "_loadTree " << remotePath << " " << localPath)

    CF::FileSystem::FileInformationSequence_var fis = fs->list(remotePath.c_str());
    if (fis->length() == 0) {
    }
    for (unsigned int i = 0; i < fis->length(); i++) {

        if (fis[i].kind == CF::FileSystem::PLAIN) {
            std::string fileName(fis[i].name);
            fs::path localFile(localPath / fileName);
            if (*(remotePath.end() - 1) == '/') {
                LOG_DEBUG(LoadableDevice_impl, "_copyFile " << remotePath + fileName << " " << localFile)
                _copyFile(fs, remotePath + fileName, localFile.string(), fileKey);
            } else {
                LOG_DEBUG(LoadableDevice_impl, "_copyFile " << remotePath << " " << localFile)
                _copyFile(fs, remotePath, localFile.string(), fileKey);
            }
        } else if (fis[i].kind == CF::FileSystem::DIRECTORY) {
            std::string directoryName(fis[i].name);
            fs::path localDirectory(localPath / directoryName);
            LOG_DEBUG(LoadableDevice_impl, "Making directory " << directoryName << " in " << localPath)
            copiedFiles.insert(copiedFiles_type::value_type(fileKey, localDirectory.string()));
            if (!fs::exists(localDirectory)) {
                fs::create_directories(localDirectory);
            }
            if (*(remotePath.end() - 1) == '/') {
                LOG_DEBUG(LoadableDevice_impl, "There")
                _loadTree(fs, remotePath + std::string("/") + directoryName, localPath, fileKey);
            } else {
                LOG_DEBUG(LoadableDevice_impl, "Here")
                _loadTree(fs, remotePath + std::string("/"), localDirectory, fileKey);
            }
        } else {
        }
    }

}




void LoadableDevice_impl::_deleteTree(const std::string &fileKey)
{

    LOG_DEBUG(LoadableDevice_impl, "_deleteTree " << fileKey)
    std::pair<copiedFiles_type::iterator, copiedFiles_type::iterator> p = copiedFiles.equal_range(fileKey);

    // perform the search backwards (so that directories are emptied before they're deleted)
    for ( ; p.first != p.second; ) {
        --p.second;
        if (fs::is_directory(((*p.second).second).c_str())) {
            if (!fs::is_empty(((*p.second).second).c_str())) {
                LOG_TRACE(LoadableDevice_impl, "Not removing " << ((*p.second).second).c_str() << " - not empty!")
                continue;
            }
        }
        LOG_TRACE(LoadableDevice_impl, "removing " << ((*p.second).second).c_str())
        fs::remove(((*p.second).second).c_str());
    }

    // need to remove entries from list....
    //
    copiedFiles.erase(fileKey);

}

void LoadableDevice_impl::_copyFile(CF::FileSystem_ptr fs, const std::string &remotePath,  const std::string &localPath, const std::string &fileKey)
{

    CF::File_var fileToLoad = fs->open(remotePath.c_str(), true);
    if ( CORBA::is_nil(fileToLoad) ) {
      LOG_ERROR(LoadableDevice_impl, "Unable to open remote file: " << remotePath );
      throw(CF::FileException());
    }

    size_t blockTransferSize = ossie::corba::giopMaxMsgSize() * 0.95;
    size_t toRead;
    CF::OctetSequence_var data;

    std::fstream fileStream;
    std::ios_base::openmode mode;
    mode = std::ios::out | std::ios::trunc;
    fileStream.open(localPath.c_str(), mode);
    if (!fileStream.is_open()) {
        LOG_ERROR(LoadableDevice_impl, "Local file " << localPath << " did not open succesfully.")
    } else {
        LOG_DEBUG(LoadableDevice_impl, "Local file " << localPath << " opened succesfully.")
    }

    copiedFiles.insert(copiedFiles_type::value_type(fileKey, localPath));
    size_t fileSize = fileToLoad->sizeOf();
    bool fe = false;
    std::ostringstream eout;
    try {
      while (fileSize > 0) {
        toRead    = std::min(fileSize, blockTransferSize);
        fileSize -= toRead;
        try {
          fileToLoad->read(data, toRead);
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
          fileStream.write((const char*)data->get_buffer(), data->length());
        } catch ( std::exception& ex ) {
          eout << "The following standard exception occurred: "<<ex.what()<<" While \"dstFile->write\"";
          throw(CF::FileException());
        } catch( ... ) {
          eout << "[FileManager::copy] \"dstFile->write\" failed with Unknown Exception\n";
          throw(CF::FileException());
        }
      }
    }
    catch(...) {
      LOG_ERROR(LoadableDevice_impl, eout.str())
      fe = true;
    }

    // close the files
    try {
      try {
        fileToLoad->close();
      } catch ( std::exception& ex ) {
        eout << "The following standard exception occurred: "<<ex.what()<<" While \"srcFile->close\"";
        throw(CF::FileException());
      } catch ( CF::File::IOException &ex ) {
        eout << "File Exception occured, \"srcFile->close\"";
        throw;
      } catch ( CF::FileException &ex ) {
        eout << "File Exception occured, \"srcFile->close\"";
        throw;
      } catch ( CORBA::Exception& ex ) {
        eout << "The following CORBA exception occurred: "<<ex._name()<<" While \"srcFile->close\"";
        throw(CF::FileException());
      } catch( ... ) {
        eout << "[FileManager::copy] \"srcFile->close\" failed with Unknown Exception\n";
        throw(CF::FileException());
      }
    }
    catch(...) {
      LOG_ERROR(LoadableDevice_impl, eout.str())
      fe = true;
    }
      fileStream.close();

    if ( fe ) {
      throw(CF::FileException());
    }
}
      

void
LoadableDevice_impl::incrementFile (std::string fileName)
{
    if (loadedFiles.count(fileName) == 0) {
        loadedFiles[fileName] = 1;
    } else {
        loadedFiles[fileName]++;
    }
}


/* unload *****************************************************************************************
    - removes one instance of a file from the device
************************************************************************************************ */
void
LoadableDevice_impl::unload (const char* fileName)
throw (CORBA::SystemException, CF::Device::InvalidState, CF::InvalidFileName)
{

    LOG_DEBUG(LoadableDevice_impl, "Unload called for " << fileName)

// verify that the device is in a valid state for loading
    if (isLocked () || isDisabled ()) {
        throw (CF::Device::
               InvalidState
               ("Cannot unload. System is either LOCKED or DISABLED."));
    }

    std::string workingFileName = fileName;

    if (loadedFiles.count(workingFileName) == 0) {
        throw (CF::InvalidFileName(CF::CF_ENOENT, workingFileName.c_str()));
    }
    if (loadedFiles[workingFileName] == 1) {
        // delete the file
        if (fileTypeTable.count(workingFileName) == 0) {
            // no record as to what the file is (yet still clearly valid)
            LOG_WARN(LoadableDevice_impl, "Unload called on a file that does not exist (" << fileName << ")")
            return;
        }
        if (fileTypeTable[workingFileName] == CF::FileSystem::PLAIN) {
            // the loaded entity is a file
            std::string relativeFileName = workingFileName;
            if (workingFileName[0] == '/') {
                relativeFileName = workingFileName.substr(1);
            }
            remove(relativeFileName.c_str());
            LOG_DEBUG(LoadableDevice_impl, "Unload ############## (" << fileName << ")")            
        } else if (fileTypeTable[workingFileName] == CF::FileSystem::DIRECTORY) {
            _deleteTree(std::string(fileName));
        } else if (fileTypeTable[workingFileName] == CF::FileSystem::FILE_SYSTEM) {
        } else {
        }
        fileTypeTable.erase(workingFileName);
    }

    // decrement the list entry counter for this file
    decrementFile (workingFileName);

}

void
LoadableDevice_impl::decrementFile (std::string fileName)
{
    if (loadedFiles.count(fileName) == 0) {
        throw (CF::InvalidFileName (CF::CF_ENOENT, fileName.c_str()));
    } else {
        loadedFiles[fileName]--;
    }
    if (loadedFiles[fileName] == 0) {
        loadedFiles.erase(fileName);
    }
}


/* isFileLoaded ***********************************************************************************
    - looks for a given file name in the loaded files list
************************************************************************************************ */
bool LoadableDevice_impl::isFileLoaded (const char* fileName)
{
    if (loadedFiles.count(fileName) == 0) {
        // see if it's really a directory that it's looking for
        std::string name(fileName);
        size_t last_slash = name.find_last_of("/");
        if (last_slash == std::string::npos)
            { return false; }
        std::string subName = name.substr(0, last_slash);
        if (loadedFiles.count(subName) == 0) {
            return false;
        } else {
            return true;
        }
    } else {
        return true;
    }
}


void LoadableDevice_impl ::configure (const CF::Properties& capacities)
throw (CF::PropertySet::PartialConfiguration, CF::PropertySet::
       InvalidConfiguration, CORBA::SystemException)
{
    Device_impl::configure(capacities);
}
