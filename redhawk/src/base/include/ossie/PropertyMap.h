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

#ifndef REDHAWK_PROPERTYMAP_H
#define REDHAWK_PROPERTYMAP_H

#include <ossie/CF/cf.h>

#include "Value.h"
#include "PropertyType.h"

namespace redhawk {

    class PropertyMap : public CF::Properties {
    public:
        typedef redhawk::PropertyType* iterator;
        typedef const redhawk::PropertyType* const_iterator;

        static inline PropertyMap& cast(_CORBA_Unbounded_Sequence<CF::DataType>& properties)
        {
            return static_cast<PropertyMap&>(properties);
        }

        static inline const PropertyMap& cast(const _CORBA_Unbounded_Sequence<CF::DataType>& properties)
        {
            return static_cast<const PropertyMap&>(properties);
        }

        PropertyMap();
        explicit PropertyMap(const CF::Properties& properties);

        PropertyMap& operator=(const CF::Properties& properties);

        bool contains (const std::string& id) const;

        bool empty () const;

        size_t size() const;

        PropertyType& operator[] (size_t index);
        const PropertyType& operator[] (size_t index) const;

        Value& operator[] (const std::string& id);
        const Value& operator[] (const std::string& id) const;

        void push_back (const CF::DataType& dt);

        iterator begin();
        iterator end();

        const_iterator begin() const;
        const_iterator end() const;

        iterator find(const std::string& id);
        const_iterator find(const std::string& id) const;

        void erase(const std::string& id);
        void erase(iterator pos);
        void erase(iterator first, iterator last);
    };

}

#endif // REDHAWK_PROPERTYMAP_H
