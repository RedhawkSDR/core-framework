#ifndef LOGGER_I_IMPL_H
#define LOGGER_I_IMPL_H

#include "logger_base.h"

class logger_i : public logger_base
{
    ENABLE_LOGGING
    public:
        logger_i(const char *uuid, const char *label);
        ~logger_i();

        void constructor();

        int serviceFunction();
        rh_logger::LoggerPtr baseline_1_logger;
        rh_logger::LoggerPtr baseline_2_logger;
        rh_logger::LoggerPtr namespaced_logger;
        rh_logger::LoggerPtr basetree_logger;
};

#endif // LOGGER_I_IMPL_H
