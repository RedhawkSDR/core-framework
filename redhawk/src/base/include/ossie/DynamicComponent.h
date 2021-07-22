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
#include <boost/filesystem.hpp>
#include "ossie/Autocomplete.h"

namespace fs = boost::filesystem;

class DynamicComponent {
public:
    DynamicComponent();
    void setHost(Device_impl* base_device);
    void setParentInstance(DynamicComponent *parent);
    void removeInstance(Device_impl* instance);
    template<class T>
    T* addChild(const std::string &instance_name)
    {
        SCOPED_LOCK(_dynamicComponentDeploymentLock);

        if (this->_dynamicComponentCount.find(instance_name) == this->_dynamicComponentCount.end())
            this->_dynamicComponentCount[instance_name] = 0;
        this->_dynamicComponentCount[instance_name] += 1;

        std::ostringstream device_name_count_stream;
        device_name_count_stream << instance_name << "_" << this->_dynamicComponentCount[instance_name];
        std::string device_name_count = device_name_count_stream.str();
        std::string device_label = ossie::corba::returnString(this->_base_device->label())+":"+device_name_count;
        std::string device_id = ossie::corba::returnString(this->_base_device->identifier())+":"+device_name_count;
        std::string profile_name = fs::path(this->_base_device->softwareProfile()).parent_path().string()+"/"+instance_name+".spd.xml";
        
        std::map< std::string, std::string > parameters;

        CORBA::ORB_ptr orb = ossie::corba::Orb();
        parameters["IDM_CHANNEL_IOR"] = _base_device->getIDMIOR();
        parameters["DEVICE_LABEL"] = device_label.c_str();
        parameters["PROFILE_NAME"] = profile_name.c_str();
        parameters["DEVICE_MGR_IOR"] = orb->object_to_string(_base_device->getDeviceManager()->getRef());
        parameters["DEVICE_ID"] = device_id.c_str();
        parameters["COMPOSITE_DEVICE_IOR"] = orb->object_to_string(_base_device->_this());

        T* device_object = this->dynamic_start_device<T>(parameters);
        _dynamicComponents.push_back((Device_impl*)device_object);
        DynamicComponent* base_dev = dynamic_cast<DynamicComponent*>(device_object);
        base_dev->setParentInstance(this);

        return device_object;
    };

    Device_impl* create_device_instance(Device_impl::ctor_type ctor, std::map< std::string, std::string > &parameters);

    template<class T>
    T* dynamic_start_device(std::map< std::string, std::string > &parameters) {
        T* devPtr = 0;
        Device_impl* new_dev = create_device_instance(boost::bind(&Device_impl::make_device<T>,boost::ref(devPtr),_1,_2,_3,_4,_5), parameters);
        devPtr = dynamic_cast<T*>(new_dev);
        return devPtr;
    }

    ~DynamicComponent();

    std::vector<Device_impl*> _dynamicComponents;
    std::map<std::string, int> _dynamicComponentCount;
    Device_impl* _base_device;

protected:
    boost::mutex _dynamicComponentDeploymentLock;
    DynamicComponent *_parentInstance;
};

#endif	/* DYNAMICCOMPONENT_H */

