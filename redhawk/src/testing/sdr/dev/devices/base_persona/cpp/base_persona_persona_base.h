#ifndef BASE_PERSONA_BASE_IMPL_REPROG_H
#define BASE_PERSONA_BASE_IMPL_REPROG_H

#include "base_persona_base.h"
#include "ossie/Device_impl.h"
#include "ossie/ExecutableDevice_impl.h"
#include "ossie/prop_helpers.h"
#include "entry_point.h"
#include <dlfcn.h>

typedef std::string ResourceId;
typedef std::map<ResourceId, Resource_impl*> ResourceMap;
typedef ResourceMap::iterator ResourceMapIter;
typedef std::map<unsigned int, ResourceId> ProcessMap;
typedef ProcessMap::iterator ProcessMapIter;
class base_persona_persona_base;

class base_persona_persona_base : public base_persona_base
{
    ENABLE_LOGGING
    public:
        base_persona_persona_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        base_persona_persona_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        base_persona_persona_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        base_persona_persona_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        virtual void construct();
        virtual void setParentDevice(Device_impl* parentDevice) { _parentDevice = parentDevice; };
        virtual Device_impl* getParentDevice() { return _parentDevice; };

        virtual void adminState(CF::Device::AdminType adminState);
        virtual CF::ExecutableDevice::ProcessID_Type execute (const char* name, const CF::Properties& options, const CF::Properties& parameters);
        virtual void terminate (CF::ExecutableDevice::ProcessID_Type processId);
        virtual void releaseObject();

    protected:

        virtual void hwLoadRequest(CF::Properties& loadRequest)=0;
        virtual CORBA::Boolean attemptToProgramParent();
        virtual CORBA::Boolean attemptToUnprogramParent();

        // Lifecycle methods around the programming of parent/hardware
        //    These methods may be overriden to provide custom functionality
        //    at different stages in the lifecycle
        virtual void beforeHardwareProgrammed() {};
        virtual void afterHardwareProgramSuccess() {};
        virtual void afterHardwareProgramFailure() {};
        virtual void beforeHardwareUnprogrammed() {};
        virtual void afterHardwareUnprogrammed() {};

        virtual bool hasRunningResources();
        virtual Resource_impl* generateResource(int argc, char* argv[], ConstructorPtr fnptr, const char* libraryName)=0;
    private:
        Device_impl*            _parentDevice;
        bool                    _parentAllocated;
        CF::Properties          _previousRequestProps;
        ResourceMap             _resourceMap;
        ProcessMap              _processMap;
        unsigned int            _processIdIncrement;

        Resource_impl* instantiateResource(const char* libraryName, const CF::Properties& options, const CF::Properties& parameters);
        virtual void formatRequestProps(const CF::Properties& requestProps, CF::Properties& formattedProps);
};

#endif // BASE_PERSONA_BASE_IMPL_REPROG_H
