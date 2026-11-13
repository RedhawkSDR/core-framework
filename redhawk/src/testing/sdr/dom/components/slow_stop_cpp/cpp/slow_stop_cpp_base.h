#ifndef SLOW_STOP_CPP_BASE_IMPL_BASE_H
#define SLOW_STOP_CPP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class slow_stop_cpp_base : public Component, protected ThreadedComponent
{
    public:
        slow_stop_cpp_base(const char *uuid, const char *label);
        ~slow_stop_cpp_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:

    private:
};
#endif // SLOW_STOP_CPP_BASE_IMPL_BASE_H
