/*
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
 */
#ifndef PROGRAMMABLEDEVICE_IMPL_REPROG_H
#define PROGRAMMABLEDEVICE_IMPL_REPROG_H

#include "ProgrammableDevice_base.h"
#include "ossie/prop_helpers.h"
#include "entry_point.h"
#include <dlfcn.h>


namespace HW_LOAD {
  
  struct default_hw_load_request_struct {
    default_hw_load_request_struct ()
    {
    };

    static std::string getId() {
        return std::string("hw_load_request");
    };

    std::string request_id;
    std::string requester_id;
    std::string hardware_id;
    std::string load_filepath;
  };

  struct default_hw_load_status_struct {
    default_hw_load_status_struct ()
    {
    };

    static std::string getId() {
        return std::string("hw_load_request");
    };

    std::string request_id;
    std::string requester_id;
    std::string hardware_id;
    std::string load_filepath;
    unsigned short state;
  };

  enum load_status_states {
    INACTIVE=0U,
    ACTIVE,
    PENDING,
    ERRORED
  };
}

inline bool operator>>= (const CORBA::Any& a, HW_LOAD::default_hw_load_request_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("request_id", props[idx].id)) {
            if (!(props[idx].value >>= s.request_id)) return false;
        }
        if (!strcmp("requester_id", props[idx].id)) {
            if (!(props[idx].value >>= s.requester_id)) return false;
        }
        if (!strcmp("hardware_id", props[idx].id)) {
            if (!(props[idx].value >>= s.hardware_id)) return false;
        }
        if (!strcmp("load_filepath", props[idx].id)) {
            if (!(props[idx].value >>= s.load_filepath)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const HW_LOAD::default_hw_load_request_struct& s) {
    CF::Properties props;
    props.length(4);
    props[0].id = CORBA::string_dup("request_id");
    props[0].value <<= s.request_id;
    props[1].id = CORBA::string_dup("requester_id");
    props[1].value <<= s.requester_id;
    props[2].id = CORBA::string_dup("hardware_id");
    props[2].value <<= s.hardware_id;
    props[3].id = CORBA::string_dup("load_filepath");
    props[3].value <<= s.load_filepath;
    a <<= props;
};

typedef std::string PersonaId;
typedef std::map<PersonaId, Device_impl*> PersonaMap;
typedef std::map<unsigned int, PersonaId> ProcessMap;
typedef PersonaMap::iterator PersonaMapIter;
typedef ProcessMap::iterator ProcessMapIter;
typedef std::vector<std::string> StrVec;


template <typename HW_LOAD_REQUEST=HW_LOAD::default_hw_load_request_struct, 
          typename HW_LOAD_STATUS=HW_LOAD::default_hw_load_status_struct>
class ProgrammableDevice_prog_base;

template <typename HW_LOAD_REQUEST, typename HW_LOAD_STATUS>
class ProgrammableDevice_prog_base : public ProgrammableDevice_base
{
    ENABLE_LOGGING;

    public:

        typedef HW_LOAD_REQUEST HwLoadRequestStruct;
        typedef HW_LOAD_STATUS  HwLoadStatusStruct;
        typedef std::vector<HwLoadRequestStruct> HwLoadRequestVec;
        typedef std::vector<HwLoadStatusStruct> HwLoadStatusVec;
        typedef ProgrammableDevice_prog_base<HW_LOAD_REQUEST BOOST_PP_COMMA()
                             HW_LOAD_STATUS> ProgrammableDevice_prog_base_type;

        ProgrammableDevice_prog_base (
                                    char            *devMgr_ior,
                                    char            *id,
                                    char            *lbl,
                                    char            *sftwrPrfl )
        : 
            ProgrammableDevice_base(devMgr_ior, id, lbl, sftwrPrfl)
        {
            construct();
        }

        ProgrammableDevice_prog_base (
                                    char            *devMgr_ior,
                                    char            *id,
                                    char            *lbl,
                                    char            *sftwrPrfl,
                                    char            *compDev )
        :
            ProgrammableDevice_base(devMgr_ior, id, lbl, sftwrPrfl, compDev)
        {
            construct();
        }

        ProgrammableDevice_prog_base (
                                    char            *devMgr_ior, 
                                    char            *id, 
                                    char            *lbl, 
                                    char            *sftwrPrfl, 
                                    CF::Properties  capacities ) 
        :
            ProgrammableDevice_base(devMgr_ior, id, lbl, sftwrPrfl, capacities)
        {
            construct();
        }

        ProgrammableDevice_prog_base (
                                    char            *devMgr_ior,
                                    char            *id,
                                    char            *lbl,
                                    char            *sftwrPrfl,
                                    CF::Properties  capacities,
                                    char            *compDev )
        :
            ProgrammableDevice_base(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev)
        {
            construct();
        }
        
        virtual ~ProgrammableDevice_prog_base() {
            // Clean up all children that were executed
            PersonaMapIter personaIter;
            for (personaIter = _personaMap.begin(); personaIter != _personaMap.end(); personaIter++) {
                delete personaIter->second;
            }
        }

        void construct() {
            setUsageState(CF::Device::BUSY);
            _personaMap.clear();
            _processMap.clear();
            _processIdIncrement = 0;

            _defaultHwLoadStatuses.clear();
            _defaultHwLoadStatuses.resize(1);

            // Set property pointers to use default properties
            _hwLoadRequestsPtr = &_defaultHwLoadRequests;
            _hwLoadStatusesPtr = &_defaultHwLoadStatuses;
        }

        void load ( CF::FileSystem_ptr           fs, 
                    const char*                  fileName, 
                    CF::LoadableDevice::LoadType loadKind )
            throw ( 
                CF::LoadableDevice::LoadFail, 
                CF::InvalidFileName, 
                CF::LoadableDevice::InvalidLoadKind,
                CF::Device::InvalidState, 
                CORBA::SystemException ) 
        {
            bool isSharedLibrary = (loadKind == CF::LoadableDevice::SHARED_LIBRARY);
            bool existsOnDevFS   = _deviceManager->fileSys()->exists(fileName);
            
            // For persona shared librariess that already reside on the dev file 
            // system, use the dev filesystem to copy into cache
            if (isSharedLibrary && existsOnDevFS) { 
                fs = _deviceManager->fileSys();;
                LOG_DEBUG(ProgrammableDevice_prog_base, __FUNCTION__ << 
                    ": File-system switched to dev");
            }

            ProgrammableDevice_base::load(fs, fileName, loadKind);
        }
        
        CF::ExecutableDevice::ProcessID_Type execute (
                        const char*             name, 
                        const CF::Properties&   options, 
                        const CF::Properties&   parameters )
            throw (
                CF::ExecutableDevice::ExecuteFail, 
                CF::InvalidFileName, 
                CF::ExecutableDevice::InvalidOptions, 
                CF::ExecutableDevice::InvalidParameters,
                CF::ExecutableDevice::InvalidFunction, 
                CF::Device::InvalidState, 
                CORBA::SystemException )
        {
            LOG_DEBUG(ProgrammableDevice_prog_base, __FUNCTION__ << 
                    ": Instantiating persona '" << name << "'... ");
            
            // Initialize local variables
            Device_impl* persona = NULL; 
            std::string personaId;

            // Attempt to instantiate the object contained in the shared library
            persona = instantiatePersona(name, options, parameters);
            if (persona == NULL) {
                LOG_FATAL(ProgrammableDevice_prog_base, __FUNCTION__ << 
                    ": Unable to instantiate '" << name << "'");
                throw (CF::ExecutableDevice::ExecuteFail());
            }
           
            // Grab the name from the instantiated object 
            personaId = ossie::corba::returnString(persona->identifier());
            
            // Save off the name-pid and name-object mappings
            _personaMap[personaId] = persona;
            _processMap[++_processIdIncrement] = personaId;

            // Update the device manager with the new device
            _deviceManager->registerDevice(persona->_this());

            LOG_DEBUG(ProgrammableDevice_prog_base, __FUNCTION__ <<
                    ": Persona '" << personaId << "' has been successfully instantiated");

            return _processIdIncrement;
        }
        
        void terminate (CF::ExecutableDevice::ProcessID_Type processId) 
            throw (
                CF::Device::InvalidState, 
                CF::ExecutableDevice::InvalidProcess, 
                CORBA::SystemException ) 
        {
            // Initialize local variables
            ProcessMapIter processIter;
            PersonaMapIter personaIter;
            PersonaId personaId;

            // Search for the personaId that related to the incoming terminate request
            processIter = _processMap.find(processId);
            if (processIter != _processMap.end()) {

                /// Search for the persona that related to the found personaId
                personaIter = _personaMap.find(processIter->second);
                if (personaIter != _personaMap.end()) {
                    _deviceManager->unregisterDevice(personaIter->second->_this());
                    personaIter->second->setAdminState(CF::Device::UNLOCKED);
                    personaIter->second->releaseObject();
                    _processMap.erase(processIter); // Erase process mapping here to minimize collisions with non-persona processIds
                    _personaMap.erase(personaIter);
                    return;
                }
            }
            LOG_WARN(ProgrammableDevice_prog_base, __FUNCTION__ << 
                    ": Unable to locate persona using pid '" << processId <<"'");
        }
        
        CORBA::Boolean allocateCapacity(const CF::Properties& capacities) 
            throw (
                CF::Device::InvalidState, 
                CF::Device::InvalidCapacity, 
                CF::Device::InsufficientCapacity, 
                CORBA::SystemException ) 
        {
            boost::mutex::scoped_lock lock(_allocationMutex);

            // Initialize local variables
            bool allocationSuccess = false;
            std::string id;
            std::string incomingPersona;
            CF::Properties loadProps;
            PersonaMapIter iter;
            HwLoadStatusVec hwLoadStatusStruct;
            HwLoadStatusVec* statusVecPtr;
            HwLoadRequestVec* loadRequestsPtr;

            // Iterate through all incoming capacities
            for (unsigned int ii = 0; ii < capacities.length(); ii++) {
                id = capacities[ii].id;

                if (id == HW_LOAD_REQUEST_PROP()) {

                    CORBA::AnySeq* anySeqPtr;
                    CF::Properties* cfPropsPtr;
                    
                    // Grab the current hw_load_request struct
                    loadRequestsPtr = getHwLoadRequests();
                    if (loadRequestsPtr == NULL) {
                        LOG_ERROR(ProgrammableDevice_prog_base, __FUNCTION__ <<
                            ": Unable to get HwLoadRequest vector! Pointer is NULL");
                        continue;
                    }

                    // Attempt to Convert Any unwrappable type
                    if (capacities[ii].value >>= anySeqPtr) {
                        // Convert AnySeq to HwLoadRequestVector
                        loadRequestsPtr->resize(anySeqPtr->length());
                        for (size_t iv = 0; iv < anySeqPtr->length(); iv++) {
                            (*anySeqPtr)[iv] >>= (*loadRequestsPtr)[iv];
                        }
                    } else if (capacities[ii].value >>= cfPropsPtr) {
                        // Convert CF Properties to HwLoadRequestVector
                        loadRequestsPtr->resize(cfPropsPtr->length());
                        for (size_t iv = 0; iv < cfPropsPtr->length(); iv++) {
                            (*cfPropsPtr)[iv].value >>= (*loadRequestsPtr)[iv];
                        }
                    } else {
                        LOG_ERROR(ProgrammableDevice_prog_base, __FUNCTION__ << 
                            ": Unable to convert HW_LOAD_REQUEST prop!");
                        continue;
                    }

                    // Validate that hw_load_requests are valid
                    if (hwLoadRequestsAreValid(*loadRequestsPtr)) {
                        
                        // Grab the current hw_load_status struct
                        statusVecPtr = getHwLoadStatuses();
                        if (statusVecPtr == NULL) {
                            LOG_ERROR(ProgrammableDevice_prog_base, __FUNCTION__ <<
                                ": Unable to get HwLoadStatus vector! Pointer is NULL");
                            continue;
                        }
                        
                        // Apply the load request onto the status vector
                        allocationSuccess = applyHwLoadRequests(*loadRequestsPtr,
                                                                *statusVecPtr);
                        if (allocationSuccess) {
                            executePropertyCallback(HW_LOAD_REQUEST_PROP());
                        }
                    }
                }
            }

            updateAdminStates();
            LOG_DEBUG(ProgrammableDevice_prog_base, __FUNCTION__ << ": Allocation Result: " << allocationSuccess);
            return allocationSuccess;
        }
        
        void deallocateCapacity(const CF::Properties& capacities) 
            throw (
                CF::Device::InvalidState, 
                CF::Device::InvalidCapacity, 
                CORBA::SystemException ) 
        {
            // Initialize local variables
            bool deallocationSuccess = false;
            std::string id;
            std::string valueStr;
            std::string errorMsg;
            HwLoadStatusVec* statusVecPtr;
            CORBA::AnySeq* anySeqPtr;
            CF::Properties* cfPropsPtr;
            HwLoadRequestVec loadRequestsToRemove;

            for (unsigned int ii = 0; ii < capacities.length(); ii++) {
                id = capacities[ii].id;

                if (id == HW_LOAD_REQUEST_PROP()) {
                    LOG_DEBUG(ProgrammableDevice_prog_base, __FUNCTION__ <<
                        ": Deallocating hw_load_requests...");
                    
                    // Attempt to Convert Any to unwrappable type
                    if (capacities[ii].value >>= anySeqPtr) {
                        // Convert AnySeq to HwLoadRequestVector
                        loadRequestsToRemove.resize(anySeqPtr->length());
                        for (size_t iv = 0; iv < anySeqPtr->length(); iv++) {
                            (*anySeqPtr)[iv] >>= loadRequestsToRemove[iv];
                        }
                    } else if (capacities[ii].value >>= cfPropsPtr) {
                        // Convert CF Properties to HwLoadRequestVector
                        loadRequestsToRemove.resize(cfPropsPtr->length());
                        for (size_t iv = 0; iv < cfPropsPtr->length(); iv++) {
                            (*cfPropsPtr)[iv].value >>= loadRequestsToRemove[iv];
                        }
                    } else {
                        LOG_ERROR(ProgrammableDevice_prog_base, __FUNCTION__ << 
                            ": Unable to convert HW_LOAD_REQUEST property");
                        continue;
                    }
                    
                    // Grab the current hw_load_status struct
                    statusVecPtr = getHwLoadStatuses();
                    if (statusVecPtr == NULL) {
                        LOG_ERROR(ProgrammableDevice_prog_base, __FUNCTION__ <<
                            ": Unable to get HwLoadStatus vector! Pointer is NULL");
                        continue;
                    }
                    
                    // Convert AnySeq to HwLoadRequestVector
                    deallocationSuccess = true;
                    for (size_t iv = 0; iv < loadRequestsToRemove.size(); iv++) {
                        deallocationSuccess |= removeHwLoadRequestFromStatus(loadRequestsToRemove[iv], (*statusVecPtr));
                    }
                }
            }

            updateAdminStates();
            if (deallocationSuccess == false) {
                throw CF::Device::InvalidCapacity("Unable to deallocation capacities", capacities);
            }
        }
        
        void releaseObject() 
            throw ( CF::LifeCycle::ReleaseError, 
                    CORBA::SystemException)
        {
            // Initialize local variables
            ProcessMapIter processIter;
            PersonaMapIter personaIter;

            // Terminate all children that were executed
            for (processIter = _processMap.begin(); processIter != _processMap.end(); processIter++) {
                this->terminate(processIter->first);
            }

            // Clean up all children that were executed
            for (personaIter = _personaMap.begin(); personaIter != _personaMap.end(); personaIter++) {
                delete personaIter->second;
                personaIter->second = NULL;
            }

            ProgrammableDevice_base::releaseObject();
            setAdminState(CF::Device::SHUTTING_DOWN);
        }
        
        HwLoadRequestVec* getHwLoadRequests() { 
            return getPropertyPtr(_hwLoadRequestsPtr, 
                                  &_defaultHwLoadRequests);
        };
        
        HwLoadStatusVec* getHwLoadStatuses() { 
            return getPropertyPtr(_hwLoadStatusesPtr, 
                                  &_defaultHwLoadStatuses);
        };

        void setHwLoadRequestsPtr(HwLoadRequestVec* propPtr) {
            if (propPtr == NULL) {
               LOG_ERROR(ProgrammableDevice_prog_base, "CANNOT SET HW_LOAD_REQUESTS_PTR: PROPERTY IS NULL");
               return;
            }
            _hwLoadRequestsPtr = propPtr;
        }
        
        void setHwLoadStatusesPtr(HwLoadStatusVec* propPtr) {
            if (propPtr == NULL) {
               LOG_ERROR(ProgrammableDevice_prog_base, "CANNOT SET HW_LOAD_STATUSES_PTR: PROPERTY IS NULL");
               return;
            }
            _hwLoadStatusesPtr = propPtr;
        }

    protected:
        PersonaMap      _personaMap;        // Mapping of name to object ptr
        ProcessMap      _processMap;         // Mapping of name to processId
        unsigned int    _processIdIncrement; // Used to generate unique ProcessIds
        boost::mutex    _allocationMutex;    // Used to ensure one allocation occurs at a time
        
        // Define the name of the properties
        virtual std::string HW_LOAD_REQUEST_PROP() { return "hw_load_requests"; };
        virtual std::string HW_LOAD_STATUS_PROP()  { return "hw_load_statuses"; };


        virtual Device_impl* generatePersona (
                                    int                         argc, 
                                    char*                       argv[], 
                                    ConstructorPtr              fnptr, 
                                    const char*                 libraryName)=0;

        virtual Device_impl* instantiatePersona (
                                    const char*                 libraryName, 
                                    const CF::Properties&       options, 
                                    const CF::Properties&       parameters) 
        {
            // Open up the cached .so file
            std::string absPath = get_current_dir_name();
            absPath.append(libraryName);
            void* pHandle = dlopen(absPath.c_str(), RTLD_NOW);
            if (!pHandle) {
                char* errorMsg = dlerror();
                LOG_FATAL(ProgrammableDevice_prog_base, __FUNCTION__ << 
                    ": Unable to open library '" << absPath.c_str() << "': " << errorMsg);
                return NULL;
            }  
            
            // Add SKIP_FLAG to properties
            CF::Properties combinedProps = parameters;
            unsigned int skipRunInd;
            skipRunInd = combinedProps.length();
            combinedProps.length(skipRunInd + 1);
            combinedProps[skipRunInd].id = CORBA::string_dup("SKIP_RUN");
            combinedProps[skipRunInd].value <<= true;

            for (size_t ii = 0; ii < combinedProps.length(); ii++) {
                std::string id(combinedProps[ii].id);
                std::string val = ossie::any_to_string(combinedProps[ii].value);
                LOG_DEBUG(ProgrammableDevice_prog_base, "ARGV[" << id << "]: " << val);
            }

            // Convert combined properties into ARGV/ARGC format
            std::string propId;
            std::string propValue;
            unsigned int argCounter = 0;
            int argc = combinedProps.length() * 2;
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
            const char* symbol = "construct";
            void* fnPtr = dlsym(pHandle, symbol);
            if (!fnPtr) {
                char* errorMsg = dlerror();
                LOG_FATAL(ProgrammableDevice_prog_base, __FUNCTION__ << 
                    ": Unable to find symbol '" << symbol << "': " << errorMsg);
                return NULL;
            }

            // Cast the symbol as a ConstructorPtr
            ConstructorPtr constructPtr = reinterpret_cast<ConstructorPtr>(fnPtr);

            // Attempt to instantiate the persona device via the constructor pointer
            Device_impl* personaPtr = NULL;
            try {
                personaPtr = generatePersona(argc, argv, constructPtr, libraryName);
            } catch (...) {
                LOG_FATAL(ProgrammableDevice_prog_base, __FUNCTION__ << 
                    ": Unable to construct persona device: '" << argv[0] << "'");
            }

            for (unsigned int i = 0; i < argCounter; i++) {
                free(argv[i]);
            }

            return personaPtr;
        }


        virtual bool hwLoadRequestsAreValid(const HwLoadRequestVec& hwLoadRequestVec) 
        {
            bool isValid = true;
            for (size_t ii = 0; ii < hwLoadRequestVec.size(); ii++) {
                isValid &= hwLoadRequestIsValid(hwLoadRequestVec[ii]);
            }
            return isValid;
        }


        virtual bool hwLoadRequestIsValid(const HwLoadRequestStruct& hwLoadRequestStruct) 
        {
            // Defaults to true... Override this method for more control
            return true;
        }


        virtual bool applyHwLoadRequests (
                                    HwLoadRequestVec&           loadRequestVec, 
                                    HwLoadStatusVec&            loadStatusVec)
        {
            // Setup local variables
            bool success = true;
            int availableStatusIndex;

            // Maintain which statuses were used incase of failures
            std::vector<int> usedStatusIndices; 
            usedStatusIndices.resize(loadRequestVec.size());
            for (size_t ii = 0; ii < usedStatusIndices.size(); ii++) {
                usedStatusIndices[ii] = -1;
            } 

            // Iterate through all requests and find available statuses
            for (size_t ii = 0; ii < loadRequestVec.size(); ii++) {
                
                // Find an available hw load status struct
                availableStatusIndex = findAvailableHwLoadStatusIndex(loadStatusVec);
                if (availableStatusIndex > -1) {
                    success |= applyHwLoadRequest(loadRequestVec[ii], loadStatusVec[availableStatusIndex]);
                    usedStatusIndices[ii] = availableStatusIndex;;
                } else {
                    LOG_ERROR(ProgrammableDevice_prog_base, __FUNCTION__ << 
                        ": Device cannot be allocated against. No load capacity");
                    success = false;
                }

                // Kick out if there was any issue
                if (success == false) {
                    // Remove any successful requests that made it through initially
                    for (size_t ii = 0; ii < usedStatusIndices.size(); ii++) {
                        resetHwLoadStatus(loadStatusVec[usedStatusIndices[ii]]);
                    }
                    break;
                }
            }
          
            // Clear out the loadRequest property since we're complete
            loadRequestVec.clear();

            return success;
        }
       
 
        virtual bool applyHwLoadRequest (
                                    const HwLoadRequestStruct&  incomingRequest, 
                                    HwLoadStatusStruct&         newStatus)
        {
            newStatus.request_id = incomingRequest.request_id;
            newStatus.requester_id = incomingRequest.requester_id;
            newStatus.hardware_id = incomingRequest.hardware_id;
            newStatus.load_filepath = incomingRequest.load_filepath;
            newStatus.state = HW_LOAD::PENDING;

            // Call user-defined 'loadHardware' and update state based on result
            if (loadHardware(newStatus)) {
                newStatus.state = HW_LOAD::ACTIVE;
                return true;
            }

            // User-defined 'loadhardware' method returned false
            newStatus.state = HW_LOAD::ERRORED;
            return false;
        }

        virtual void resetHwLoadStatus(HwLoadStatusStruct& loadStatusStruct) {
            unloadHardware(loadStatusStruct);
            loadStatusStruct.request_id = "";
            loadStatusStruct.requester_id = "";
            loadStatusStruct.hardware_id = "";
            loadStatusStruct.load_filepath = "";
            loadStatusStruct.state = HW_LOAD::INACTIVE;
        }
        
        // Override this method to physically load the hardware
        virtual bool loadHardware(HwLoadStatusStruct& newStatus) { 
            return true; 
        }

        // Override this method to physically unload the hardware
        virtual void unloadHardware(const HwLoadStatusStruct& loadStatusStruct) {
        }

        virtual bool removeHwLoadRequestFromStatus (
                                    const HwLoadRequestStruct&  hwLoadRequest,
                                    HwLoadStatusVec&            hwStatusVec)
        {
            // Iterate through status vector and reset struct that matches request_id
            for (size_t ii = 0; ii < hwStatusVec.size(); ii++) {
                if (hwLoadRequest.request_id == hwStatusVec[ii].request_id) {
                    resetHwLoadStatus(hwStatusVec[ii]);
                    return true;
                }
            }
            return false;
        }

        virtual bool hasAnInactiveHwLoadStatus() 
        {
            HwLoadStatusVec* statusVecPtr = getHwLoadStatuses();
            if (statusVecPtr == NULL) {
                LOG_ERROR(ProgrammableDevice_prog_base, __FUNCTION__ <<
                    ": Unable to get HwLoadStatus vector! Pointer is NULL");
                return false;
            }
            return (findAvailableHwLoadStatusIndex((*statusVecPtr)) >= 0);
        }

        virtual void updateAdminStates()
        {
            if (hasAnInactiveHwLoadStatus()) {
                // Unlock all devices if there is capacity for more loads
                PersonaMapIter iter;
                for (iter = _personaMap.begin(); iter != _personaMap.end(); iter++) {
                    iter->second->adminState(CF::Device::UNLOCKED);
                }
                setAdminState(CF::Device::UNLOCKED);
            } else {
                // Lock all personas that are not loaded onto the device
                
                // Grab the current hw_load_status struct
                HwLoadStatusVec* statusVecPtr = getHwLoadStatuses();
                if (statusVecPtr == NULL) {
                    LOG_ERROR(ProgrammableDevice_prog_base, __FUNCTION__ <<
                        ": Unable to get HwLoadStatus vector! Pointer is NULL");
                    return;
                }

                // Grab all of the Requester Ids to compare against
                std::vector<std::string> allRequesterIds;
                allRequesterIds.resize(statusVecPtr->size());
                for (size_t ii = 0; ii < statusVecPtr->size(); ii++) {
                    allRequesterIds.push_back(statusVecPtr->at(ii).requester_id);
                }

                // Any persona that has a load running should not be locked
                PersonaMapIter iter;
                for (iter = _personaMap.begin(); iter != _personaMap.end(); iter++) {
                    if (strVecContainsStr(allRequesterIds, iter->first)) {
                        continue; // Skip the running personas
                    }
                    LOG_DEBUG(ProgrammableDevice_prog_base, __FUNCTION__ <<
                        ": Locking device '" << ossie::corba::returnString(iter->second->identifier()) << "'");
                    iter->second->adminState(CF::Device::LOCKED);
                }
                setAdminState(CF::Device::LOCKED);
            }
        }

    private:
        // Property pointers
        HwLoadRequestVec* _hwLoadRequestsPtr;
        HwLoadStatusVec*  _hwLoadStatusesPtr;

        // Default properties (incase child does not set own properties)
        HwLoadRequestVec  _defaultHwLoadRequests;
        HwLoadStatusVec   _defaultHwLoadStatuses;

        int findAvailableHwLoadStatusIndex(const HwLoadStatusVec& loadStatusVec)
        {
            // Initialize local variables
            int returnVal = -1;

            // Iterate through all statuses and look for an INACTIVE one
            for (size_t ii = 0; ii < loadStatusVec.size(); ii++) {
                if (loadStatusVec[ii].state == HW_LOAD::INACTIVE) {
                    returnVal = ii;
                    break;
                }
            }

            return returnVal;
        }

        static bool strVecContainsStr(StrVec& strVec, const std::string& value) {
            return std::find(strVec.begin(), strVec.end(), value) != strVec.end();
        }
        
        template <typename TYPE>
        TYPE* getPropertyPtr(TYPE* propertyPtr, TYPE* defaultProperty) {
            if (propertyPtr != NULL) return propertyPtr;
            return defaultProperty;
        }
};

template <typename HW_LOAD_REQUEST, typename HW_LOAD_STATUS>
PREPARE_ALT_LOGGING(ProgrammableDevice_prog_base<HW_LOAD_REQUEST BOOST_PP_COMMA() 
                                 HW_LOAD_STATUS>, ProgrammableDevice_prog_base);

#endif
