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

#ifndef MODULELOADER_H
#define MODULELOADER_H

#include <string>
#include <map>
#include <vector>

#include <dlfcn.h>

#include <ossie/debug.h>

namespace redhawk {

    class Module {
    public:
        const std::string& name() const;
        const std::string& path() const;
        void* symbol(const std::string& name);

        bool modified();

        static bool IsLoadable(const std::string& path);

    private:
        Module(const std::string& path, void* handle);

        void incref();
        bool decref();

        bool close();

        time_t _getModTime();

        friend class ModuleLoader;

        const std::string _path;
        const std::string _name;
        void* _handle;
        int _refcount;
        time_t _modtime;
    };

    class ModuleLoader {

        ENABLE_LOGGING;

    public:
        enum LoadBinding {
            LAZY = RTLD_LAZY,
            NOW = RTLD_NOW
        };

        enum LoadVisibility {
            GLOBAL = RTLD_GLOBAL,
            LOCAL = RTLD_LOCAL
        };

        static void Preload(const std::string& path, LoadBinding binding, LoadVisibility visibility);

        static Module* Load(const std::string& path, LoadBinding binding, LoadVisibility visibility);
        static void Unload(Module* module);

    private:
        ModuleLoader();

        static ModuleLoader& Instance();

        void preload(const std::string& path, LoadBinding binding, LoadVisibility visibility);

        Module* load(const std::string& path, LoadBinding binding, LoadVisibility visibility);
        void unload(Module* module);

        Module* findModule(const std::string& path);

        friend class Module;

        typedef std::map<std::string,Module*> LoadTable;
        LoadTable _modules;
    };

    class ModuleBundle {
    public:
        ModuleBundle(const std::string& name);

        ~ModuleBundle();

        const std::string& name() const;

        Module* load(const std::string& path, ModuleLoader::LoadBinding binding, ModuleLoader::LoadVisibility visibility);
        void loadDirectory(const std::string& path, ModuleLoader::LoadBinding binding, ModuleLoader::LoadVisibility visibility);

        void unload();

    private:
        const std::string _name;
        typedef std::vector<Module*> ModuleList;
        ModuleList _modules;
    };

}

#endif // MODULELOADER_H
