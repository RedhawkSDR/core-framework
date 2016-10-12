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

  

#include "BasicTestDevice_cpp_impl1.h"

BasicTestDevice_cpp_impl1_i::BasicTestDevice_cpp_impl1_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
          ExecutableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl) , AggregateDevice_impl (){
    initResource(devMgr_ior, id, lbl, sftwrPrfl);
};


BasicTestDevice_cpp_impl1_i::BasicTestDevice_cpp_impl1_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, 
                                                        char *compDev) :
          ExecutableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl, compDev) , AggregateDevice_impl (){
    initResource(devMgr_ior, id, lbl, sftwrPrfl);
};

BasicTestDevice_cpp_impl1_i::BasicTestDevice_cpp_impl1_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, 
                                                        CF::Properties capacities) :
          ExecutableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl) , AggregateDevice_impl (){
    initResource(devMgr_ior, id, lbl, sftwrPrfl);
};

BasicTestDevice_cpp_impl1_i::BasicTestDevice_cpp_impl1_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, 
                                                        CF::Properties capacities, char *compDev) :
          ExecutableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl, compDev) , AggregateDevice_impl (){
    initResource(devMgr_ior, id, lbl, sftwrPrfl);
};

void BasicTestDevice_cpp_impl1_i::initResource(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) 
{
    loadProperties();

    // component_alive flag is turned to false to terminate the main processing thread
    naming_service_name = lbl;

    //Initialize variables
    // TODO
    thread_started = false;
};
      
BasicTestDevice_cpp_impl1_i::~BasicTestDevice_cpp_impl1_i() {
};

CORBA::Boolean BasicTestDevice_cpp_impl1_i::allocateCapacity (const CF::Properties & capacities)
    throw (CORBA::SystemException, CF::Device::InvalidCapacity, CF::Device::InvalidState) 
{
    
    bool response = true;
    std::string tmp;
    propertyContainer *pCptr;

    for (unsigned int i = 0; i < capacities.length(); i++) {
        tmp = capacities[i].id;
        pCptr = getPropFromId(tmp);
        if (!pCptr) {
            response = false;
            break;
        } else if (pCptr->compare(capacities[i].value) > 0) {
            response = false;
            break;
        }
    }
    if (!response) {
        return false;
    }
        
    for (unsigned int i = 0; i < capacities.length(); i++) {
        tmp = capacities[i].id;
        pCptr = getPropFromId(tmp);
        pCptr->decrement(capacities[i].value);
    }
    return true;
}

void BasicTestDevice_cpp_impl1_i::deallocateCapacity (const CF::Properties & capacities)
    throw (CORBA::SystemException, CF::Device::InvalidCapacity, CF::Device::InvalidState) 
{

    std::string tmp;
    propertyContainer *pCptr;

    for (unsigned int i=0; i<capacities.length(); i++) {
        tmp = capacities[i].id;
        pCptr = getPropFromId(tmp);
        if (!pCptr) {
            continue;
        }
        pCptr->increment(capacities[i].value);
    }
}
