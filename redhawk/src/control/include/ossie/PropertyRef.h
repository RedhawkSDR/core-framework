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

#ifndef __PROPERTYREF_H__
#define __PROPERTYREF_H__

#include <boost/shared_ptr.hpp>

namespace ossie {

    class ComponentProperty;
    
    class DependencyRef {
    public:
        std::string type;

        virtual const std::string asString() const = 0;

        virtual ~DependencyRef() {};
    };

    class PropertyRef : public DependencyRef
    {
    public:
        PropertyRef() :
            property()
        {
        }

        PropertyRef(ComponentProperty* prop) :
            property(prop)
        {
        }

        PropertyRef(const ComponentProperty &prop) :
            property(prop.clone())
        {
        }
        
        PropertyRef(const PropertyRef& other)
        {
            if (other.property) {
                property.reset(other.property->clone());
            }
        }

        boost::shared_ptr<ossie::ComponentProperty> property;

        virtual const std::string asString() const;
        virtual ~PropertyRef();
    };

    template< typename charT, typename Traits>
    std::basic_ostream<charT, Traits>& operator<<(std::basic_ostream<charT, Traits> &out, const DependencyRef& ref)
    {
        out << ref.asString();
        return out;
    }

}

#endif
