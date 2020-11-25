#ifndef TIMEPROP_CPP_BASE_IMPL_BASE_H
#define TIMEPROP_CPP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class timeprop_cpp_base : public Component, protected ThreadedComponent
{
    public:
        timeprop_cpp_base(const char *uuid, const char *label);
        ~timeprop_cpp_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Property: prop
        std::string prop;

    private:
};
#endif // TIMEPROP_CPP_BASE_IMPL_BASE_H
