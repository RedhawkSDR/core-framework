#ifndef HUGE_MSG_CPP_BASE_IMPL_BASE_H
#define HUGE_MSG_CPP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include <ossie/MessageInterface.h>
#include "struct_props.h"

class huge_msg_cpp_base : public Component, protected ThreadedComponent
{
    public:
        huge_msg_cpp_base(const char *uuid, const char *label);
        ~huge_msg_cpp_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Message structure definition for my_msg
        my_msg_struct my_msg;

        // Ports
        /// Port: output
        MessageSupplierPort *output;

    private:
};
#endif // HUGE_MSG_CPP_BASE_IMPL_BASE_H
