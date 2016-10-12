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

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include <boost/filesystem/path.hpp>

#include <ossie/debug.h>
#include <ossie/ossieSupport.h>
#include <ossie/prop_helpers.h>
#include <ossie/DeviceManagerConfiguration.h>
#include <ossie/CorbaUtils.h>
#include <ossie/EventChannelSupport.h>
#include <ossie/ComponentDescriptor.h>
#include <ossie/FileStream.h>

#include "DeviceManager_impl.h"

namespace fs = boost::filesystem;

PREPARE_LOGGING(DeviceManager_impl);

using namespace ossie;

DeviceManager_impl::~DeviceManager_impl ()
{
}

void DeviceManager_impl::killPendingDevices() {
    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);

    for (DeviceList::iterator deviceIter = _pendingDevices.begin(); 
         deviceIter != _pendingDevices.end(); 
         ++deviceIter) {

        pid_t devicePid = (*deviceIter)->pid;

        // Try an orderly shutdown.
        // NOTE: If the DeviceManager was terminated with a ^C, sending this signal may cause the
        //       original SIGINT to be forwarded to all other children (which is harmless, but be aware).
        kill(devicePid, SIGTERM);
    }

    // Release the lock and allow time for the devices to exit.
    // Need to release the lock to allow the device to be removed from
    // the pending devices list if it dies.
    lock.unlock();
    usleep(500000);

    sigkillPendingDevices();
}

void DeviceManager_impl::sigkillPendingDevices() {
    // Send a SIGKILL to any remaining devices.
    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);
    for (DeviceList::iterator deviceIter = _pendingDevices.begin(); 
         deviceIter != _pendingDevices.end(); 
         ++deviceIter) {

        pid_t devicePid = (*deviceIter)->pid;
        kill(devicePid, SIGKILL);
    }
}

void DeviceManager_impl::checkDeviceConfigurationProfile(){

    LOG_TRACE(DeviceManager_impl, "Using DCD profile " << _deviceConfigurationProfile);
    try {
        _fileSys->exists(_deviceConfigurationProfile.c_str());
    } catch( CF::InvalidFileName& _ex ) {
        LOG_FATAL(DeviceManager_impl, "terminating device manager; DCD file " << _deviceConfigurationProfile << " does not exist; " << _ex.msg);
        throw std::runtime_error("invalid dcd file name");
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while checking if " << _deviceConfigurationProfile << " exists";
        LOG_FATAL(DeviceManager_impl, eout.str())
        throw std::runtime_error(eout.str().c_str());
    } catch ( CORBA::Exception& ex ) {
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while checking if " << _deviceConfigurationProfile << " exists";
        LOG_FATAL(DeviceManager_impl, eout.str())
        throw std::runtime_error(eout.str().c_str());
    } catch( ... ) {
        LOG_FATAL(DeviceManager_impl, "terminating device manager; unknown exception checking if " << _deviceConfigurationProfile << " exists; ");
        throw std::runtime_error("unexpected error");
    }
}

/**
 * Get Device Manager attributes (deviceConfigurationProfile, identifier and label)
 * from DCD file.
 */
void DeviceManager_impl::parseDCDProfile(
        DeviceManagerConfiguration& DCDParser,
        const char* overrideDomainName) {

    LOG_TRACE(DeviceManager_impl, "Parsing DCD profile")
    try {
        File_stream _dcd(_fileSys, _deviceConfigurationProfile.c_str());
        DCDParser.load(_dcd);
        _dcd.close();
    } catch ( ossie::parser_error& e ) {
        LOG_FATAL(DeviceManager_impl, "exiting device manager; failure parsing DCD file " << _deviceConfigurationProfile << "; " << e.what());
        throw std::runtime_error(e.what());
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while parsing the DCD file";
        LOG_FATAL(DeviceManager_impl, eout.str())
        throw std::runtime_error(eout.str().c_str());
    } catch ( CORBA::Exception& ex ) {
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while parsing the DCD file";
        LOG_FATAL(DeviceManager_impl, eout.str())
        throw std::runtime_error(eout.str().c_str());
    } catch ( ... ) {
        LOG_FATAL(DeviceManager_impl, "exiting device manager; unexpected failure parsing DCD file ");
        throw std::runtime_error("unexpected error");
    }

    CORBA::String_var tmp_identifier = DCDParser.getID();
    _identifier = (char *)tmp_identifier;
    CORBA::String_var tmp_label = DCDParser.getName();
    _label = (char *)tmp_label;
    LOG_TRACE(DeviceManager_impl, "DeviceManager id: " << DCDParser.getID() << " name: " << DCDParser.getName());

    if (overrideDomainName == NULL) {
        LOG_TRACE(DeviceManager_impl, "Reading domainname from DCD file")
        CORBA::String_var tmp_domainManagerName = DCDParser.getDomainManagerName();
        _domainManagerName = (char *)tmp_domainManagerName;
        _domainName = _domainManagerName.substr(0, _domainManagerName.find_first_of("/"));
    } else {
        LOG_TRACE(DeviceManager_impl, "Overriding domainname from DCD file")
        _domainName = overrideDomainName;
        _domainManagerName = _domainName + "/" + _domainName;
    }
}

void DeviceManager_impl::getDevManImpl(
        const SPD::Implementation*& devManImpl,
        SoftPkg&                    devmgrspdparser) {

    std::vector<SPD::Implementation>::const_iterator itr;

    const std::vector<SPD::Implementation>& _allDevManImpls = devmgrspdparser.getImplementations();
    if (_allDevManImpls.size() == 0) {
        LOG_ERROR(DeviceManager_impl, "Device manager SPD has no implementations");
        throw std::runtime_error("unexpected error");
    }

    for (itr = _allDevManImpls.begin(); itr != _allDevManImpls.end(); ++itr) {
        const std::vector<std::string>& supportedProcessors = (*itr).getProcessors();
        if (count(supportedProcessors.begin(), supportedProcessors.end(), _uname.machine) > 0) {
            devManImpl = &(*itr);
        }
    }
    if (devManImpl == 0) {
        LOG_ERROR(DeviceManager_impl, "Unable to find device manager implementation for " << _uname.machine);
        throw std::runtime_error("unexpected error");
    }
    LOG_TRACE(DeviceManager_impl, "Using device manager implementation " << devManImpl->getID());
}

void DeviceManager_impl::resolveNamingContext(){
    base_context = ossie::corba::stringToName(_domainName);
    bool warnedMissing = false;
    while (true) {
        try {
            CORBA::Object_var obj = ossie::corba::InitialNamingContext()->resolve(base_context);
            rootContext = CosNaming::NamingContext::_narrow(obj);
            LOG_TRACE(DeviceManager_impl, "Connected");
            break;
        } catch ( ... ) {
            if (!warnedMissing) {
                warnedMissing = true;
                LOG_WARN(DeviceManager_impl, "Unable to find naming context " << _domainManagerName << "; retrying");
            }
        }
        // Sleep for a tenth of a second to give the DomainManager a chance to
        // create its naming context.
        usleep(10000);

        // If a shutdown occurs while waiting, turn it into an exception.
        if (*_internalShutdown) {
            throw std::runtime_error("Interrupted waiting to locate DomainManager naming context");
        }
    }
    LOG_TRACE(DeviceManager_impl, "Resolved DomainManager naming context");
}

/** 
 * Store location of the common device PRF by calling deviceProperties.load.
 *
 * Handle exceptions: if an exception occurs, log an error and return false, 
 * which indicates a failure.  Otherwise, return true.
 */
bool DeviceManager_impl::storeCommonDevicePrfLocation(
        SoftPkg& SPDParser, 
        ossie::Properties& deviceProperties) {

    bool returnVal = true;

    LOG_TRACE(DeviceManager_impl, "Loading device PRF file, if any")
    if ( SPDParser.getPRFFile() ) {
        try {
            LOG_TRACE(DeviceManager_impl, "Loading device PRF file " << SPDParser.getPRFFile());
            File_stream prfStream(_fileSys, SPDParser.getPRFFile());
            deviceProperties.load(prfStream);
            LOG_TRACE(DeviceManager_impl, "Loaded device PRF file " << SPDParser.getPRFFile());
            prfStream.close();
        } catch (ossie::parser_error& ex) {
            LOG_ERROR(DeviceManager_impl, 
                "Skipping instantiation of device '" << SPDParser.getSoftPkgName() << "' - '" << SPDParser.getSoftPkgID() << "; "
                << "error parsing file " << SPDParser.getPRFFile() << "; " << ex.what())
            returnVal = false;
        } catch ( std::exception& ex ) {
            std::ostringstream eout;
            eout << "The following standard exception occurred: "<<ex.what()<<". Skipping instantiation of device '" << SPDParser.getSoftPkgName() << "' - '" << SPDParser.getSoftPkgID() << "; " << "error parsing file " << SPDParser.getPRFFile();
            LOG_ERROR(DeviceManager_impl, eout.str())
            returnVal = false;
        } catch ( CORBA::Exception& ex ) {
            std::ostringstream eout;
            eout << "The following CORBA exception occurred: "<<ex._name()<<". Skipping instantiation of device '" << SPDParser.getSoftPkgName() << "' - '" << SPDParser.getSoftPkgID() << "; " << "error parsing file " << SPDParser.getPRFFile();
            LOG_ERROR(DeviceManager_impl, eout.str())
            returnVal = false;
        }
    } else {
        LOG_TRACE(DeviceManager_impl, "Device does not provide softpkg PRF file");
    }

    return returnVal;
}

bool DeviceManager_impl::storeImplementationSpecificDevicePrfLocation(
        SoftPkg&                    SPDParser, 
        ossie::Properties&          deviceProperties,
        const SPD::Implementation*& matchedDeviceImpl) {

    bool returnVal = true;    
    // store location of implementation specific PRF file
    if ( matchedDeviceImpl->getPRFFile()) {
        LOG_TRACE(DeviceManager_impl, "Joining implementation PRF file" << matchedDeviceImpl->getPRFFile());
        try {
            LOG_TRACE(DeviceManager_impl, "Joining implementation PRF file" << matchedDeviceImpl->getPRFFile());
            File_stream prfStream(_fileSys, matchedDeviceImpl->getPRFFile());
            deviceProperties.join(prfStream);
            prfStream.close();
        } catch (ossie::parser_error& ex) {
            LOG_ERROR(DeviceManager_impl, 
                "Skipping instantiation of device '" << SPDParser.getSoftPkgName() << "' - '" << SPDParser.getSoftPkgID() << "; "
                << "error parsing file " << SPDParser.getPRFFile() << "; " << ex.what())
            returnVal = false;
        } catch ( std::exception& ex ) {
            std::ostringstream eout;
            eout << "The following standard exception occurred: "<<ex.what()<<". Skipping instantiation of device '" << SPDParser.getSoftPkgName() << "' - '" << SPDParser.getSoftPkgID() << "; " << "error parsing file " << SPDParser.getPRFFile();
            LOG_ERROR(DeviceManager_impl, eout.str())
            returnVal = false;
        } catch ( CORBA::Exception& ex ) {
            std::ostringstream eout;
            eout << "The following CORBA exception occurred: "<<ex._name()<<". Skipping instantiation of device '" << SPDParser.getSoftPkgName() << "' - '" << SPDParser.getSoftPkgID() << "; " << "error parsing file " << SPDParser.getPRFFile();
            LOG_ERROR(DeviceManager_impl, eout.str())
            returnVal = false;
        }
    } else {
        LOG_TRACE(DeviceManager_impl, "Device does not provide implementation specific PRF file");
    }

    return returnVal;
}

bool DeviceManager_impl::loadSPD(
        SoftPkg&                    SPDParser,
        DeviceManagerConfiguration& DCDParser,
        const ComponentPlacement&   componentPlacement) {

    LOG_TRACE(DeviceManager_impl, "Getting file name for refid " << componentPlacement.getFileRefId());
    const char* spdFile = DCDParser.getFileNameFromRefId(componentPlacement.getFileRefId());
    if (spdFile == 0) {
        LOG_ERROR(DeviceManager_impl, "cannot instantiate component; component file for id " << componentPlacement.getFileRefId() << " isn't defined")
        return false;
    }

   try {
        File_stream _spd(_fileSys, spdFile);
        SPDParser.load( _spd, spdFile);  
        _spd.close();
    } catch ( ossie::parser_error& e ) {
        LOG_FATAL(DeviceManager_impl, "stopping device manager; error parsing SPD " << spdFile << "; " << e.what());
        throw std::runtime_error("unexpected error");
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while parsing the SPD " << spdFile;
        LOG_FATAL(DeviceManager_impl, eout.str())
        throw std::runtime_error(eout.str().c_str());
    } catch ( CORBA::Exception& ex ) {
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while parsing the SPD " << spdFile;
        LOG_FATAL(DeviceManager_impl, eout.str())
        throw std::runtime_error(eout.str().c_str());
    } catch ( ... ) {
        LOG_FATAL(DeviceManager_impl, "stopping device manager; unknown error parsing SPD")
        throw std::runtime_error("unexpected error");
    }

    return true;
}

/**
 * Join properties from common PRF and implementation PRFs.
 */
bool DeviceManager_impl::joinDevicePropertiesFromPRFs(
        SoftPkg&                    SPDParser,
        Properties&                 deviceProperties,
        const SPD::Implementation*& matchedDeviceImpl) {

    if (!storeCommonDevicePrfLocation(SPDParser, deviceProperties )) {
        return false;
    }

    if (!storeImplementationSpecificDevicePrfLocation(
            SPDParser, deviceProperties, matchedDeviceImpl)) {
        return false;
    }

    return true;
}

/**
 * Record the mapping of the component instantiation id to the matched 
 * implementation id.  The scope is needed to remain consistent with the 
 * scoped lock protection for the map.
 */
void DeviceManager_impl::recordComponentInstantiationId(
        const ComponentInstantiation& instantiation,
        const SPD::Implementation*&   matchedDeviceImpl) {

    boost::mutex::scoped_try_lock lock(componentImplMapmutex);
    _componentImplMap[instantiation.getID()] = matchedDeviceImpl->getID();
}

bool DeviceManager_impl::getCodeFilePath(
        std::string&                codeFilePath,
        const SPD::Implementation*& matchedDeviceImpl,
        SoftPkg&                    SPDParser,
        FileSystem_impl*&           fs_servant) {

    // get code file (the path to the device that must be run)
    fs::path codeFile = fs::path(matchedDeviceImpl->getCodeFile());
    if (!codeFile.has_root_directory()) {
        codeFile = fs::path(SPDParser.getSPDPath()) / codeFile;
        LOG_TRACE(DeviceManager_impl, "code localfile had relative path; absolute path: " << codeFile);
    }
    codeFile = codeFile.normalize();

    fs::path entryPoint;
    if (matchedDeviceImpl->getEntryPoint() != 0) {
        LOG_TRACE(DeviceManager_impl, "Using provided entry point: " << matchedDeviceImpl->getEntryPoint())
        entryPoint = fs::path(matchedDeviceImpl->getEntryPoint());
        if (!entryPoint.has_root_directory()) {
            entryPoint = fs::path(SPDParser.getSPDPath()) / entryPoint;
            LOG_TRACE(DeviceManager_impl, "code entrypoint had relative path; absolute path: " << entryPoint);
        }
        entryPoint = entryPoint.normalize();
    } else if (matchedDeviceImpl->getEntryPoint() == 0) {
        LOG_ERROR(DeviceManager_impl, "not instantiating device; no entry point provided");
        return false;
    }

    codeFilePath = fs_servant->getLocalPath(entryPoint.string().c_str());

    if (codeFilePath.length() == 0) {
        LOG_WARN(DeviceManager_impl, "Invalid device file. Could not find executable for " << codeFile)
        return false;
    }

    LOG_TRACE(DeviceManager_impl, "Code file path: " << codeFilePath)

    return true;
}

/**
 * Call rootContext->bind_new_context and handle any exceptions.
 */
void DeviceManager_impl::bindNamingContext() {
    CosNaming::Name devMgrContextName;
    devMgrContextName.length(1);
    devMgrContextName[0].id = CORBA::string_dup(_label.c_str());
    try {
        devMgrContext = rootContext->bind_new_context(devMgrContextName);
    } catch (CosNaming::NamingContext::AlreadyBound&) {
        LOG_WARN(DeviceManager_impl, "Device manager name already bound")
        rootContext->unbind(devMgrContextName);
        devMgrContext = rootContext->bind_new_context(devMgrContextName);
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while creating the Device Manager naming context";
        LOG_FATAL(DeviceManager_impl, eout.str())
        throw std::runtime_error(eout.str().c_str());
    } catch ( CORBA::Exception& ex ) {
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while creating the Device Manager naming context";
        LOG_FATAL(DeviceManager_impl, eout.str())
        throw std::runtime_error(eout.str().c_str());
    } catch( ... ) {
        LOG_FATAL(DeviceManager_impl, "Unable to create device manager context")
        throw std::runtime_error("unexpected error");
    }
}

/**
 * Populate devmgrspdparser by parsing contents of DCDParser.
 *
 * Handle any exceptions associated with loading the SPD to the 
 * devmgrspdparser.
 */
void DeviceManager_impl::parseSpd(
        const DeviceManagerConfiguration& DCDParser, 
        SoftPkg&                          devmgrspdparser) {

    CORBA::String_var tmp_devmgrsoftpkg = DCDParser.getDeviceManagerSoftPkg();
    std::string devmgrsoftpkg = (char *)tmp_devmgrsoftpkg;

    if (devmgrsoftpkg[0] != '/') {
        std::string dcdPath = _deviceConfigurationProfile.substr(0, _deviceConfigurationProfile.find_last_of('/'));
        devmgrsoftpkg = dcdPath + '/' + devmgrsoftpkg;
    }

    try {
        File_stream spd(_fileSys, devmgrsoftpkg.c_str());
        devmgrspdparser.load(spd, devmgrsoftpkg.c_str());
        spd.close();
    } catch (ossie::parser_error& e) {
        LOG_ERROR(DeviceManager_impl, "creating device manager error; error parsing spd " << devmgrsoftpkg << "; " << e.what());
        throw std::runtime_error("unexpected error");
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while parsing the spd " << devmgrsoftpkg;
        LOG_ERROR(DeviceManager_impl, eout.str())
        throw std::runtime_error(eout.str().c_str());
    } catch ( CORBA::Exception& ex ) {
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while parsing the spd " << devmgrsoftpkg;
        LOG_ERROR(DeviceManager_impl, eout.str())
        throw std::runtime_error(eout.str().c_str());
    } catch( ... ) {
        LOG_ERROR(DeviceManager_impl, "creating device manager error; unknown error parsing spd " << devmgrsoftpkg);
        throw std::runtime_error("unexpected error");
    }
}

/**
 * Populates _domainManagerName by calling getDomainManagerReference.
 *
 * If an exception is thrown by getDomainManagerReference, this method will 
 * catch it, log ean error, and rethrow the exception.
 */
void DeviceManager_impl::getDomainManagerReferenceAndCheckExceptions() {

    LOG_INFO(DeviceManager_impl, "Connecting to Domain Manager " << _domainManagerName)
    try {
        getDomainManagerReference(_domainManagerName);
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while attempting to reach the Domain Manager";
        LOG_FATAL(DeviceManager_impl, eout.str())
        throw std::runtime_error(eout.str().c_str());
    } catch ( CORBA::Exception& ex ) {
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while attempting to reach the Domain Manager";
        LOG_FATAL(DeviceManager_impl, eout.str())
        throw std::runtime_error(eout.str().c_str());
    } catch ( ... ) {
        LOG_FATAL(DeviceManager_impl, "[DeviceManager::post_constructor] Unable to get a reference to the DomainManager");
        throw std::runtime_error("unexpected error");
    }

    if (CORBA::is_nil(_dmnMgr)) {
        LOG_FATAL(DeviceManager_impl, "Failure getting Domain Manager")
        throw std::runtime_error("unexpected error");
    }
}

void DeviceManager_impl::registerDeviceManagerWithDomainManager(
        CF::DeviceManager_var& my_object_var) {

    LOG_TRACE(DeviceManager_impl, "Registering with DomainManager");
    while (true) {
        if (*_internalShutdown) {
            throw std::runtime_error("Interrupted waiting to register with DomainManager");
        }
        try {
            _dmnMgr->registerDeviceManager(my_object_var);
            break;
        } catch (const CORBA::TRANSIENT& ex) {
            // The DomainManager isn't currently reachable, but it may become accessible again.
            usleep(100000);
        } catch (const CORBA::OBJECT_NOT_EXIST& ex) {
            // This error occurs while the DomainManager is still being constructed 
            usleep(100000);
        } catch (const CF::DomainManager::RegisterError& e) {
            LOG_ERROR(DeviceManager_impl, "Failed to register with domain manager due to: " << e.msg);
            throw std::runtime_error("Error registering with Domain Manager");
        } catch (const CF::InvalidObjectReference& _ex) {
            LOG_FATAL(DeviceManager_impl, "While registering DevMgr with DomMgr: " << _ex.msg);
            throw std::runtime_error("Error registering with Domain Manager");
        } catch ( std::exception& ex ) {
            std::ostringstream eout;
            eout << "The following standard exception occurred: "<<ex.what()<<" while registering the Device Manager with the Domain Manager";
            LOG_FATAL(DeviceManager_impl, eout.str())
            throw std::runtime_error(eout.str().c_str());
        } catch ( CORBA::Exception& ex ) {
            std::ostringstream eout;
            eout << "The following CORBA exception occurred: "<<ex._name()<<" while registering the Device Manager with the Domain Manager";
            LOG_FATAL(DeviceManager_impl, eout.str())
            throw std::runtime_error(eout.str().c_str());
        } catch (...) {
            LOG_FATAL(DeviceManager_impl, "While registering DevMgr with DomMgr: Unknown Exception");
            throw std::runtime_error("Error registering with Domain Manager");
        }
    }
}

void DeviceManager_impl::getCompositeDeviceIOR(
        std::string&                                  compositeDeviceIOR, 
        const std::vector<ossie::ComponentPlacement>& componentPlacements,
        const ossie::ComponentPlacement&              componentPlacementInst) {

    //see if component is composite part of device
    LOG_TRACE(DeviceManager_impl, "Checking composite part of device");
    if (componentPlacementInst.isCompositePartOf()) {
        std::string parentDeviceRefid = componentPlacementInst.getCompositePartOfDeviceID();
        LOG_TRACE(DeviceManager_impl, "CompositePartOfDevice: <" << parentDeviceRefid << ">");
        //find parent ID and stringify the IOR
        for (unsigned int cp_idx = 0; cp_idx < componentPlacements.size(); cp_idx++) {
            // must match to a particular instance
            for (unsigned int ci_idx = 0; ci_idx < componentPlacements[cp_idx].getInstantiations().size(); ci_idx++) {
                const char* instanceID = componentPlacements[cp_idx].instantiations[ci_idx].getID();
                if (strcmp(instanceID, parentDeviceRefid.c_str()) == 0) {
                    LOG_TRACE(DeviceManager_impl, "CompositePartOfDevice: Found parent device instance <" 
                            << componentPlacements[cp_idx].getInstantiations()[ci_idx].getID() 
                            << "> for child device <" << componentPlacementInst.getFileRefId() << ">");
                    // now get the associated IOR
                    while (true) {
                        std::string tmpior = getIORfromID(instanceID);
                        if (!tmpior.empty()) {
                            compositeDeviceIOR = tmpior;
                            LOG_TRACE(DeviceManager_impl, "CompositePartOfDevice: Found parent device IOR <" << compositeDeviceIOR << ">");
                            break;
                        }
                        usleep(100);
                    }
                }

            }
        }
    }
}

void DeviceManager_impl::getOverloadprops(
        std::map<std::string, std::string>& overloadprops, 
        const std::vector<ComponentProperty*>& instanceprops,
        const ossie::Properties& deviceProperties) {

    std::vector<const Property*>::const_iterator jprops_iter;
    std::vector<ComponentProperty*>::const_iterator iprops_iter;

    const std::vector<const Property*>& deviceExecParams = deviceProperties.getExecParamProperties();
    LOG_TRACE(DeviceManager_impl, "getting exec params. Num properties: " << deviceExecParams.size());
    for (jprops_iter = deviceExecParams.begin(); jprops_iter != deviceExecParams.end(); jprops_iter++) {
        LOG_TRACE(DeviceManager_impl, "using execparam: " << (*jprops_iter)->getID());
        assert((*jprops_iter) != 0);

        // if no instantiation property overrode the execparm, then just use what came from the joined PRF property set
        // but make sure there actually was a default value
        if ((*jprops_iter)->isNone() == false) {
            if (dynamic_cast<const SimpleProperty*>(*jprops_iter) != NULL) {
                const SimpleProperty* tmp = dynamic_cast<const SimpleProperty*>(*jprops_iter);
                LOG_TRACE(DeviceManager_impl, "setting execparam " << (*jprops_iter)->getID() << " to " <<  tmp->getValue());
                overloadprops[(*jprops_iter)->getID()] = tmp->getValue();
            } else {
                LOG_WARN(DeviceManager_impl, "PRF file error, exec parameters must be simple type");
            }
        } else {
            LOG_WARN(DeviceManager_impl, "skipping exec param with null value")
        }

        // do not allow readonly execparams to be overloaded (the default from the PRF is passed)
        if (std::string((*jprops_iter)->getMode()) == "readonly") {
            LOG_WARN(DeviceManager_impl, "DCD requested that readonly execparam " << (*jprops_iter)->getID() << " " << (*jprops_iter)->getName() << " be over-written; ignoring it")
            continue;
        }

        LOG_TRACE(DeviceManager_impl, "looking for DCD overloaded props. Num props: " << instanceprops.size());
        // see if this exec param has been overloaded (NOTE: we know that execparams are simple elements via the spec)
        for (iprops_iter = instanceprops.begin(); iprops_iter != instanceprops.end(); iprops_iter++) {
            // property has been overloaded in instantiation from DCD
            if (strcmp((*iprops_iter)->getID(), (*jprops_iter)->getID()) == 0) {
                if (dynamic_cast<const SimplePropertyRef*>(*iprops_iter) == NULL) {
                    LOG_WARN(DeviceManager_impl, "ignoring attempt to override exec param with non-simple ref");
                } else {
                    const SimplePropertyRef* simpleref = dynamic_cast<const SimplePropertyRef*>(*iprops_iter);
                    // do some error checking - the property should have one value
                    if (simpleref->getValue() == 0) {
                        LOG_WARN(DeviceManager_impl, "value is empty for property: " << simpleref->getID());
                        continue;
                    }

                    LOG_TRACE(DeviceManager_impl, "overloading execparam " << (*jprops_iter)->getName() << " with value " << simpleref->getValue() << " from DCD");
                    overloadprops[(*jprops_iter)->getID()] = simpleref->getValue();
                    break;
                }
            }
        }

    }
}

bool DeviceManager_impl::loadScdToParser(
        ComponentDescriptor& scdParser, 
        const SoftPkg& SPDParser) {

    bool success = false;

    fs::path scdpath = fs::path(SPDParser.getSCDFile());
    scdpath = scdpath.normalize();
    LOG_TRACE(DeviceManager_impl, "SCD file path: " << scdpath)

    try {
        File_stream scd(_fileSys, scdpath.string().c_str());
        scdParser.load(scd);
        scd.close();
        success = true;
    } catch (ossie::parser_error& ex) {
        LOG_ERROR(DeviceManager_impl, 
                  "SCD file failed validation; parser error on file " <<  scdpath << "; " << ex.what());
    } catch (CF::InvalidFileName ex) {
        LOG_ERROR(DeviceManager_impl, "Failed to validate SCD due to invalid file name " << ex.msg);
    } catch (CF::FileException ex) {
        LOG_ERROR(DeviceManager_impl, "Failed to validate SCD due to file exception" << ex.msg);
    } catch ( std::exception& ex ) {
        LOG_ERROR(DeviceManager_impl, "The following standard exception occurred: "<<ex.what() <<". Unable to parse the SCD")
    } catch ( ... ) {
        LOG_ERROR(DeviceManager_impl, "Unexpected error parsing SCD " << scdpath);
    }

    return success;
}

/** 
 * Get the type, which should be either "device" or 
 * "service" ("executabledevice" and "loadabledevice" are 
 * considered "device"s).  If the type is neither "device" nor
 * service, log an error.
 */
bool DeviceManager_impl::getDeviceOrService(
        std::string& type, 
        const ComponentDescriptor& scdParser) {

    bool supported = false;

    type = scdParser.getComponentType();
    LOG_TRACE(DeviceManager_impl, "Softpkg type " << type)

    // Normalize type into either device or service
    // This is contrary to the spec, but existing devices/service may depend
    // on this behavior
    if ((type == "device") ||
        (type == "loadabledevice") ||
        (type == "executabledevice")) {
        type = "device";
    } 

    if ((type == "device") || (type == "service")) {
        supported = true;
    }
    else {
        LOG_ERROR(DeviceManager_impl, 
                  "Attempt to launch unsupported component type " << type)
    }

    return supported;
}


void DeviceManager_impl::createDeviceCacheLocation(
        std::string& devcache,
        std::string& usageName, 
        const ossie::ComponentInstantiation& instantiation)
{
    // No log messages within this method as it is called between exec and fork

    // create device cache location
    std::string baseDevCache = _cacheroot + "/." + _label;
    if (instantiation.getUsageName() == 0) {
        // no usage name was given, so create one. By definition, the instantiation id must be unique
        usageName = instantiation.instantiationId;
    } else {
        usageName = instantiation.getUsageName();
    }

    devcache = baseDevCache + "/" + usageName;
    bool retval = this->makeDirectory(devcache);
    if (not retval) {
        LOG_ERROR(DeviceManager_impl, "Unable to create the Device cache: " << devcache)
        exit(-1);
    }
}

/**
 * Populate new_argv, which is a list of character strings
 * to be passed to execv.
 */
void DeviceManager_impl::createDeviceExecStatement(
        const char*                                   new_argv[], 
        const ossie::ComponentPlacement&              componentPlacement,
        const std::string&                            componentType,
        std::map<std::string, std::string>*           pOverloadprops,
        const std::string&                            codeFilePath,
        ossie::DeviceManagerConfiguration&            DCDParser,
        const ossie::ComponentInstantiation&          instantiation,
        const std::string&                            usageName,
        const std::vector<ossie::ComponentPlacement>& componentPlacements,
        const std::string&                            compositeDeviceIOR,
        const std::vector<ComponentProperty*>&        instanceprops) {
    
    // DO not put any LOG calls in this method, as it is called beteen
    // fork() and execv().
        
    std::vector<ComponentProperty*>::const_iterator iprops_iter;

    unsigned long new_argc = 0;

    deviceMgrIOR = ossie::corba::objectToString(myObj);
    if (getenv("VALGRIND")) {
        new_argv[new_argc] =  "/usr/local/bin/valgrind";
        new_argc++;
        std::string logFile = "--log-file=";
        logFile += codeFilePath;
        new_argv[new_argc] = (char*)logFile.c_str();
        new_argc++;
    }
    new_argv[new_argc] = (char*)codeFilePath.c_str();
    new_argc++;
    new_argv[new_argc] = "DEVICE_MGR_IOR";
    new_argc++;
    new_argv[new_argc] = (char*)deviceMgrIOR.c_str();
    new_argc++;
    if (componentType == "device") {
        new_argv[new_argc] = "PROFILE_NAME";
        new_argc++;
        new_argv[new_argc] = (char*)DCDParser.getFileNameFromRefId(componentPlacement.getFileRefId());
        new_argc++;
        new_argv[new_argc] = "DEVICE_ID";
        new_argc++;
        new_argv[new_argc] = (char*)instantiation.getID();
        new_argc++;
        new_argv[new_argc] = "DEVICE_LABEL";
        new_argc++;
        new_argv[new_argc] = (char*)usageName.c_str();
        new_argc++;
        if (componentPlacement.isCompositePartOf()) {
            new_argv[new_argc] = "COMPOSITE_DEVICE_IOR";
            new_argc++;
            new_argv[new_argc] = (char*)compositeDeviceIOR.c_str();
            new_argc++;
        }
#if ENABLE_EVENTS
        if (!CORBA::is_nil(IDM_channel)) {
            new_argv[new_argc] = "IDM_CHANNEL_IOR";
            new_argc++;
            new_argv[new_argc] = (char*)IDM_IOR.c_str();
            new_argc++;
        }
#endif
    } else if (componentType == "service") {
        new_argv[new_argc] = "SERVICE_NAME";
        new_argc++;
        new_argv[new_argc] = (char*)usageName.c_str();
        new_argc++;
    }

    logging_uri = "";
    for (iprops_iter = instanceprops.begin(); iprops_iter != instanceprops.end(); iprops_iter++) {
        if ((strcmp((*iprops_iter)->getID(), "LOGGING_CONFIG_URI") == 0)
                && (dynamic_cast<const SimplePropertyRef*>(*iprops_iter) != NULL)) {
            const SimplePropertyRef* simpleref = dynamic_cast<const SimplePropertyRef*>(*iprops_iter);
            logging_uri = simpleref->getValue();
            break;
        }
    }

    if (logging_uri.empty()) {
        if (!logging_config_prop->isNil()) {
            logging_uri = logging_config_uri;
        }
    }

    std::string debug_level;
    if (!logging_uri.empty()) {
        if (logging_uri.substr(0, 4) == "sca:") {
            logging_uri += ("?fs=" + fileSysIOR);
        }
        new_argv[new_argc] = "LOGGING_CONFIG_URI";
        new_argc++;
        new_argv[new_argc] = (char*)logging_uri.c_str();
        new_argc++;
    } else {
        // Pass along the current debug level setting.
#if HAVE_LOG4CXX
        int level = LoggingConfigurator::getLevel();
#else
        int level = 3;
#endif
        // Convert the numeric level directly into its ASCII equivalent.
        debug_level.push_back(char(0x30 + level));
        new_argv[new_argc++] = "DEBUG_LEVEL";
        new_argv[new_argc++] = const_cast<char*>(debug_level.c_str());
    }

    std::map<std::string, std::string>::iterator prop_iter;
    for (prop_iter = pOverloadprops->begin(); prop_iter != pOverloadprops->end(); prop_iter++) {
        new_argv[new_argc] = (char*)prop_iter->first.c_str();
        new_argc++;
        new_argv[new_argc] = (char*)prop_iter->second.c_str();
        new_argc++;                        
    }

    // end the args with a NULL
    new_argv[new_argc] = NULL;
}                    

void DeviceManager_impl::createDeviceThreadAndHandleExceptions(
        const ossie::ComponentPlacement&              componentPlacement,
        const std::string&                            componentType,
        std::map<std::string, std::string>*           pOverloadprops,
        const std::string&                            codeFilePath,
        ossie::DeviceManagerConfiguration&            DCDParser,
        const ossie::ComponentInstantiation&          instantiation,
        const std::vector<ossie::ComponentPlacement>& componentPlacements,
        const std::string&                            compositeDeviceIOR,
        const std::vector<ComponentProperty*>&        instanceprops) {

    try {
        std::string devcache; 
        std::string usageName; 
        createDeviceCacheLocation(devcache, usageName, instantiation);
        createDeviceThread(componentPlacement,
                           componentType,
                           pOverloadprops,
                           codeFilePath,
                           DCDParser,
                           instantiation,
                           devcache,
                           usageName,
                           componentPlacements,
                           compositeDeviceIOR,
                           instanceprops);
    } catch (std::runtime_error& ex) {
        LOG_ERROR(DeviceManager_impl, 
                  "The following runtime exception occurred: "<<ex.what()<<" while launching a Device")
        throw;
    } catch ( std::exception& ex ) {
        LOG_ERROR(DeviceManager_impl, 
                  "The following standard exception occurred: "<<ex.what()<<" while launching a Device")
        throw;
    } catch ( const CORBA::Exception& ex ) {
        LOG_ERROR(DeviceManager_impl, 
                  "The following CORBA exception occurred: "<<ex._name()<<" while launching a Device")
        throw;
    } catch ( ... ) {
        LOG_TRACE(DeviceManager_impl, 
                  "Launching Device file failed with an unknown exception")
        throw;
    }
}

void DeviceManager_impl::createDeviceThread(
        const ossie::ComponentPlacement&              componentPlacement,
        const std::string&                            componentType,
        std::map<std::string, std::string>*           pOverloadprops,
        const std::string&                            codeFilePath,
        ossie::DeviceManagerConfiguration&            DCDParser,
        const ossie::ComponentInstantiation&          instantiation,
        const std::string&                            devcache,
        const std::string&                            usageName,
        const std::vector<ossie::ComponentPlacement>& componentPlacements,
        const std::string&                            compositeDeviceIOR,
        const std::vector<ComponentProperty*>&        instanceprops) {
 
    int pid;

    LOG_TRACE(DeviceManager_impl, "Launching " << componentType << " file " 
                                  << codeFilePath << " Usage name " 
                                  << instantiation.getUsageName());

    if ((pid = fork()) > 0) {
        // parent process: pid is the process ID of the child
        LOG_TRACE(DeviceManager_impl, "Resulting PID: " << pid);

        // Add the new device/service to the pending list. When it registers, the remaining
        // fields will be filled out and it will be moved to the registered list.
        if (componentType == "service") {
            ServiceNode* serviceNode = new ServiceNode;
            serviceNode->identifier = instantiation.getID();
            serviceNode->label = usageName;
            serviceNode->pid = pid;
            boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);
            _pendingServices.push_back(serviceNode);
        } else {
            DeviceNode* deviceNode = new DeviceNode;
            deviceNode->identifier = instantiation.getID();
            deviceNode->label      = usageName;
            deviceNode->pid        = pid;
            boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);
            _pendingDevices.push_back(deviceNode);
        }
    }
    else if (pid == 0) {
        // Child process

        //////////////////////////////////////////////////////////////
        // DO not put any LOG calls between the fork and the execv
        //////////////////////////////////////////////////////////////

        // switch to working directory
        chdir(devcache.c_str());

        const char* new_argv[pOverloadprops->size() + 30];

        createDeviceExecStatement(new_argv, 
                                  componentPlacement,
                                  componentType,
                                  pOverloadprops,
                                  codeFilePath,
                                  DCDParser,
                                  instantiation,
                                  usageName,
                                  componentPlacements,
                                  compositeDeviceIOR,
                                  instanceprops) ;

        // now exec - we should not return from this
        execv(new_argv[0], (char * const*) new_argv);

        LOG_ERROR(DeviceManager_impl, new_argv[0] << " did not execute : " << strerror(errno));
        exit(-1);
    }
    else {
        // The system cannot support deployment of the device
        // The most likely cause is the operating system running out of 
        // threads, in which case the system is in bad shape.  Exit
        // with an error to allow the system to recover.
        LOG_ERROR(DeviceManager_impl, "[DeviceManager::execute] Cannot create device thread: " << strerror(errno)); 

        // If we could not get a thread to create the device, previously created 
        // devices will likely have trouble with registration.
        abort();

        throw;
    }
}

void DeviceManager_impl::abort() {
    sigkillPendingDevices();
    shutdown();
}

DeviceManager_impl::DeviceManager_impl(
        const char*     DCDInput, 
        const char*     _rootfs, 
        const char*     _cachepath, 
        const char*     _logconfig_uri, 
        struct utsname uname, 
        bool*          internalShutdown) :
    _registeredDevices()
{
    // These should probably be execparams at some point
    _fsroot                     = _rootfs;
    _cacheroot                  = _cachepath;
    _deviceConfigurationProfile = DCDInput;
    _uname                      = uname;
    _internalShutdown           = internalShutdown;

    // Initialize properties
    logging_config_prop = (StringProperty*)addProperty(logging_config_uri, 
                                                       "LOGGING_CONFIG_URI", 
                                                       "LOGGING_CONFIG_URI",
                                                       "readonly", 
                                                       "", 
                                                       "external", 
                                                       "configure");
    if (_logconfig_uri) {
        logging_config_prop->setValue(_logconfig_uri);
    }

    addProperty(HOSTNAME,
               "HOSTNAME",
               "HOSTNAME",
               "readonly",
               "",
               "external",
               "configure");
    
    char _hostname[1024];
    gethostname(_hostname, 1024);
    std::string hostname(_hostname);
    HOSTNAME = hostname;
}

/**
 * Parsing constructor
 *
 * Parse the device manager configuration files.
 *
 * Register with the Domain Manager
 *
 * Loop through through the DeviceManager's associated devices and
 * create a thread for each device.
 */
void DeviceManager_impl::post_constructor (
        CF::DeviceManager_var my_object_var, 
        const char* overrideDomainName) 
    throw (CORBA::SystemException, std::runtime_error)

{
    myObj = my_object_var;

    // Create the device file system in the DeviceManager POA.
    LOG_TRACE(DeviceManager_impl, "Creating device file system")
    FileSystem_impl* fs_servant = new FileSystem_impl(_fsroot.c_str());
    PortableServer::POA_var poa = ossie::corba::RootPOA()->find_POA("DeviceManager", 1);
    PortableServer::ObjectId_var oid = poa->activate_object(fs_servant);
    fs_servant->_remove_ref();
    _fileSys = fs_servant->_this();

    fileSysIOR = ossie::corba::objectToString(_fileSys);

    checkDeviceConfigurationProfile();

    DeviceManagerConfiguration DCDParser;
    parseDCDProfile(DCDParser, overrideDomainName);

    resolveNamingContext();

    bindNamingContext();

    SoftPkg devmgrspdparser;
    parseSpd(DCDParser, devmgrspdparser);

    const SPD::Implementation* devManImpl = 0;
    getDevManImpl(devManImpl, devmgrspdparser);

    getDomainManagerReferenceAndCheckExceptions();

    registerDeviceManagerWithDomainManager(my_object_var);

    // Now that we've successfully communicated with the DomainManager, allow
    // for 1 retry in the event that it crashes and recovers, leaving us with a
    // valid reference but a stale connection.
    ossie::corba::setObjectCommFailureRetries(_dmnMgr, 1);

#if ENABLE_EVENTS
    IDM_channel = ossie::event::connectToEventChannel(rootContext, "IDM_Channel");
    if (CORBA::is_nil(IDM_channel)) {
        LOG_INFO(DeviceManager_impl, "IDM channel not found. Continuing without using the IDM channel");
    } else {
        IDM_IOR = ossie::corba::objectToString(IDM_channel);
    }
#endif

    _adminState = DEVMGR_REGISTERED;

    // create device manager cache location
    std::string devmgrcache(_cacheroot + "/." + _label);
    LOG_TRACE(DeviceManager_impl, "Creating DevMgr cache: " << devmgrcache)
    bool retval = this->makeDirectory(devmgrcache);
    if (not retval) {
        LOG_ERROR(DeviceManager_impl, "Unable to create the Device Manager cache: " << devmgrcache)
    }

    //parse filesystem names

    //Parse local components from DCD files
    LOG_TRACE(DeviceManager_impl, "Grabbing component placements")
    const std::vector<ossie::ComponentPlacement>& componentPlacements = DCDParser.getComponentPlacements();
    LOG_TRACE(DeviceManager_impl, "ComponentPlacement size is " << componentPlacements.size())

    std::vector<ossie::ComponentPlacement>::const_iterator compPlaceIter;
    for (compPlaceIter =  componentPlacements.begin(); 
         compPlaceIter != componentPlacements.end();  
         compPlaceIter++) {

        SoftPkg SPDParser;

        if (!loadSPD(SPDParser, DCDParser, *compPlaceIter)) {
            continue;
        }

        //get code file name from implementation
        LOG_TRACE(DeviceManager_impl, "Matching device to device manager implementation")

        // first find the correct implementation of the GPP from the Device Manager
        // implementation. second, determine if the Device Manager has any
        // <dependency> tags, and if they exist, get a reference to these devices
        //
        // (this algorithm assumes only a single GPP is being deployed in the node, and
        // it also assumes that there is a single device manager implementation)

        // get Device Manager implementation
        const SPD::Implementation* matchedDeviceImpl = NULL;

        // If there is no deployon element, deploy the implementation that
        // matches the device manager implementation.  Otherwise, access
        // the deployon device and match implementation properties
        if (!compPlaceIter->isDeployOn()) {
            matchedDeviceImpl = locateMatchingDeviceImpl(SPDParser, devManImpl);
        } else {
            // Before we can implement this, we need to determine the following:
            //  1. Does the refid reference only devices in the DCD or in the entire domain
            //  2. The DeviceManager needs to spawn any deployondependencies first
            //     so it can issue the ExecutableDevice->execute() function on those devices
            //  3. How are matching properties to be used in this context?
            LOG_ERROR(DeviceManager_impl, 
                "Skipping instantiation of device '" << SPDParser.getSoftPkgName() << "' - '" << SPDParser.getSoftPkgID() << "; "
                << "The <deployondevice> element is currently not supported");
            continue;
        }

        if (matchedDeviceImpl == NULL) {
            LOG_ERROR(DeviceManager_impl, 
                  "Skipping instantiation of device '" << SPDParser.getSoftPkgName() << "' - '" << SPDParser.getSoftPkgID() << "; "
                  << "no available device implementations match device manager properties")
            continue;
        }

        // store the matchedDeviceImpl's implementation ID in a map for use with "getComponentImplementationId"

        ossie::Properties deviceProperties;
        if (!loadDeviceProperties(SPDParser, *matchedDeviceImpl, deviceProperties)) {
            LOG_INFO(DeviceManager_impl, "Skipping instantiation of device '" << SPDParser.getSoftPkgName() << "'");
            continue;
        }

        std::string compositeDeviceIOR;
        getCompositeDeviceIOR(compositeDeviceIOR, 
                              componentPlacements, 
                              *compPlaceIter);

        std::vector<ComponentInstantiation>::const_iterator cpInstIter;
        for (cpInstIter =  compPlaceIter->getInstantiations().begin(); 
             cpInstIter != compPlaceIter->getInstantiations().end(); 
             cpInstIter++) {

            const ComponentInstantiation instantiation = *cpInstIter;
            LOG_TRACE(DeviceManager_impl, "Placing component " << instantiation.getID());

            recordComponentInstantiationId(instantiation, matchedDeviceImpl);

            // get overloaded properties for exec params
            const std::vector<ComponentProperty*>& instanceprops = instantiation.getProperties();
            std::map<std::string, std::string> overloadprops;

            getOverloadprops(overloadprops, instanceprops, deviceProperties);

            //spawn device

            std::string codeFilePath;
            if (!getCodeFilePath(codeFilePath,
                                 matchedDeviceImpl,
                                 SPDParser,
                                 fs_servant)) {
                continue;
            }

            // Use the SPDParser to populate componentType
            ComponentDescriptor scdParser;
            if (!loadScdToParser(scdParser, SPDParser)) {
                continue;
            }
            std::string componentType;
            if (!getDeviceOrService(componentType, scdParser)) {
                // We got a type other than "device" or "service"
                continue;
            }

            // Attempt to create the requested device or service
            createDeviceThreadAndHandleExceptions(
                *compPlaceIter,
                componentType,
                &overloadprops,
                codeFilePath,
                DCDParser,
                instantiation,
                componentPlacements,
                compositeDeviceIOR,
                instanceprops);
        }
    }
}

const SPD::Implementation* DeviceManager_impl::locateMatchingDeviceImpl(const SoftPkg& devSpd, const SPD::Implementation* deployOnImpl)
{
    TRACE_ENTER(DeviceManager_impl)
    const SPD::Implementation* result = NULL;

    LOG_TRACE(DeviceManager_impl, "Matching candidate device impls to implementation " << deployOnImpl->getID() );

    // TODO - support all matching properties, not just processor_name, os_name, and os_version
    // TODO - support the generic PRF file and not just the implementation PRF file
    Properties PRFparser;
    if (deployOnImpl->getPRFFile() && (strlen(deployOnImpl->getPRFFile()) > 0)) {
        // Parse the implementations propertyfile...we are looking for processor_name, os_name, and os_version
        std::string localfile = deployOnImpl->getPRFFile();
        if (localfile[0] != '/') {
            localfile = std::string(devSpd.getSPDPath()) + "/" + localfile;
        }

        try {
            File_stream prfStream(_fileSys, localfile.c_str());
            PRFparser.load(prfStream);
            prfStream.close();
        } catch( CF::InvalidFileName& _ex ) {
            LOG_ERROR(DeviceManager_impl, "While opening PRF " << _ex.msg)
            throw(CF::InvalidObjectReference());
        } catch( CF::FileException& _ex ) {
            LOG_ERROR(DeviceManager_impl, "While opening PRF " << _ex.msg)
            throw(CF::InvalidObjectReference());
        } catch( CORBA::SystemException& se ) {
            LOG_ERROR(DeviceManager_impl, "Failed with CORBA::SystemException\n")
            throw(CF::InvalidObjectReference());
        } catch ( std::exception& ex ) {
            LOG_ERROR(DeviceManager_impl, "The following standard exception occurred: "<<ex.what()<<" While opening PRF "<<localfile)
            throw(CF::InvalidObjectReference());
        } catch ( const CORBA::Exception& ex ) {
            LOG_ERROR(DeviceManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" While opening PRF "<<localfile)
            throw(CF::InvalidObjectReference());
        } catch( ... ) {
            LOG_ERROR(DeviceManager_impl, "failed with Unknown Exception\n")
            throw(CF::InvalidObjectReference());
        }
    } else {
        LOG_ERROR(DeviceManager_impl, "Cannot instantiate device; device manager implementation does not specify a properties file.")
        return 0;
    }

    const std::vector<SPD::Implementation>& impls = devSpd.getImplementations();
    const std::vector<const Property*>& props = PRFparser.getAllocationProperties();
    // flip through all component implementations the device supports

    unsigned int implIndex = 0;
    while( implIndex < impls.size() ) {
        LOG_TRACE(DeviceManager_impl, 
                  "Attempting to match device " << devSpd.getSoftPkgName() 
                  << " implementation id: " << impls[implIndex].getID()
                  << " to device manager implementation " << deployOnImpl->getID());

        if (checkProcessorAndOs(impls[implIndex], props)) {
            LOG_TRACE(DeviceManager_impl, 
                      "found matching processing device implementation to the Device Manager's implementation.");
            result = &impls[implIndex];
            break;
        } else {
            LOG_TRACE(DeviceManager_impl, 
                      "failed to match device processor and/or os to device manager implementation");
        }
        implIndex++;
    }

    // if a component implementation cannot be matched to the device, error out
    if (!result) {
        LOG_WARN(DeviceManager_impl, "Could not match an implementation.");
    }
    LOG_TRACE(DeviceManager_impl, "Done finding matching device implementation");
    return result;
}

bool DeviceManager_impl::checkProcessorAndOs(const SPD::Implementation& device, const std::vector<const Property*>& props)
{
    bool matchProcessor = checkProcessor(device.getProcessors(), props);
    bool matchOs = checkOs(device.getOsDeps(), props);

    if (!matchProcessor) {
        LOG_TRACE(DeviceManager_impl, "Failed to match device processor to device manager allocation properties");
    }
    if (!matchOs) {
        LOG_TRACE(DeviceManager_impl, "Failed to match device os to device manager allocation properties");
    }
    return matchProcessor && matchOs;
}

bool DeviceManager_impl::loadDeviceProperties (const SoftPkg& softpkg, const SPD::Implementation& deviceImpl, ossie::Properties& properties)
{
    // store location of the common device PRF
    if (softpkg.getPRFFile()) {
        const std::string prfFile = softpkg.getPRFFile();
        LOG_TRACE(DeviceManager_impl, "Loading device softpkg PRF file " << prfFile);
        if (!joinPRFProperties(prfFile, properties)) {
            return false;
        }
    } else {
        LOG_TRACE(DeviceManager_impl, "Device does not provide softpkg PRF file");
    }

    // store location of implementation specific PRF file
    if (deviceImpl.getPRFFile()) {
        const std::string prfFile = deviceImpl.getPRFFile();
        LOG_TRACE(DeviceManager_impl, "Joining implementation-specific PRF file " << prfFile);
        if (!joinPRFProperties(prfFile, properties)) {
            return false;
        }
    } else {
        LOG_TRACE(DeviceManager_impl, "Device does not provide implementation-specific PRF file");
    }

    return true;
}

bool DeviceManager_impl::joinPRFProperties (const std::string& prfFile, ossie::Properties& properties)
{
    try {
        // Check for the existence of the PRF file first so we can give a more meaningful error message.
        if (!_fileSys->exists(prfFile.c_str())) {
            LOG_ERROR(DeviceManager_impl, "PRF file " << prfFile << " does not exist");
        } else {
            LOG_TRACE(DeviceManager_impl, "Loading PRF file " << prfFile);
            File_stream prfStream(_fileSys, prfFile.c_str());
            properties.join(prfStream);
            LOG_TRACE(DeviceManager_impl, "Loaded PRF file " << prfFile);
            prfStream.close();
            return true;
        }
    } catch (const ossie::parser_error& ex) {
        LOG_ERROR(DeviceManager_impl, "XML parser error '" << ex.what() << "' in PRF file " << prfFile);
    } CATCH_LOG_ERROR(DeviceManager_impl, "Failure parsing PRF file " << prfFile);

    return false;
}

void
DeviceManager_impl::getDomainManagerReference (const std::string& domainManagerName)
{
    CORBA::Object_var obj = CORBA::Object::_nil();

/// \todo sleep prevents system from beating Name Service to death, Fix better
    bool warned = false;
    do {
        try {
            obj = ossie::corba::objectFromName(domainManagerName);
        } catch (const CosNaming::NamingContext::NotFound&) {
            if (!warned) {
                warned = true;
                LOG_WARN(DeviceManager_impl, "DomainManager not registered with NameService; retrying");
            }
        } catch( CORBA::SystemException& se ) {
            LOG_ERROR(DeviceManager_impl, "[DeviceManager::getDomainManagerReference] \"get_object_from_name\" failed with CORBA::SystemException")
            throw;
        } catch ( std::exception& ex ) {
            LOG_ERROR(DeviceManager_impl, "The following standard exception occurred: "<<ex.what()<<" while attempting \"get_object_from_name\"")
            throw;
        } catch ( const CORBA::Exception& ex ) {
            LOG_ERROR(DeviceManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" while attempting \"get_object_from_name\"")
            throw;
        } catch( ... ) {
            LOG_ERROR(DeviceManager_impl, "[DeviceManager::getDomainManagerReference] \"get_object_from_name\" failed with Unknown Exception")
            throw;
        }

        // Sleep for a tenth of a second to give the DomainManager a chance to
        // bind itself into the naming context.
        usleep(10000);

        // If a shutdown occurs while waiting, turn it into an exception.
        if (*_internalShutdown) {
            throw std::runtime_error("Interrupted waiting to lookup DomainManager in NameService");
        }
    } while(CORBA::is_nil(obj));

    try {
        _dmnMgr = CF::DomainManager::_narrow (obj);
    } catch ( std::exception& ex ) {
        LOG_ERROR(DeviceManager_impl, "The following standard exception occurred: "<<ex.what()<<" while attempting to narrow on the Domain Manager")
        throw;
    } catch ( const CORBA::Exception& ex ) {
        LOG_ERROR(DeviceManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" while attempting to narrow on the Domain Manager")
        throw;
    } catch( ... ) {
        LOG_ERROR(DeviceManager_impl, "[DeviceManager::getDomainManagerReference] \"CF:DomainManager::_narrow\" failed with Unknown Exception")
        throw;
    }
}


char* DeviceManager_impl::deviceConfigurationProfile ()
throw (CORBA::SystemException)
{
    return CORBA::string_dup(_deviceConfigurationProfile.c_str());
}


CF::FileSystem_ptr DeviceManager_impl::fileSys ()throw (CORBA::
                                                        SystemException)
{
    CF::FileSystem_var result = _fileSys;
    return result._retn();
}


char* DeviceManager_impl::identifier ()
throw (CORBA::SystemException)
{
    return CORBA::string_dup (_identifier.c_str());
}


char* DeviceManager_impl::label ()
throw (CORBA::SystemException)
{
    return CORBA::string_dup (_label.c_str());
}


CF::DeviceManager::ServiceSequence *
DeviceManager_impl::registeredServices ()throw (CORBA::SystemException)
{
    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);

    CF::DeviceManager::ServiceSequence_var result;

    try {
        result = new CF::DeviceManager::ServiceSequence();
        result->length(_registeredServices.size());
        for (CORBA::ULong ii = 0; ii < _registeredServices.size(); ++ii){
            result[ii].serviceObject = CORBA::Object::_duplicate(_registeredServices[ii]->service);
            result[ii].serviceName = _registeredServices[ii]->label.c_str();
        }
    } catch ( ... ){
        result = new CF::DeviceManager::ServiceSequence();
    }

    return result._retn();
}

void
DeviceManager_impl::registerDevice (CF::Device_ptr registeringDevice)
throw (CORBA::SystemException, CF::InvalidObjectReference)
{
    if (CORBA::is_nil (registeringDevice)) {
        LOG_WARN(DeviceManager_impl, "Attempted to register NIL device")
        throw (CF::InvalidObjectReference("[DeviceManager::registerDevice] Cannot register Device. registeringDevice is a nil reference."));
    }

    if (*_internalShutdown) // do not service a registration request if the Device Manager is shutting down
        return;
    
    LOG_INFO(DeviceManager_impl, "Registering device " << ossie::corba::returnString(registeringDevice->label()) << " on Device Manager " << _label)

    CORBA::String_var deviceLabel = registeringDevice->label();

    // This lock needs to be here because we add the device to
    // the registeredDevices list at the top...therefore
    // getting the registeredDevices attribute could
    // show the device as registered before it actually gets
    // registered.
    //
    // This lock should be after as may CORBA calls as possible
    // (e.g., registeringDevice->label()) in case omniORB blocks
    // due to a lack of threads (which would result in blocking
    // the mutex lock, which would prevent shutdown from killing
    // this).
    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);
 

    // Register the device with the Device manager, unless it is already
    // registered
    if (!deviceIsRegistered (registeringDevice)) {
        // if the device is not registered, then add it to the naming context
        LOG_TRACE(DeviceManager_impl, "Binding device to name " << deviceLabel)
        CosNaming::Name_var device_name = ossie::corba::stringToName(static_cast<char*>(deviceLabel));
        try {
            devMgrContext->bind(device_name, registeringDevice);
        } catch ( ... ) {
            // there is already something bound to that name
            // from the perspective of this framework implementation, the multiple names are not acceptable
            // consider this a registered device
            LOG_WARN(DeviceManager_impl, "Device is already registered")
            return;
        }
        increment_registeredDevices(registeringDevice);
    } else {
        LOG_WARN(DeviceManager_impl, "Device is already registered")
        return;
    }

    LOG_INFO(DeviceManager_impl, "Initializing device " << deviceLabel << " on Device Manager " << _label)
    try {
        registeringDevice->initialize();
    } catch (CF::LifeCycle::InitializeError& ex) {
        LOG_WARN(DeviceManager_impl, "Device "<< deviceLabel << " threw a CF::LifeCycle::InitializeError exception")
    } catch ( std::exception& ex ) {
        LOG_ERROR(DeviceManager_impl, "The following standard exception occurred: "<<ex.what()<<" while attempting to initialize Device " << deviceLabel)
    } catch ( const CORBA::Exception& ex ) {
        LOG_ERROR(DeviceManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" while attempting to initialize Device " << deviceLabel)
    }
    
    //Get properties from SPD
    std::string spdFile = ossie::corba::returnString(registeringDevice->softwareProfile());

    // Open the SPD file using the SCA FileSystem
    LOG_TRACE(DeviceManager_impl, "Parsing Device SPD")

    SoftPkg SPDParser;
    try {
        File_stream _spd(_fileSys, spdFile.c_str());
        SPDParser.load(_spd, spdFile.c_str());
        _spd.close();
    } catch ( ossie::parser_error& e ) {
        LOG_ERROR(DeviceManager_impl, "[DeviceManager::registerDevice] Failed to parse SPD; " << e.what())
        throw(CF::InvalidObjectReference());
    } catch ( std::exception& ex ) {
        LOG_ERROR(DeviceManager_impl, "The following standard exception occurred: "<<ex.what()<<" while attempting to parse "<<spdFile)
        throw(CF::InvalidObjectReference());
    } catch ( const CORBA::Exception& ex ) {
        LOG_ERROR(DeviceManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" while attempting to parse "<<spdFile)
        throw(CF::InvalidObjectReference());
    } catch ( ... ) {
        LOG_ERROR(DeviceManager_impl, "[DeviceManager::registerDevice] Failed to parse SPD")
        throw(CF::InvalidObjectReference());
    }

    if ( SPDParser.getPRFFile() ) {
        // TODO Handle implementation specific PRF paths
        LOG_TRACE(DeviceManager_impl, "[DeviceManager::registerDevice] opening PRF " << SPDParser.getPRFFile())


        Properties PRFparser;
        try {
            File_stream prfStream(_fileSys, SPDParser.getPRFFile());
            PRFparser.load(prfStream);
            prfStream.close();
        } catch ( ossie::parser_error& ex ) {
            LOG_ERROR(DeviceManager_impl, "[DeviceManager::registerDevice] Failed to parse PRF" << ex.what())
            throw(CF::InvalidObjectReference());
        } catch( CF::InvalidFileName& ex ) {
            LOG_ERROR(DeviceManager_impl, "[DeviceManager::registerDevice] While opening PRF " << ex.msg)
            throw(CF::InvalidObjectReference());
        } catch( CF::FileException& ex ) {
            LOG_ERROR(DeviceManager_impl, "[DeviceManager::registerDevice] While opening PRF " << ex.msg)
            throw(CF::InvalidObjectReference());
        } catch ( std::exception& ex ) {
            LOG_ERROR(DeviceManager_impl, "The following standard exception occurred: "<<ex.what()<<" while attempting to open "<<SPDParser.getPRFFile())
            throw(CF::InvalidObjectReference());
        } catch ( const CORBA::Exception& ex ) {
            LOG_ERROR(DeviceManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" while attempting to open "<<SPDParser.getPRFFile())
            throw(CF::InvalidObjectReference());
        } catch( ... ) {
            LOG_ERROR(DeviceManager_impl, "[DeviceManager::registerDevice] Unknown Exception While opening SPD")
            throw(CF::InvalidObjectReference());
        }

        // TODO fix this mess

        DeviceManagerConfiguration DCDParser;
        try {
            File_stream _dcd(_fileSys, _deviceConfigurationProfile.c_str());
            DCDParser.load(_dcd);
            _dcd.close();
        } catch ( std::exception& ex ) {
            LOG_ERROR(DeviceManager_impl, "The following standard exception occurred: "<<ex.what()<<" while attempting to parse "<<_deviceConfigurationProfile)
            throw(CF::InvalidObjectReference());
        } catch ( const CORBA::Exception& ex ) {
            LOG_ERROR(DeviceManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" while attempting to parse "<<_deviceConfigurationProfile)
            throw(CF::InvalidObjectReference());
        } catch ( ... ) {
            LOG_ERROR(DeviceManager_impl, "[DeviceManager::registerDevice] Failed to parse DCD")
            throw(CF::InvalidObjectReference());
        }

        // get properties from device PRF that matches the registering device
        std::string deviceid = ossie::corba::returnString(registeringDevice->identifier());
        try {
            const ComponentInstantiation& instantiation = DCDParser.getComponentInstantiationById(deviceid);
            if (instantiation.getUsageName() != NULL)
                std::string tmp_name = instantiation.getUsageName(); // this is here to get rid of a warning
        } catch (std::out_of_range& e) {
            LOG_ERROR(DeviceManager_impl, "[DeviceManager::registerDevice] Failed to parse DCD")
            throw(CF::InvalidObjectReference());
        } catch ( std::exception& ex ) {
            LOG_ERROR(DeviceManager_impl, "The following standard exception occurred: "<<ex.what()<<" while attempting to parse "<<_deviceConfigurationProfile)
            throw(CF::InvalidObjectReference());
        } catch ( const CORBA::Exception& ex ) {
            LOG_ERROR(DeviceManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" while attempting to parse "<<_deviceConfigurationProfile)
            throw(CF::InvalidObjectReference());
        }

        // now see if any of those properties have been overridden in the componentinstantiation
        const std::vector<const Property*>& prfSimpleProp = PRFparser.getConfigureProperties ();
        CF::Properties componentProperties;

        for (unsigned int i = 0; i < prfSimpleProp.size (); i++) {
            // Only add the property if it's not readonly
            if (std::string(prfSimpleProp[i]->getMode()) != "readonly") {
                componentProperties.length (componentProperties.length() + 1);
                componentProperties[componentProperties.length() - 1] = convertPropertyToDataType(prfSimpleProp[i]);
            }
        }

        const ComponentInstantiation& instantiation = DCDParser.getComponentInstantiationById(deviceid);
        const std::vector<ComponentProperty*>& overrideProps = instantiation.getProperties();
        if (overrideProps.size() > 0) {
            for (unsigned int i = 0; i < prfSimpleProp.size (); i++) {
                CF::DataType prop = convertPropertyToDataType(prfSimpleProp[i]);
                //LOG_TRACE(DeviceManager_impl, "Checking for override of " << prfSimpleProp[i]->toString() << " with mode: " << std::string(prfSimpleProp[i]->getMode()));
                // Check for any overrides from DCD componentproperites
                for (unsigned int j = 0; j < overrideProps.size (); j++) {
                    std::string propid = static_cast<char*>(prop.id);
                    std::string refid(overrideProps[j]->getID());
            
                    LOG_TRACE(DeviceManager_impl, "Comparing DCD component instantiation ID " << propid << " to " << refid);
                    if (refid == propid) {
                        // only store property if it has a mode of ['readwrite' or 'writeonly']
                        if (std::string(prfSimpleProp[i]->getMode()) == "readonly") {
                            LOG_WARN(DeviceManager_impl, "Ignoring DCD override of property '" << prfSimpleProp[i]->getName() << "' - '" << prfSimpleProp[i]->getID() << "' " 
                                 << "in file '" << _deviceConfigurationProfile << "'; "
                                 << "device property is readonly")
                        } else {
                            LOG_TRACE(DeviceManager_impl, "override value with " << *overrideProps[j]);
                            prop = overridePropertyValue(prfSimpleProp[i], overrideProps[j]);
                        }
                    }
                }
                // Only add the property if it's not readonly
                if (std::string(prfSimpleProp[i]->getMode()) != "readonly") {
                    for (unsigned int j=0; j<componentProperties.length(); j++) {
                        if (!strcmp(prfSimpleProp[i]->getID(),componentProperties[j].id)) {
                            componentProperties[j] = prop;
                            break;
                        }
                    }
                }
            }
        }

        //configure properties
        LOG_TRACE(DeviceManager_impl, "Configuring device")
        try {
            CF::Properties configureProperties = ossie::getNonNilConfigureProperties(componentProperties);
            registeringDevice->configure (configureProperties);
        } catch (CF::PropertySet::PartialConfiguration& ex) {
            LOG_WARN(DeviceManager_impl, "Device '" << SPDParser.getSoftPkgName() << "' - '" << SPDParser.getSoftPkgID() << "' may not have been configured correctly; "
                                      << "Call to configure() resulted in PartialConfiguration exception")
        } catch (CF::PropertySet::InvalidConfiguration& ex) {
            LOG_ERROR(DeviceManager_impl, "Device '" << SPDParser.getSoftPkgName() << "' - '" << SPDParser.getSoftPkgID() << "' may not have been configured correctly; "
                                      << "Call to configure() resulted in InvalidConfiguration exception")
        } catch ( std::exception& ex ) {
            LOG_ERROR(DeviceManager_impl, "The following standard exception occurred: "<<ex.what()<<" while attempting to configure "<<SPDParser.getSoftPkgName())
        } catch ( const CORBA::Exception& ex ) {
            LOG_ERROR(DeviceManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" while attempting to configure "<<SPDParser.getSoftPkgName())
        }
    }

    // If this Device Manager is registered with a Domain Manager, register
    // the new device with the Domain Manager
    if (_adminState == DEVMGR_REGISTERED) { 
        try {
            LOG_INFO(DeviceManager_impl, "Registering device " << deviceLabel << " on Domain Manager")
            _dmnMgr->registerDevice (registeringDevice, myObj);
        } catch( CF::DomainManager::RegisterError& e ) {
            LOG_ERROR(DeviceManager_impl, "Failed to register device to domain manager due to: " << e.msg);
        } catch ( std::exception& ex ) {
            LOG_ERROR(DeviceManager_impl, "The following standard exception occurred: "<<ex.what()<<" while attempting to register with the Domain Manager")
        } catch( const CORBA::Exception& e ) {
            LOG_ERROR(DeviceManager_impl, "Failed to register device to domain manager due to: " << e._name());
        }
    } else {
        LOG_WARN(DeviceManager_impl, "Skipping DomainManager registerDevice because the device manager isn't registered")
    }

    LOG_TRACE(DeviceManager_impl, "Done registering device " << deviceLabel)

//The registerDevice operation shall write a FAILURE_ALARM log record to a
//DomainManagers Log, upon unsuccessful registration of a Device to the DeviceManagers
//registeredDevices.
}

//This function returns TRUE if the input serviceName is contained in the _registeredServices list attribute
bool DeviceManager_impl::serviceIsRegistered (const char* serviceName)
{
    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);
    for (ServiceList::const_iterator serviceIter = _registeredServices.begin(); serviceIter != _registeredServices.end(); ++serviceIter){
        if (strcmp((*serviceIter)->label.c_str(), serviceName) == 0){
            return true;
        }
    }
    return false;
}


void
DeviceManager_impl::unregisterDevice (CF::Device_ptr registeredDevice)
throw (CORBA::SystemException, CF::InvalidObjectReference)
{
    TRACE_ENTER(DeviceManager_impl)

    std::string dev_id;
    std::string dev_name;

    bool deviceFound = false;
    if (CORBA::is_nil (registeredDevice)) {       //|| !deviceIsRegistered(registeredDevice) )
//The unregisterDevice operation shall write a FAILURE_ALARM log record, when it cannot
//successfully remove a registeredDevice from the DeviceManagers registeredDevices.

//The unregisterDevice operation shall raise the CF InvalidObjectReference when the input
//registeredDevice is a nil CORBA object reference or does not exist in the DeviceManagers
//registeredDevices attribute.
        /*writeLogRecord(FAILURE_ALARM,invalid reference input parameter.); */
        LOG_ERROR(DeviceManager_impl, "Attempt to unregister nil device")
        throw (CF::InvalidObjectReference("Cannot unregister Device. registeringDevice is a nil reference."));
    }

//The unregisterDevice operation shall remove the input registeredDevice from the
//DeviceManagers registeredDevices attribute.
    try {
        dev_id = ossie::corba::returnString(registeredDevice->identifier());
        dev_name = ossie::corba::returnString(registeredDevice->label());
    } catch ( std::exception& ex ) {
        LOG_ERROR(DeviceManager_impl, "The following standard exception occurred: "<<ex.what()<<" while trying to retrieve the identifier and label of the registered device")
        throw(CF::InvalidObjectReference());
    } catch ( const CORBA::Exception& ex ) {
        LOG_ERROR(DeviceManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" while trying to retrieve the identifier and label of the registered device")
        throw(CF::InvalidObjectReference());
    } catch ( ... ) {
        throw (CF:: InvalidObjectReference("Cannot Unregister Device. Invalid reference"));
    }

//Look for registeredDevice in _registeredDevices
    deviceFound = decrement_registeredDevices(registeredDevice);
    if (!deviceFound) {
        /*writeLogRecord(FAILURE_ALARM,invalid reference input parameter.); */
        LOG_ERROR(DeviceManager_impl, "Cannot unregister Device. registeringDevice was not registered.")
        throw (CF::InvalidObjectReference("Cannot unregister Device. registeringDevice was not registered."));
    }

    TRACE_EXIT(DeviceManager_impl);
}

void DeviceManager_impl::deleteFileSystems()
{
    PortableServer::POA_var poa = ossie::corba::RootPOA()->find_POA("DeviceManager", 0);
    PortableServer::ObjectId_var oid = poa->reference_to_id(_fileSys);
    poa->deactivate_object(oid);
    _fileSys = CF::FileSystem::_nil();
}

void
DeviceManager_impl::shutdown ()
throw (CORBA::SystemException)
{
    *_internalShutdown = true;

    if ((_adminState == DEVMGR_SHUTTING_DOWN) || (_adminState == DEVMGR_SHUTDOWN)) {
        return;
    }

    _adminState = DEVMGR_SHUTTING_DOWN;

    // SR:501
    // The shutdown operation shall unregister the DeviceManager from the DomainManager.
    // Although unclear, a failure here should NOT prevent us from trying to clean up
    // everything per SR::503
    try {
        CF::DeviceManager_var self = _this();
        _dmnMgr->unregisterDeviceManager(self);
    } catch( ... ) {
    }

    // SR:502
    //The shutdown operation shall perform releaseObject on all of the DeviceManagers registered
    //Devices (DeviceManagers registeredDevices attribute).
    // releaseObject for AggregateDevices calls releaseObject on all of their child devices
    // ergo a while loop must be used vice a for loop
    clean_registeredServices();
    clean_externalServices();
    clean_registeredDevices();

    try {
        CosNaming::Name devMgrContextName;
        devMgrContextName.length(1);
        devMgrContextName[0].id = CORBA::string_dup(_label.c_str());
        if (!CORBA::is_nil(rootContext)) {
            rootContext->unbind(devMgrContextName);
        }
    } catch ( ... ) {
    }

    try {
        deleteFileSystems();
    } catch ( ... ) {
    }

    try {
        _adminState = DEVMGR_SHUTDOWN;
    } catch ( ... ) {
    }

    // Only attempt to shut down the ORB if it is not shared with a DomainManager.
    try {
        ossie::corba::OrbShutdown(false);
    } catch ( ... ) {
    }
}

void
DeviceManager_impl::registerService (CORBA::Object_ptr registeringService,
                                     const char* name)
throw (CORBA::SystemException, CF::InvalidObjectReference)
{
    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);
    LOG_INFO(DeviceManager_impl, "Registering service " << name)

    if (CORBA::is_nil (registeringService)) {
        throw (CF::InvalidObjectReference("Cannot register service, registeringService is a nil reference."));
    }

    // Register the service with the Device manager, unless it is already
    // registered
    if (!serviceIsRegistered(name)) {
        // Per the specification, service usagenames are not optional and *MUST* be
        // unique per each service type.  Therefore, a domain cannot have two
        // services of the same usagename.
        LOG_TRACE(DeviceManager_impl, "Binding service to name " << name);
        CosNaming::Name_var service_name = ossie::corba::stringToName(name);
        try {
             rootContext->rebind(service_name, registeringService);
        } catch ( ... ) {
            // there is already something bound to that name
            // from the perspective of this framework implementation, the multiple names are not acceptable
            // consider this a registered device
            LOG_WARN(DeviceManager_impl, "Service is already registered")
            return;
        }

        increment_registeredServices(registeringService, name);

    } else {
        LOG_WARN(DeviceManager_impl, "Service is already registered")
        return;
    }

    //The registerService operation shall register the registeringService with the DomainManager
    //when the DeviceManager has already registered to the DomainManager and the
    //registeringService has been successfully added to the DeviceManagers registeredServices
    //attribute.
    if (_adminState == DEVMGR_REGISTERED) {
        try {
            _dmnMgr->registerService(registeringService, myObj, name);
        } catch ( ... ) {
            CosNaming::Name_var service_name = ossie::corba::stringToName(name);
            rootContext->unbind(service_name);
            _registeredServices.pop_back();
            LOG_ERROR(DeviceManager_impl, "Failed to register service to the domain manager; unregistering the service from the device manager")
            throw;
        }
    }

//The registerService operation shall write a FAILURE_ALARM log record, upon unsuccessful
//registration of a Service to the DeviceManagers registeredServices.
//The registerService operation shall raise the CF InvalidObjectReference exception when the
//input registeringService is a nil CORBA object reference.
}

void
DeviceManager_impl::unregisterService (CORBA::Object_ptr registeredService,
                                       const char* name)
throw (CORBA::SystemException, CF::InvalidObjectReference)
{
    LOG_INFO(DeviceManager_impl, "Unregistering service " << name)

    if (CORBA::is_nil (registeredService)) {
        /*writeLogRecord(FAILURE_ALARM,invalid reference input parameter.); */
        throw (CF::InvalidObjectReference("Cannot unregister Service. registeringService is a nil reference."));
    }

    //Look for registeredService in _registeredServices
    bool serviceFound = decrement_registeredServices(registeredService, name);
    if (serviceFound)
        return;


//If it didn't find registeredDevice, then throw an exception
    /*writeLogRecord(FAILURE_ALARM,invalid reference input parameter.);*/
    throw (CF::InvalidObjectReference("Cannot unregister Service. registeringService was not registered."));
//The unregisterService operation shall write a FAILURE_ALARM log record, when it cannot
//successfully remove a registeredService from the DeviceManagers registeredServices.
//The unregisterService operation shall raise the CF InvalidObjectReference when the input
//registeredService is a nil CORBA object reference or does not exist in the DeviceManagers
//registeredServices attribute.
}


char * DeviceManager_impl::getComponentImplementationId (const char* componentInstantiationId)
throw (CORBA::SystemException)
{
//The getComponentImplementationId operation shall return the SPD implementation elements
//ID attribute that matches the SPD implementation element used to create the component
//identified by the input componentInstantiationId parameter.

    boost::mutex::scoped_try_lock lock(componentImplMapmutex);

    std::map <std::string, std::string>::iterator map_iter;

    // make sure componentInstantiationId is in the map
    map_iter = _componentImplMap.find(componentInstantiationId);
    if (map_iter != _componentImplMap.end()) {
        // return associated SPD implementation element id
        return CORBA::string_dup(_componentImplMap[componentInstantiationId].c_str());
    } else {
        return CORBA::string_dup("");
    }

//The getComponentImplementationId operation shall return an empty string when the input
//componentInstantiationId parameter does not match the ID attribute of any SPD implementation
//element used to create the component.
}

bool DeviceManager_impl::makeDirectory(std::string path)
{
    bool done = false;

    std::string initialDir;
    if (path[0] == '/') {
        initialDir = "/";
    } else {
        initialDir = "";
    }

    std::string workingFileName;

    if (path[path.length()-1] == '/') {
        workingFileName = path;
    } else {
        workingFileName = path + "/";
    }
    std::string::size_type begin_pos = 0;
    std::string::size_type last_slash = workingFileName.find_last_of("/");
    
    bool success = true;

    if (last_slash != std::string::npos) {
        while (!done) {
            std::string::size_type pos = workingFileName.find_first_of("/", begin_pos);
            if (pos == begin_pos) { // first slash - don't do anything
                begin_pos++;
                continue;
            }
            if (pos == std::string::npos) //
                { break; }

            initialDir += workingFileName.substr(begin_pos, (pos - begin_pos)) + std::string("/");
            int retval = mkdir(initialDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (retval == -1) {
                if (errno == ENOENT) {
                    LOG_WARN(DeviceManager_impl, "Failed to create directory (from " << workingFileName << ") " << initialDir <<". Non-existent root directory.")
                    success = false;
                } else if (errno == EEXIST) {
                    LOG_TRACE(DeviceManager_impl, "Directory (from " << workingFileName << ") " << initialDir <<" already exists. No need to make a new one.")
                } else if (errno == EACCES) {
                    LOG_WARN(DeviceManager_impl, "Failed to create directory (from " << workingFileName << ") " << initialDir <<". Please check your write permissions.")
                    success = false;
                } else if (errno == ENOTDIR) {
                    LOG_WARN(DeviceManager_impl, "Failed to create directory (from " << workingFileName << ") " << initialDir <<". One of the components of the path is not a directory.")
                    success = false;
                } else if (errno == ELOOP) {
                    LOG_WARN(DeviceManager_impl, "Failed to create directory (from " << workingFileName << ") " << initialDir <<". A loop exists in the symbolic links in the path.")
                    success = false;
                } else if (errno == EMLINK) {
                    LOG_WARN(DeviceManager_impl, "Failed to create directory (from " << workingFileName << ") " << initialDir <<". The link count of the parent directory exceeds LINK_MAX.")
                    success = false;
                } else if (errno == ENAMETOOLONG) {
                    LOG_WARN(DeviceManager_impl, "Failed to create directory (from " << workingFileName << ") " << initialDir <<". The path name is too long.")
                    success = false;
                } else if (errno == EROFS) {
                    LOG_WARN(DeviceManager_impl, "Failed to create directory (from " << workingFileName << ") " << initialDir <<". This is a read-only file system.")
                    success = false;
                } else {
                    LOG_WARN(DeviceManager_impl, "Attempt to create directory (from " << workingFileName << ") " << initialDir <<" failed with the following error number: " << errno)
                    success = false;
                }
            } else {
                LOG_TRACE(DeviceManager_impl, "Creating directory (from " << workingFileName << ") " << initialDir)
            }
            begin_pos = pos + 1;
        }
    }
    bool retval = checkWriteAccess(path);
    if (not retval) {
        LOG_WARN(DeviceManager_impl, "The Device Manager (or one of its children) does not have write permission to one or more files in the cache. This may lead to unexpected behavior.")
    }
    return success;
}

bool DeviceManager_impl::checkWriteAccess(std::string &path)
{
    DIR *dp;
    struct dirent *ep;
    dp = opendir(path.c_str());
    if (dp == NULL) {
        if (errno == ENOENT) {
            LOG_WARN(DeviceManager_impl, "Failed to create directory " << path <<".")
        } else if (errno == EACCES) {
            LOG_WARN(DeviceManager_impl, "Failed to create directory " << path <<". Please check your write permissions.")
        } else if (errno == ENOTDIR) {
            LOG_WARN(DeviceManager_impl, "Failed to create directory " << path <<". One of the components of the path is not a directory.")
        } else if (errno == EMFILE) {
            LOG_WARN(DeviceManager_impl, "Failed to create directory " << path <<". Too many file descriptors open by the process.")
        } else if (errno == ENFILE) {
            LOG_WARN(DeviceManager_impl, "Failed to create directory " << path <<". Too many file descriptors open by the system.")
        } else if (errno == ENOMEM) {
            LOG_WARN(DeviceManager_impl, "Failed to create directory " << path <<". Insufficient memory to complete the operation.")
        } else {
            LOG_WARN(DeviceManager_impl, "Attempt to create directory " << path <<" failed with the following error number: " << errno)
        }
        return false;
    }
    while ((ep = readdir(dp)) != NULL) {
        std::string name = ep->d_name;
        if ((name == ".") or (name == "..")) continue;
        std::string full_name = path + "/" + name;
        if (access(full_name.c_str(), W_OK) == -1) {
            LOG_WARN(DeviceManager_impl, "The file '" << full_name << "' cannot be overwritten by the Device Manager process (or one of its children).")
            (void) closedir(dp);
            return false;
        }
        if (ep->d_type == DT_DIR) {
            bool retval = checkWriteAccess(full_name);
            if (not retval) {
                (void) closedir(dp);
                return retval;
            }
        }
    }
    (void) closedir(dp);
    return true;
}

/****************************************************************************

 The following functions manage the _registeredDevices sequence as well
  as a couple of associated data structures (the have to be synchronized)

****************************************************************************/

bool DeviceManager_impl::decrement_registeredServices(CORBA::Object_ptr registeredService, const char* name)
{
    bool serviceFound = false;

    //The unregisterService operation shall remove the input registeredService from the
    //DeviceManagers registeredServices attribute. The unregisterService operation shall unregister
    //the input registeredService from the DomainManager when the input registeredService is
    //registered with the DeviceManager and the DeviceManager is not in the shutting down state.

    // Acquire the registered device mutex so that no one else can read of modify the list.
    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);

    for (ServiceList::iterator serviceIter = _registeredServices.begin(); serviceIter != _registeredServices.end(); ++serviceIter){
        ServiceNode* serviceNode = *serviceIter;
        if (strcmp((*serviceIter)->label.c_str(), name) == 0){
            serviceFound = true;

            // Unbind service from the naming service
            std::string temp_name((*serviceIter)->label);
            // Per the specification, service usagenames are not optional and *MUST* be
            // unique per each service type.  Therefore, a domain cannot have two
            // services of the same usagename.
            CosNaming::Name_var tmpServiceName = ossie::corba::stringToName(temp_name);
            try {
                rootContext->unbind(tmpServiceName);
            } catch ( ... ){
            }

            // Ddon't unregisterService from the domain manager if we are SHUTTING_DOWN
            if (_adminState == DEVMGR_REGISTERED){
                try {
                    _dmnMgr->unregisterService(registeredService, name);
                } catch ( ... ) {}
            }
            
            // Remove the service from the list of registered services
            _registeredServices.erase(serviceIter);

            std::string label = serviceNode->label;
            if (serviceNode->pid != 0){
                // The service process has not terminated, so add it back to the pending list.
                _pendingServices.push_back(serviceNode);
            }

            break;
        }
    }


    return serviceFound;
}

bool DeviceManager_impl::decrement_registeredDevices(CF::Device_ptr registeredDevice)
{
    bool deviceFound = false;
    const std::string deviceIOR = ossie::corba::objectToString(registeredDevice);

    // Acquire the registered device mutex so that no one else can read or modify the list.
    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);
    for (DeviceList::iterator deviceIter = _registeredDevices.begin(); 
         deviceIter != _registeredDevices.end(); 
         ++deviceIter) {
        DeviceNode* deviceNode = *deviceIter;
        if (deviceNode->IOR == deviceIOR) {
            deviceFound = true;
            
            // Remove device from the list of registered devices.
            _registeredDevices.erase(deviceIter);

            std::string label = deviceNode->label;
            if (deviceNode->pid != 0) {
                // The device process has not terminated, so add it back to the pending list.
                _pendingDevices.push_back(deviceNode);
            }

            // Release the registered device mutex, now that we are done modifying the list. If we unregister
            // the device from the DomainManager, it will call 'registeredDevices()', which requires the mutex.
            lock.unlock();


            // Unbind device from the naming service
            CosNaming::Name_var tmpDeviceName = ossie::corba::stringToName(label);
            devMgrContext->unbind(tmpDeviceName);

            // Per SR:490, don't unregisterDevice from the domain manager if we are SHUTTING_DOWN
            if (_adminState == DEVMGR_REGISTERED) {
                try {
                    _dmnMgr->unregisterDevice(registeredDevice);
                } catch( ... ) {
                }
            }

            break;
        }
    }
    
    return deviceFound;
}

/*
 * increment the registered services sequences along with the id and table tables
 */
void DeviceManager_impl::increment_registeredServices(CORBA::Object_ptr registeringService, const char* name)
{
    // Find the device in the pending list. If we launched the device process, it should be found here.
    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);

    ServiceNode* serviceNode = 0;
    for (ServiceList::iterator serviceIter = _pendingServices.begin(); serviceIter != _pendingServices.end(); ++serviceIter) {
        if (strcmp((*serviceIter)->label.c_str(), name) == 0){
            serviceNode = *serviceIter;
            _pendingServices.erase(serviceIter);
            break;
        }
    }

    if (!serviceNode){
        // A service is registering that was not launched by this DeviceManager. Create a node
        // to manage it, but mark the PID as 0, as there is no process to monitor.
        LOG_WARN(DeviceManager_impl, "Registering service " << name << " was not launched by this DeviceManager");
        serviceNode = new ServiceNode;
        serviceNode->identifier = name;
        serviceNode->pid = 0;
    }

    //The registerService operation shall add the input registeringService to the DeviceManagers
    //registeredServices attribute when the input registeringService does not already exist in the
    //registeredServices attribute. The registeringService is ignored when duplicated.
    serviceNode->label = name;
    serviceNode->IOR = ossie::corba::objectToString(registeringService);
    serviceNode->service = CORBA::Object::_duplicate(registeringService);

    _registeredServices.push_back(serviceNode);
}

/*
* increment the registered devices sequences along with the id and label tables
*/
void DeviceManager_impl::increment_registeredDevices(CF::Device_ptr registeringDevice)
{
    const std::string identifier = ossie::corba::returnString(registeringDevice->identifier());

    // Find the device in the pending list. If we launched the device process, it should be found here.
    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);
    DeviceNode* deviceNode = 0;
    for (DeviceList::iterator deviceIter = _pendingDevices.begin(); 
         deviceIter != _pendingDevices.end(); 
         ++deviceIter) {

        if ((*deviceIter)->identifier == identifier) {
            deviceNode = *deviceIter;
            _pendingDevices.erase(deviceIter);
            break;
        }
    }

    if (!deviceNode) {
        // A device is registering that was not launched by this DeviceManager. Create a node
        // to manage it, but mark the PID as 0, as there is no process to monitor.
        LOG_WARN(DeviceManager_impl, "Registering device " << identifier << " was not launched by this DeviceManager");
        deviceNode = new DeviceNode;
        deviceNode->identifier = identifier;
        deviceNode->pid = 0;
    }

    // Fill in the device node fields that were not known at launch time (label has probably
    // not changed, but we consider the device authoritative).
    deviceNode->label = ossie::corba::returnString(registeringDevice->label());
    deviceNode->IOR = ossie::corba::objectToString(registeringDevice);
    deviceNode->device = CF::Device::_duplicate(registeringDevice);

    _registeredDevices.push_back(deviceNode);
}

/*
* This function returns TRUE if the input registeredDevice is contained in the _registeredDevices list attribute
*/
bool DeviceManager_impl::deviceIsRegistered (CF::Device_ptr registeredDevice)
{
    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);
    for (DeviceList::const_iterator deviceIter = _registeredDevices.begin(); 
         deviceIter != _registeredDevices.end(); 
         ++deviceIter) {

        if ((*deviceIter)->device->_is_equivalent(registeredDevice)) {
            return true;
        }
    }
    return false;
}

CF::DeviceSequence* DeviceManager_impl::registeredDevices () throw (CORBA::SystemException)
{
    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);
    CF::DeviceSequence_var result;
    try {
        result = new CF::DeviceSequence();
        result->length(_registeredDevices.size());
        for (CORBA::ULong ii = 0; ii < _registeredDevices.size(); ++ii) {
            result[ii] = CF::Device::_duplicate(_registeredDevices[ii]->device);
        }
    } catch ( ... ) {
        result = new CF::DeviceSequence();
    }
    return result._retn();
}

std::string DeviceManager_impl::getIORfromID(const char* instanceid)
{
    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);

    for (DeviceList::const_iterator deviceIter = _registeredDevices.begin(); 
         deviceIter != _registeredDevices.end(); 
         ++deviceIter) {

        if ((*deviceIter)->identifier == instanceid) {
            return (*deviceIter)->IOR;
        }
    }
    return std::string();
}


/* Removes any services that were registered from an external source  */
void DeviceManager_impl::clean_externalServices(){
    ServiceNode* serviceNode = 0;
    {
        boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);

        for (ServiceList::iterator serviceIter = _registeredServices.begin(); serviceIter != _registeredServices.end(); ++serviceIter) {
            if ((*serviceIter)->pid == 0) {
                serviceNode = (*serviceIter);
                break;
            }
        }
    }

    if (serviceNode){
        decrement_registeredServices(serviceNode->service, serviceNode->label.c_str());
    }
}

void DeviceManager_impl::clean_registeredServices(){

    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);
    std::vector<unsigned int> pids;

    for (ServiceList::iterator serviceIter = _registeredServices.begin(); serviceIter != _registeredServices.end(); ++serviceIter) {
        pids.push_back((*serviceIter)->pid);
    }
    for (ServiceList::iterator serviceIter = _pendingServices.begin(); serviceIter != _pendingServices.end(); ++serviceIter) {
        pids.push_back((*serviceIter)->pid);
    }

    // Clean up service processes.
    for (ServiceList::iterator serviceIter = _pendingServices.begin(); serviceIter != _pendingServices.end(); ++serviceIter) {
        pid_t servicePid = (*serviceIter)->pid;

        // Try an orderly shutdown.
        // NOTE: If the DeviceManager was terminated with a ^C, sending this signal may cause the
        //       original SIGINT to be forwarded to all other children (which is harmless, but be aware).
        kill(servicePid, SIGTERM);
    }

    // Send a SIGTERM to any services that haven't yet unregistered
    for (ServiceList::iterator serviceIter = _registeredServices.begin(); serviceIter != _registeredServices.end(); ++serviceIter){
        pid_t servicePid = (*serviceIter)->pid;
        // Only kill services that were launched by this device manager
        if (servicePid != 0){
            kill(servicePid, SIGTERM);
        }
    }

    lock.unlock();

    // Release the lock and allow time for the devices to exit.
    if (pids.size() != 0) {
        struct timeval tmp_time;
        struct timezone tmp_tz;
        gettimeofday(&tmp_time, &tmp_tz);
        double wsec_begin = tmp_time.tv_sec;
        double fsec_begin = tmp_time.tv_usec / 1e6;
        double wsec_end = wsec_begin;
        double fsec_end = fsec_begin;
        double time_diff = (wsec_end + fsec_end)-(wsec_begin + fsec_begin);
        bool registered_pending_pid_gone = false;
        while ((time_diff < 0.5) and (not registered_pending_pid_gone)) {
            registered_pending_pid_gone = true;
            for (std::vector<unsigned int>::iterator p_pid = pids.begin(); p_pid != pids.end(); ++p_pid) {
                if (kill(*p_pid, 0) != -1) {
                    registered_pending_pid_gone = false;
                    break;
                }
            }
            if (not registered_pending_pid_gone) {
                gettimeofday(&tmp_time, &tmp_tz);
                wsec_end = tmp_time.tv_sec;
                fsec_end = tmp_time.tv_usec / 1e6;
                time_diff = (wsec_end + fsec_end)-(wsec_begin + fsec_begin);
                usleep(1000);
            }
        }
    }
    lock.lock();

    // Send a SIGKILL to any remaining services.
    for (ServiceList::iterator serviceIter = _pendingServices.begin(); serviceIter != _pendingServices.end(); ++serviceIter) {
        pid_t servicePid = (*serviceIter)->pid;
        kill(servicePid, SIGKILL);
    }

    // Send a SIGKILL to any services that haven't yet unregistered
    for (ServiceList::iterator serviceIter = _registeredServices.begin(); serviceIter != _registeredServices.end(); ++serviceIter){
        pid_t servicePid = (*serviceIter)->pid;
        // Only kill services that were launched by this device manager
        if (servicePid != 0){
            kill(servicePid, SIGKILL);
        }
    }
}

void DeviceManager_impl::clean_registeredDevices()
{
    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);
    while (!_registeredDevices.empty()) {
        DeviceNode* deviceNode = _registeredDevices[0];
        std::string label = deviceNode->label;

        try {
            lock.unlock();
            unsigned long timeout = 3; // seconds
            omniORB::setClientCallTimeout(deviceNode->device, timeout * 1000);
            deviceNode->device->releaseObject();
            lock.lock();
        } catch (const CORBA::Exception& ex) {
            lock.lock();
            if (_registeredDevices[0] == deviceNode) {
                if (_registeredDevices.size() != 0) { // this check is here just in case the error happens after the base class releaseObject finishes
                    _registeredDevices.erase(_registeredDevices.begin());
                }
                // this is here in the unlikely event that the releaseObject succeeded only partially
                //  and the device managed to unregister itself from the DeviceManager
                bool foundPid = false;
                for (unsigned int i=0; i<_pendingDevices.size();i++) {
                    if (_pendingDevices[i]->pid == deviceNode->pid)
                        foundPid = true;
                }
                if (not foundPid)
                    _pendingDevices.push_back(deviceNode);
            }
        } catch ( std::exception& ex ) {
            lock.lock();
            if (_registeredDevices[0] == deviceNode) {
                if (_registeredDevices.size() != 0) { // this check is here just in case the error happens after the base class releaseObject finishes
                    _registeredDevices.erase(_registeredDevices.begin());
                }
                // this is here in the unlikely event that the releaseObject succeeded only partially
                //  and the device managed to unregister itself from the DeviceManager
                bool foundPid = false;
                for (unsigned int i=0; i<_pendingDevices.size();i++) {
                    if (_pendingDevices[i]->pid == deviceNode->pid)
                        foundPid = true;
                }
                if (not foundPid)
                    _pendingDevices.push_back(deviceNode);
            }
        }
    }

    // Clean up device processes.
    for (DeviceList::iterator deviceIter = _pendingDevices.begin(); deviceIter != _pendingDevices.end(); ++deviceIter) {
        pid_t devicePid = (*deviceIter)->pid;

        // Try an orderly shutdown.
        // NOTE: If the DeviceManager was terminated with a ^C, sending this signal may cause the
        //       original SIGINT to be forwarded to all other children (which is harmless, but be aware).
        kill(devicePid, SIGTERM);
    }

    lock.unlock();

    // Release the lock and allow time for the devices to exit.
    bool registered_pending_list_empty = _pendingDevices.empty() and
            _registeredDevices.empty();
    if (not registered_pending_list_empty) {
        struct timeval tmp_time;
        struct timezone tmp_tz;
        gettimeofday(&tmp_time, &tmp_tz);
        double wsec_begin = tmp_time.tv_sec;
        double fsec_begin = tmp_time.tv_usec / 1e6;
        double wsec_end = wsec_begin;
        double fsec_end = fsec_begin;
        double time_diff = (wsec_end + fsec_end)-(wsec_begin + fsec_begin);
        while ((time_diff < 0.5) and (not registered_pending_list_empty)) {
            registered_pending_list_empty = _pendingDevices.empty() and
                    _registeredDevices.empty();
            gettimeofday(&tmp_time, &tmp_tz);
            wsec_end = tmp_time.tv_sec;
            fsec_end = tmp_time.tv_usec / 1e6;
            time_diff = (wsec_end + fsec_end)-(wsec_begin + fsec_begin);
            usleep(1000);
        }
    }

    lock.lock();
    killPendingDevices();
}

/**
 * Set pDeviceNode if the pid was found in either the _pendingDevices or
 * in the _registeredDevices.
 */
void DeviceManager_impl::getDeviceNode(DeviceNode** pDeviceNode, const pid_t pid) {
    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);

    // Try to find a device that has already unregistered or has not yet registered.
    for (DeviceList::iterator deviceIter = _pendingDevices.begin(); 
         deviceIter != _pendingDevices.end(); 
         ++deviceIter) {
        if ((*deviceIter)->pid == pid) {
            *pDeviceNode = (*deviceIter);
            _pendingDevices.erase(deviceIter);
            break;
        }
    }

    // If there was not an unregistered device, check if a registered device terminated early.
    if (!*pDeviceNode) {
        for (DeviceList::iterator deviceIter = _registeredDevices.begin(); 
             deviceIter != _registeredDevices.end(); 
             ++deviceIter) {
            if ((*deviceIter)->pid == pid) {
                *pDeviceNode = (*deviceIter);
                (*pDeviceNode)->pid = 0;
                decrement_registeredDevices((*pDeviceNode)->device);
                break;
            }
        }
    }
}

void DeviceManager_impl::childExited (pid_t pid, int status)
{
    DeviceNode* deviceNode = 0;
    getDeviceNode(&deviceNode, pid);

    ServiceNode* serviceNode = 0;
    {
        boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);

        // Try to find a service that has already unregistered
        for (ServiceList::iterator serviceIter = _pendingServices.begin(); serviceIter != _pendingServices.end(); ++serviceIter){
            if ((*serviceIter)->pid == pid){
                serviceNode = (*serviceIter);
                _pendingServices.erase(serviceIter);
                break;
            }
        }

        // If there was not an unregistered device, check if a registered device terminated early.
        if (!serviceNode) {
            for (ServiceList::iterator serviceIter = _registeredServices.begin(); serviceIter != _registeredServices.end(); ++serviceIter) {
                if ((*serviceIter)->pid == pid) {
                    serviceNode = (*serviceIter);
                    // Flag the pid as 0 so that it can be unregistered below.
                    serviceNode->pid = 0;
                    break;
                }
            }
        }
    }

    // The pid should always be found; if it is not, it must be a logic error.
    if (!deviceNode && !serviceNode) {
        return;
    }

    std::string label;
    if (deviceNode){
        label = deviceNode->label;
    } else {
        label = serviceNode->label;
    }

    // If the device terminated unexpectedly, unregister it.
    if (deviceNode && deviceNode->pid == 0) {
        decrement_registeredDevices(deviceNode->device);
    } else if (serviceNode && serviceNode->pid == 0){
        decrement_registeredServices(serviceNode->service, serviceNode->label.c_str());
    }

    if (deviceNode) {
        delete deviceNode;
    } else {
        delete serviceNode;
    }
}

bool DeviceManager_impl::allChildrenExited ()
{
    boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);

    if ((_pendingDevices.size() == 0) && (_registeredDevices.size() == 0)) {
        return true;
    }

    return false;
}
