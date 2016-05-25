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
#include "applicationSupport.h"
#include "connectionSupport.h"

class Application_impl;

namespace ossie {

    class UsesDeviceAssignment
    {
    public:
        UsesDeviceAssignment(const UsesDevice* usesDevice);

        const UsesDevice* getUsesDevice() const;

        void setAssignedDevice(CF::Device_ptr device);
        CF::Device_ptr getAssignedDevice() const;

    private:
        const UsesDevice* usesDevice;
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

    class SoftpkgDeployment
    {
    public:
        typedef std::vector<SoftpkgDeployment*> DeploymentList;

        SoftpkgDeployment(const SoftPkg* softpkg, const SPD::Implementation* implementation=0);
        ~SoftpkgDeployment();

        const SoftPkg* getSoftPkg() const;

        void setImplementation(const SPD::Implementation* implementation);
        const SPD::Implementation* getImplementation() const;

        std::string getLocalFile();
        CF::LoadableDevice::LoadType getCodeType() const;

        void addDependency(SoftpkgDeployment* dependency);
        const DeploymentList& getDependencies();
        void clearDependencies();

        std::vector<std::string> getDependencyLocalFiles();

    protected:
        void load(Application_impl* application, CF::FileSystem_ptr fileSystem,
                  CF::LoadableDevice_ptr device, const std::string& componentId);

        const SoftPkg* softpkg;
        const SPD::Implementation* implementation;
        DeploymentList dependencies;
    };

    class ComponentDeployment : public SoftpkgDeployment, public UsesDeviceDeployment
    {
    public:
        ComponentDeployment(ComponentInfo* component, const ComponentInstantiation* instantiation,
                            const std::string& identifier);

        ComponentInfo* getComponent();

        /**
         * @brief  Returns the component's runtime identifier
         */
        const std::string& getIdentifier() const;

        const ComponentInstantiation* getInstantiation() const;

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

        CF::Properties getConfigureProperties() const;
        CF::Properties getConstructProperties() const;

        redhawk::PropertyMap getInitialConfigureProperties() const;
        redhawk::PropertyMap getInitializeProperties() const;

        void setAssignedDevice(const boost::shared_ptr<DeviceNode>& device);
        boost::shared_ptr<DeviceNode> getAssignedDevice();

        void setResourcePtr(CF::Resource_ptr resource);
        CF::Resource_ptr getResourcePtr() const;

        void load(Application_impl* application, CF::FileSystem_ptr fileSystem,
                  CF::LoadableDevice_ptr device);

    protected:
        CF::DataType getPropertyValue(const Property* property) const;
        const ComponentProperty* getPropertyOverride(const std::string& id) const;

        ComponentInfo* component;
        const ComponentInstantiation* instantiation;
        const std::string identifier;

        boost::shared_ptr<DeviceNode> assignedDevice;
        CF::Resource_var resource;

        std::string nicAssignment;
        ossie::optional_value<float> cpuReservation;
        redhawk::PropertyMap affinityOptions;
    };

    class PlacementPlan
    {
    public:
        typedef std::vector<ComponentInfo*> ComponentList;

        PlacementPlan();
        PlacementPlan(const std::string& id, const std::string& name);
        ~PlacementPlan();
 
        const std::string& getId() const;
        const std::string& getName() const;

        const ComponentList& getComponents() const;

        void addComponent(ComponentInfo* component);

        ComponentInfo* getComponent(const std::string& instantiationId);

    protected:
        std::string id;
        std::string name;
        ComponentList components;
    };

    class ApplicationDeployment : public ComponentLookup, public DeviceLookup, public UsesDeviceDeployment
    {
    public:
        typedef std::vector<PlacementPlan*> PlacementList;
        typedef std::vector<ComponentDeployment*> ComponentList;

        ApplicationDeployment(const SoftwareAssembly& sad, const std::string& instanceName);
        ~ApplicationDeployment();

        const std::string& getIdentifier() const;

        void addPlacement(PlacementPlan* placement);

        const PlacementList& getPlacements() const;

        ComponentInfo* getAssemblyController();

        ComponentInfo* getComponent(const std::string& instantiationId);

        ComponentDeployment* createComponentDeployment(ComponentInfo* component);

        const ComponentList& getComponentDeployments();
        ComponentDeployment* getComponentDeployment(const std::string& instantiationId);

        // Adapt interfaces for component and device search to support
        // ConnectionManager
        // ComponentLookup interface
        virtual CF::Resource_ptr lookupComponentByInstantiationId(const std::string& identifier);

        // DeviceLookup interface
        CF::Device_ptr lookupDeviceThatLoadedComponentInstantiationId(const std::string& componentId);
        CF::Device_ptr lookupDeviceUsedByComponentInstantiationId(const std::string& componentId,
                                                                  const std::string& usesId);
        CF::Device_ptr lookupDeviceUsedByApplication(const std::string& usesRefId);

    protected:
        const std::string identifier;
        const std::string instanceName;
        PlacementList placements;
        ComponentList components;
    };
}

#endif // DEPLOYMENT_H
