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

#include <stdexcept>
#include <fstream>
#include <cstring>
#include <set>

#include <elf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/filesystem.hpp>

#if BOOST_FILESYSTEM_VERSION < 3
#define BOOST_PATH_STRING(x) (x)
#else
#define BOOST_PATH_STRING(x) (x).string()
#endif

#include "ModuleLoader.h"

using namespace redhawk;
namespace fs = boost::filesystem;

namespace {
    static std::string real_path(const std::string& filename)
    {
        char buf[PATH_MAX];
        ::realpath(filename.c_str(), &buf[0]);
        return std::string(buf);
    }
}

bool Module::IsLoadable(const std::string& filename)
{
    std::ifstream file(filename.c_str());
    if (!file) {
        return false;
    }
    char ident[EI_NIDENT];
    file.read(ident, EI_NIDENT);
    if (!file) {
        return false;
    }
    if (std::strncmp(ident, ELFMAG, SELFMAG)) {
        return false;
    }
    return true;
}

Module::Module(const std::string& path, void* handle) :
    _path(path),
    _name(BOOST_PATH_STRING(fs::path(path).filename())),
    _handle(handle),
    _refcount(1),
    _modtime(_getModTime())
{
}

const std::string& Module::path() const
{
    return _path;
}

const std::string& Module::name() const
{
    return _name;
}

bool Module::modified()
{
    return (_getModTime() != _modtime);
}

void Module::incref()
{
    LOG_TRACE(ModuleLoader, "Incrementing reference count for " << _path);
    ++_refcount;
}

bool Module::decref()
{
    LOG_TRACE(ModuleLoader, "Decrementing reference count for " << _path);
    _refcount--;
    return (_refcount > 0);
}

bool Module::close()
{
    if (dlclose(_handle)) {
        LOG_ERROR(ModuleLoader, "Error closing dynamic library " << _path << ": " << dlerror());
        return false;
    }

    // Try to reopen the library without loading it (RTLD_NOLOAD); if it is
    // successful, the library could not be unloaded, usually due to unique
    // symbols in the library (or a dependency)
    _handle = dlopen(_path.c_str(), RTLD_LAZY | RTLD_NOLOAD);
    return (_handle == 0);
}

void* Module::symbol(const std::string& name)
{
    // Clear error state; dlysm() can succeed but return null if the symbol is
    // supposed to be null, so checking dlerror() is the only reliable way to
    // catch failures
    dlerror();
    void *symbol = dlsym(_handle, name.c_str());
    if (const char* error = dlerror()) {
        throw std::runtime_error(error);
    }
    return symbol;
}

time_t Module::_getModTime()
{
    struct stat file_status;
    stat(_path.c_str(), &file_status);
    return file_status.st_mtime;
}

PREPARE_LOGGING(ModuleLoader);

ModuleLoader::ModuleLoader()
{
}

ModuleLoader& ModuleLoader::Instance()
{
    static ModuleLoader loader;
    return loader;
}

void ModuleLoader::Preload(const std::string& path, LoadBinding binding, LoadVisibility visibility)
{
    Instance().preload(path, binding, visibility);
}

Module* ModuleLoader::Load(const std::string& path, LoadBinding binding, LoadVisibility visibility)
{
    return Instance().load(path, binding, visibility);
}

void ModuleLoader::Unload(Module* module)
{
    Instance().unload(module);
}

void ModuleLoader::preload(const std::string& filename, LoadBinding binding, LoadVisibility visibility)
{
    // Preloading does not go through the normal module resolution that loading
    // does, because the use case is different:
    // * filename will usually be just a module name (e.g., "libbulkio-2.1.so")
    //   that is located via LD_LIBRARY_PATH
    // * preloaded libraries are not unloaded, so no need to reference count
    LOG_DEBUG(ModuleLoader, "Preloading module " << filename);
    int flags = binding | visibility;
    void* handle = dlopen(filename.c_str(), flags);
    if (!handle) {
        throw std::runtime_error(dlerror());
    }
}

Module* ModuleLoader::load(const std::string& filename, LoadBinding binding, LoadVisibility visibility)
{
    // Use the real path (symlinks followed, relative paths removed) to avoid
    // loading the same module under different aliases
    const std::string path = real_path(filename);
    Module* module = findModule(path);
    if (!module) {
        LOG_DEBUG(ModuleLoader, "Loading module " << path);
        int flags = binding | visibility;
        void* handle = dlopen(path.c_str(), flags);
        if (!handle) {
            throw std::runtime_error(dlerror());
        }
        module = new Module(path, handle);
        _modules[path] = module;
    } else {
        if (module->modified()) {
            throw std::runtime_error(path + ": library is already loaded but has been modified");
        }
        module->incref();
    }
    return module;
}

void ModuleLoader::unload(Module* module)
{
    if (module->decref()) {
        return;
    }

    LOG_DEBUG(ModuleLoader, "Unloading module " << module->path());
    if (module->close()) {
        // Close succeeded, clean up
        _modules.erase(module->path());
        delete module;
    } else {
        // Close failed, reset refcount to 1; this library must hang
        // around for the life of the process
        LOG_WARN(ModuleLoader, "Dynamic library " << module->name() << " could not be unloaded,"
                 << " some symbols may still be in use (" << module->path() << ")");
        module->incref();
    }
}

Module* ModuleLoader::findModule(const std::string& path)
{
    LoadTable::iterator module = _modules.find(path);
    if (module != _modules.end()) {
        return module->second;
    }
    return 0;
}


ModuleBundle::ModuleBundle(const std::string& name) :
    _name(name)
{
}

ModuleBundle::~ModuleBundle()
{
    unload();
}

const std::string& ModuleBundle::name() const
{
    return _name;
}

Module* ModuleBundle::load(const std::string& path, ModuleLoader::LoadBinding binding, ModuleLoader::LoadVisibility visibility)
{
    Module* module = ModuleLoader::Load(path, binding, visibility);
    _modules.push_back(module);
    return module;
}

void ModuleBundle::loadDirectory(const std::string& path, ModuleLoader::LoadBinding binding, ModuleLoader::LoadVisibility visibility)
{
    // Shared libraries often have multiple symbolic links to the same library
    // (e.g., libxxx.so->libxxx.so.0) that can lead to a lot of excess loads.
    // By remembering the real path of everything, we can load only unique
    // libraries.
    std::set<std::string> known_files;

    for (fs::directory_iterator entry = fs::directory_iterator(path); entry != fs::directory_iterator(); ++entry) {
        if (!fs::is_directory(entry->symlink_status())) {
            // Get the filename and run it through realpath to follow symlinks,
            // relative paths (e.g., "../") and extra slashes
            std::string filename = real_path(entry->path().string());

            // Skip files that we've already seen
            if (!known_files.insert(filename).second) {
                continue;
            }

            if (Module::IsLoadable(filename)) {
                load(filename, binding, visibility);
            }
        }
    }
}

void ModuleBundle::unload()
{
    // Unload modules in reverse order of loading
    std::for_each(_modules.rbegin(), _modules.rend(), &ModuleLoader::Unload);
    _modules.clear();
}
