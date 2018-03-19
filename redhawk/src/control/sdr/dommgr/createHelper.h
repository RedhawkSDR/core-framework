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

#include <ossie/ComponentDescriptor.h>

#include "PersistenceStore.h"
#include "ApplicationDeployment.h"
#include "ProfileCache.h"

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
    // Used for storing the current state of the OE & create process
    const ApplicationFactory_impl& _appFact;

    const rh_logger::LoggerPtr _createHelperLog;

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

    redhawk::ProfileCache _profileCache;

    typedef std::vector<redhawk::ComponentDeployment*> DeploymentList;
    typedef std::vector<redhawk::ContainerDeployment*> ContainerList;
    typedef std::vector<std::string> ProcessorList;
    typedef std::vector<ossie::SPD::NameVersionPair> OSList;
    typedef std::vector<ossie::Reservation> ReservationList;

    // createHelper helper methods
    void assignPlacementsToDevices(redhawk::ApplicationDeployment& appDeployment,
                                   const DeviceAssignmentMap& devices,
                                   const std::map<std::string,float>& specialized_reservations);
    void _resolveAssemblyController(redhawk::ApplicationDeployment& appDeployment);
    void _validateDAS(redhawk::ApplicationDeployment& appDeployment, const DeviceAssignmentMap& deviceAssignments);
    void checkOptions();
    void setUpExternalPorts(redhawk::ApplicationDeployment& appDeployment, Application_impl* application);
    void setUpExternalProperties(redhawk::ApplicationDeployment& appDeployment, Application_impl* application);
    std::vector<ossie::Reservation> overloadReservations(const ossie::SoftwareAssembly::HostCollocation& collocation,
                                                         const std::map<std::string,float>& specialized_reservations);
    void _placeHostCollocation(redhawk::ApplicationDeployment& appDeployment,
                               const ossie::SoftwareAssembly::HostCollocation& collocation,
                               const DeviceAssignmentMap& devices,
                               const std::map<std::string,float>& specialized_reservations);
    bool placeHostCollocation(redhawk::ApplicationDeployment& appDeployment,
                              const DeploymentList& components,
                              DeploymentList::const_iterator current,
                              ossie::DeviceList& deploymentDevices,
                              const redhawk::PropertyMap& deviceRequires=redhawk::PropertyMap(),
                              const ReservationList& reservations=ReservationList(),
                              const ProcessorList& processorDeps=ProcessorList(),
                              const OSList& osDeps=OSList());

    void _handleUsesDevices(redhawk::ApplicationDeployment& appDeployment,
                            const std::string& appName);
    std::vector<std::string> _getFailedUsesDevices(const std::vector<ossie::UsesDevice>& usesDevices,
                                                   redhawk::UsesDeviceDeployment& assignedDevices);
    bool _allDevicesBusy(ossie::DeviceList& devices);

    CF::Properties _consolidateAllocations(redhawk::ApplicationDeployment& appDeployment, const DeploymentList& implementations);
    void _evaluateMATHinRequest(CF::Properties &request, const CF::Properties &configureProperties);
    void _castRequestProperties(CF::Properties& allocationProperties, const std::vector<ossie::PropertyRef> &prop_refs, unsigned int offset=0);

    // Supports allocation
    bool allocateUsesDevices(const std::vector<ossie::UsesDevice>& usesDevices,
                             const CF::Properties& configureProperties,
                             redhawk::UsesDeviceDeployment& assignedDevices,
                             ScopedAllocations& allocations);
    CF::AllocationManager::AllocationResponseSequence* allocateUsesDeviceProperties(
        const std::vector<ossie::UsesDevice>& component,
        const CF::Properties& configureProperties);
    void allocateComponent(redhawk::ApplicationDeployment& appDeployment,
                           redhawk::ComponentDeployment* deployment,
                           const std::string& assignedDeviceId,
                           const std::map<std::string,float>& specialized_reservations);

    ossie::AllocationResult allocateComponentToDevice(redhawk::ComponentDeployment* deployment,
                                                      const std::string& assignedDeviceId,
                                                      const std::string& appIdentifier,
                                                      const std::map<std::string,float>& specialized_reservations);

    bool checkPartitionMatching( ossie::DeviceNode& node,
                                 const CF::Properties& devicerequires );

    bool allocateHostCollocation(redhawk::ApplicationDeployment& appDeployment,
                                 const DeploymentList& components,
                                 ossie::DeviceList& deploymentDevices,
                                 const ProcessorList& processorDeps,
                                 const OSList& osDeps,
                                 const redhawk::PropertyMap &,
                                 const ReservationList& reservations);

    bool resolveSoftpkgDependencies(redhawk::ApplicationDeployment& appDeployment,
                                    redhawk::SoftPkgDeployment* deployment,
                                    ossie::DeviceNode& device);
    redhawk::SoftPkgDeployment* resolveDependencyImplementation(redhawk::ApplicationDeployment& appDeployment,
                                                                const ossie::SPD::SoftPkgRef& ref,
                                                                ossie::DeviceNode& device);
    
    // Supports loading, executing, initializing, configuring, & connecting
    void loadAndExecuteContainers(const ContainerList& containers,
                                  CF::ApplicationRegistrar_ptr _appReg);
    void waitForContainerRegistration(redhawk::ApplicationDeployment& appDeployment);

    void loadAndExecuteComponents(const DeploymentList& deployments,
                                  CF::ApplicationRegistrar_ptr _appReg);
    void applyApplicationAffinityOptions(const DeploymentList& deployments);

    void attemptComponentExecution(CF::ApplicationRegistrar_ptr registrar, redhawk::ComponentDeployment* deployment);

    void waitForComponentRegistration(redhawk::ApplicationDeployment& appDeployment);
    void initializeComponents(const DeploymentList& deployments);

    void configureComponents(const DeploymentList& deployments);
    void connectComponents(redhawk::ApplicationDeployment& appDeployment,
        std::vector<ossie::ConnectionNode>& connections, 
        std::string                         base_naming_context);

    int  resolveDebugLevel( const std::string &level_in );
    void resolveLoggingConfiguration(redhawk::ComponentDeployment* deployment, redhawk::PropertyMap &execParams );
    std::vector<std::string> getStartOrder(const DeploymentList& deployments);
    void verifyNoCpuSpecializationCollisions(const ossie::SoftwareAssembly& sad, std::map<std::string,float> specialized_reservations);
    std::vector<std::string> getComponentUsageNames(redhawk::ApplicationDeployment& appDeployment);
    std::vector<std::string> getHostCollocationsIds();

    // Cleanup - used when create fails/doesn't succeed for some reason
    bool _isComplete;
    void _cleanupFailedCreate();
    Application_impl* _application;
    float _stopTimeout;
    bool _aware;
};
#endif
