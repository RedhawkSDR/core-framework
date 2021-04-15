#ifndef SRC_BASE_IMPL_BASE_H
#define SRC_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include <bulkio/bulkio.h>

class src_base : public Component, protected ThreadedComponent
{
    public:
        src_base(const char *uuid, const char *label);
        ~src_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:

        // Ports
        /// Port: dataFloat
        bulkio::OutFloatPort *dataFloat;

    private:
};
#endif // SRC_BASE_IMPL_BASE_H
