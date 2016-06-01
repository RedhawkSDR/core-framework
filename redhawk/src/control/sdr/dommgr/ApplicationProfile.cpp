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

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include <ossie/FileStream.h>
#include <ossie/ComponentDescriptor.h>
#include <ossie/Properties.h>

#include "ApplicationProfile.h"

using namespace ossie;

////////////////////////////////////////////////////
/*
 * ApplicationProfile member function definitions
 */
PREPARE_LOGGING(ApplicationProfile);

ApplicationProfile::ApplicationProfile()
{
}

ApplicationProfile::~ApplicationProfile()
{
}

const std::string& ApplicationProfile::getIdentifier() const
{
    return identifier;
}

void ApplicationProfile::load(CF::FileSystem_ptr fileSystem, const SoftwareAssembly& sad)
{
    identifier = sad.getID();

    // Walk through the host collocations first
    BOOST_FOREACH(const SoftwareAssembly::HostCollocation& collocation, sad.getHostCollocations()) {
        LOG_TRACE(ApplicationProfile, "Building component info for host collocation "
                  << collocation.getID());

        BOOST_FOREACH(const ComponentPlacement& placement, collocation.getComponents()) {
            assert(placement.componentFile);
            loadProfile(fileSystem, placement.componentFile->getFileName());
        }
    }

    // Then, walk through the remaining non-collocated components
    BOOST_FOREACH(const ComponentPlacement& placement, sad.getComponentPlacements()) {
        assert(placement.componentFile);
        loadProfile(fileSystem, placement.componentFile->getFileName());
    }
}

const SoftPkg* ApplicationProfile::getSoftPkg(const std::string& filename) const
{
    BOOST_FOREACH(const SoftPkg& softpkg, profiles) {
        if (softpkg.getSPDFile() == filename) {
            return &softpkg;
        }
    }

    throw std::logic_error(filename + " was never loaded");
}

const SoftPkg* ApplicationProfile::loadProfile(CF::FileSystem_ptr fileSystem,
                                               const std::string& filename)
{
    BOOST_FOREACH(const SoftPkg& profile, profiles) {
        if (profile.getSPDFile() == filename) {
            LOG_TRACE(ApplicationProfile, "Found existing profile " << filename);
            return &profile;
        }
    }

    LOG_TRACE(ApplicationProfile, "Loading SPD file " << filename);
    File_stream spd_stream(fileSystem, filename.c_str());
    SoftPkg* spd = new SoftPkg(spd_stream, filename);
    profiles.push_back(spd);
    spd_stream.close();

    const SPD::Implementations& spd_impls = spd->getImplementations();
    for (SPD::Implementations::const_iterator impl = spd_impls.begin(); impl != spd_impls.end(); ++impl) {
        const SPD::SoftPkgDependencies& deps = impl->getSoftPkgDependencies();
        for (SPD::SoftPkgDependencies::const_iterator dep = deps.begin(); dep != deps.end(); ++dep) {
            SPD::SoftPkgRef& ref = const_cast<SPD::SoftPkgRef&>(*dep);
            LOG_TRACE(ApplicationProfile, "Resolving soft package reference " << ref.localfile);
            loadProfile(fileSystem, ref.localfile);
        }
    }

    if (spd->getPRFFile()) {
        LOG_TRACE(ApplicationProfile, "Loading PRF file " << spd->getPRFFile());
        try {
            spd->setProperties(boost::make_shared<Properties>());
            File_stream prf_stream(fileSystem, spd->getPRFFile());
            spd->getProperties()->load(prf_stream);
        } catch (const std::exception& exc) {
            LOG_ERROR(ApplicationProfile, "Invalid PRF file " << spd->getPRFFile() << ": " << exc.what());
        }
    }

    if (spd->getSCDFile()) {
        LOG_TRACE(ApplicationProfile, "Loading SCD file " << spd->getSCDFile());
        try {
            spd->setDescriptor(boost::make_shared<ComponentDescriptor>());
            File_stream scd_stream(fileSystem, spd->getSCDFile());
            spd->getDescriptor()->load(scd_stream);
        } catch (const std::exception& exc) {
            LOG_ERROR(ApplicationProfile, "Invalid SCD file " << spd->getSCDFile() << ": " << exc.what());
        }
    }

    return spd;
}
