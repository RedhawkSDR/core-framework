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


#include <sstream>
#include <vector>
#include <stdlib.h>
#include <omniORB4/CORBA.h>

#include <ossie/debug.h>
#include <ossie/SoftPkg.h>
#include <ossie/Properties.h>
#include <ossie/FileStream.h>
#include <ossie/ComponentDescriptor.h>
#include <ossie/DeviceManagerConfiguration.h>
#include <ossie/prop_utils.h>
#include <ossie/CorbaUtils.h>
#include <ossie/affinity.h>
#include "applicationSupport.h"
#include "PersistenceStore.h"
#include "ossie/PropertyMap.h"


using namespace ossie;

static void addProperty(const CF::DataType& dt, CF::Properties& prop)
{
    for (unsigned int ii = 0; ii < prop.length(); ++ii) {
        if (strcmp(dt.id, prop[ii].id) == 0) {
            // Overwrite existing value
            prop[ii].value = dt.value;
            return;
        }
    }

    // New id, add property at end
    unsigned int index = prop.length();
    prop.length(index + 1);
    prop[index] = dt;
}


////////////////////////////////////////////////////
/*
 * ComponentInfo member function definitions
 */
PREPARE_CF_LOGGING(ComponentInfo);

ComponentInfo* ComponentInfo::buildComponentInfoFromSPDFile(const SoftPkg* softpkg,
                                                            const ComponentInstantiation* instantiation)
{
    LOG_TRACE(ComponentInfo, "Building component info from softpkg " << softpkg->getName());

    ossie::ComponentInfo* newComponent = new ossie::ComponentInfo(softpkg, instantiation);
    LOG_TRACE(ComponentInfo, "Done building component info from soft package " << softpkg->getName());
    return newComponent;
}

ComponentInfo::ComponentInfo(const SoftPkg* softpkg, const ComponentInstantiation* instantiation) :
    spd(softpkg),
    instantiation(instantiation)
{
}

ComponentInfo::~ComponentInfo ()
{
}

const ComponentInstantiation* ComponentInfo::getInstantiation() const
{
    return instantiation;
}
