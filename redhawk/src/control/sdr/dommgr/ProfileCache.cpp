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

#include <ossie/FileStream.h>
#include <ossie/SoftPkg.h>

#include "ProfileCache.h"

using namespace redhawk;
using namespace ossie;

PREPARE_LOGGING(ProfileCache);

ProfileCache::ProfileCache(CF::FileSystem_ptr fileSystem) :
    fileSystem(CF::FileSystem::_duplicate(fileSystem))
{
}

const SoftPkg* ProfileCache::loadProfile(const std::string& spdFilename)
{
    // Use basic load to find or load the SPD, then cast the const away so that
    // the PRF and SCD can be loaded if needed
    SoftPkg* softpkg = const_cast<SoftPkg*>(loadSoftPkg(spdFilename));

    // If the SPD has a PRF reference, and it hasn't already been loaded, try
    // to load it
    if (softpkg->getPRFFile() && !softpkg->getProperties()) {
        const std::string prf_file = softpkg->getPRFFile();
        LOG_TRACE(ProfileCache, "Loading PRF file " << prf_file);
        try {
            File_stream prf_stream(fileSystem, prf_file.c_str());
            softpkg->loadProperties(prf_stream);
        } catch (const std::exception& exc) {
            std::string message = spdFilename + " has invalid PRF file " + prf_file + ": " + exc.what();
            throw invalid_profile(spdFilename, message);
        }
    }

    // If the SPD has an SCD reference, and it hasn't already been loaded, try
    // to load it
    if (softpkg->getSCDFile() && !softpkg->getDescriptor()) {
        const std::string scd_file = softpkg->getSCDFile();
        LOG_TRACE(ProfileCache, "Loading SCD file " << scd_file);
        try {
            File_stream scd_stream(fileSystem, scd_file.c_str());
            softpkg->loadDescriptor(scd_stream);
        } catch (const std::exception& exc) {
            std::string message = spdFilename + " has invalid SCD file " + scd_file + ": " + exc.what();
            throw invalid_profile(spdFilename, message);
        }
    }

    return softpkg;
}

const SoftPkg* ProfileCache::loadSoftPkg(const std::string& filename)
{
    // Check the cache first
    BOOST_FOREACH(SoftPkg& softpkg, profiles) {
        if (softpkg.getSPDFile() == filename) {
            LOG_TRACE(ProfileCache, "Found existing SPD " << filename);
            return &softpkg;
        }
    }

    LOG_TRACE(ProfileCache, "Loading SPD file " << filename);
    try {
        File_stream spd_stream(fileSystem, filename.c_str());
        SoftPkg* softpkg = new SoftPkg(spd_stream, filename);
        profiles.push_back(softpkg);
        return softpkg;
    } catch (const std::exception& exc) {
        std::string message = filename + " is invalid: " + exc.what();
        throw invalid_profile(filename, message);
    }
}
