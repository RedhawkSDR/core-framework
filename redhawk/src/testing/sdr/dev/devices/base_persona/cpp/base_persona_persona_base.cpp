/**************************************************************************

    This is the device code. This file contains the child class where
    custom functionality can be added to the device. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "base_persona_persona_base.h"

PREPARE_LOGGING(base_persona_persona_base)

base_persona_persona_base::base_persona_persona_base (
                        char            *devMgr_ior, 
                        char            *id, 
                        char            *lbl, 
                        char            *sftwrPrfl ) 
:
    base_persona_base(devMgr_ior, id, lbl, sftwrPrfl),
    _previousRequestProps()
{
    construct();
}

base_persona_persona_base::base_persona_persona_base (
                        char            *devMgr_ior, 
                        char            *id, 
                        char            *lbl, 
                        char            *sftwrPrfl, 
                        char            *compDev ) 
:
    base_persona_base(devMgr_ior, id, lbl, sftwrPrfl, compDev),
    _previousRequestProps()
{
    construct();
}

base_persona_persona_base::base_persona_persona_base (
                        char            *devMgr_ior, 
                        char            *id, 
                        char            *lbl, 
                        char            *sftwrPrfl, 
                        CF::Properties  capacities ) 
:
    base_persona_base(devMgr_ior, id, lbl, sftwrPrfl, capacities),
    _previousRequestProps()
{
    construct();
}

base_persona_persona_base::base_persona_persona_base (
                        char            *devMgr_ior, 
                        char            *id, 
                        char            *lbl, 
                        char            *sftwrPrfl, 
                        CF::Properties  capacities, 
                        char            *compDev ) 
:
    base_persona_base(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev),
    _previousRequestProps()
{
    construct();
}

void base_persona_persona_base::construct() 
{
    // Initialize state to safe defaults
    _parentDevice = NULL;
    _parentAllocated = false;
    _processIdIncrement = 0;
}

// TODO: This was overriden since setting admin state is not accessible via the current IDL
void base_persona_persona_base::adminState(CF::Device::AdminType adminState) 
    throw (CORBA::SystemException)
{
    // Force admin state to change usage state since usage state is currently protected
    switch (adminState) {
        case CF::Device::LOCKED:
            setUsageState(CF::Device::BUSY);
            break;
        case CF::Device::UNLOCKED:
            setUsageState(CF::Device::IDLE);
            break;
        case CF::Device::SHUTTING_DOWN:
            // Do nothing
            break;
    }

    base_persona_base::adminState(adminState);
}

void base_persona_persona_base::releaseObject() 
    throw (
        CF::LifeCycle::ReleaseError, 
        CORBA::SystemException ) 
{
    // Terminate all children that were executed
    ProcessMapIter iter;
    for (iter = _processMap.begin(); iter != _processMap.end(); iter++) {
        this->terminate(iter->first);
    }

    // deactivate ports
    releaseInPorts();
    releaseOutPorts();

    // SR:419
    LOG_DEBUG(base_persona_persona_base, __FUNCTION__ << ": Receive releaseObject call");
    if (_adminState == CF::Device::UNLOCKED) {
        LOG_DEBUG(base_persona_persona_base, __FUNCTION__ << ": Releasing Device")
        setAdminState(CF::Device::SHUTTING_DOWN);

        // SR:418
        // TODO Release aggregate devices if more than one exists
        if (!CORBA::is_nil(_aggregateDevice)) {
            try {
                _aggregateDevice->removeDevice(this->_this());
            } catch (...) {
            }
        }

        LOG_DEBUG(base_persona_persona_base, __FUNCTION__ << ": Done Releasing Device")
    }
}


CORBA::Boolean base_persona_persona_base::attemptToProgramParent() 
{
    // Return false if there is no reference to the parent
    if (_parentDevice == NULL) {
        LOG_ERROR(base_persona_persona_base, __FUNCTION__ << 
            ": No reference to parent exists!");
        return false;
    }

    if (_parentAllocated == false) {

        LOG_DEBUG(base_persona_persona_base, __FUNCTION__ << 
            ": About to allocate parent device");
        
        beforeHardwareProgrammed();

        // Grab user-defined allocation request and format them properly
        CF::Properties requestProps;
        CF::Properties formattedRequestProps;

        hwLoadRequest(requestProps);
        formatRequestProps(requestProps, formattedRequestProps);

        _parentAllocated = _parentDevice->allocateCapacity(formattedRequestProps);

        if (_parentAllocated) {
            _previousRequestProps = formattedRequestProps;
            afterHardwareProgramSuccess();
        } else {
            afterHardwareProgramFailure();
        }
    }
    return _parentAllocated;
}

CORBA::Boolean base_persona_persona_base::attemptToUnprogramParent() 
{
    // Return false if there is no reference to the parent
    if (_parentDevice == NULL) {
        LOG_ERROR(base_persona_persona_base, __FUNCTION__ << ": No reference to parent exists!");
        return false;
    }
    
    if (_parentAllocated) {
        beforeHardwareUnprogrammed();
        
        // Grab previous user-defined allocation request
        if (_previousRequestProps.length() == 0) {
            LOG_ERROR(base_persona_persona_base, __FUNCTION__ << ": Previously requested hw_load Props empty!");
            return false;
        }

        _parentDevice->deallocateCapacity(_previousRequestProps);
        _parentAllocated = false;
        afterHardwareUnprogrammed();
    }
    return !_parentAllocated;
}

CF::ExecutableDevice::ProcessID_Type base_persona_persona_base::execute (
                        const char*                 name, 
                        const CF::Properties&       options, 
                        const CF::Properties&       parameters )
    throw ( 
        CF::ExecutableDevice::ExecuteFail, 
        CF::InvalidFileName, 
        CF::ExecutableDevice::InvalidOptions, 
        CF::ExecutableDevice::InvalidParameters,
        CF::ExecutableDevice::InvalidFunction, 
        CF::Device::InvalidState, 
        CORBA::SystemException ) 
{
    // Initialize local variables
    std::string propId;
    std::string propValue;
    std::string resourceId;
    Resource_impl* resourcePtr = NULL;
    
    // Iterate through all parameters for debugging purposes
    for (unsigned int ii = 0; ii < parameters.length(); ii++) {
        propId = parameters[ii].id;
        propValue = ossie::any_to_string(parameters[ii].value);
        LOG_DEBUG(base_persona_persona_base, __FUNCTION__ << 
            ": InstantiateResourceProp: ID['" << propId << "'] = " << propValue);
    }

    // Attempt to create and verify the resource
    resourcePtr = instantiateResource(name, options, parameters);
    if (resourcePtr == NULL) {
        LOG_FATAL(base_persona_persona_base, __FUNCTION__ << 
            ": Unable to instantiate '" << name << "'");
        throw (CF::ExecutableDevice::ExecuteFail());
    }

    resourceId = ossie::corba::returnString(resourcePtr->identifier());
    _resourceMap[resourceId] = resourcePtr;                 // Store the resourcePtr
    _processMap[++_processIdIncrement] = resourceId;        // Store the resourcePtr Process for termination
    //_deviceManager->registerDevice(resourcePtr->_this());

    return (CORBA::Long) _processIdIncrement;
}

void base_persona_persona_base::terminate(CF::ExecutableDevice::ProcessID_Type processId)
    throw (
        CF::Device::InvalidState, 
        CF::ExecutableDevice::InvalidProcess, 
        CORBA::SystemException ) 
{
    // Initialize local variables
    ProcessMapIter processIter;
    ResourceMapIter resourceIter;
    ResourceId resourceId;

    // Search for the resourceId that's related to the incoming terminate request
    processIter = _processMap.find(processId);
    if (processIter != _processMap.end()) {

        /// Search for the persona that related to the found resourceId
        resourceIter = _resourceMap.find(processIter->second);
        if (resourceIter != _resourceMap.end()) {
            _processMap.erase(processIter);
            _resourceMap.erase(resourceIter);

            // We don't need to call releaseObject here since HW Components will
            // be released by the application factory
            delete resourceIter->second;

            return;
        }
    }
}

bool base_persona_persona_base::hasRunningResources() 
{
    return (!_resourceMap.empty());
}

Resource_impl* base_persona_persona_base::instantiateResource(
                        const char*                 libraryName, 
                        const CF::Properties&       options, 
                        const CF::Properties&       parameters) 
{
    // Initialize local variables
    std::string absPath = get_current_dir_name();
    void* pHandle = NULL;
    char* errorMsg = NULL;
    CF::Properties combinedProps;
    unsigned int skipRunInd = 0;
   
    std::string propId;
    std::string propValue;

    const char* symbol = "construct";
    void* fnPtr = NULL; 
    unsigned long argc = 0;
    unsigned int argCounter = 0;
    
    ConstructorPtr constructorPtr = NULL;
    Resource_impl* resourcePtr = NULL;


    // Open up the cached .so file
    absPath.append(libraryName);
    pHandle = dlopen(absPath.c_str(), RTLD_NOW);
    if (!pHandle) {
        errorMsg = dlerror();
        LOG_FATAL(base_persona_persona_base, __FUNCTION__ <<  
                ": Unable to open library '" << absPath.c_str() << "': " << errorMsg);
        return NULL;
    }

    // Add SKIP_FLAG to properties
    combinedProps = parameters;
    skipRunInd = combinedProps.length();
    combinedProps.length(skipRunInd + 1);
    combinedProps[skipRunInd].id = CORBA::string_dup("SKIP_RUN");
    combinedProps[skipRunInd].value <<= true;

    // Convert combined properties into ARGV/ARGC format
    argc = combinedProps.length() * 2;
    char* argv[argc];
    for (unsigned int i = 0; i < combinedProps.length(); i++) {
        propId = combinedProps[i].id;
        propValue = ossie::any_to_string(combinedProps[i].value);

        argv[argCounter] = (char*) malloc(propId.size() + 1);
        strcpy(argv[argCounter++], propId.c_str());

        argv[argCounter] = (char*) malloc(propValue.size() + 1);
        strcpy(argv[argCounter++], propValue.c_str());
    }

    // Look for the 'construct' C-method
    fnPtr = dlsym(pHandle, symbol);
    if (!fnPtr) {
        errorMsg = dlerror();
        LOG_FATAL(base_persona_persona_base, __FUNCTION__ << 
            ": Unable to find symbol '" << symbol << "': " << errorMsg);
        return NULL;
    }

    // Cast the symbol as a ConstructorPtr
    constructorPtr = reinterpret_cast<ConstructorPtr>(fnPtr);
    
    // Attempt to instantiate the resource via the constructor pointer
    try {
        resourcePtr = generateResource(argc, argv, constructorPtr, libraryName);
    } catch (...) {
        LOG_FATAL(base_persona_persona_base, __FUNCTION__ << 
            ": Unable to construct persona device: '" << argv[0] << "'");
    }

    // Free the memory used to create argv
    for (unsigned int i = 0; i < argCounter; i++) {
        free(argv[i]);
    }

    return resourcePtr;
}

// Transforms user-supplied properties into safe-usable CF::Properties
void base_persona_persona_base::formatRequestProps(
                    const CF::Properties&       requestProps, 
                    CF::Properties&             formattedProps) 
{
    // Initialize local variables
    std::string propId;
    bool allPropsAreHwLoadRequest = false;
    bool foundRequestId = false;
    CF::Properties hwLoadRequest;

    // Sanity check - Kick out if properties are empty
    if (requestProps.length() == 0) {
        LOG_ERROR(base_persona_persona_base, __FUNCTION__ << 
            ": Unable to format hw_load_request properties.  Properties are empty!");
        return;
    }

    // Case 1 - Properties are already properly formatted
    if (requestProps.length() == 1) {
        propId = requestProps[0].id;
        if (propId == "hw_load_requests") {
            LOG_DEBUG(base_persona_persona_base, __FUNCTION__ <<
                ": No formatting occurred - Request properties are properly formatted!");
            formattedProps = requestProps;
            return;
        }
    }

    // Inspect the properties further
    allPropsAreHwLoadRequest = true;
    for (size_t ii = 0; ii < requestProps.length(); ii++) {
        propId = requestProps[ii].id;

        allPropsAreHwLoadRequest &= (propId == "hw_load_request");
        foundRequestId    |= (propId == "request_id") ||
                             (propId == "hw_load_request::request_id");
    }
  
    // Case 2 - Properties are multiple hw_load_request structs
    if (allPropsAreHwLoadRequest) {
        LOG_DEBUG(base_persona_persona_base, __FUNCTION__ << 
            ": Found hw_load_request array - Formatting to structseq");
        formattedProps.length(1);
        formattedProps[0].id = "hw_load_requests";
        formattedProps[0].value <<= requestProps;
        return;
    }

    // Case 3 - Properties reprensent the contents of a single hw_load_request
    if (foundRequestId) {
        LOG_DEBUG(base_persona_persona_base, __FUNCTION__ <<
            ": Found hw_load_request contents - Formatting to struct and structseq");
        
        hwLoadRequest.length(1);
        hwLoadRequest[0].id = "hw_load_request";
        hwLoadRequest[0].value <<= requestProps;

        formattedProps.length(1);
        formattedProps[0].id = "hw_load_requests";
        formattedProps[0].value <<= hwLoadRequest;
        return;
    }
    
    LOG_ERROR(base_persona_persona_base, __FUNCTION__ <<
        ": Unable to format hw_load_request properties - Format unknown!");
}
