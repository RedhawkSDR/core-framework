#ifndef LONGDEVICE_I_IMPL_H
#define LONGDEVICE_I_IMPL_H

#include "LongDevice_base.h"
#include <boost/thread/condition_variable.hpp>

class LongDevice_i : public LongDevice_base
{
    ENABLE_LOGGING
    public:
        LongDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        LongDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        LongDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        LongDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~LongDevice_i();

        void constructor();
        void stop() throw (CF::Resource::StopError, CORBA::SystemException);
        int serviceFunction();

        char* identifier () throw (CORBA::SystemException);
        CORBA::Boolean  started () throw (CORBA::SystemException);
        char *softwareProfile() throw (CORBA::SystemException);

        CF::Device::UsageType usageState() throw (CORBA::SystemException);
        CF::Device::AdminType adminState() throw (CORBA::SystemException);
        CF::Device::OperationalType operationalState() throw (CORBA::SystemException);
        char *label() throw (CORBA::SystemException);
        CORBA::Boolean allocateCapacity( const CF::Properties &props )
            throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CF::Device::InsufficientCapacity, CORBA::SystemException);
        void  deallocateCapacity( const CF::Properties &props )
            throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CORBA::SystemException);

    protected:
        void updateUsageState();
        void _delay( const std::string &from);
        boost::condition_variable condition;
        boost::mutex  delay_mutex;
};

#endif // LONGDEVICE_I_IMPL_H
