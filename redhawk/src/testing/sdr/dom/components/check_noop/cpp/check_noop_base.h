#ifndef CHECK_NOOP_BASE_IMPL_BASE_H
#define CHECK_NOOP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class check_noop_base : public Component, protected ThreadedComponent
{
    public:
        check_noop_base(const char *uuid, const char *label);
        ~check_noop_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Property: noop_delay
        float noop_delay;
        /// Property: evaluate
        std::string evaluate;
        /// Property: iterations
        short iterations;
        /// Property: average_delay
        float average_delay;

    private:
};
#endif // CHECK_NOOP_BASE_IMPL_BASE_H
