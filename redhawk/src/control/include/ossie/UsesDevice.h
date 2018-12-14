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

#ifndef __USESDEVICE_H__
#define __USESDEVICE_H__

#include <string>
#include <vector>

#include "PropertyRef.h"

namespace ossie {

    class UsesDeviceRef {
    public:
        std::string id;

        const std::string& getID() const {
            return id;
        }

    };

    inline std::ostream& operator<<(std::ostream& out, const UsesDeviceRef& usesdev)
    {
        out << "UsesDeviceRef id: " << usesdev.id;
        return out;
    }


    class UsesDevice {
    public:
        std::string id;
        std::string type;
        std::vector<PropertyRef> dependencies;
        
        const std::string& getID() const {
            return id;
        }

        const std::string& getType() const {
            return type;
        }
            
        const std::vector<PropertyRef>& getDependencies() const {
            return dependencies;
        }
    };

    inline std::ostream& operator<<(std::ostream& out, const UsesDevice& usesdev)
    {
        out << "Uses Device id: " << usesdev.id << " type: " << usesdev.type;
        return out;
    }
}

#endif
