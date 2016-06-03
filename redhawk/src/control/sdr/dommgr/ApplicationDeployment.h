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

namespace ossie {

    class ApplicationDeployment : public ComponentLookup, public DeviceLookup, public UsesDeviceDeployment
    {
        ENABLE_LOGGING;

    public:
        typedef std::vector<ComponentDeployment*> ComponentList;
        typedef std::map<std::string,float> CpuReservations;

        ApplicationDeployment(const SoftwareAssembly& sad,
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

        ComponentDeployment* createComponentDeployment(const SoftPkg* softpkg,
                                                       const ComponentInstantiation* instantiation);

        const ComponentList& getComponentDeployments();
        ComponentDeployment* getComponentDeployment(const std::string& instantiationId);

        void applyCpuReservations(const CpuReservations& reservations);

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
        void overrideAssemblyControllerProperties(ComponentDeployment* deployment);
        void overrideExternalProperties(ComponentDeployment* deployment);

        const SoftwareAssembly& sad;
        const std::string identifier;
        const std::string instanceName;
        redhawk::PropertyMap initConfiguration;
        ComponentList components;
    };
}

#endif // APPLICATIONDEPLOYMENT_H
