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

Resource_impl* DynamicComponent::addInstance(std::string instance_name)
{
    if (this->_parentInstance == NULL) {
        throw std::runtime_error("No parent device set, setParentInstance should have been invoked on device deployment");
    }

    Resource_impl* instance_device = this->_parentInstance->addInstance(instance_name);//, self)
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

Resource_impl* DynamicComponentParent::addInstance(std::string instance_name, DynamicComponent *parent)
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
    std::vector< std::string > parameters;

    CORBA::ORB_ptr orb = ossie::corba::Orb();
    parameters.push_back("IDM_CHANNEL_IOR");
    parameters.push_back(_base_device->getIDM());
    parameters.push_back("DEVICE_LABEL");
    parameters.push_back(device_label.c_str());
    parameters.push_back("PROFILE_NAME");
    parameters.push_back("");
    parameters.push_back("DEVICE_MGR_IOR");
    parameters.push_back(orb->object_to_string(_base_device->getDeviceManager()->getRef()));
    parameters.push_back("DEVICE_ID");
    parameters.push_back(device_id.c_str());
    parameters.push_back("COMPOSITE_DEVICE_IOR");
    parameters.push_back(orb->object_to_string(_base_device->_this()));

    /*mod = __import__(device_name)
    kclass = getattr(mod, device_name+'_i')
    device_object = self.local_start_device(kclass, this, argc, argv)
    parent._dynamicComponents.append(device_object)
    self.__dynamicComponentRegistry.register(device_object, parent)*/
    return device_object;
}

void DynamicComponentParent::local_start_device(Device_impl::ctor_type ctor, DynamicComponent* parent, std::vector< std::string > &parameters)
{
    /*char* devMgr_ior = 0;
    char* id = 0;
    char* label = 0;
    char* profile = 0;
    //char* idm_channel_ior = 0;
    std::string idm_channel_ior("");
    char* composite_device = 0;
    const char* logging_config_uri = 0;
    int debug_level = -1; // use log level from configuration file 
    std::string logcfg_uri("");
    std::string log_dpath("");
    std::string log_id("");
    std::string log_label("");
    bool skip_run = false;
    bool enablesigfd=false;

    for (int index = 1; index < argc; ++index) {
        if (std::string(argv[index]) == std::string("-i")) {
            std::cout<<"Interactive mode (-i) no longer supported. Please use the sandbox to run Components/Devices/Services outside the scope of a Domain"<<std::endl;
            exit(-1);
        }
    }

    std::map<std::string, char*> execparams;
    for (int i = 0; i < argc; i++) {
        if (strcmp("DEVICE_MGR_IOR", argv[i]) == 0) {
            devMgr_ior = argv[++i];
        } else if (strcmp("PROFILE_NAME", argv[i]) == 0) {
            profile = argv[++i];
        } else if (strcmp("DEVICE_ID", argv[i]) == 0) {
            id = argv[++i];
            log_id = id;
        } else if (strcmp("DEVICE_LABEL", argv[i]) == 0) {
            label = argv[++i];
            log_label = label;
        } else if (strcmp("IDM_CHANNEL_IOR", argv[i]) == 0) {
            idm_channel_ior = argv[++i];
        } else if (strcmp("COMPOSITE_DEVICE_IOR", argv[i]) == 0) {
            composite_device = argv[++i];
        } else if (strcmp("LOGGING_CONFIG_URI", argv[i]) == 0) {
            logging_config_uri = argv[++i];
        } else if (strcmp("DEBUG_LEVEL", argv[i]) == 0) {
            debug_level = atoi(argv[++i]);
        } else if (strcmp("DOM_PATH", argv[i]) == 0) {
            log_dpath = argv[++i];
        } else if (strcmp("USESIGFD", argv[i]) == 0){
            enablesigfd = true;
        } else if (strcmp("SKIP_RUN", argv[i]) == 0){
            skip_run = true;
            i++;             // skip flag has bogus argument need to skip over so execparams is processed correctly
        } else if (i > 0) {  // any other argument besides the first one is part of the execparams
            std::string paramName = argv[i];
            execparams[paramName] = argv[++i];
        }
    }

    std::string logname = log_label+".system.Device";

    // check if logging config URL was specified...
    if ( logging_config_uri ) logcfg_uri=logging_config_uri;

    // setup logging context for this resource
    ossie::logging::ResourceCtxPtr ctx( new ossie::logging::DeviceCtx( log_label, log_id, log_dpath ) );

    if ((devMgr_ior == 0) || (id == 0) || (profile == 0) || (label == 0)) {
        RH_NL_FATAL(logname, "Per SCA specification SR:478, DEVICE_MGR_IOR, PROFILE_NAME, DEVICE_ID, and DEVICE_LABEL must be provided");
        exit(-1);
    }

    RH_NL_DEBUG(logname, "Identifier = " << id << "Label = " << label << " Profile = " << profile << " IOR = " << devMgr_ior);

    Device_impl* device = ctor(devMgr_ior, id, label, profile, composite_device);

    // assign logging context to the resource..to support logging interface
    device->saveLoggingContext( logcfg_uri, debug_level, ctx );

    // setting all the execparams passed as argument, this method resides in the Resource_impl class
    device->setExecparamProperties(execparams);

    //perform post construction operations for the device
    std::string tmp_devMgr_ior = devMgr_ior;
    std::string tmp_profile = profile;
    std::string nic = "";
    int sig_fd=-1;
    device->postConstruction( tmp_profile, tmp_devMgr_ior, idm_channel_ior, nic, sig_fd);*/
}

DynamicComponentParent::~DynamicComponentParent()
{
}
