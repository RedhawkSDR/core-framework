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

#ifndef DEPLOYMENTEXCEPTIONS_H
#define DEPLOYMENTEXCEPTIONS_H

#include <string>
#include <vector>
#include <stdexcept>

#include <boost/shared_ptr.hpp>

#include <ossie/SoftwareAssembly.h>
#include <ossie/PropertyMap.h>

namespace ossie {
    class ComponentInstantiation;
    class DeviceNode;
}

namespace redhawk {

    class ApplicationDeployment;
    class ComponentDeployment;

    class DeploymentError : public std::runtime_error {
    public:
        DeploymentError(const std::string& message) :
            std::runtime_error(message)
        {
        }
    };

    class UsesDeviceFailure : public DeploymentError {
    public:
        UsesDeviceFailure(const ApplicationDeployment& application, const std::vector<std::string>& ids);
        UsesDeviceFailure(const ComponentDeployment* component, const std::vector<std::string>& ids);

        virtual ~UsesDeviceFailure() throw()
        {
        }

        const std::string& context() const
        {
            return _context;
        }

        const std::vector<std::string>& ids() const
        {
            return _ids;
        }

    private:
        std::string _context;
        std::vector<std::string> _ids;
    };

    class ConnectionError : public DeploymentError {
    public:
        ConnectionError(const std::string& identifier, const std::string& message) :
            DeploymentError(message),
            _identifier(identifier)
        {
        }

        virtual ~ConnectionError() throw()
        {
        }

        const std::string& identifier() const
        {
            return _identifier;
        }

    private:
        const std::string _identifier;
    };

    class PlacementFailure : public DeploymentError {
    public:
        PlacementFailure(const ossie::ComponentInstantiation* instantiation, const std::string& message);

        PlacementFailure(const ossie::SoftwareAssembly::HostCollocation& collocation, const std::string& message);

        virtual ~PlacementFailure() throw ()
        {
        }

        const std::string& name() const
        {
            return _name;
        }

    private:
        std::string _name;
    };

    class ComponentError : public DeploymentError {
    public:
        ComponentError(const ComponentDeployment* deployment, const std::string& message);

        virtual ~ComponentError() throw ()
        {
        }

        const std::string& identifier() const
        {
            return _identifier;
        }

        const std::string& implementation() const
        {
            return _implementation;
        }

    private:
        std::string _identifier;
        std::string _implementation;
    };

    class ExecuteError : public ComponentError {
    public:
        ExecuteError(const ComponentDeployment* deployment,
                     const boost::shared_ptr<ossie::DeviceNode>& device,
                     const std::string& message) :
            ComponentError(deployment, message),
            _device(device)
        {
        }

        const boost::shared_ptr<ossie::DeviceNode>& device() const
        {
            return _device;
        }

        virtual ~ExecuteError() throw ()
        {
        }

    private:
        boost::shared_ptr<ossie::DeviceNode> _device;
    };

    class PropertiesError : public ComponentError {
    public:
        PropertiesError(const ComponentDeployment* deployment,
                         const CF::Properties& properties,
                         const std::string& message) :
            ComponentError(deployment, message),
            _properties(properties)
        {
        }

        virtual ~PropertiesError() throw ()
        {
        }

        const redhawk::PropertyMap& properties() const
        {
            return _properties;
        }

    private:
        const redhawk::PropertyMap _properties;
    };

}

#endif // DEPLOYMENTEXCEPTIONS_H
