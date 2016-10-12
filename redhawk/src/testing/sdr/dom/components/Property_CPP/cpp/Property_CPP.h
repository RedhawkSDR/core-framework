#ifndef PROPERTY_CPP_I_IMPL_H
#define PROPERTY_CPP_I_IMPL_H

#include "Property_CPP_base.h"

class Property_CPP_i : public Property_CPP_base
{
    ENABLE_LOGGING
    public:
        Property_CPP_i(const char *uuid, const char *label);
        ~Property_CPP_i();
        int serviceFunction();
        void initializeProperties(const CF::Properties& ctorProps)
          throw (CF::PropertySet::PartialConfiguration,
                 CF::PropertySet::InvalidConfiguration, CORBA::SystemException);
};

#endif // CPROPERTY_I_IMPL_H
