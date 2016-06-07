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
#include <stdexcept>

#include <boost/shared_ptr.hpp>

#include <ossie/SoftwareAssembly.h>
#include <ossie/PropertyMap.h>

namespace ossie {
    class ComponentInstantiation;
    class DeviceNode;
}

namespace redhawk {

    class ComponentDeployment;

    class deployment_error : public std::runtime_error {
    public:
        deployment_error(const ComponentDeployment* deployment, const std::string& message);

        virtual ~deployment_error() throw ()
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

    class placement_failure : public std::runtime_error {
    public:
        placement_failure(const ossie::ComponentInstantiation* instantiation, const std::string& message);

        placement_failure(const ossie::SoftwareAssembly::HostCollocation& collocation, const std::string& message);

        virtual ~placement_failure() throw ()
        {
        }

        const std::string& name() const
        {
            return _name;
        }

    private:
        std::string _name;
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

}

#endif // DEPLOYMENTEXCEPTIONS_H
