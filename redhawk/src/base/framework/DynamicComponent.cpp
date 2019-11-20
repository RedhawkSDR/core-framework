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


#include "ossie/DynamicComponent.h"

DynamicComponent::DynamicComponent(Device_impl* base_device)
{
    _parentInstance = NULL;
    _base_device = base_device;
}

DynamicComponent::~DynamicComponent()
{
    //delete _devices;
}

void DynamicComponent::setParentInstance(DynamicComponent *parent)
{
    _parentInstance = parent;
}

Resource_impl* DynamicComponent::addInstance(Device_impl::ctor_type ctor, std::string instance_name)
{
    if (this->_parentInstance == NULL) {
        throw std::runtime_error("No parent device set, setParentInstance should have been invoked on device deployment");
    }

    Resource_impl* instance_device = this->_parentInstance->addInstance(ctor, instance_name);//, self)
    return  instance_device;
}

void DynamicComponent::removeInstance(Resource_impl* instance)
{
    /*unsigned int devSeqLength = _devices->length();

    for (unsigned int i = 0; i < devSeqLength; i++) {
        if (!strcmp(associatedDevice->identifier(), (*_devices)[i]->identifier())) {
            for (unsigned int j = i + 1; j < devSeqLength; j++) {
                (*_devices)[j-1] = (*_devices)[j];
            }
            _devices->length(devSeqLength - 1);
        }
    }

    return;*/
}

DynamicComponentParent::DynamicComponentParent(Device_impl* base_device) : DynamicComponent(base_device)
{
}

void DynamicComponentParent::removeInstance(Resource_impl* instance)
{
}

Resource_impl* DynamicComponentParent::addInstance(Device_impl::ctor_type ctor, std::string instance_name, DynamicComponent *parent)
{
    SCOPED_LOCK(dynamicComponentDeploymentLock);

    if (parent==NULL)
        parent = this;

    Resource_impl* device_object = NULL;

    if (parent->_dynamicComponentCount.find(instance_name) == parent->_dynamicComponentCount.end())
        parent->_dynamicComponentCount[instance_name] = 0;
    parent->_dynamicComponentCount[instance_name] += 1;

    std::ostringstream device_name_count_stream;
    device_name_count_stream << instance_name << "_" << parent->_dynamicComponentCount[instance_name];
    std::string device_name_count = device_name_count_stream.str();
    std::string device_label = ossie::corba::returnString(parent->_base_device->label())+":"+device_name_count;
    std::string device_id = ossie::corba::returnString(parent->_base_device->identifier())+":"+device_name_count;
    std::map< std::string, std::string > parameters;

    CORBA::ORB_ptr orb = ossie::corba::Orb();
    parameters["IDM_CHANNEL_IOR"] = _base_device->getIDM();
    parameters["DEVICE_LABEL"] = device_label.c_str();
    //parameters["PROFILE_NAME"] = "";
    parameters["DEVICE_MGR_IOR"] = orb->object_to_string(_base_device->getDeviceManager()->getRef());
    parameters["DEVICE_ID"] = device_id.c_str();
    parameters["COMPOSITE_DEVICE_IOR"] = orb->object_to_string(_base_device->_this());

    device_object = this->local_start_device(ctor, parent, parameters);

    /*parent._dynamicComponents.append(device_object)
    self.__dynamicComponentRegistry.register(device_object, parent)*/
    return device_object;
}

Resource_impl* DynamicComponentParent::local_start_device(Device_impl::ctor_type ctor, DynamicComponent* parent, std::map< std::string, std::string > &parameters)
{
    char* devMgr_ior = (char *)parameters["DEVICE_MGR_IOR"].c_str();
    char* id = (char *)parameters["DEVICE_ID"].c_str();
    char* label = (char *)parameters["DEVICE_LABEL"].c_str();
    std::string idm_channel_ior = parameters["IDM_CHANNEL_IOR"];
    char* composite_device = (char *)parameters["COMPOSITE_DEVICE_IOR"].c_str();
    std::string s_profile;
    char* profile = (char *)s_profile.c_str();

    std::string logging_config_uri;
    int debug_level = -1; // use log level from configuration file 
    std::string logcfg_uri;
    std::string log_dpath;

    std::string logname = parameters["DEVICE_LABEL"]+".system.Device";

    // check if logging config URL was specified...
    if ( not logging_config_uri.empty() )
        logcfg_uri=logging_config_uri;

    // setup logging context for this resource
    ossie::logging::ResourceCtxPtr ctx( new ossie::logging::DeviceCtx( label, id, log_dpath ) );

    RH_NL_DEBUG(logname, "Identifier = " << id << "Label = " << label << " Profile = " << profile << " IOR = " << devMgr_ior);

    Device_impl* device = ctor(devMgr_ior, id, label, profile, composite_device);

    // assign logging context to the resource..to support logging interface
    device->saveLoggingContext( logcfg_uri, debug_level, ctx );

    //perform post construction operations for the device
    std::string nic;
    std::string s_devMgr_ior = parameters["DEVICE_MGR_IOR"];
    int sig_fd=-1;
    device->postConstruction( s_profile, s_devMgr_ior, idm_channel_ior, nic, sig_fd);
    return device;
}

DynamicComponentParent::~DynamicComponentParent()
{
}
