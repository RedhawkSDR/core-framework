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

#ifndef DYNAMICCOMPONENT_H
#define	DYNAMICCOMPONENT_H
#include <boost/scoped_ptr.hpp>
#include "Resource_impl.h"
#include "Device_impl.h"
#include "ossie/debug.h"
#include "ossie/Events.h"
#include "ossie/Autocomplete.h"

class DynamicComponent {
public:
    DynamicComponent(Device_impl* base_device);
    void setParentInstance(DynamicComponent *parent);
    void removeInstance(Resource_impl* instance);
    Resource_impl* addInstance(std::string instance_name);
    ~DynamicComponent();

    std::vector<Resource_impl*> _dynamicComponents;
    std::map<std::string, int> _dynamicComponentCount;
    Device_impl* _base_device;

protected:
    DynamicComponent *_parentInstance;
};

class DynamicComponentParent : public DynamicComponent {
public:
    DynamicComponentParent(Device_impl* base_device);
    void removeInstance(Resource_impl* instance);
    Resource_impl* addInstance(std::string instance_name, DynamicComponent *parent=NULL);
    void local_start_device(Device_impl::ctor_type ctor, DynamicComponent* parent, std::vector< std::string > &parameters);
    ~DynamicComponentParent();

protected:
    boost::mutex dynamicComponentDeploymentLock;
};

#endif	/* DYNAMICCOMPONENT_H */

