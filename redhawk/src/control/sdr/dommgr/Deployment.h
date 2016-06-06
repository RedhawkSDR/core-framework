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

#ifndef DEPLOYMENT_H
#define DEPLOYMENT_H

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <ossie/PropertyMap.h>
#include "PersistenceStore.h"

class Application_impl;

namespace redhawk {

    class ComponentDeployment;

    class deployment_error : public std::runtime_error {
    public:
        deployment_error(const ComponentDeployment* deployment, const std::string& message) :
            std::runtime_error(message),
            _deployment(deployment)
        {
        }

        virtual ~deployment_error() throw ()
        {
        }

        const ComponentDeployment* deployment() const
        {
            return _deployment;
        }

    private:
        const ComponentDeployment* _deployment;
    };

    class execute_error : public deployment_error {
    public:
        execute_error(const ComponentDeployment* deployment,
                      const boost::shared_ptr<ossie::DeviceNode>& device,
                      const std::string& message) :
            deployment_error(deployment, message),
            _device(device)
        {
        }

        const boost::shared_ptr<ossie::DeviceNode>& device() const
        {
            return _device;
        }

        virtual ~execute_error() throw ()
        {
        }

    private:
        boost::shared_ptr<ossie::DeviceNode> _device;
    };

    class properties_error : public deployment_error {
    public:
        properties_error(const ComponentDeployment* deployment,
                         const CF::Properties& properties,
                         const std::string& message) :
            deployment_error(deployment, message),
            _properties(properties)
        {
        }

        virtual ~properties_error() throw ()
        {
        }

        const redhawk::PropertyMap& properties() const
        {
            return _properties;
        }

    private:
        const redhawk::PropertyMap _properties;
    };

    class UsesDeviceAssignment
    {
    public:
        UsesDeviceAssignment(const ossie::UsesDevice* usesDevice);

        const ossie::UsesDevice* getUsesDevice() const;

        void setAssignedDevice(CF::Device_ptr device);
        CF::Device_ptr getAssignedDevice() const;

    private:
        const ossie::UsesDevice* usesDevice;
        CF::Device_var assignedDevice;
    };

    class UsesDeviceDeployment
    {
    public:
        typedef std::vector<UsesDeviceAssignment*> AssignmentList;

        ~UsesDeviceDeployment();

        void addUsesDeviceAssignment(UsesDeviceAssignment* assignment);
        UsesDeviceAssignment* getUsesDeviceAssignment(const std::string identifier);
        const AssignmentList& getUsesDeviceAssignments();

        void transferUsesDeviceAssignments(UsesDeviceDeployment& other);

    protected:
        AssignmentList assignments;
    };

    class SoftPkgDeployment
    {
    public:
        typedef std::vector<SoftPkgDeployment*> DeploymentList;

        SoftPkgDeployment(const ossie::SoftPkg* softpkg, const ossie::SPD::Implementation* implementation=0);
        ~SoftPkgDeployment();

        const ossie::SoftPkg* getSoftPkg() const;

        void setImplementation(const ossie::SPD::Implementation* implementation);
        const ossie::SPD::Implementation* getImplementation() const;

        std::string getLocalFile();
        CF::LoadableDevice::LoadType getCodeType() const;
        bool isExecutable() const;

        void addDependency(SoftPkgDeployment* dependency);
        const DeploymentList& getDependencies();
        void clearDependencies();

        std::vector<std::string> getDependencyLocalFiles();

    protected:
        void load(Application_impl* application, CF::FileSystem_ptr fileSystem,
                  CF::LoadableDevice_ptr device, const std::string& componentId);

        const ossie::SoftPkg* softpkg;
        const ossie::SPD::Implementation* implementation;
        DeploymentList dependencies;
    };

    class ComponentDeployment : public SoftPkgDeployment, public UsesDeviceDeployment
    {
    public:
        ComponentDeployment(const ossie::SoftPkg* softpkg,
                            const ossie::ComponentInstantiation* instantiation,
                            const std::string& identifier);

        /**
         * @brief  Returns the component's runtime identifier
         */
        const std::string& getIdentifier() const;

        const ossie::ComponentInstantiation* getInstantiation() const;

        bool isResource() const;
        bool isConfigurable() const;

        bool isAssemblyController() const;
        void setIsAssemblyController(bool state);

        std::string getEntryPoint();

        redhawk::PropertyMap getOptions();

        redhawk::PropertyMap getAffinityOptionsWithAssignment() const;
        void mergeAffinityOptions(const CF::Properties& affinity);

        void setNicAssignment(const std::string& nic);
        bool hasNicAssignment() const;
        const std::string& getNicAssignment() const;

        void setCpuReservation(float reservation);
        bool hasCpuReservation() const;
        float getCpuReservation() const;

        /**
         * Returns the properties used for evaluating math statements in
         * allocation
         */
        redhawk::PropertyMap getAllocationContext() const;

        /**
         * Returns the properties whose values are passed on the command line
         * in execute
         */
        redhawk::PropertyMap getCommandLineParameters() const;

        void overrideProperty(const std::string& id, const CORBA::Any& value);

        void setAssignedDevice(const boost::shared_ptr<ossie::DeviceNode>& device);
        boost::shared_ptr<ossie::DeviceNode> getAssignedDevice();

        void setResourcePtr(CF::Resource_ptr resource);
        CF::Resource_ptr getResourcePtr() const;

        void load(Application_impl* application, CF::FileSystem_ptr fileSystem,
                  CF::LoadableDevice_ptr device);

        std::string getLoggingConfiguration() const;

        /**
         * @brief  Initializes the deployed component
         * @exception  ossie::properties_error  invalid properties in property
         *             initialization
         * @exception  ossie::deployment_error  initialization failed
         *
         * Handles initialization of new-style 'property' kind properties and
         * calls initialize on the component.
         */
        void initialize();

        /**
         * @brief  Configures legacy properties to initial values
         * @exception  ossie::properties_error  invalid properties
         * @exception  ossie::deployment_error  configure failed
         *
         * Handles configuration of legacy 'configure' kind properties.
         */
        void configure();

    protected:
        /**
         * Returns the properties used for the initial call to configure()
         * during deployment
         */
        redhawk::PropertyMap getInitialConfigureProperties() const;

        /**
         * Returns the properties used for initializePropertes() during
         * deployment
         */
        redhawk::PropertyMap getInitializeProperties() const;

        CF::DataType getPropertyValue(const ossie::Property* property) const;
        const ossie::ComponentProperty* getPropertyOverride(const std::string& id) const;

        void initializeProperties();

        const ossie::ComponentInstantiation* instantiation;
        const std::string identifier;
        bool assemblyController;

        boost::shared_ptr<ossie::DeviceNode> assignedDevice;
        CF::Resource_var resource;

        redhawk::PropertyMap overrides;
        std::string nicAssignment;
        ossie::optional_value<float> cpuReservation;
        redhawk::PropertyMap affinityOptions;
    };

}

#endif // DEPLOYMENT_H
