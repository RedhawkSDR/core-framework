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


#ifndef APPLICATIONPROFILE_H
#define APPLICATIONPROFILE_H

#include <string>
#include <vector>

#include <boost/ptr_container/ptr_vector.hpp>

#include <ossie/debug.h>
#include <ossie/SoftPkg.h>
#include <ossie/SoftwareAssembly.h>

namespace ossie {

    /* Base class to contain data for applications
     *  - Used to store information about about:
     *       -> ExternalPorts
     *       -> External Properties
     *       -> UsesDevice relationships
     */
    class ApplicationProfile
    {
        ENABLE_LOGGING;

    public:
        ApplicationProfile();
        ~ApplicationProfile();

        const std::string& getIdentifier() const;

        void load(CF::FileSystem_ptr fileSystem, const SoftwareAssembly& sad);

        const SoftPkg* getSoftPkg(const std::string& filename) const;

    protected:
        typedef boost::ptr_vector<SoftPkg> ProfileList;

        const SoftPkg* loadProfile(CF::FileSystem_ptr fileSystem, const std::string& filename);

        std::string identifier;
        ProfileList profiles;
    };

}

#endif // APPLICATIONPROFILE_H
