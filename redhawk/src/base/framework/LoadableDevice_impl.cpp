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
/*
 * STL algorithm predicate for detecting empty string.
 */
struct IsEmptyString
{
    bool operator()( const std::string& str ) const
    {
        return str.empty();
    }
};

EnvironmentPathParser::EnvironmentPathParser( const std::string& path )
{
    from_string(path);
}

EnvironmentPathParser::EnvironmentPathParser( const char* path )
{
    if( path )
    {
        from_string(path);
    }
}

void EnvironmentPathParser::merge_front( const std::string& path )
{
    if( std::find(paths.begin(), paths.end(), path) == paths.end() )
    {
        paths.insert( paths.begin(), path );
    }
}

void EnvironmentPathParser::from_string( const std::string& path )
{
    std::istringstream _path(path);
    std::string path_item;
    while(std::getline(_path,path_item,':')) {
        paths.push_back(path_item);
    }
    strip_empty_paths();
}

void  EnvironmentPathParser::strip_empty_paths()
{
    paths.erase( std::remove_if(paths.begin(), paths.end(), IsEmptyString()), paths.end() );
}

std::string EnvironmentPathParser::to_string() const
{
    std::string ret_str = "";
    for (std::size_t path_idx=0; path_idx!= paths.size(); path_idx++) {
        ret_str.append(paths[path_idx]+":");
    }
    if (ret_str.size()!=0)
        ret_str.erase(ret_str.size()-1);
    return ret_str;
}

/* LoadableDevice_impl ****************************************************************************
    - constructor 1: no capacities defined
************************************************************************************************ */
LoadableDevice_impl::LoadableDevice_impl (char* devMgr_ior, char* id, char* lbl, char* sftwrPrfl):
    Device_impl (devMgr_ior, id, lbl, sftwrPrfl)
{
  _init();

}


/* LoadableDevice_impl ****************************************************************************
    - constructor 2: capacities defined
************************************************************************************************ */
LoadableDevice_impl::LoadableDevice_impl (char* devMgr_ior, char* id, char* lbl, char* sftwrPrfl,
                                          CF::Properties capacities):
    Device_impl (devMgr_ior, id, lbl, sftwrPrfl, capacities)
{

  _init();
}

/* LoadableDevice_impl ****************************************************************************
    - constructor 1: no capacities defined
************************************************************************************************ */
LoadableDevice_impl::LoadableDevice_impl (char* devMgr_ior, char* id, char* lbl, char* sftwrPrfl, 
                                          char* composite_ior):
    Device_impl (devMgr_ior, id, lbl, sftwrPrfl, composite_ior)
{
  _init();
}


/* LoadableDevice_impl ****************************************************************************
    - constructor 2: capacities defined
************************************************************************************************ */
LoadableDevice_impl::LoadableDevice_impl (char* devMgr_ior, char* id, char* lbl, char* sftwrPrfl,
                                          CF::Properties capacities, char* composite_ior):
    Device_impl (devMgr_ior, id, lbl, sftwrPrfl, capacities, composite_ior)
{
  _init();
}



void LoadableDevice_impl::_init () {

  sharedPkgs.clear();

  transferSize=-1;
  addProperty(transferSize,
              -1,
              "LoadableDevice::transfer_size",
              "LoadableDevice::transfer_size",
              "readwrite",
              "bytes",
              "external",
              "configure");

  // Default to the current working directory
  cacheDirectory = ossie::getCurrentDirName();
  setLogger(this->_baseLog->getChildLogger("LoadableDevice", "system"));
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

void LoadableDevice_impl::setLogger(rh_logger::LoggerPtr logptr)
{
    _loadabledeviceLog = logptr;
}

void LoadableDevice_impl::update_ld_library_path (CF::FileSystem_ptr fs, const char* fileName, CF::LoadableDevice::LoadType loadKind) throw (CORBA::SystemException, CF::Device::InvalidState, CF::LoadableDevice::InvalidLoadKind, CF::InvalidFileName, CF::LoadableDevice::LoadFail)
{
	// Update environment to use newly-loaded library
	if (loadKind == CF::LoadableDevice::SHARED_LIBRARY)
	{
        merge_front_environment_path("LD_LIBRARY_PATH", ossie::getCurrentDirName() + fileName );
	}
}

void LoadableDevice_impl::update_octave_path (CF::FileSystem_ptr fs, const char* fileName, CF::LoadableDevice::LoadType loadKind) throw (CORBA::SystemException, CF::Device::InvalidState, CF::LoadableDevice::InvalidLoadKind, CF::InvalidFileName, CF::LoadableDevice::LoadFail)
{
	// Update environment to use newly-loaded library
	if (loadKind == CF::LoadableDevice::SHARED_LIBRARY) 
	{
        merge_front_environment_path("OCTAVE_PATH", ossie::getCurrentDirName() + fileName );
	}
}

void LoadableDevice_impl::merge_front_environment_path( const char* environment_variable, const std::string& path ) const
{
    EnvironmentPathParser parser( getenv(environment_variable) );
    parser.merge_front( path );
    setenv(environment_variable, parser.to_string().c_str(), 1);
    RH_DEBUG(_loadabledeviceLog, "Updated environment path " << environment_variable << ": " << parser.to_string() );
}

void
LoadableDevice_impl::load (CF::FileSystem_ptr fs, const char* fileName,
                           CF::LoadableDevice::LoadType loadKind)
throw (CORBA::SystemException, CF::Device::InvalidState,
       CF::LoadableDevice::InvalidLoadKind, CF::InvalidFileName,
       CF::LoadableDevice::LoadFail)
{
    boost::recursive_mutex::scoped_lock lock(load_execute_lock);
    try
    {
        do_load(fs, fileName, loadKind);
        update_ld_library_path(fs, fileName, loadKind);
        update_octave_path(fs, fileName, loadKind);
    }
    catch( CF::File::IOException & e ) {
        std::stringstream errstr;
        errstr << "IO Exception occurred, file: " << fileName;
        RH_ERROR(_loadabledeviceLog, __FUNCTION__ << ": " << errstr.str() );
        throw CF::LoadableDevice::LoadFail(e.errorNumber, e.msg);
    }
    catch( CF::FileException & e ) {
        std::stringstream errstr;
        errstr << "File Exception occurred, file: " << fileName;
        RH_ERROR(_loadabledeviceLog, __FUNCTION__ << ": " << errstr.str() );
        throw CF::LoadableDevice::LoadFail(e.errorNumber, e.msg);
    }
    catch( const boost::thread_resource_error& e )
    {
        std::stringstream errstr;
        errstr << "Error acquiring lock (errno=" << e.native_error() << " msg=\"" << e.what() << "\")";
        RH_ERROR(_loadabledeviceLog, __FUNCTION__ << ": " << errstr.str() );
        throw CF::Device::InvalidState(errstr.str().c_str());
    }
}

/* load *******************************************************************************************
    - loads a file into the device
************************************************************************************************ */
void
LoadableDevice_impl::do_load (CF::FileSystem_ptr fs, const char* fileName,
                           CF::LoadableDevice::LoadType loadKind)
throw (CORBA::SystemException, CF::Device::InvalidState,
       CF::LoadableDevice::InvalidLoadKind, CF::InvalidFileName,
       CF::LoadableDevice::LoadFail,  CF::FileException )
{
    RH_DEBUG(_loadabledeviceLog, "load " << fileName)

// verify that the device is in a valid state for loading
    if (!isUnlocked () || isDisabled ()) {
        RH_ERROR(_loadabledeviceLog, "Cannot load. System is either LOCKED, SHUTTING DOWN or DISABLED.")
        RH_DEBUG(_loadabledeviceLog, "Unlocked: " << isUnlocked ())
        RH_DEBUG(_loadabledeviceLog, "isDisabled: " << isDisabled ())
        throw (CF::Device::
               InvalidState
               ("Cannot load. System is either LOCKED, SHUTTING DOWN or DISABLED."));
    }

    RH_DEBUG(_loadabledeviceLog, "It's not locked and not disabled")

// verify that the loadKind is supported (only executable is supported by this version)
    if ((loadKind != CF::LoadableDevice::EXECUTABLE) && (loadKind != CF::LoadableDevice::SHARED_LIBRARY)) {
        RH_ERROR(_loadabledeviceLog, "It's not CF::LoadableDevice::EXECUTABLE or CF::LoadableDevice::SHARED_LIBRARY")
        throw CF::LoadableDevice::InvalidLoadKind ();
    }

    std::string workingFileName = fileName;

// verify the file name exists in the file system and get a pointer to it
// NOTE: in this context, this step is redundant; the 'installApplication' method
// already performs this existence check
    try {
        if (!fs->exists (workingFileName.c_str())) {
            RH_ERROR(_loadabledeviceLog, "File " << workingFileName << " does not exist")
            throw (CF::InvalidFileName (CF::CF_ENOENT, "Cannot load. File name is invalid."));
        }
    } catch ( ... ) {
        RH_ERROR(_loadabledeviceLog, "Exception raised when calling the file system: " << workingFileName  );
        throw;
    }

    RH_DEBUG(_loadabledeviceLog, "Cleaning name " << fileName)
    // Get rid of all the directories in the given name (if any)
    CF::FileSystem::FileInformationSequence_var contents = fs->list(workingFileName.c_str());
    std::string simpleName;
    std::string::size_type pos = workingFileName.find_last_of("/");
    if (pos == std::string::npos) {
        simpleName = workingFileName;
    } else {
        simpleName = workingFileName.substr(pos + 1);
    }

    RH_DEBUG(_loadabledeviceLog, "Is " << fileName << " a directory?")
    CF::FileSystem::FileInformationType* fileInfo = 0;
    for (unsigned int i = 0; i < contents->length(); i++) {
        RH_DEBUG(_loadabledeviceLog, "comparing " << simpleName << " and " << contents[i].name)
        if (!simpleName.compare(contents[i].name)) {
            fileInfo = &contents[i];
            break;
        }
    }
    if (!fileInfo) {
        RH_ERROR(_loadabledeviceLog, "The file system couldn't find " << fileName)
        throw (CF::InvalidFileName (CF::CF_ENOENT, "Cannot load. File name is invalid."));
    }

    std::string relativeFileName = "";
    if (loadedFiles.count(workingFileName)) {
        relativeFileName = workingFileName;
        if (workingFileName[0] == '/') {
            relativeFileName = workingFileName.substr(1);
        }
        bool fexists = false;
        try {
            fexists = fs::exists(relativeFileName);
        }
        catch(const fs::filesystem_error& ex){
            throw CF::InvalidFileName(CF::CF_NOTSET, ex.what());
        }
        if (fexists) {
            bool reload = false;
            if (fileInfo->kind == CF::FileSystem::DIRECTORY) {
                reload = !this->_treeIntact(std::string(fileName));
            }
            if (!reload) {
                // Check if the remote file is newer than the local file, and if so, update the file
                // in the cache. No consideration is given to clock sync differences between systems.
                time_t remoteModifiedTime = getModTime(fileInfo->fileProperties);
                time_t cacheModifiedTime = cacheTimestamps[workingFileName];
                RH_TRACE(_loadabledeviceLog, "Remote modified: " << remoteModifiedTime << " Local modified: " << cacheModifiedTime);
                if (remoteModifiedTime > cacheModifiedTime) {
                    RH_DEBUG(_loadabledeviceLog, "Remote file is newer than local file");
                } else {
                    RH_DEBUG(_loadabledeviceLog, "File exists in cache");
                    incrementFile(workingFileName);
                    return;
                }
            }
        }
    }

    if (fileInfo->kind != CF::FileSystem::DIRECTORY) {
        // The target file is a file
      RH_DEBUG(_loadabledeviceLog, "Loading the file " << fileName);

        std::fstream fileStream;
        std::ios_base::openmode mode;
        mode = std::ios::out;
        std::string _relativeFileName = workingFileName;
        if (workingFileName[0] == '/') {
            _relativeFileName = workingFileName.substr(1);
        }
        relativeFileName = prependCacheIfAvailable(_relativeFileName);
        
        // Create a local directory to copy the file to
        fs::path parentDir;
        if (relativeFileName == _relativeFileName) {
            parentDir = fs::path(relativeFileName).parent_path().relative_path();
        } else {
            parentDir = fs::path(relativeFileName).parent_path();
        }
        try {
          if ( !parentDir.string().empty() && fs::create_directories(parentDir)) {
                RH_DEBUG(_loadabledeviceLog, "Created parent directory " << parentDir.string());
            }
        } catch (const fs::filesystem_error& ex) {
            RH_ERROR(_loadabledeviceLog, "Unable to create parent directory " << parentDir.string() << ": " << ex.what());
            throw CF::LoadableDevice::LoadFail(CF::CF_NOTSET, "Device SDR cache write error");
        }

        // copy the file
        RH_DEBUG(_loadabledeviceLog, "Copying " << workingFileName << " to the device's cache")
        fileStream.open(relativeFileName.c_str(), mode);
        bool text_file_busy = false;
        if (!fileStream.is_open()) {
            if (errno == ETXTBSY) {
                text_file_busy = true;
            } else {
                RH_ERROR(_loadabledeviceLog, "Could not create file " << relativeFileName.c_str());
                throw CF::LoadableDevice::LoadFail(CF::CF_NOTSET, "Device SDR cache write error");
            }
        }
        if (text_file_busy) {
            std::stringstream new_filename;
            new_filename << relativeFileName << "_timestamp_" << cacheTimestamps[workingFileName];
            rename(relativeFileName.c_str(), new_filename.str().c_str());
            duplicate_filenames[workingFileName].push_back(new_filename.str());
            cacheTimestamps[workingFileName] = getModTime(fileInfo->fileProperties);
            fileStream.open(relativeFileName.c_str(), mode);
            if (!fileStream.is_open()) {
                RH_ERROR(_loadabledeviceLog, "Could not create file " << relativeFileName.c_str());
                throw CF::LoadableDevice::LoadFail(CF::CF_NOTSET, "Device SDR cache write error");
            }
        }

        _copyFile( fs, workingFileName, relativeFileName, workingFileName );
        chmod(relativeFileName.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        fileTypeTable[workingFileName] = CF::FileSystem::PLAIN;
    } else {
        // The target file is a directory
        RH_DEBUG(_loadabledeviceLog, "Copying the file " << fileName << " as a directory to the cache as " << workingFileName)
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
    RH_DEBUG(_loadabledeviceLog, "Incrementing " << workingFileName << " vs " << fileName)
    incrementFile (workingFileName);
    if (cacheTimestamps.count(workingFileName) == 0) {
        cacheTimestamps[workingFileName] = getModTime(fileInfo->fileProperties);
    }

    // Update environment to use newly-loaded library
    if (loadKind == CF::LoadableDevice::SHARED_LIBRARY) {
        sharedLibraryStorage env_changes;
        env_changes.setFilename(workingFileName);
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
            env_changes.addModification("LD_LIBRARY_PATH", additionalPath);
            CLibrary = true;
        }
        // Check to see if it's a Python module
        if (!CLibrary) {
            currentPath = ossie::getCurrentDirName();
            std::string::size_type lastSlash = relativeFileName.find_last_of("/");
            if (lastSlash == std::string::npos) { // there are no slashes in the name
                std::string fileOrDirectoryName = relativeFileName;
                std::string relativePath = "";
                std::string::size_type iext = fileOrDirectoryName.find_last_of(".");
                if (iext != std::string::npos) {
                    std::string extension = fileOrDirectoryName.substr(iext);
                    if ((extension == ".py") || (extension == ".pyc")) {
                        fileOrDirectoryName.erase(iext);
                    }
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
                    env_changes.addModification("PYTHONPATH", additionalPath);
                    PythonPackage = true;
                }
                chdir(currentPath.c_str());
            } else {
                // grab the last string after the slash
                std::string fileOrDirectoryName = "";
                std::string relativePath = "";
                fileOrDirectoryName.assign(relativeFileName, lastSlash+1, relativeFileName.size()-lastSlash);
                std::string::size_type iext = fileOrDirectoryName.find_last_of(".");
                if (iext != std::string::npos) {
                    std::string extension = fileOrDirectoryName.substr(iext);
                    if ((extension == ".py") || (extension == ".pyc")) {
                        fileOrDirectoryName.erase(iext);
                    }
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
                        RH_DEBUG(_loadabledeviceLog, "cmd= " << command << 
                                " relativeFileName: " << relativeFileName <<
                                " relativePath: " << relativePath);

                        // The import worked
                        std::string additionalPath = "";
                        if (fileInfo->kind == CF::FileSystem::DIRECTORY) {
                            additionalPath = currentPath+std::string("/")+relativePath;
                        } else {
                            additionalPath = currentPath+std::string("/")+relativePath;
                        }
                        env_changes.addModification("PYTHONPATH", additionalPath);
                        RH_DEBUG(_loadabledeviceLog, "Adding " << additionalPath << " to PYTHONPATH");
                        PythonPackage = true;
                    }
                }
                chdir(currentPath.c_str());
            }
        }

        // Check to see if it's a Java package
        if (!CLibrary and !PythonPackage) {
          int retval = ossie::helpers::is_jarfile( relativeFileName );
          if ( retval == 0 ) {
            currentPath = ossie::getCurrentDirName();
            std::string additionalPath = currentPath+std::string("/")+relativeFileName;
            env_changes.addModification("CLASSPATH", additionalPath);
            JavaJar = true;
          }
        }
    
        // It doesn't match anything, assume that it's a set of libraries
        if (!(CLibrary || PythonPackage || JavaJar)) {
            const std::string additionalPath = currentPath+std::string("/")+relativeFileName;
            env_changes.addModification("LD_LIBRARY_PATH", additionalPath);
            env_changes.addModification("OCTAVE_PATH", additionalPath);
        }
        update_path(env_changes);
        sharedPkgs[env_changes.filename] = env_changes;
    }
}

void LoadableDevice_impl::update_selected_paths(std::vector<sharedLibraryStorage> &paths) {
    for (std::vector<sharedLibraryStorage>::iterator pD = paths.begin();pD!=paths.end();pD++) {
        if (this->sharedPkgs.find(pD->filename) != this->sharedPkgs.end()) {
            update_path(*pD);
        }
    };
}

void LoadableDevice_impl::update_path(sharedLibraryStorage &packageDescription) {
    for (std::vector<std::pair<std::string,std::string> >::iterator mod = packageDescription.modifications.begin();mod!=packageDescription.modifications.end();mod++) {
        std::string current_path;
        if (getenv(mod->first.c_str())) {
            current_path = getenv(mod->first.c_str());
        } else {
            current_path.clear();
        }
        // Make sure that the current path is not already in the current path
        if (!checkPath(current_path, mod->second)) {
            std::string path_mod = mod->second;
            if (!current_path.empty()) {
                path_mod += ":" + current_path;
            }
            setenv(mod->first.c_str(), path_mod.c_str(), 1);
        }
    }
}

void LoadableDevice_impl::_loadTree(CF::FileSystem_ptr fs, std::string remotePath, fs::path& localPath, std::string fileKey)
{

    RH_DEBUG(_loadabledeviceLog, "_loadTree " << remotePath << " " << localPath)
    fs::path mod_localPath = prependCacheIfAvailable(localPath.string());

    CF::FileSystem::FileInformationSequence_var fis = fs->list(remotePath.c_str());
    for (unsigned int i = 0; i < fis->length(); i++) {
      if (fis[i].kind == CF::FileSystem::PLAIN) {
            std::string fileName(fis[i].name);
            fs::path localFile(mod_localPath / fileName);
            if (*(remotePath.end() - 1) == '/') {
                RH_DEBUG(_loadabledeviceLog, "_copyFile " << remotePath + fileName << " " << localFile)
                _copyFile(fs, remotePath + fileName, localFile.string(), fileKey);
            } else {
                RH_DEBUG(_loadabledeviceLog, "_copyFile " << remotePath << " " << localFile)
                _copyFile(fs, remotePath, localFile.string(), fileKey);
            }
            const redhawk::PropertyMap& fileprops = redhawk::PropertyMap::cast(fis[i].fileProperties);
            if (fileprops.get("EXECUTABLE", false).toBoolean()) {
                chmod(localFile.string().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            }
        } else if (fis[i].kind == CF::FileSystem::DIRECTORY) {
            std::string directoryName(fis[i].name);
            fs::path localDirectory(mod_localPath / directoryName);
            RH_DEBUG(_loadabledeviceLog, "Making directory " << directoryName << " in " << mod_localPath)
            copiedFiles.insert(copiedFiles_type::value_type(fileKey, localDirectory.string()));
            bool dexists = false;
            try {
                dexists = fs::exists(localDirectory);
            } catch(const fs::filesystem_error& ex) {
                std::string tmsg("Device SDR cache write error, ");
                tmsg += ex.what();
                throw CF::LoadableDevice::LoadFail(CF::CF_NOTSET, tmsg.c_str());
            }
            if (!dexists) {
                fs::create_directories(localDirectory);
            }
            if (*(remotePath.end() - 1) == '/') {
                RH_DEBUG(_loadabledeviceLog, "There")
                _loadTree(fs, remotePath + std::string("/") + directoryName, mod_localPath, fileKey);
            } else {
                RH_DEBUG(_loadabledeviceLog, "Here")
                _loadTree(fs, remotePath + std::string("/"), localDirectory, fileKey);
            }
        } else {
        }
    }

}

void LoadableDevice_impl::_deleteTree(const std::string &fileKey)
{

    RH_DEBUG(_loadabledeviceLog, "_deleteTree " << fileKey)
    std::pair<copiedFiles_type::iterator, copiedFiles_type::iterator> p = copiedFiles.equal_range(fileKey);

    // perform the search backwards (so that directories are emptied before they're deleted)
    for ( ; p.first != p.second; ) {
        --p.second;
        if (fs::is_directory(((*p.second).second).c_str())) {
            if (!fs::is_empty(((*p.second).second).c_str())) {
                RH_TRACE(_loadabledeviceLog, "Not removing " << ((*p.second).second).c_str() << " - not empty!")
                continue;
            }
        }
        RH_TRACE(_loadabledeviceLog, "removing " << ((*p.second).second).c_str())
        fs::remove(((*p.second).second).c_str());
    }

    copiedFiles.erase(fileKey);
}

bool LoadableDevice_impl::_treeIntact(const std::string &fileKey)
{

    RH_DEBUG(_loadabledeviceLog, "_treeIntact " << fileKey)
    std::pair<copiedFiles_type::iterator, copiedFiles_type::iterator> p = copiedFiles.equal_range(fileKey);

    for ( ; p.first != p.second; ) {
        --p.second;
        bool fexists=false;
        try {
            fexists=fs::exists(((*p.second).second).c_str());
        } catch(...){
            // if error occurs then exists == false
        }
        if (not fexists)
            return false;
    }
    return true;
}

std::string LoadableDevice_impl::prependCacheIfAvailable(const std::string &localPath) {
    std::string mod_localPath = localPath;
    if (this->getPropertyFromId("cacheDirectory")) {
        std::string cache_dir = ((StringProperty*)this->getPropertyFromId("cacheDirectory"))->getValue();
        if (!cache_dir.empty()) {
            if (localPath.find(cache_dir) == std::string::npos) {
                if (!cache_dir.compare(cache_dir.length()-1, 1, "/")) {
                    mod_localPath = cache_dir + mod_localPath;
                } else {
                    mod_localPath = cache_dir + std::string("/") + mod_localPath;
                }
            }
        }
    }
    return mod_localPath;
}

void LoadableDevice_impl::_copyFile(CF::FileSystem_ptr fs, const std::string &remotePath, const std::string &localPath, const std::string &fileKey)
{
    std::string mod_localPath(prependCacheIfAvailable(localPath));
    CF::File_var fileToLoad = CF::File::_nil();
    try {
       fileToLoad= fs->open(remotePath.c_str(), true);
       if ( CORBA::is_nil(fileToLoad) ) {
           std::string msg("Unable to open remote file: ");
           msg += remotePath;
           RH_ERROR(_loadabledeviceLog, msg);
           throw CF::LoadableDevice::LoadFail( CF::CF_NOTSET, msg.c_str());
       }
    }
       catch(const CF::FileException &ex) {
           std::string msg("Unable to access remote file: ");
           msg += remotePath;
           throw CF::LoadableDevice::LoadFail( CF::CF_NOTSET, msg.c_str() );
    }
    catch(...) {
        std::string msg("Unable to open remote file: ");
        msg += remotePath;
        throw CF::LoadableDevice::LoadFail( CF::CF_NOTSET, msg.c_str() );
    }

    if ( transferSize < 1 ) 
        transferSize = ossie::corba::giopMaxMsgSize() * 0.95;
    std::size_t blockTransferSize = transferSize;
    std::size_t toRead=0;
    std::fstream fileStream;
    std::ios_base::openmode mode;
    mode = std::ios::out | std::ios::trunc;
    fileStream.open(mod_localPath.c_str(), mode);
    if (!fileStream.is_open()) {
        RH_ERROR(_loadabledeviceLog, "Local file " << mod_localPath << " did not open succesfully.")
    } else {
        RH_DEBUG(_loadabledeviceLog, "Local file " << mod_localPath << " opened succesfully.")
    }

    copiedFiles.insert(copiedFiles_type::value_type(fileKey, mod_localPath));

    std::size_t fileSize = fileToLoad->sizeOf();
    bool fe=false;
    try  {
      while (fileSize > 0) {
      CF::OctetSequence_var data;
      toRead = std::min(fileSize, blockTransferSize);
      fileSize -= toRead;

      //RH_TRACE(_loadabledeviceLog, "READ Local file " << mod_localPath << " length:" << toRead << " filesize/bts " << fileSize << "/" << blockTransferSize );
      try {
        fileToLoad->read(data, toRead);
        fileStream.write((const char*)data->get_buffer(), data->length());
      }
      catch ( CF::File::IOException &e ) {
        RH_WARN(_loadabledeviceLog, "READ Local file exception, " << ossie::corba::returnString(e.msg) );
        throw;
      }
        
      }
    }catch(...) {
      fe=true;
    }

    // need to close the files...
    try {
      fileToLoad->close();
    }
    catch(...) {
      RH_ERROR(_loadabledeviceLog, "Closing remote file encountered exception, file:" << remotePath );
      fe=true;
    }

    fileStream.close();

    if (fe) {
      throw CF::FileException();
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

void
LoadableDevice_impl::unload (const char* fileName)
throw (CORBA::SystemException, CF::Device::InvalidState, CF::InvalidFileName)
{
    boost::recursive_mutex::scoped_lock lock(load_execute_lock);
    try
    {
        do_unload(fileName);
    }
    catch( const boost::thread_resource_error& e )
    {
        std::stringstream errstr;
        errstr << "Error acquiring lock (errno=" << e.native_error() << " msg=\"" << e.what() << "\")";
        RH_ERROR(_loadabledeviceLog, __FUNCTION__ << ": " << errstr.str() );
        throw CF::Device::InvalidState(errstr.str().c_str());
    }
}
/* unload *****************************************************************************************
    - removes one instance of a file from the device
************************************************************************************************ */
void
LoadableDevice_impl::do_unload (const char* fileName)
throw (CORBA::SystemException, CF::Device::InvalidState, CF::InvalidFileName)
{

    RH_DEBUG(_loadabledeviceLog, "Unload called for " << fileName)

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
            RH_WARN(_loadabledeviceLog, "Unload called on a file that does not exist (" << fileName << ")")
            return;
        }
        if (fileTypeTable[workingFileName] == CF::FileSystem::PLAIN) {
            // the loaded entity is a file
            std::string relativeFileName = workingFileName;
            if (workingFileName[0] == '/') {
                relativeFileName = workingFileName.substr(1);
            }
            remove(relativeFileName.c_str());
            RH_DEBUG(_loadabledeviceLog, "Unload ############## (" << fileName << ")")
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

void LoadableDevice_impl::removeDuplicateFiles(std::string fileName) {
    if (duplicate_filenames.count(fileName) != 0) {
        for (unsigned int i=0; i<duplicate_filenames[fileName].size(); i++) {
            remove(duplicate_filenames[fileName][i].c_str());
        }
    }
    duplicate_filenames.erase(fileName);
}

void
LoadableDevice_impl::decrementFile (std::string fileName)
{
    if (loadedFiles.count(fileName) == 0) {
        if (cacheTimestamps.count(fileName) != 0) {
            cacheTimestamps.erase(fileName);
        }
        removeDuplicateFiles(fileName);
        throw (CF::InvalidFileName (CF::CF_ENOENT, fileName.c_str()));
    } else {
        loadedFiles[fileName]--;
    }
    if (loadedFiles[fileName] == 0) {
        if (this->sharedPkgs.find(fileName) != this->sharedPkgs.end()) {
            this->sharedPkgs.erase(fileName);
        }
        loadedFiles.erase(fileName);
        cacheTimestamps.erase(fileName);
        removeDuplicateFiles(fileName);
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


const std::string& LoadableDevice_impl::getCacheDirectory()
{
    if (this->getPropertyFromId("cacheDirectory")) {
        std::string cache_dir = ((StringProperty*)this->getPropertyFromId("cacheDirectory"))->getValue();
        if (!cache_dir.empty()) {
            if (cacheDirectory != cache_dir) {
                cacheDirectory = cache_dir;
            }
        }
    }
    return cacheDirectory;
}
