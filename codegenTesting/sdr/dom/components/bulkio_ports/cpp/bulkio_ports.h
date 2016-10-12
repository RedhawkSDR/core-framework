#ifndef BULKIO_PORTS_IMPL_H
#define BULKIO_PORTS_IMPL_H

#include "bulkio_ports_base.h"

class bulkio_ports_i;

class bulkio_ports_i : public bulkio_ports_base
{
    ENABLE_LOGGING
    public:
        bulkio_ports_i(const char *uuid, const char *label);
        ~bulkio_ports_i();
        int serviceFunction();

        void mqdChanged (const std::string&);

    private:
};

#endif
