#ifndef MSG_THROUGH_CPP_I_IMPL_H
#define MSG_THROUGH_CPP_I_IMPL_H

#include "msg_through_cpp_base.h"

class msg_through_cpp_i : public msg_through_cpp_base
{
    ENABLE_LOGGING
    public:
        msg_through_cpp_i(const char *uuid, const char *label);
        ~msg_through_cpp_i();

        void constructor();

        int serviceFunction();
        void msg_callback(const std::string& id, const foo_struct& msg);
};

#endif // MSG_THROUGH_CPP_I_IMPL_H
