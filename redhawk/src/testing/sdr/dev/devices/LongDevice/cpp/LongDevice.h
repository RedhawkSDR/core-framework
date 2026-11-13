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
        void stop();
        int serviceFunction();

        char* identifier ();
        CORBA::Boolean  started ();
        char *softwareProfile();

        CF::Device::UsageType usageState();
        CF::Device::AdminType adminState();
        CF::Device::OperationalType operationalState();
        char *label();
        CORBA::Boolean allocateCapacity( const CF::Properties &props );
        void  deallocateCapacity( const CF::Properties &props );

    protected:
        void updateUsageState();
        void _delay( const std::string &from);
        boost::condition_variable condition;
        boost::mutex  delay_mutex;
};

#endif // LONGDEVICE_I_IMPL_H
