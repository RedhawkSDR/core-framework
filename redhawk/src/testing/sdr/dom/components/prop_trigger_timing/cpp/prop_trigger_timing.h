#ifndef PROP_TRIGGER_TIMING_I_IMPL_H
#define PROP_TRIGGER_TIMING_I_IMPL_H

#include "prop_trigger_timing_base.h"

class prop_trigger_timing_i : public prop_trigger_timing_base
{
    ENABLE_LOGGING
    public:
        prop_trigger_timing_i(const char *uuid, const char *label);
        ~prop_trigger_timing_i();

        void constructor();

        int serviceFunction();
        void prop_1_changed(const std::string* oldValue, const std::string* newValue);
        void prop_2_changed(const std::vector<std::string>* oldValue, const std::vector<std::string>* newValue);
        void prop_3_changed(const prop_3_struct* oldValue, const prop_3_struct* newValue);
        void prop_4_changed(const std::vector<prop_4_a_struct>* oldValue, const std::vector<prop_4_a_struct>* newValue);
};

#endif // PROP_TRIGGER_TIMING_I_IMPL_H
