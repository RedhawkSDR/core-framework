#ifndef BASE_PERSONA_I_IMPL_H
#define BASE_PERSONA_I_IMPL_H

#include "base_persona_persona_base.h"

class base_persona_i;

class base_persona_i : public base_persona_persona_base
{
    ENABLE_LOGGING
    public:
        base_persona_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        base_persona_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        base_persona_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        base_persona_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~base_persona_i();
        int serviceFunction();
        CORBA::Boolean allocateCapacity(const CF::Properties& capacities);
        void deallocateCapacity(const CF::Properties& capacities);

    protected:
        void hwLoadRequest(CF::Properties& request);

    private:
        Resource_impl* generateResource(int argc, char* argv[], ConstructorPtr fnptr, const char* libName);
};

#endif // BASE_PERSONA_I_IMPL_H
