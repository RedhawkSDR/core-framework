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
            std::runtime_error(message),
            _errorNumber(CF::CF_NOTSET)
        {
        }

        DeploymentError(CF::ErrorNumberType errorNum, const std::string& message) :
            std::runtime_error(message),
            _errorNumber(errorNum)
        {
        }

        virtual std::string message() const
        {
            return std::string(what());
        }

        CF::ErrorNumberType errorNumber() const
        {
            return _errorNumber;
        }

    protected:
        void errorNumber(CF::ErrorNumberType errorNum)
        {
            _errorNumber = errorNum;
        }

    private:
        CF::ErrorNumberType _errorNumber;
    };

    class NoExecutableDevices : public DeploymentError {
    public:
        NoExecutableDevices() :
            DeploymentError(CF::CF_ENODEV, "Domain has no executable devices (GPPs) to run components")
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

        virtual std::string message() const;

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
            DeploymentError(CF::CF_EIO, message),
            _identifier(identifier)
        {
        }

        virtual ~ConnectionError() throw()
        {
        }

        virtual std::string message() const;

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

        virtual std::string message() const;

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

        virtual std::string message() const;

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
        ExecuteError(const ComponentDeployment* deployment, const std::string& message);

        const boost::shared_ptr<ossie::DeviceNode>& device() const
        {
            return _device;
        }

        virtual ~ExecuteError() throw ()
        {
        }

        virtual std::string message() const;

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

        virtual std::string message() const;

        const redhawk::PropertyMap& properties() const
        {
            return _properties;
        }

    private:
        const redhawk::PropertyMap _properties;
    };

    class BadExternalPort : public DeploymentError {
    public:
        BadExternalPort(const ossie::SoftwareAssembly::Port& port, const std::string& message);

        virtual ~BadExternalPort() throw ()
        {
        }

        virtual std::string message() const;

        const std::string& name() const
        {
            return _name;
        }

        const std::string& component() const
        {
            return _component;
        }

    private:
        const std::string _name;
        const std::string _component;
    };

    class ComponentTerminated : public DeploymentError {
    public:
        ComponentTerminated(const std::string& identifier) :
            DeploymentError("component terminated abnormally"),
            _identifier(identifier)
        {
        }

        virtual ~ComponentTerminated() throw ()
        {
        }

        virtual std::string message() const;

        const std::string& identifier() const
        {
            return _identifier;
        }

    private:
        const std::string _identifier;
    };
}

#endif // DEPLOYMENTEXCEPTIONS_H
