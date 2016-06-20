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

#include <elf.h>

#include <boost/filesystem.hpp>

#include "ModuleLoader.h"

using namespace redhawk;
namespace fs = boost::filesystem;

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
    _handle(handle),
    _refcount(1)
{
}

const std::string& Module::path() const
{
    return _path;
}

void Module::load()
{
    ++_refcount;
}

void Module::unload()
{
    _refcount--;
    if (_refcount == 0) {
        ModuleLoader::Instance().unloaded(this);
        delete this;
    }
}

void Module::close()
{
    dlclose(_handle);
    _handle = 0;
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

PREPARE_LOGGING(ModuleLoader);

ModuleLoader::ModuleLoader()
{
}

ModuleLoader& ModuleLoader::Instance()
{
    static ModuleLoader loader;
    return loader;
}

Module* ModuleLoader::Load(const std::string& path, LoadBinding binding, LoadVisibility visibility)
{
    return Instance().load(path, binding, visibility);
}

Module* ModuleLoader::load(const std::string& path, LoadBinding binding, LoadVisibility visibility)
{
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
        module->load();
    }
    return module;
}

void ModuleLoader::unloaded(Module* module)
{
    LOG_DEBUG(ModuleLoader, "Unloading module " << module->path());
    _modules.erase(module->path());
    module->close();
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
    for (fs::directory_iterator entry = fs::directory_iterator(path); entry != fs::directory_iterator(); ++entry) {
        if (!fs::is_directory(entry->path())) {
            const std::string filename = entry->path().string();
            if (Module::IsLoadable(filename)) {
                load(filename, binding, visibility);
            }
        }
    }
}

void ModuleBundle::unload()
{
    // Unload modules in reverse order of loading
    for (ModuleList::reverse_iterator module = _modules.rbegin(); module != _modules.rend(); ++module) {
        (*module)->unload();
    }
    _modules.clear();
}
