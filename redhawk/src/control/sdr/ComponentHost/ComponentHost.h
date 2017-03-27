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

#ifndef COMPONENTHOST_H
#define COMPONENTHOST_H

#include <ossie/Component.h>
#include <ossie/ExecutorService.h>

#include "ModuleLoader.h"

namespace redhawk {
    class ComponentEntry;

    class ComponentHost : public Component, public virtual POA_CF::ExecutableDevice
    {
        ENABLE_LOGGING;

    public:
        ComponentHost(const char* identifier, const char* label);
        ~ComponentHost();

        void constructor();

        // Device functions
        virtual CORBA::Boolean allocateCapacity(const CF::Properties& capacities);
        virtual void deallocateCapacity(const CF::Properties& capacites);
        virtual CF::Device::UsageType usageState();
        virtual CF::Device::AdminType adminState();
        virtual void adminState(CF::Device::AdminType state);
        virtual CF::Device::OperationalType operationalState();
        virtual char* label();
        virtual CF::AggregateDevice_ptr compositeDevice();

        // Loadable device functions
        virtual void load(CF::FileSystem_ptr fs, const char* fileName, CF::LoadableDevice::LoadType loadKind);
        virtual void unload(const char* fileName);

        // Executable device functions
        virtual void terminate(CF::ExecutableDevice::ProcessID_Type processId);
        virtual CF::ExecutableDevice::ProcessID_Type execute(const char* name, const CF::Properties& options, const CF::Properties& parameters);
        virtual CF::ExecutableDevice::ProcessID_Type executeLinked(const char* name, const CF::Properties& options, const CF::Properties& parameters, const CF::StringSequence& deps);

    private:
        void loadProperties();

        void componentReleased(Resource_impl* object);
        void cleanupComponent(ComponentEntry* entry);

        std::string getRealPath(const std::string& path);

        int counter;

        boost::mutex loadMutex;
        typedef std::map<int,ComponentEntry*> ComponentTable;
        ComponentTable activeComponents;

        // Threaded service for performing cleanup checks
        redhawk::ExecutorService executorService;

        /// Property: preload
        std::vector<std::string> preload;
    };
}

#endif // COMPONENTHOST_H
