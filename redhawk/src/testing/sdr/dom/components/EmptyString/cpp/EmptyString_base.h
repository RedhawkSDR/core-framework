#ifndef EMPTYSTRING_BASE_IMPL_BASE_H
#define EMPTYSTRING_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class EmptyString_base : public Component, protected ThreadedComponent
{
    public:
        EmptyString_base(const char *uuid, const char *label);
        ~EmptyString_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Property: estr
        std::string estr;

    private:
};
#endif // EMPTYSTRING_BASE_IMPL_BASE_H
