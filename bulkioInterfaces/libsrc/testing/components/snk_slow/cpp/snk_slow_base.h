#ifndef SNK_SLOW_BASE_IMPL_BASE_H
#define SNK_SLOW_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include <bulkio/bulkio.h>

class snk_slow_base : public Component, protected ThreadedComponent
{
    public:
        snk_slow_base(const char *uuid, const char *label);
        ~snk_slow_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:

        // Ports
        /// Port: dataFloat
        bulkio::InFloatPort *dataFloat;

    private:
};
#endif // SNK_SLOW_BASE_IMPL_BASE_H
