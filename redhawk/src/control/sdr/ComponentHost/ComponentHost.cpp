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

#include <boost/filesystem.hpp>

#include "ComponentHost.h"

using namespace redhawk;
namespace fs = boost::filesystem;

namespace redhawk {
    struct ComponentEntry {
        boost::scoped_ptr<ModuleBundle> bundle;
        Resource_impl* servant;
    };
}

PREPARE_LOGGING(ComponentHost);

ComponentHost::ComponentHost(const char* identifier, const char* label) :
    Component(identifier, label),
    counter(0)
{
    loadProperties();
}

ComponentHost::~ComponentHost()
{
    executorService.stop();
}

void ComponentHost::loadProperties()
{
    addProperty(preload,
                preload,
                "preload",
                "",
                "readwrite",
                "",
                "external",
                "property");
}

void ComponentHost::constructor()
{
    executorService.start();

    // Preload libraries as given in initial configuration (in most cases, this
    // will be the PRF value, because ComponentHost is implicitly launched by
    // the Domain or the Sandbox). This allows us to prevent common libraries
    // like BulkIO from being loaded implicitly by components, which can lead
    // to the component library being unable to be unloaded.
    LOG_DEBUG(ComponentHost, "Preloading " << preload.size() << " libraries");
    for (std::vector<std::string>::iterator libname = preload.begin(); libname != preload.end(); ++libname) {
        try {
            ModuleLoader::Preload(*libname, ModuleLoader::LAZY, ModuleLoader::GLOBAL);
        } catch (const std::exception& exc) {
            // NB: The library name should be at the front of the error message
            LOG_WARN(ComponentHost, "Unable to preload library " << exc.what());
        }
    }
}

CORBA::Boolean ComponentHost::allocateCapacity(const CF::Properties& capacities)
{
    return false;
}

void ComponentHost::deallocateCapacity(const CF::Properties& capacites)
{
}

CF::Device::UsageType ComponentHost::usageState()
{
    return CF::Device::IDLE;
}

CF::Device::AdminType ComponentHost::adminState()
{
    return CF::Device::UNLOCKED;
}

void ComponentHost::adminState(CF::Device::AdminType state)
{
}

CF::Device::OperationalType ComponentHost::operationalState()
{
    return CF::Device::ENABLED;
}

char* ComponentHost::label()
{
    return Resource_impl::identifier();
}

CF::AggregateDevice_ptr ComponentHost::compositeDevice()
{
    return CF::AggregateDevice::_nil();
}

void ComponentHost::load(CF::FileSystem_ptr, const char* fileName, CF::LoadableDevice::LoadType loadKind)
{
}

void ComponentHost::unload(const char* fileName)
{
}

void ComponentHost::terminate(CF::ExecutableDevice::ProcessID_Type processId)
{
    Resource_impl* component = 0;
    {
        boost::mutex::scoped_lock lock(loadMutex);
        ComponentTable::iterator entry = activeComponents.find(processId);
        if (entry == activeComponents.end()) {
            throw CF::ExecutableDevice::InvalidProcess(CF::CF_EINVAL, "No such component");
        }
        component = entry->second->servant;
    }
    component->releaseObject();
}

CF::ExecutableDevice::ProcessID_Type ComponentHost::execute(const char* name, const CF::Properties& options, const CF::Properties& parameters)
{
    return executeLinked(name, options, parameters, CF::StringSequence());
}

std::string ComponentHost::getRealPath(const std::string& path)
{
    // Assume that all paths are relative to the deployment root, which is
    // given by the launching device (or the Sandbox)
    fs::path realpath = fs::path(getDeploymentRoot()) / path;
    if (!fs::exists(realpath)) {
        std::string message = "File " + path + " does not exist";
        throw CF::InvalidFileName(CF::CF_EEXIST, message.c_str());
    }
    return realpath.string();
}

CF::ExecutableDevice::ProcessID_Type ComponentHost::executeLinked(const char* name, const CF::Properties& options, const CF::Properties& parameters, const CF::StringSequence& deps)
{
    const std::string path = getRealPath(name);

    boost::scoped_ptr<ModuleBundle> bundle(new ModuleBundle(path));

    boost::mutex::scoped_lock lock(loadMutex);
    for (size_t ii = 0; ii < deps.length(); ++ii) {
        const std::string libpath = getRealPath(std::string(deps[ii]));
        LOG_DEBUG(ComponentHost, "Loading dependency: " << libpath);
        try {
            // We don't know which symbols are needed from this library; they
            // just need to be accessible to the component entry point. Loading
            // them as "local" instead of "global" allows symbol conflicts to
            // be resolved correctly (it seems).
            if (fs::is_directory(libpath)) {
                bundle->loadDirectory(libpath, ModuleLoader::LAZY, ModuleLoader::LOCAL);
            } else {
                bundle->load(libpath, ModuleLoader::LAZY, ModuleLoader::LOCAL);
            }
        } catch (const std::exception& exc) {
            LOG_ERROR(ComponentHost, "Unable to load dependency: " << exc.what());
            throw CF::ExecutableDevice::ExecuteFail(CF::CF_EINVAL, exc.what());
        }
    }

    LOG_DEBUG(ComponentHost, "Loading component module: " << path);
    Module* module;
    try {
        // Resolve all required symbols now so that we can catch the error and
        // turn it into an exception, rather than having the process exit at
        // point-of-use
        module = bundle->load(path, ModuleLoader::NOW, ModuleLoader::LOCAL);
    } catch (const std::exception& exc) {
        LOG_ERROR(ComponentHost, "Unable to load module: " << exc.what())
        throw CF::ExecutableDevice::ExecuteFail(CF::CF_EINVAL, exc.what());
    }

    typedef Resource_impl* (*ConstructorPtr)(const std::string&, const std::string&);
    ConstructorPtr make_component;
    try {
        LOG_DEBUG(ComponentHost, "Resolving module entry point");
        make_component = reinterpret_cast<ConstructorPtr>(module->symbol("make_component"));
    } catch (const std::exception& exc) {
        LOG_ERROR(ComponentHost, "Unable to load module entry point: " << exc.what())
        throw CF::ExecutableDevice::InvalidFunction();
    }

    LOG_DEBUG(ComponentHost, "Creating component");
    Resource_impl* servant = Resource_impl::create_component(make_component, parameters);

    ComponentEntry* component = new ComponentEntry;
    component->bundle.swap(bundle);
    component->servant = servant;

    int thread_id = ++counter;
    activeComponents[thread_id] = component;
    LOG_DEBUG(ComponentHost, "Assigning thread ID " << thread_id);

    servant->addReleaseListener(this, &ComponentHost::componentReleased);

    return thread_id;
}

void ComponentHost::componentReleased(Resource_impl* component)
{
    LOG_DEBUG(ComponentHost, "Component released: " << component->getIdentifier());
    boost::mutex::scoped_lock lock(loadMutex);
    ComponentTable::iterator entry;
    for (entry = activeComponents.begin(); entry != activeComponents.end(); ++entry) {
        if (entry->second->servant == component) {
            break;
        }
    }

    if (entry == activeComponents.end()) {
        LOG_DEBUG(ComponentHost, "Received release notification from unmanaged component "
                  << component->getIdentifier());
        return;
    }

    executorService.execute(&ComponentHost::cleanupComponent, this, entry->second);
    activeComponents.erase(entry);
}

void ComponentHost::cleanupComponent(ComponentEntry* component)
{
    // Only if this is the last reference to the servant can we safely unload
    // its shared libraries, because we need to know that it has been deleted
    if (component->servant->_refcount_value() == 1) {
        component->servant->_remove_ref();
        LOG_DEBUG(ComponentHost, "Unloading bundle " << component->bundle->name());
        component->bundle->unload();
        delete component;
        return;
    }

    // Try again after a small delay
    LOG_DEBUG(ComponentHost, "Rescheduling component cleanup");
    boost::system_time when = boost::get_system_time() + boost::posix_time::microseconds(125);
    executorService.schedule(when, &ComponentHost::cleanupComponent, this, component);
}

