#ifndef SRI_IMPL_H
#define SRI_IMPL_H

#include "sri_base.h"

class sri_i;

class sri_i : public sri_base
{
    ENABLE_LOGGING
    public:
        sri_i(const char *uuid, const char *label);
        ~sri_i();
        int serviceFunction();

        void mqdChanged (const std::string&);
};

#endif
