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

#ifndef APPLICATIONDEPLOYMENT_H
#define APPLICATIONDEPLOYMENT_H

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <ossie/debug.h>
#include <ossie/PropertyMap.h>
#include <ossie/SoftwareAssembly.h>

#include "connectionSupport.h"
#include "Deployment.h"

namespace redhawk {

    class ProfileCache;

    class ContainerDeployment : public ComponentDeployment
    {
    public:
        ContainerDeployment(const ossie::SoftPkg* softpkg,
                            ossie::ComponentInstantiation* instantiation,
                            const std::string& identifier);

    protected:
        // The instantiation does not appear in a SAD; take ownership here
        boost::scoped_ptr<ossie::ComponentInstantiation> instance;
    };

    class ApplicationDeployment : public ossie::ComponentLookup, public ossie::DeviceLookup, public UsesDeviceDeployment
    {
        ENABLE_LOGGING;

    public:
        typedef std::vector<ComponentDeployment*> ComponentList;
        typedef std::vector<ContainerDeployment*> ContainerList;
        typedef std::map<std::string,float> CpuReservations;

        ApplicationDeployment(const ossie::SoftwareAssembly& sad,
                              const std::string& instanceName,
                              const CF::Properties& initConfiguration);
        ~ApplicationDeployment();

        const std::string& getIdentifier() const;

        /**
         * Returns the properties used for evaluating math statements in
         * allocation
         */
        redhawk::PropertyMap getAllocationContext() const;

        ComponentDeployment* getAssemblyController();

        ComponentDeployment* createComponentDeployment(const ossie::SoftPkg* softpkg,
                                                       const ossie::ComponentInstantiation* instantiation);

        const ComponentList& getComponentDeployments();
        ComponentDeployment* getComponentDeployment(const std::string& instantiationId);
        ComponentDeployment* getComponentDeploymentByUniqueId(const std::string& identifier);

        void applyCpuReservations(const CpuReservations& reservations);

        const ContainerList& getContainerDeployments();
        ContainerDeployment* createContainer(redhawk::ProfileCache& cache,
                                             const boost::shared_ptr<ossie::DeviceNode>& device);

        // Adapt interfaces for component and device search to support
        // ConnectionManager
        // ComponentLookup interface
        virtual CF::Resource_ptr lookupComponentByInstantiationId(const std::string& identifier);

        // DeviceLookup interface
        CF::Device_ptr lookupDeviceThatLoadedComponentInstantiationId(const std::string& componentId);
        CF::Device_ptr lookupDeviceUsedByComponentInstantiationId(const std::string& componentId,
                                                                  const std::string& usesId);
        CF::Device_ptr lookupDeviceUsedByApplication(const std::string& usesRefId);

        void setLogger(rh_logger::LoggerPtr log) {
            _appDeploymentLog = log;
        };

    protected:
        void overrideAssemblyControllerProperties(ComponentDeployment* deployment);
        void overrideExternalProperties(ComponentDeployment* deployment);
        void overrideImpliedProperties(ComponentDeployment* deployment);

        ContainerDeployment* getContainer(const std::string& deviceId);

        const ossie::SoftwareAssembly& sad;
        const std::string identifier;
        const std::string instanceName;
        redhawk::PropertyMap initConfiguration;
        ComponentList components;
        ContainerList containers;
        ComponentDeployment *ac;
        rh_logger::LoggerPtr _appDeploymentLog;
    };
}

#endif // APPLICATIONDEPLOYMENT_H
