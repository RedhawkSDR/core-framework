#ifndef ZERO_LENGTH_BASE_IMPL_BASE_H
#define ZERO_LENGTH_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include "struct_props.h"

class zero_length_base : public Component, protected ThreadedComponent
{
    public:
        zero_length_base(const char *uuid, const char *label);
        ~zero_length_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Property: mystruct
        mystruct_struct mystruct;

    private:
};
#endif // ZERO_LENGTH_BASE_IMPL_BASE_H
