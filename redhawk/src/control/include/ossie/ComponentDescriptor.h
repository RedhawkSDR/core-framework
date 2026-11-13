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


#ifndef SCDPARSER_H
#define SCDPARSER_H
#include <string>
#include <istream>
#include <boost/shared_ptr.hpp>

#include "ossie/CF/cf.h"
#include "ossie/debug.h"
#include "ossie/ossieparser.h"
#include "ossie/exceptions.h"

namespace ossie {
    class ComponentDescriptor
    {
    // Internal classes
    public:
        class SCD {
        public:
            std::string componentType;
        };

    public:
        ComponentDescriptor() {}

        ComponentDescriptor(std::istream& input);

        ~ComponentDescriptor();

        ComponentDescriptor( const ComponentDescriptor &src);

        ComponentDescriptor& operator=(const ComponentDescriptor &other);

        void load(std::istream& istream);

        const char* getComponentType() const;

        bool isDevice() const; 

        bool isResource() const;

        bool isApplication() const;

        bool isDomainManager() const;

        bool isDeviceManager() const;

        bool isService() const;

        bool isConfigurable() const;

    protected:
        boost::shared_ptr<SCD> _scd; // the parsed data
    };
}
#endif
