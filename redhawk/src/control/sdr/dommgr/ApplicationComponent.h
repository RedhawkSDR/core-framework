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

        const std::string& getSoftwareProfile() const;
        void setSoftwareProfile(const std::string& softwareProfile);

        bool hasNamingContext() const;
        const std::string& getNamingContext() const;
        void setNamingContext(const std::string& namingContext);

        const std::string& getImplementationId() const;
        void setImplementationId(const std::string& implementationId);

        unsigned long getProcessId() const;
        void setProcessId(unsigned long processId);

        bool isTerminated() const;
        bool isRegistered() const;

        CORBA::Object_ptr getComponentObject() const;
        void setComponentObject(CORBA::Object_ptr object);

        CF::Resource_ptr getResourcePtr() const;

        CF::Device_ptr getAssignedDevice() const;
        void setAssignedDevice(CF::Device_ptr assignedDevice);

        std::vector<std::string> loadedFiles;
        bool isContainer;

        void releaseObject();
        void terminate();
        void unloadFiles();

    private:
        std::string _identifier;
        std::string _softwareProfile;
        std::string _namingContext;
        std::string _implementationId;
        unsigned long _processId;
        CORBA::Object_var _componentObject;
        CF::Device_var _assignedDevice;
    };
}

#endif // APPLICATIONCOMPONENT_H
