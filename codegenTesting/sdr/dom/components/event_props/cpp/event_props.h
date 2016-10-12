 
#ifndef EVENT_PROPS_IMPL_H
#define EVENT_PROPS_IMPL_H

#include "event_props_base.h"

class event_props_i;

class event_props_i : public event_props_base
{
    ENABLE_LOGGING
    public: 
        event_props_i(const char *uuid, const char *label);
        ~event_props_i();
        int serviceFunction();

        void propToSendChanged (const std::string&);
};

#endif
