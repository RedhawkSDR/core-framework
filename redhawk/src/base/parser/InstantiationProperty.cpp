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


#include <iostream>
#include <string>

#include "ossie/InstantiationProperty.h"

InstantiationProperty::InstantiationProperty(const char* _id, const char* _value): id(_id)
{

    values.push_back(_value);
}

InstantiationProperty::InstantiationProperty(const char* _id): id(_id)
{

}

InstantiationProperty::~InstantiationProperty()
{
}

void InstantiationProperty::setID(const char* _id)
{
    id = _id;
}

void InstantiationProperty::setValue(const char* _value)
{
    values.push_back( _value);
}

const char* InstantiationProperty::getID()
{
    return id.c_str();
}


const std::vector <std::string> InstantiationProperty::getValues()
{
    return values;
}
