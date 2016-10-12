#ifndef NOCOMMANDLINE_PROP_BASE_IMPL_BASE_H
#define NOCOMMANDLINE_PROP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class nocommandline_prop_base : public Component, protected ThreadedComponent
{
    public:
        nocommandline_prop_base(const char *uuid, const char *label);
        ~nocommandline_prop_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Property: testprop
        std::string testprop;

    private:
};
#endif // NOCOMMANDLINE_PROP_BASE_IMPL_BASE_H
