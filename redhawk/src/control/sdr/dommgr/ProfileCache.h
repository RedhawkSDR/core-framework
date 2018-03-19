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

#ifndef PROFILECACHE_H
#define PROFILECACHE_H

#include <string>
#include <vector>

#include <boost/ptr_container/ptr_vector.hpp>

#include <ossie/CF/cf.h>
#include <ossie/debug.h>
#include <ossie/SoftPkg.h>

namespace redhawk {

    /**
     * @brief  An exception raised when a SoftPkg profile is invalid
     */
    class invalid_profile : public std::runtime_error
    {
    public:
        invalid_profile(const std::string& filename, const std::string& message) :
            std::runtime_error(message),
            filename(filename)
        {
        }

        virtual ~invalid_profile() throw()
        {
            // Only defined because the compiler-generated destructor has a
            // looser throw specification than runtime_error's destructor
        }

        /**
         * @brief  Returns the filename of the invalid SoftPkg profile
         */
        const std::string& get_filename() const
        {
            return filename;
        }

    private:
        const std::string filename;
    };

    /**
     * @brief  Caching softpkg profile loader
     */
    class ProfileCache
    {
        ENABLE_LOGGING;

    public:
        /**
         * @brief  Creates a new cache
         * @param fileSystem  the CF::FileSystem used to load files
         *
         * Creates a new empty cache. When this cache is destroyed, all loaded
         * profiles are deleted.
         */
        ProfileCache(CF::FileSystem_ptr fileSystem, rh_logger::LoggerPtr log);

        /**
         * @brief  Loads an SPD file and its PRF and SCD, if available
         * @param spdFilename  the path to the SPD file
         * @return  a pointer to the loaded SoftPkg
         * @exception redhawk::invalid_profile  a file is invalid or cannot be
         *            parsed
         *
         * Reads and parses the SPD file @a spdFilename and its referenced PRF
         * and SCD files (if any), caching the result. Subsequent calls with
         * the same filename will return the cached object.
         *
         * The returned SoftPkg is owned by this object, not the caller.
         */
        const ossie::SoftPkg* loadProfile(const std::string& spdFilename);

        /**
         * @brief  Loads an SPD file
         * @param spdFilename  the path to the SPD file
         * @return  a pointer to the loaded SoftPkg
         * @exception redhawk::invalid_profile  file is invalid or cannot be
         *            parsed
         *
         * Reads and parses the SPD file @a spdFilename and its referenced PRF
         * and SCD files (if any), caching the result. Subsequent calls with
         * the same filename will return the cached object.

         * The returned SoftPkg is owned by this object, not the caller.
         */
        const ossie::SoftPkg* loadSoftPkg(const std::string& filename);

    protected:
        ossie::SoftPkg* findSoftPkg(const std::string& filename);

        std::string _extractVersion(const std::string& filename);

        CF::FileSystem_var fileSystem;
        boost::ptr_vector<ossie::SoftPkg> profiles;

        rh_logger::LoggerPtr _profilecache_log;
    };
}

#endif // PROFILECACHE_H
