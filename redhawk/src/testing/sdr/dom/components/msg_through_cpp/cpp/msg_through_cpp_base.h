#ifndef MSG_THROUGH_CPP_BASE_IMPL_BASE_H
#define MSG_THROUGH_CPP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include <ossie/MessageInterface.h>
#include "struct_props.h"

class msg_through_cpp_base : public Component, protected ThreadedComponent
{
    public:
        msg_through_cpp_base(const char *uuid, const char *label);
        ~msg_through_cpp_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Message structure definition for foo
        foo_struct foo;

        // Ports
        /// Port: input
        MessageConsumerPort *input;
        /// Port: output
        MessageSupplierPort *output;

    private:
};
#endif // MSG_THROUGH_CPP_BASE_IMPL_BASE_H
