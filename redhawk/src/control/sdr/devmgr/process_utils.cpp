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

#include "process_utils.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <ossie/debug.h>


void  PackageMod::setPkgId(const std::string &pkgid ) {
    pkgId = pkgid;
};

// Define the path modification for the given filename
void  PackageMod::addModification(const std::string &_path_to_modify, const std::string &mod) {
    RH_NL_TRACE("PackageMod", "Adding To  Env: " << _path_to_modify << " Value :" << mod );
    modifications[_path_to_modify].push_back( mod);
};

    // Define the path modification for the given filename
void  PackageMod::addBinPath(const std::string &mod) {
    RH_NL_TRACE("PackageMod", "Adding To PATH Value :" << mod );
    modifications["PATH"].push_back(mod);
};

    // Define the path modification for the given filename
void  PackageMod::addLibPath(const std::string &mod) {
    RH_NL_TRACE("PackageMod", "Adding To LD_LIBRARY_PATH Value :" << mod );
    modifications["LD_LIBRARY_PATH"].push_back(mod);
};

    // Define the path modification for the given filename
void  PackageMod::addOctavePath(const std::string &mod) {
    RH_NL_TRACE("PackageMod", "Adding To OCTAVE_PATH Value :" << mod );
        modifications["OCTAVE_PATH"].push_back(mod);
    };

    // Define the path modification for the given filename
void  PackageMod::addPythonPath(const std::string &mod) {
    RH_NL_TRACE("PackageMod", "Adding To PYTHONPATH Value :" << mod );
    modifications["PYTHONPATH"].push_back( mod);
};

    // Define the path modification for the given filename
void  PackageMod::addJavaPath(const std::string &mod ) {
    RH_NL_TRACE("PackageMod", "Adding To CLASSPATH Value :" << mod );
    modifications["CLASSPATH"].push_back( mod );
};




ProcessEnvironment::ProcessEnvironment( bool autoRestore ) : 
    _restore(autoRestore) 
{
    _get_environ();
    addRestore("PATH");
    addRestore("LD_LIBRARY_PATH");
    addRestore("PYTHONPATH");
    addRestore("CLASSPATH");
    addRestore("OCTAVE_PATH");
}

ProcessEnvironment::~ProcessEnvironment()  {
    if  (_restore) restore();
}
void ProcessEnvironment::_get_environ( ) {
    
    extern char **environ;
    char **eiter = environ;
    for ( ; *eiter != 0; eiter++ ) {
        std::string kv(*eiter);
        std::string k;
        std::string v;
        std::size_t pos = kv.find_first_of("=");
        if ( pos != std::string::npos ) {
            k = kv.substr(0,pos);
            v = kv.substr(pos+1,std::string::npos);
        }
        _saveEnv[k] = v;
        _environ[k] = v;
    }

}


bool ProcessEnvironment::checkPath(const std::string& var, const std::string& pattern, char delim )
{
    
    if ( _environ.find(var) == _environ.end() ) {
        return false;
    }

    std::string envpath = getenv(var);

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


void ProcessEnvironment::addRestore( const std::string &var  ) {
    if ( std::find( _restoreList.begin(),_restoreList.end(), var ) == _restoreList.end() )  {
        _restoreList.push_back( var );
        if ( ::getenv(var.c_str()) == 0  ){
            _unsetList.push_back(var);
            _environ[var]="";
        }
        else {
            _environ[var]  = ::getenv(var.c_str());
            _saveEnv[var]  = ::getenv(var.c_str());
        }
    }
}

void ProcessEnvironment::restore() {
    KeyList::iterator iter=_restoreList.begin();
    for ( ; iter != _restoreList.end(); iter++ ) {
        std::string key = *iter;
        if ( _saveEnv.find(key)  != _saveEnv.end() ) {
            RH_NL_TRACE("ProcessEnviron", "Restoring EnvVar:"  << key << " To: " << _saveEnv[key] );
            ::setenv(key.c_str(), _saveEnv[key].c_str(), 1);
        }
    }
    iter = _unsetList.begin();
    for ( ; iter != _unsetList.end(); iter++ ) {
        ::unsetenv( iter->c_str() );
    }
}


void ProcessEnvironment::set() {
    Variables::iterator iter = _environ.begin();
    for ( ; iter != _environ.end(); iter++ ) {
        ::setenv( iter->first.c_str(), iter->second.c_str(), 1);
    }
}

void ProcessEnvironment::apply() {
    set();
}
