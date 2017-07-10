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
#ifndef _PROCESS_UTILS_H
#define _PROCESS_UTILS_H
#include <stdlib.h>
#include <unistd.h>
#include <stdexcept>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <dirent.h>

class PackageMod {
public:
    typedef std::map< std::string, std::vector< std::string> > PathModifications;

    PackageMod() {};
 PackageMod( const std::string &pkgid  ) : pkgId(pkgid) {};

    // Set the pkg id
    void setPkgId(const std::string &id);
    const std::string &getPkgId() { return pkgId; };

    // Define the path modification for the given filename
    void addModification(const std::string &_path_to_modify, const std::string &mod);
    void addBinPath(const std::string &mod);
    void addLibPath(const std::string &mod);
    void addOctavePath(const std::string &mod);
    void addPythonPath(const std::string &mod);
    void addJavaPath(const std::string &mod );

    std::string pkgId;

    // Set of path modifications that apply
    PathModifications  modifications;
};

/* 
 *  This class stores the current LD_LIBRARY_PATH, PYTHONPATH, CLASSPATH, and OCTAVE_PATH
 *  when it is instantiated. It can then be used to restore the system settings
 */
class ProcessEnvironment {

 private:
    typedef std::vector< std::string > KeyList;

public:
    
    typedef std::map< std::string, std::string > Variables;
    
    ProcessEnvironment( bool autoRestore=true );

    virtual ~ProcessEnvironment();

    void addRestore( const std::string &var );

    void restore();
    
    void set();

    void apply();

    bool checkPath( const std::string &var, 
                    const std::string &pattern,
                    char delim=':' );

    void setenv( const std::string &k, const std::string &v ) {
        _environ[k] = v;
    }

    void unsetenv( const std::string &k ) {
        try {
            _environ.erase(k);
        }
        catch(...){
        }
    }

    const std::string &getenv( const std::string &var ) {
        if ( _environ.find(var) != _environ.end() ) {
            return _environ[var];
        }
        throw std::logic_error("index out of bounds");
    }

    const Variables &environ() {
        return _environ;
    }
    
 private:
    void _get_environ();

    Variables _saveEnv;
    Variables _environ;
    KeyList   _restoreList;
    KeyList   _unsetList;
    bool _restore;
};


#endif
