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


#ifndef APPLICATIONSUPPORT_H
#define APPLICATIONSUPPORT_H

#include <string>
#include <vector>
#include <list>

#include <ossie/CF/cf.h>
#include <ossie/SoftPkg.h>
#include <ossie/SoftwareAssembly.h>
#include <ossie/ComponentDescriptor.h>
#include <ossie/Properties.h>
#include <ossie/ossieparser.h>
#include <ossie/componentProfile.h>

// Application support routines

// Base class to contain data for the components required to
// create an application
// Application support routines

namespace ossie
{
    struct ApplicationComponent {
        std::string identifier;
        std::string softwareProfile;
        std::string namingContext;
        std::string implementationId;
        std::vector<std::string> loadedFiles;
        unsigned long processId;
        CORBA::Object_var componentObject;
        CF::Device_var assignedDevice;
    };
    typedef std::list<ApplicationComponent> ComponentList;

    /* Base class to contain data for components
     *  - Used to store information about about components
     */
    class ComponentInfo
    {
        ENABLE_LOGGING

    public:
      typedef ossie::ComponentInstantiation::AffinityProperties AffinityProperties;

        ComponentInfo (const SoftPkg* softpkg, const ComponentInstantiation* instantiation);
        ~ComponentInfo ();

        const SoftPkg* spd;

        const ComponentInstantiation* getInstantiation() const;

        void setAffinity( const AffinityProperties &affinity );

        CF::Properties getAffinityOptions() const;

        static ComponentInfo* buildComponentInfoFromSPDFile(const SoftPkg* softpkg,
                                                            const ComponentInstantiation* instantiation);

    private:
        ComponentInfo (const ComponentInfo&);

        ossie::Properties _affinity_prf;

        CF::Properties affinityOptions;

        const ComponentInstantiation* instantiation;
    };

}
#endif
