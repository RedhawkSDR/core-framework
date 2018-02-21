#ifndef LOGGER_I_IMPL_H
#define LOGGER_I_IMPL_H

#include "logger_base.h"
#include <logging/rh_logger_p.h>

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
        log4cxx::LoggerPtr my_l4;
        rh_logger::LoggerPtr basel4_logger;
};

#endif // LOGGER_I_IMPL_H
