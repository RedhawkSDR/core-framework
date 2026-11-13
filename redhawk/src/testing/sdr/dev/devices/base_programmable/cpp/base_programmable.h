#ifndef BASE_PROGRAMMABLE_I_IMPL_H
#define BASE_PROGRAMMABLE_I_IMPL_H

#include "base_programmable_prog_base.h"

typedef base_programmable_prog_base<> base_programmable_prog_base_type;


class base_programmable_i;

class base_programmable_i : public base_programmable_prog_base_type
{
    ENABLE_LOGGING
    public:
        base_programmable_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        base_programmable_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        base_programmable_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        base_programmable_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~base_programmable_i();
        int serviceFunction();
        void initialize();

    protected:
        Device_impl* generatePersona(int argc, char* argv[], ConstructorPtr fnptr, const char* libName);
        bool loadHardware(HwLoadStatusStruct& requestStatus);
        void unloadHardware(const HwLoadStatusStruct& requestStatus);
        
};

#endif // BASE_PROGRAMMABLE_I_IMPL_H
