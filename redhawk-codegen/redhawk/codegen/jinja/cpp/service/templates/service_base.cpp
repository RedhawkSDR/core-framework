/*#
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
 #*/
//% set className = component.baseclass.name
//% set baseClass = component.superclass.name
//% set artifactType = component.artifacttype
#include "${component.baseclass.header}"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the ${artifactType} class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

${className}::${className}(char *devMgr_ior, char *name) :
    ${baseClass}(devMgr_ior, name)
{
}

void ${className}::registerServiceWithDevMgr ()
{
    _deviceManager->registerService(this->_this(), this->_name.c_str());
}

void ${className}::terminateService ()
{
    try {
        _deviceManager->unregisterService(this->_this(), this->_name.c_str());
    } catch (...) {
    }
    
    PortableServer::POA_ptr root_poa = ossie::corba::RootPOA();
    PortableServer::ObjectId_var oid = root_poa->servant_to_id(this);
    root_poa->deactivate_object(oid);
}
    
