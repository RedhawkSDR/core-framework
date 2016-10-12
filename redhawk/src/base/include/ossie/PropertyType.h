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

#ifndef REDHAWK_PROPERTYTYPE_H
#define REDHAWK_PROPERTYTYPE_H

#include "Value.h"

namespace redhawk {

    class PropertyType : public CF::DataType {
    public:

        static inline PropertyType& cast(CF::DataType& dt)
        {
            return static_cast<PropertyType&>(dt);
        }

        static inline const PropertyType& cast(const CF::DataType& dt)
        {
            return static_cast<const PropertyType&>(dt);
        }

        PropertyType();
        explicit PropertyType(const CF::DataType& dt);

        PropertyType& operator=(const CF::DataType& dt);

        void setId(const std::string& identifier);
        const std::string getId() const;
        
        Value& getValue();
        const Value& getValue() const;

        template <typename T>
        void setValue(const T& value)
        {
            getValue() = value;
        }
    };

}

#endif // REDHAWK_PROPERTYTYPE_H
