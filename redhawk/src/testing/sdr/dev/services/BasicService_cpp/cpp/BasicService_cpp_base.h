#ifndef BASICSERVICE_CPP_BASE_IMPL_BASE_H
#define BASICSERVICE_CPP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Service_impl.h>
#include <CF/cf.h>

class BasicService_cpp_base : public Service_impl, public virtual POA_CF::FileManager
{
    public:
        BasicService_cpp_base(char *devMgr_ior, char *name);

        void registerServiceWithDevMgr ();
        void terminateService ();
        void construct ();

};
#endif // BASICSERVICE_CPP_BASE_IMPL_BASE_H
