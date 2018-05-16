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

namespace redhawk {

    extern rh_logger::LoggerPtr deploymentLog;

    class ApplicationComponent;

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
        void load(redhawk::ApplicationComponent* appComponent, CF::FileSystem_ptr fileSystem,
                  CF::LoadableDevice_ptr device);

        const ossie::SoftPkg* softpkg;
        const ossie::SPD::Implementation* implementation;
        DeploymentList dependencies;
    };

    class ComponentDeployment : public SoftPkgDeployment, public UsesDeviceDeployment
    {
    public:
        typedef ossie::ComponentInstantiation::LoggingConfig    LoggingConfig;

        ComponentDeployment(const ossie::SoftPkg* softpkg,
                            const ossie::ComponentInstantiation* instantiation,
                            const std::string& identifier);

        /**
         * @brief  Returns the component's runtime identifier
         */
        const std::string& getIdentifier() const;

        const ossie::ComponentInstantiation* getInstantiation() const;

        void setContainer(ComponentDeployment* container);
        ComponentDeployment* getContainer();

        bool isResource() const;
        bool isConfigurable() const;

        bool isAssemblyController() const;
        void setIsAssemblyController(bool state);

        std::string getEntryPoint();

        redhawk::PropertyMap getOptions();

        redhawk::PropertyMap getAffinityOptionsWithAssignment() const;
        void mergeAffinityOptions(const CF::Properties& affinity);

        redhawk::PropertyMap getLoggingConfiguration() const;

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
        const boost::shared_ptr<ossie::DeviceNode>& getAssignedDevice() const;

        void setResourcePtr(CF::Resource_ptr resource);
        CF::Resource_ptr getResourcePtr() const;

        void load(CF::FileSystem_ptr fileSystem, CF::LoadableDevice_ptr device);

        redhawk::PropertyMap getDeviceRequires() const;
        void  setDeviceRequires( const redhawk::PropertyMap &devRequires );

        redhawk::ApplicationComponent* getApplicationComponent();
        void setApplicationComponent(redhawk::ApplicationComponent* appComponent);

        /**
         * @brief  Initializes the deployed component
         * @exception  ossie::properties_error  invalid properties in property
         *             initialization
         * @exception  ossie::component_error  initialization failed
         *
         * Handles initialization of new-style 'property' kind properties and
         * calls initialize on the component.
         */
        void initialize();

        /**
         * @brief  Configures legacy properties to initial values
         * @exception  ossie::properties_error  invalid properties
         * @exception  ossie::component_error  configure failed
         *
         * Handles configuration of legacy 'configure' kind properties.
         */
        void configure();

        /**
         * Returns the properties used for the initial call to configure()
         * during deployment
         */
        redhawk::PropertyMap getAllInitialProperties() const;

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
        ComponentDeployment* container;
        CF::Resource_var resource;

        redhawk::ApplicationComponent* appComponent;

        redhawk::PropertyMap overrides;
        std::string nicAssignment;
        ossie::optional_value<float> cpuReservation;
        redhawk::PropertyMap affinityOptions;
        redhawk::PropertyMap deviceRequires;
        redhawk::PropertyMap loggingConfig;
    };

}

#endif // DEPLOYMENT_H
