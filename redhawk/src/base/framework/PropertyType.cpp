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

#include <ossie/PropertyType.h>

using namespace redhawk;

PropertyType::PropertyType() :
    CF::DataType()
{
}

PropertyType::PropertyType(const CF::DataType& dt) :
    CF::DataType(dt)
{
}

PropertyType& PropertyType::operator=(const CF::DataType& dt)
{
    CF::DataType::operator=(dt);
    return *this;
}

void PropertyType::setId(const std::string& identifier)
{
    id = identifier.c_str();
}

const std::string PropertyType::getId() const
{
    return std::string(id);
}

Value& PropertyType::getValue()
{
    return Value::cast(value);
}

const Value& PropertyType::getValue() const
{
    return Value::cast(value);
}
