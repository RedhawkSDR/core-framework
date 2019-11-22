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

DynamicComponent::DynamicComponent()
{
    _parentInstance = NULL;
}

DynamicComponent::~DynamicComponent()
{
    //delete _devices;
}

void DynamicComponent::setHost(Device_impl* base_device)
{
    _base_device = base_device;
}

void DynamicComponent::setParentInstance(DynamicComponent *parent)
{
    _parentInstance = parent;
}

void DynamicComponent::removeInstance(Device_impl* instance)
{
}

DynamicComponentParent::DynamicComponentParent()
{
}

void DynamicComponentParent::removeInstance(Device_impl* instance)
{
}

Device_impl* DynamicComponentParent::create_device_instance(Device_impl::ctor_type ctor, DynamicComponent* parent, std::map< std::string, std::string > &parameters)
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
    device->initialize();
    return device;
}

DynamicComponentParent::~DynamicComponentParent()
{
}
