/*#
 * This file is protected by Copyright. Please refer to the COPYRIGHT file 
 * distributed with this source distribution.
 * 
 * This file is part of REDHAWK core.
 * 
 * REDHAWK core is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version.
 * 
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 #*/
/*{% block license %}*/
/*# Allow child templates to include license #*/
/*{% endblock %}*/
//% set className = component.reprogclass.name
//% set baseClass = component.baseclass.name
//% set artifactType = component.artifacttype
/**************************************************************************

    This is the ${artifactType} code. This file contains the child class where
    custom functionality can be added to the ${artifactType}. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "${component.reprogclass.header}"

PREPARE_LOGGING(${className})

/*{% if component is device %}*/
${className}::${className} (
                        char            *devMgr_ior, 
                        char            *id, 
                        char            *lbl, 
                        char            *sftwrPrfl ) 
:
    ${baseClass}(devMgr_ior, id, lbl, sftwrPrfl),
    _previousRequestProps()
{
    construct();
}

${className}::${className} (
                        char            *devMgr_ior, 
                        char            *id, 
                        char            *lbl, 
                        char            *sftwrPrfl, 
                        char            *compDev ) 
:
    ${baseClass}(devMgr_ior, id, lbl, sftwrPrfl, compDev),
    _previousRequestProps()
{
    construct();
}

${className}::${className} (
                        char            *devMgr_ior, 
                        char            *id, 
                        char            *lbl, 
                        char            *sftwrPrfl, 
                        CF::Properties  capacities ) 
:
    ${baseClass}(devMgr_ior, id, lbl, sftwrPrfl, capacities),
    _previousRequestProps()
{
    construct();
}

${className}::${className} (
                        char            *devMgr_ior, 
                        char            *id, 
                        char            *lbl, 
                        char            *sftwrPrfl, 
                        CF::Properties  capacities, 
                        char            *compDev ) 
:
    ${baseClass}(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev),
    _previousRequestProps()
{
    construct();
}
/*{% else %}*/
${className}::${className} (
                        const char      *uuid, 
                        const char      *label ) 
:
    ${baseClass}(uuid, label),
    _previousRequestProps()
{
    construct();
}
/*{% endif %}*/

void ${className}::construct() 
{
    // Initialize state to safe defaults
    _parentDevice = NULL;
    _parentAllocated = false;
//% if component is executabledevice
    _processIdIncrement = 0;
//% endif 
}

// TODO: This was overriden since setting admin state is not accessible via the current IDL
void ${className}::adminState(CF::Device::AdminType adminState) 
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

    ${baseClass}::adminState(adminState);
}

void ${className}::releaseObject() 
    throw (
        CF::LifeCycle::ReleaseError, 
        CORBA::SystemException ) 
{
/*{% if component is executabledevice %}*/
    // Terminate all children that were executed
    ProcessMapIter iter;
    for (iter = _processMap.begin(); iter != _processMap.end(); iter++) {
        this->terminate(iter->first);
    }
/*{% endif %}*/
    // deactivate ports
    releaseInPorts();
    releaseOutPorts();

    // SR:419
    RH_DEBUG(this->_deviceLog, __FUNCTION__ << ": Receive releaseObject call");
    if (_adminState == CF::Device::UNLOCKED) {
        RH_DEBUG(this->_deviceLog, __FUNCTION__ << ": Releasing Device")
        setAdminState(CF::Device::SHUTTING_DOWN);

        // SR:418
        // TODO Release aggregate devices if more than one exists
        if (!CORBA::is_nil(_aggregateDevice)) {
            try {
                _aggregateDevice->removeDevice(this->_this());
            } catch (...) {
            }
        }

        RH_DEBUG(this->_deviceLog, __FUNCTION__ << ": Done Releasing Device")
    }
}


CORBA::Boolean ${className}::attemptToProgramParent() 
{
    // Return false if there is no reference to the parent
    if (_parentDevice == NULL) {
        RH_ERROR(this->_deviceLog, __FUNCTION__ << 
            ": No reference to parent exists!");
        return false;
    }

    if (_parentAllocated == false) {

        RH_DEBUG(this->_deviceLog, __FUNCTION__ << 
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

CORBA::Boolean ${className}::attemptToUnprogramParent() 
{
    // Return false if there is no reference to the parent
    if (_parentDevice == NULL) {
        RH_ERROR(this->_deviceLog, __FUNCTION__ << ": No reference to parent exists!");
        return false;
    }
    
    if (_parentAllocated) {
        beforeHardwareUnprogrammed();
        
        // Grab previous user-defined allocation request
        if (_previousRequestProps.length() == 0) {
            RH_ERROR(this->_deviceLog, __FUNCTION__ << ": Previously requested hw_load Props empty!");
            return false;
        }

        _parentDevice->deallocateCapacity(_previousRequestProps);
        _parentAllocated = false;
        afterHardwareUnprogrammed();
    }
    return !_parentAllocated;
}

/*{% if component is executabledevice %}*/
CF::ExecutableDevice::ProcessID_Type ${className}::execute (
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
        RH_DEBUG(this->_deviceLog, __FUNCTION__ << 
            ": InstantiateResourceProp: ID['" << propId << "'] = " << propValue);
    }

    // Attempt to create and verify the resource
    resourcePtr = instantiateResource(name, options, parameters);
    if (resourcePtr == NULL) {
        RH_FATAL(this->_deviceLog, __FUNCTION__ << 
            ": Unable to instantiate '" << name << "'");
        throw (CF::ExecutableDevice::ExecuteFail());
    }

    resourceId = ossie::corba::returnString(resourcePtr->identifier());
    _resourceMap[resourceId] = resourcePtr;                 // Store the resourcePtr
    _processMap[++_processIdIncrement] = resourceId;        // Store the resourcePtr Process for termination
    //_deviceManager->registerDevice(resourcePtr->_this());

    return (CORBA::Long) _processIdIncrement;
}

void ${className}::terminate(CF::ExecutableDevice::ProcessID_Type processId)
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

bool ${className}::hasRunningResources() 
{
    return (!_resourceMap.empty());
}

Resource_impl* ${className}::instantiateResource(
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
        RH_FATAL(this->_deviceLog, __FUNCTION__ <<  
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
        RH_FATAL(this->_deviceLog, __FUNCTION__ << 
            ": Unable to find symbol '" << symbol << "': " << errorMsg);
        return NULL;
    }

    // Cast the symbol as a ConstructorPtr
    constructorPtr = reinterpret_cast<ConstructorPtr>(fnPtr);
    
    // Attempt to instantiate the resource via the constructor pointer
    try {
        resourcePtr = generateResource(argc, argv, constructorPtr, libraryName);
    } catch (...) {
        RH_FATAL(this->_deviceLog, __FUNCTION__ << 
            ": Unable to construct persona device: '" << argv[0] << "'");
    }

    // Free the memory used to create argv
    for (unsigned int i = 0; i < argCounter; i++) {
        free(argv[i]);
    }

    return resourcePtr;
}
/*{% endif %}*/

// Transforms user-supplied properties into safe-usable CF::Properties
void ${className}::formatRequestProps(
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
        RH_ERROR(this->_deviceLog, __FUNCTION__ << 
            ": Unable to format hw_load_request properties.  Properties are empty!");
        return;
    }

    // Case 1 - Properties are already properly formatted
    if (requestProps.length() == 1) {
        propId = requestProps[0].id;
        if (propId == "hw_load_requests") {
            RH_DEBUG(this->_deviceLog, __FUNCTION__ <<
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
        RH_DEBUG(this->_deviceLog, __FUNCTION__ << 
            ": Found hw_load_request array - Formatting to structseq");
        formattedProps.length(1);
        formattedProps[0].id = "hw_load_requests";
        formattedProps[0].value <<= requestProps;
        return;
    }

    // Case 3 - Properties reprensent the contents of a single hw_load_request
    if (foundRequestId) {
        RH_DEBUG(this->_deviceLog, __FUNCTION__ <<
            ": Found hw_load_request contents - Formatting to struct and structseq");
        
        hwLoadRequest.length(1);
        hwLoadRequest[0].id = "hw_load_request";
        hwLoadRequest[0].value <<= requestProps;

        formattedProps.length(1);
        formattedProps[0].id = "hw_load_requests";
        formattedProps[0].value <<= hwLoadRequest;
        return;
    }
    
    RH_ERROR(this->_deviceLog, __FUNCTION__ <<
        ": Unable to format hw_load_request properties - Format unknown!");
}
