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

#ifndef APPLICATIONCOMPONENT_H
#define APPLICATIONCOMPONENT_H

#include <string>
#include <vector>

#include <ossie/CF/cf.h>

namespace redhawk {
    struct ApplicationComponent {
        ApplicationComponent(const std::string& identifier);

        const std::string& getIdentifier() const;

        unsigned long getProcessId() const;
        void setProcessId(unsigned long processId);

        std::string softwareProfile;
        std::string namingContext;
        std::string implementationId;
        std::vector<std::string> loadedFiles;
        CORBA::Object_var componentObject;
        CF::Device_var assignedDevice;
        bool isContainer;

    private:
        std::string _identifier;
        unsigned long _processId;
    };
}

#endif // APPLICATIONCOMPONENT_H
