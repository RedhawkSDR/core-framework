#ifndef EVENT_PROPS_I_IMPL_H
#define EVENT_PROPS_I_IMPL_H

#include "event_props_base.h"

class event_props_i : public event_props_base
{
    ENABLE_LOGGING
    public:
        event_props_i(const char *uuid, const char *label);
        ~event_props_i();

        void constructor();

        int serviceFunction();

    protected:
};

#endif // EVENT_PROPS_I_IMPL_H
