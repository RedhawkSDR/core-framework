/**************************************************************************

    This is the device code. This file contains the child class where
    custom functionality can be added to the device. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "dev_alloc_cpp.h"

PREPARE_LOGGING(dev_alloc_cpp_i)

dev_alloc_cpp_i::dev_alloc_cpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    dev_alloc_cpp_base(devMgr_ior, id, lbl, sftwrPrfl)
{
}

dev_alloc_cpp_i::dev_alloc_cpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    dev_alloc_cpp_base(devMgr_ior, id, lbl, sftwrPrfl, compDev)
{
}

dev_alloc_cpp_i::dev_alloc_cpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    dev_alloc_cpp_base(devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
}

dev_alloc_cpp_i::dev_alloc_cpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    dev_alloc_cpp_base(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev)
{
}

dev_alloc_cpp_i::~dev_alloc_cpp_i()
{
}

void dev_alloc_cpp_i::constructor()
{
    /***********************************************************************************
     This is the RH constructor. All properties are properly initialized before this function is called 
    ***********************************************************************************/
    this->setAllocationImpl(s_prop, this, &dev_alloc_cpp_i::alloc_s_prop, &dev_alloc_cpp_i::dealloc_s_prop);
    this->setAllocationImpl(si_prop, this, &dev_alloc_cpp_i::alloc_si_prop, &dev_alloc_cpp_i::dealloc_si_prop);
    this->setAllocationImpl(se_prop, this, &dev_alloc_cpp_i::alloc_se_prop, &dev_alloc_cpp_i::dealloc_se_prop);
    this->setAllocationImpl(sq_prop, this, &dev_alloc_cpp_i::alloc_sq_prop, &dev_alloc_cpp_i::dealloc_sq_prop);
}

/**************************************************************************

    This is called automatically after allocateCapacity or deallocateCapacity are called.
    Your implementation should determine the current state of the device:

       setUsageState(CF::Device::IDLE);   // not in use
       setUsageState(CF::Device::ACTIVE); // in use, with capacity remaining for allocation
       setUsageState(CF::Device::BUSY);   // in use, with no capacity remaining for allocation

**************************************************************************/
void dev_alloc_cpp_i::updateUsageState()
{
}

bool dev_alloc_cpp_i::alloc_s_prop(const s_prop_struct &value)
{
    // perform logic
    return true; // successful allocation
}
void dev_alloc_cpp_i::dealloc_s_prop(const s_prop_struct &value)
{
    // perform logic
}
bool dev_alloc_cpp_i::alloc_si_prop(const short &value)
{
    // perform logic
    return true; // successful allocation
}
void dev_alloc_cpp_i::dealloc_si_prop(const short &value)
{
    // perform logic
}
bool dev_alloc_cpp_i::alloc_se_prop(const std::vector<float> &value)
{
    // perform logic
    return true; // successful allocation
}
void dev_alloc_cpp_i::dealloc_se_prop(const std::vector<float> &value)
{
    // perform logic
}
bool dev_alloc_cpp_i::alloc_sq_prop(const std::vector<sq_prop_s_struct> &value)
{
    // perform logic
    return true; // successful allocation
}
void dev_alloc_cpp_i::dealloc_sq_prop(const std::vector<sq_prop_s_struct> &value)
{
    // perform logic
}

int dev_alloc_cpp_i::serviceFunction()
{
    LOG_DEBUG(dev_alloc_cpp_i, "serviceFunction() example log message");
    
    return NOOP;
}

