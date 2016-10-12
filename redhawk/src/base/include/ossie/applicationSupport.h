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


#ifndef APPLICATIONSUPPORT_H
#define APPLICATIONSUPPORT_H

#include <string>
#include <vector>
#include <map>

#ifdef HAVE_OMNIORB4_CORBA_H
#include "omniORB4/CORBA.h"
#endif

#include "ossie/CF/cf.h"
#include "ossie/CF/StandardEvent.h"
#include "ossie/SoftPkg.h"
#include "ossie/ComponentDescriptor.h"
#include "ossie/Properties.h"
#include "ossie/exceptions.h"
#include "ossie/ossieparser.h"
#include "ossie/componentProfile.h"
#include "ossie/DeviceManagerConfiguration.h"

// Application support routines

// Base class to contain data for the components required to
// create an application
// Application support routines

namespace ossie
{
    /**
     *
     */
    class AllocPropsInfo {
    public:
        AllocPropsInfo() {}

        AllocPropsInfo(CF::Device_ptr _device, CF::Properties& _properties) {
            device = CF::Device::_duplicate(_device);
            properties = _properties;
        }

        AllocPropsInfo(const AllocPropsInfo& other) {
            device = CF::Device::_duplicate(other.device);
            properties = other.properties;
        }

        AllocPropsInfo& operator=(const AllocPropsInfo& other) {
            if (this != &other) {
                device = CF::Device::_duplicate(other.device);
                properties = other.properties;
            }
            return *this;
        }

    public:
        CF::Device_var device;
        CF::Properties properties;
    };

    /**
     *
     */
    class UsesDeviceInfo
    {

    public:
        UsesDeviceInfo(const std::string& id, const std::string& type, const std::vector<SPD::PropertyRef>& _properties);
        ~UsesDeviceInfo();

        const std::string& getId() const;
        const std::string& getType() const;
        const std::vector<SPD::PropertyRef>& getProperties() const;
        const std::string& getAssignedDeviceId() const;

        void setAssignedDeviceId(const std::string& deviceId);

    private:
        std::string id;
        std::string type;
        std::vector<SPD::PropertyRef> properties;
        std::string assignedDeviceId;
    };

    /** Base class to contain data for components and implementations
     *  - Used to store information about about UsesDevice relationships
     *    since these are handled the same for both components and
     *    component implementations.
     */
    class ComponentImplementationInfo
    {
        ENABLE_LOGGING;

    public:
        ComponentImplementationInfo();
        virtual ~ComponentImplementationInfo();

        bool checkUsesDevices(ossie::Properties& _prf, CF::Properties& allocProps, unsigned int usesDevIdx, const CF::Properties& configureProperties) const;
        void addUsesDevice(UsesDeviceInfo* usesDevice);
        const std::vector<UsesDeviceInfo*>& getUsesDevices() const;
        virtual const UsesDeviceInfo* getUsesDeviceById(const std::string& id) const = 0;


    protected:
        std::vector<UsesDeviceInfo*> usesDevices;
    };

    /** Base class to contain data for implementations
     *  - Used to store information about about implementations
     */
    class ImplementationInfo : public ComponentImplementationInfo
    {
        ENABLE_LOGGING;

    public:
        ImplementationInfo(const SPD::Implementation& spdImpl);
        ~ImplementationInfo();

        const std::string& getId() const;
        CF::LoadableDevice::LoadType getCodeType() const;
        const std::vector<std::string>& getProcessorDeps() const;
        const std::vector<ossie::SPD::NameVersionPair>& getOsDeps() const;
        const std::string& getLocalFileName() const;
        const std::string& getEntryPoint() const;
        const CORBA::ULong getStackSize() const;
        const CORBA::ULong getPriority() const;
        const bool hasStackSize() const;
        const bool hasPriority() const;
        const UsesDeviceInfo* getUsesDeviceById(const std::string& id) const;
        const std::vector<SPD::PropertyRef>& getDependencyProperties() const;
        const std::vector<SPD::SoftPkgRef>& getSoftPkgDependency() const;

        CF::Properties getAllocationProperties(const ossie::Properties& prf, const CF::Properties& configureProperties) const
            throw (ossie::PropertyMatchingError);
        bool checkProcessorAndOs(const ossie::Properties& prf) const;
        bool checkMatchingDependencies(const ossie::Properties& prf, const std::string& softwareProfile, const CF::DeviceManager_var& devMgr) const;

    private:
        ImplementationInfo (const ImplementationInfo&);
        void setLocalFileName(const char* fileName);
        void setEntryPoint(const char* fileName);
        void setCodeType(const char* _type);
        void setStackSize(const unsigned long long *_stackSize);
        void setPriority(const unsigned long long *_priority);
        void addDependencyProperty(const ossie::SPD::PropertyRef& property);
        void addSoftPkgDependency(const SPD::SoftPkgRef& softpkg);

        std::string id;
        CF::LoadableDevice::LoadType codeType;
        std::string localFileName;
        std::string entryPoint;
        CORBA::ULong stackSize;
        CORBA::ULong priority;
        bool _hasStackSize;
        bool _hasPriority;
        std::vector<std::string> processorDeps;
        std::vector<ossie::SPD::NameVersionPair> osDeps;
        std::vector<SPD::PropertyRef> dependencyProperties;
        std::vector<SPD::SoftPkgRef> softPkgDependencies;

    };

    /** Base class to contain data for components
     *  - Used to store information about about components
     */
    class ComponentInfo : public ComponentImplementationInfo
    {
        ENABLE_LOGGING

    public:
        ComponentInfo ();
        ~ComponentInfo ();

        void setName(const char* name);
        void setIdentifier(const char* identifier, std::string instance_id);
        void setAssignedDeviceId(const char* name);
        void setSelectedImplementation(const ImplementationInfo* implementation);
        void addImplementation(ImplementationInfo* impl);
        void setImplPRFFile(const char* _PRFFile);
        void setNamingService(const bool isNamingService);
        void setNamingServiceName(const char* NamingServiceName);
        void setUsageName(const char* usageName);
        void setIsAssemblyController(bool isAssemblyController);
        void setIsScaCompliant(bool isScaCompliant);
        void setSpdFileName(const char* spdFileName);

        void setDeployedOnResourceFactory(bool _deployedOnResourceFactory);
        void setDeployedOnExecutableDevice(bool _deployedOnExecutableDevice);
        void setPID(CF::ExecutableDevice::ProcessID_Type _pid);
        void setDeployedOnLoadableDevice(bool _deployedOnLoadableDevice);

        const UsesDeviceInfo* getUsesDeviceById(const std::string& id) const;

        void addFactoryParameter(CF::DataType dt);
        void addExecParameter(CF::DataType dt);
        void addDependencyProperty(std::string implId, CF::DataType dt);
        void addConfigureProperty(CF::DataType dt);

        void overrideProperty(const ossie::ComponentProperty* propref);
        void overrideProperty(const char* id, const CORBA::Any& value);
        void overrideSimpleProperty(const char* id, const std::string value);

        void setResourcePtr(CF::Resource_ptr);
        void setDevicePtr(CF::Device_ptr);

        const char* getName();
        const char* getInstantiationIdentifier();
        const char* getIdentifier();
        const char* getAssignedDeviceId();
        const ImplementationInfo* getSelectedImplementation() const;
        const std::vector<ImplementationInfo*>& getImplementations() const;
        const char* getImplPRFFile();
        const bool  getNamingService();
        const char* getUsageName();
        const char* getNamingServiceName();
        const char* getSpdFileName();
        const bool  isResource();
        const bool  isConfigurable();
        const bool  isAssemblyController();
        const bool  isScaCompliant();

        bool isAssignedToDevice() const;

        const bool  getDeployedOnResourceFactory();
        const bool  getDeployedOnExecutableDevice();
        const CF::ExecutableDevice::ProcessID_Type getPID();
        bool  getDeployedOnLoadableDevice() const;

        CF::Properties getNonNilConfigureProperties();
        CF::Properties getConfigureProperties();
        CF::Properties getOptions();
        CF::Properties getExecParameters();

        CF::Properties getAllocationProperties(const ossie::Properties& prf, std::string implId);
        bool checkUsesDevice(const ossie::Properties& _prf, CF::Properties& allocProps, unsigned int usesDevIdx);

        CF::Resource_ptr getResourcePtr();
        CF::Device_ptr getDevicePtr();

        static ComponentInfo* buildComponentInfoFromSPDFile(CF::FileManager_ptr fileMgr, const char* _SPDFile);
        SoftPkg spd;
        ComponentDescriptor scd;
        ossie::Properties prf;

    private:
        ComponentInfo (const ComponentInfo&);

        void process_overrides(CF::Properties* props, const char* id, CORBA::Any value);
        bool _isAssemblyController;
        bool _isConfigurable;
        bool _isScaCompliant;
        bool isNamingService;
        bool deployedOnResourceFactory;
        bool deployedOnLoadableDevice;
        bool deployedOnExecutableDevice;

        std::string name;    // Component name from SPD File
        std::string assignedDeviceId;  // Device to deploy component on from DAS.
        const ImplementationInfo* selectedImplementation; // Implementation selected to run on assigned device.

        std::vector<ImplementationInfo*> implementations;
        std::string implPRF;
        std::string usageName;
        std::string identifier;
        std::string instantiationId;
        std::string namingServiceName;
        std::string SpdFileName;
        std::map<std::string, bool> isReadWrite;

        // CF::LoadableDevice::LoadType codeType;

        CF::Properties configureProperties;
        CF::Properties options;
        CF::Properties factoryParameters;
        CF::Properties execParameters;

        CF::ExecutableDevice::ProcessID_Type PID;

        CF::Resource_var rsc;
        CF::Device_var devicePtr;

    };

    /**
     * Class to store information for device assignment support
     */
    class DeviceAssignmentInfo
    {

    public:
        CF::DeviceAssignmentType deviceAssignment;
        CF::Device_var  device;
    };
}
#endif
