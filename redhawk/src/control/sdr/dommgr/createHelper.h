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

#ifndef CREATEHELPER_H
#define CREATEHELPER_H

#include <list>
#include <vector>
#include <string>

#include "PersistenceStore.h"
#include "applicationSupport.h"
#include "Placement.h"
#include "Deployment.h"

class Application_impl;
class AllocationManager_impl;

class ScopedAllocations {
public:
    ScopedAllocations(AllocationManager_impl& allocator);
    ~ScopedAllocations();

    void push_back(const std::string& allocationID);

    template <class T>
    void transfer(T& dest);

    void transfer(ScopedAllocations& dest);

    void deallocate();

private:
    AllocationManager_impl& _allocator;
    std::list<std::string> _allocations;
};

class createHelper
{

public:
    typedef std::map<std::string,std::string> DeviceAssignmentMap;

    createHelper (const ApplicationFactory_impl& appFact,
                  std::string waveformContextName,
                  std::string base_naming_context,
                  CosNaming::NamingContext_ptr WaveformContext,
                  CosNaming::NamingContext_ptr DomainContext);
    ~createHelper ();

    CF::Application_ptr create (const char* name,
                                const CF::Properties& initConfiguration,
                                const DeviceAssignmentMap& deviceAssignments);

private:

    // list of components that are part of a collocation
    typedef std::vector <ossie::ComponentInfo* >                       PlacementList;

    // Used for storing the current state of the OE & create process
    const ApplicationFactory_impl& _appFact;

    // Local pointer to the allocation manager
    AllocationManager_impl* _allocationMgr;
 
    // Tracks allocation IDs made during creation, and automates cleanup on
    // failure
    ScopedAllocations _allocations;

    ossie::DeviceList _registeredDevices;
    ossie::DeviceList _executableDevices;

    // waveform instance-specific naming context (unique to the instance of the waveform)
    std::string _waveformContextName; 

    // full (includes full context path) waveform instance-specific naming context
    std::string _baseNamingContext; 

    // CORBA naming context
    CosNaming::NamingContext_var _waveformContext; 
    CosNaming::NamingContext_ptr _domainContext; 

    ossie::ApplicationInfo _appInfo;

    typedef std::vector<ossie::ComponentDeployment*> DeploymentList;

    // createHelper helper methods
    void overrideExternalProperties(ossie::ApplicationPlacement& appPlacement,
                                    const CF::Properties& initConfiguration);
    void overrideProperties(const CF::Properties& initConfiguration, ossie::ComponentInfo* component);
    void assignPlacementsToDevices(ossie::ApplicationPlacement& appPlacement,
                                   ossie::ApplicationDeployment& appDeployment,
                                   const std::string& appIdentifier,
                                   const DeviceAssignmentMap& devices);
    void _validateDAS(ossie::ApplicationPlacement& appPlacement, const DeviceAssignmentMap& deviceAssignments);
    void _connectComponents(ossie::ApplicationDeployment& appDeployment,
        std::vector<ossie::ConnectionNode>& connections);
    void _configureComponents(const DeploymentList& deployments);
    void _checkAssemblyController(
        CF::Resource_ptr      assemblyController,
        ossie::ComponentInfo* assemblyControllerComponent) const;
    void setUpExternalPorts(ossie::ApplicationDeployment& appDeployment, Application_impl* application);
    void setUpExternalProperties(ossie::ApplicationDeployment& appDeployment, Application_impl* application);
    void _placeHostCollocation(ossie::ApplicationDeployment& appDeployment,
                               const std::string& appIdentifier,
                               const PlacementList& collocatedComponents,
                               const DeviceAssignmentMap& devices);
    void _handleUsesDevices(ossie::ApplicationPlacement& appPlacement,
                            ossie::ApplicationDeployment& appDeployment,
                            const std::string& appName);
    void _resolveImplementations(PlacementList::iterator comp, PlacementList& compList, std::vector<ossie::ImplementationInfo::List> &res_vec);
    void _removeUnmatchedImplementations(std::vector<ossie::ImplementationInfo::List> &res_vec);
    void _consolidateAllocations(const ossie::ImplementationInfo::List& implementations, CF::Properties& allocs);
    void _evaluateMATHinRequest(CF::Properties &request, const CF::Properties &configureProperties);
    void _castRequestProperties(CF::Properties& allocationProperties, const std::vector<ossie::SPD::PropertyRef> &prop_refs, unsigned int offset=0);
    void _castRequestProperties(CF::Properties& allocationProperties, const std::vector<ossie::SoftwareAssembly::PropertyRef> &prop_refs,
            unsigned int offset=0);

    // Populate _requiredComponents vector
    void getRequiredComponents(CF::FileSystem_ptr fileSys,
                               const ossie::SoftwareAssembly& sadParser,
                               ossie::ApplicationPlacement& appPlacement);
    ossie::ComponentInfo* buildComponentInfo(CF::FileSystem_ptr fileSys,
                                             const ossie::SoftwareAssembly& sadParser,
                                             const ossie::ComponentPlacement& component);

    // Supports allocation
    bool allocateUsesDevices(const ossie::UsesDeviceInfo::List& usesDevices,
                             const CF::Properties& configureProperties,
                             std::vector<ossie::UsesDeviceAssignment*>& assignedDevices,
                             ScopedAllocations& allocations);
    CF::AllocationManager::AllocationResponseSequence* allocateUsesDeviceProperties(
        const ossie::UsesDeviceInfo::List& component,
        const CF::Properties& configureProperties);
    ossie::ComponentDeployment* allocateComponent(ossie::ComponentInfo* component,
                                                  const std::string& assignedDeviceId,
                                                  const std::string& appIdentifier);

    ossie::AllocationResult allocateComponentToDevice(ossie::ComponentDeployment* deployment,
                                                      const std::string& assignedDeviceId,
                                                      const std::string& appIdentifier);

    bool resolveSoftpkgDependencies(ossie::SoftpkgDeployment* deployment, ossie::DeviceNode& device);
    ossie::SoftpkgDeployment* resolveDependencyImplementation(ossie::SoftpkgInfo* softpkg, ossie::DeviceNode& device);
    
    // Supports loading, executing, initializing, configuring, & connecting
    void loadDependencies(const ossie::ComponentInfo& component,
                          CF::LoadableDevice_ptr device,
                          const std::vector<ossie::SoftpkgDeployment*>& dependencies);

    void loadAndExecuteComponents(const DeploymentList& deployments,
                                  CF::ApplicationRegistrar_ptr _appReg);
    void applyApplicationAffinityOptions(const DeploymentList& deployments);

    void attemptComponentExecution(CF::ApplicationRegistrar_ptr registrar, ossie::ComponentDeployment* deployment);

    void waitForComponentRegistration(const DeploymentList& deployments);
    void initializeComponents(const DeploymentList& deployments);

    void configureComponents(const DeploymentList& deployments);
    void connectComponents(ossie::ApplicationDeployment& appDeployment,
        std::vector<ossie::ConnectionNode>& connections, 
        std::string                         base_naming_context);

    std::vector<CF::Resource_var> getStartOrder(const DeploymentList& deployments);

    // Cleanup - used when create fails/doesn't succeed for some reason
    bool _isComplete;
    void _cleanupFailedCreate();
    Application_impl* _application;
};
#endif
