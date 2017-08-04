#ifndef HUGE_MSG_CPP_I_IMPL_H
#define HUGE_MSG_CPP_I_IMPL_H

#include "huge_msg_cpp_base.h"

class huge_msg_cpp_i : public huge_msg_cpp_base
{
    ENABLE_LOGGING
    public:
        huge_msg_cpp_i(const char *uuid, const char *label);
        ~huge_msg_cpp_i();

        void constructor();

        int serviceFunction();
};

#endif // HUGE_MSG_CPP_I_IMPL_H
