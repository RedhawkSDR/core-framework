#ifndef ECM_CPP_IMPL_H
#define ECM_CPP_IMPL_H

#include "ECM_CPP_base.h"

class ECM_CPP_i : public ECM_CPP_base
{
    ENABLE_LOGGING
    public:
        ECM_CPP_i(const char *uuid, const char *label);
        ~ECM_CPP_i();
        int serviceFunction();
        void initialize() throw  (CF::LifeCycle::InitializeError, CORBA::SystemException);
        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        redhawk::events::ManagerPtr ecm;
        redhawk::events::PublisherPtr pub;
        redhawk::events::SubscriberPtr sub;
        int msg_id;
};

#endif // ECM_CPP_IMPL_H
