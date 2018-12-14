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
#include <ossie/Versions.h>

#include "ProfileCache.h"

using namespace redhawk;
using namespace ossie;

PREPARE_CF_LOGGING(ProfileCache);

namespace {
    static std::string getVersionMismatchMessage(const std::string& version)
    {
        if (redhawk::compareVersions(VERSION, version) > 0) {
            return " (attempting to load a profile from version " + version + " on REDHAWK version " VERSION ")";
        } else {
            return std::string();
        }
    }
}

ProfileCache::ProfileCache(CF::FileSystem_ptr fileSystem, rh_logger::LoggerPtr log) :
    fileSystem(CF::FileSystem::_duplicate(fileSystem)),
    _profilecache_log(log)
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
        RH_TRACE(_profilecache_log, "Loading PRF file " << prf_file);
        try {
            File_stream prf_stream(fileSystem, prf_file.c_str());
            softpkg->loadProperties(prf_stream);
        } catch (const std::exception& exc) {
            std::string message = spdFilename + " has invalid PRF file " + prf_file + ": " + exc.what();
            message += ::getVersionMismatchMessage(softpkg->getSoftPkgType());
            throw invalid_profile(spdFilename, message);
        }
    }

    // If the SPD has an SCD reference, and it hasn't already been loaded, try
    // to load it
    if (softpkg->getSCDFile() && !softpkg->getDescriptor()) {
        const std::string scd_file = softpkg->getSCDFile();
        RH_TRACE(_profilecache_log, "Loading SCD file " << scd_file);
        try {
            File_stream scd_stream(fileSystem, scd_file.c_str());
            softpkg->loadDescriptor(scd_stream);
        } catch (const std::exception& exc) {
            std::string message = spdFilename + " has invalid SCD file " + scd_file + ": " + exc.what();
            message += ::getVersionMismatchMessage(softpkg->getSoftPkgType());
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
            RH_TRACE(_profilecache_log, "Found existing SPD " << filename);
            return &softpkg;
        }
    }

    RH_TRACE(_profilecache_log, "Loading SPD file " << filename);
    SoftPkg* softpkg = 0;
    try {
        File_stream spd_stream(fileSystem, filename.c_str());
        softpkg = new SoftPkg(spd_stream, filename);
    } catch (const std::exception& exc) {
        std::string message = filename + " is invalid: " + exc.what();
        std::string softpkg_version = _extractVersion(filename);
        if (!softpkg_version.empty()) {
            message += ::getVersionMismatchMessage(softpkg_version);
        }
        throw invalid_profile(filename, message);
    }

    profiles.push_back(softpkg);
    return softpkg;
}

std::string ProfileCache::_extractVersion(const std::string& filename)
{
    // When the SPD itself cannot be parsed, try to recover the type attribute
    // from the <softpkg> element manually. If the SPD is from a newer version
    // of REDHAWK that has extended the XSD, this allows for a more helpful
    // error message.
    try {
        File_stream stream(fileSystem, filename.c_str());
        std::string line;
        while (std::getline(stream, line)) {
            std::string::size_type type_idx = line.find("type");
            if (type_idx != std::string::npos) {
                std::string::size_type first_quote = line.find('"', type_idx);
                if (first_quote != std::string::npos) {
                    size_t second_quote = line.find('"', first_quote + 1);
                    if (second_quote != std::string::npos) {
                        return line.substr(first_quote + 1, second_quote-(first_quote+1));
                    }
                }
            }
        }
    } catch (...) {
        // Ignore all errors
    }
    return std::string();
}
