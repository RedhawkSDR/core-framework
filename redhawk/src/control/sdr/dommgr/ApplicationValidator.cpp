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

#include <set>

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#include <ossie/Properties.h>
#include <ossie/PropertyMap.h>
#include <ossie/prop_utils.h>
#include "ApplicationValidator.h"

using namespace redhawk;
using namespace ossie;
namespace fs = boost::filesystem;

class bad_implementation : public redhawk::validation_error
{
public:
    bad_implementation(const SoftPkg* softpkg, const SPD::Implementation& impl, const std::string& message) :
        redhawk::validation_error("Soft package " + softpkg->getSPDFile() + " has invalid implementation " + impl.getID() + ": " + message)
    {
    }
};

class no_valid_implemenation : public redhawk::validation_error
{
public:
    no_valid_implemenation(const SoftPkg* softpkg) :
        redhawk::validation_error("Soft package " + softpkg->getSPDFile() + " has no valid implementations")
    {
    }
};

PREPARE_CF_LOGGING(ApplicationValidator);

ApplicationValidator::ApplicationValidator(CF::FileSystem_ptr fileSystem, rh_logger::LoggerPtr log) :
    fileSystem(CF::FileSystem::_duplicate(fileSystem)),
    cache(fileSystem, log),
    _appFactoryLog(log)
{
}

void ApplicationValidator::validate(const SoftwareAssembly& sad)
{
    // Check partitioning
    BOOST_FOREACH(const SoftwareAssembly::HostCollocation& collocation, sad.getHostCollocations()) {
        validateHostCollocation(collocation);
    }
    BOOST_FOREACH(const ComponentPlacement& placement, sad.getComponentPlacements()) {
        validateComponentPlacement(placement);
    }

    // Check externally-promoted ports and properties
    validateExternalPorts(sad.getExternalPorts());

    const Properties* ac_props = getAssemblyControllerProperties(sad);
    validateExternalProperties(ac_props, sad.getExternalProperties());
}

void ApplicationValidator::validateExternalPorts(const std::vector<SoftwareAssembly::Port>& ports)
{
    // Make sure all external port names are unique
    // NB: Should check component references are valid as well
    std::set<std::string> port_names;
    BOOST_FOREACH(const SoftwareAssembly::Port& port, ports) {
        const std::string& name = port.getExternalName();
        if (port_names.count(name) == 0) {
            port_names.insert(name);
        } else {
            throw validation_error("Duplicate external port name " + name);
        }
    }
}

void ApplicationValidator::validateExternalProperties(const Properties* acProperties,
                                                      const std::vector<SoftwareAssembly::Property>& properties)
{
    // Makes sure all external property names are unique
    // NB: Should check component references are valid as well
    std::set<std::string> property_names;
    BOOST_FOREACH(const SoftwareAssembly::Property& property, properties) {
        const std::string& name = property.getExternalID();
        if (property_names.count(name) == 0) {
            property_names.insert(name);
        } else {
            throw validation_error("Duplicate external property name " + name);
        }
    }

    // Make sure AC property IDs aren't in conflict with external ones
    if (acProperties) {
        BOOST_FOREACH(const Property* property, acProperties->getProperties()) {
            const std::string& name = property->getID();
            if (property_names.count(name) == 0) {
                property_names.insert(name);
            } else {
                throw validation_error("Assembly controller property " + name + " in use as external property");
            }
        }
    }
}

void ApplicationValidator::validateSoftPkgRef(const SPD::SoftPkgRef& softpkgref)
{
    const std::string& spd_file = softpkgref.localfile;
    if (spd_file.empty()) {
        throw validation_error("empty softpkgref");
    }

    // Basic checking for valid, existing filename
    RH_TRACE(_appFactoryLog, "Validating SPD " << spd_file);
    if (!fileExists(spd_file)) {
        throw validation_error("softpkgref " + spd_file + " does not exist");
    }
    if (!endsWith(spd_file, ".spd.xml")) {
        RH_WARN(_appFactoryLog, "SPD file " << spd_file << " should end with .spd.xml");
    }

    // If this fails, it will throw redhawk::invalid_profile
    const SoftPkg* softpkg = cache.loadSoftPkg(spd_file);

    // Check implementation(s)
    if (softpkgref.implref.isSet()) {
        // A specific implementation is given; make sure it exists and is valid
        const std::string& impl_id = *(softpkgref.implref);
        const SPD::Implementation* implementation = softpkg->getImplementation(impl_id);
        if (!implementation) {
            throw validation_error("softpkgref " + spd_file + " has no implementation " + impl_id);
        }

        validateImplementation(softpkg, *implementation, false);
    } else {
        // Validate all implementations
        int valid_implementations = 0;
        BOOST_FOREACH(const SPD::Implementation& implementation, softpkg->getImplementations()) {
            try {
                validateImplementation(softpkg, implementation, false);
                valid_implementations++;
            } catch (const validation_error& err) {
                RH_WARN(_appFactoryLog, err.what());
            }
        }
        if (valid_implementations == 0) {
            throw no_valid_implemenation(softpkg);
        }
    }
}

void ApplicationValidator::validateImplementation(const SoftPkg* softpkg,
                                                  const SPD::Implementation& implementation,
                                                  bool executable)
{
    RH_TRACE(_appFactoryLog, "Validating SPD implementation " << implementation.getID());

    // Always ensure that the localfile exists
    std::string localfile = _relativePath(softpkg, implementation.getCodeFile());
    RH_TRACE(_appFactoryLog, "Validating code localfile " << localfile);
    if (!fileExists(localfile)) {
        throw bad_implementation(softpkg, implementation, "missing localfile " + localfile);
    }

    // If the implementation needs to be executable (i.e., would be used for a
    // component instantiation), make sure it has a valid entry point
    if (executable) {
        if (!implementation.getEntryPoint()) {
            throw bad_implementation(softpkg, implementation, "has no entry point");
        }
        std::string entry_point = _relativePath(softpkg, implementation.getEntryPoint());
        RH_TRACE(_appFactoryLog, "Validating code entry point " << entry_point);
        if (!fileExists(entry_point)) {
            throw bad_implementation(softpkg, implementation, "missing entrypoint " + entry_point);
        }
    }

    // Check all softpkg references
    BOOST_FOREACH(const SPD::SoftPkgRef& spdref, implementation.getSoftPkgDependencies()) {
        try {
            validateSoftPkgRef(spdref);
        } catch (const validation_error& exc) {
            // Turn the exception into a more detailed bad_implementation that
            // includes enough context to debug the XML
            throw bad_implementation(softpkg, implementation, exc.what());
        }
    }
}

void ApplicationValidator::validateHostCollocation(const SoftwareAssembly::HostCollocation& collocation)
{
    std::pair< std::string, redhawk::PropertyMap > devReq(std::string(""),redhawk::PropertyMap());
    BOOST_FOREACH(const ComponentPlacement& placement, collocation.getComponents()) {
        validateComponentPlacement(placement);

        // check if placement has deviceRequires, if so there can only be one or all must match
        BOOST_FOREACH(const ComponentInstantiation& instantiation, placement.getInstantiations()) {
            redhawk::PropertyMap deviceRequires;
            ossie::convertComponentProperties(instantiation.getDeviceRequires(),deviceRequires);
            if (!deviceRequires.empty() ) {
                if ( !devReq.first.empty() ) {
                    if ( devReq.first == instantiation.getID() ) {
                        throw validation_error("hostcollocation contains multiple devicerequires, componentinstantiation: " +instantiation.getID() );
                    }

                    if ( devReq.second != deviceRequires ) {
                        throw validation_error("hostcollocation contains multiple devicerequires that are different, componentinstantiation: " + instantiation.getID() );
                    }
                }
                else {
                    devReq.first = instantiation.getID();
                    devReq.second = deviceRequires;
                    RH_TRACE(_appFactoryLog, "devicerequires collocation: " << collocation.getName() << " instantiation :" << devReq.first << " devicerequires: " << devReq.second);
                }
            }
        }
    }
}

void ApplicationValidator::validateComponentPlacement(const ComponentPlacement& placement)
{
    const std::string& spd_file = placement.filename;
    if (spd_file.empty()) {
        throw validation_error("componentfile " + placement._componentFileRef + " filename is empty");
    }

    // Basic checking for valid, existing filename
    RH_TRACE(_appFactoryLog, "Validating SPD " << spd_file);
    if (!fileExists(spd_file)) {
        throw validation_error("componentfile " + placement._componentFileRef + " points to non-existent file " + spd_file);
    }
    if (!endsWith(spd_file, ".spd.xml")) {
        RH_WARN(_appFactoryLog, "SPD file " << spd_file << " should end with .spd.xml");
    }

    // If this fails, it will throw redhawk::invalid_profile
    const SoftPkg* softpkg = cache.loadProfile(spd_file);

    // Check the PRF and SCD filenames
    if (softpkg->getPRFFile()) {
        std::string prf_file = softpkg->getPRFFile();
        if (!endsWith(prf_file, ".prf.xml")) {
            RH_WARN(_appFactoryLog, "PRF file " << prf_file << " should end with .prf.xml");
        }
    }
    if (softpkg->getSCDFile()) {
        std::string scd_file = softpkg->getSCDFile();
        if (!endsWith(scd_file, ".scd.xml")) {
            RH_WARN(_appFactoryLog, "SCD file " << scd_file << " should end with .scd.xml");
        }
    }

    int valid_implementations = 0;
    BOOST_FOREACH(const SPD::Implementation& implementation, softpkg->getImplementations()) {
        try {
            validateImplementation(softpkg, implementation, true);
            valid_implementations++;
        } catch (const validation_error& err) {
            RH_WARN(_appFactoryLog, err.what());
        }
    }
    if (valid_implementations == 0) {
        throw no_valid_implemenation(softpkg);
    }

    // If the softpkg is SCA-compliant, make sure it has a descriptor
    if (softpkg->isScaCompliant() && !softpkg->getDescriptor()) {
        std::string message = spd_file + " is SCA-compliant but does not have an SCD";
        throw validation_error(message);
    }
}

const Properties* ApplicationValidator::getAssemblyControllerProperties(const SoftwareAssembly& sad)
{
    // Search through all placements for the instantiation that is the assembly
    // controller, then get the SoftPkg (which has already been loaded) and
    // return its Properties
    BOOST_FOREACH(const ComponentPlacement& placement, sad.getAllComponents()) {
        BOOST_FOREACH(const ComponentInstantiation& instantiation, placement.getInstantiations()) {
            if (instantiation.getID() == sad.getAssemblyControllerRefId()) {
                const SoftPkg* softpkg = cache.loadProfile(placement.filename);
                return softpkg->getProperties();
            }
        }
    }

    return 0;
}

bool ApplicationValidator::fileExists(const std::string& filename)
{
    RH_TRACE(_appFactoryLog, "Checking existence of file '" << filename << "'");
    try {
        return fileSystem->exists(filename.c_str());
    } catch (...) {
        // Turn all exceptions into negative result; in this context, at least,
        // CORBA errors mean the same thing--the file is not usable
        return false;
    }
}

bool ApplicationValidator::endsWith(const std::string& filename, const std::string& suffix)
{
    if (filename.size() < suffix.size()) {
        return false;
    }
    // Compare the end of the filename to the entirety of the suffix
    std::string::size_type start = filename.size() - suffix.size();
    return filename.compare(start, suffix.size(), suffix) == 0;
}

std::string ApplicationValidator::_relativePath(const SoftPkg* softpkg, const std::string& path)
{
    if (path.find('/') == 0) {
        return path;
    } else {
        fs::path abspath = fs::path(softpkg->getSPDPath()) / path;
        return abspath.string();
    }
}
