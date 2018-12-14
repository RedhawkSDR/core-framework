#include "BasicService_cpp_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the service class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

BasicService_cpp_base::BasicService_cpp_base(char *devMgr_ior, char *name) :
    Service_impl(devMgr_ior, name)
{
}

void BasicService_cpp_base::registerServiceWithDevMgr ()
{
    _deviceManager->registerService(this->_this(), this->_name.c_str());
}

void BasicService_cpp_base::terminateService ()
{
    try {
        _deviceManager->unregisterService(this->_this(), this->_name.c_str());
    } catch (...) {
    }
    
    PortableServer::POA_ptr root_poa = ossie::corba::RootPOA();
    PortableServer::ObjectId_var oid = root_poa->servant_to_id(this);
    root_poa->deactivate_object(oid);
}
    
