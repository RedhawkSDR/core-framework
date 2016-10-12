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

#include <ossie/Value.h>
#include <ossie/PropertyMap.h>
#include <ossie/prop_helpers.h>

using namespace redhawk;

Value::Value() :
    CORBA::Any()
{
}

Value::Value(const CORBA::Any& any) :
    CORBA::Any(any)
{
}

Value::Value(const Value& any) :
    CORBA::Any(any)
{
}

Value& Value::operator=(const CORBA::Any& any)
{
    CORBA::Any::operator=(any);
    return *this;
}

Value& Value::operator=(const Value& any)
{
    return operator=(static_cast<const CORBA::Any&>(any));
}

std::string Value::toString() const
{
    return ossie::any_to_string(*this);
}

bool Value::toBoolean() const
{
    return ossie::any::toBoolean(*this);
}

float Value::toFloat() const
{
    return ossie::any::toFloat(*this);
}

double Value::toDouble() const
{
    return ossie::any::toDouble(*this);
}

CORBA::Octet Value::toOctet() const
{
    return ossie::any::toOctet(*this);
}

CORBA::Short Value::toShort() const
{
    return ossie::any::toShort(*this);
}

CORBA::UShort Value::toUShort() const
{
    return ossie::any::toUShort(*this);
}

CORBA::Long Value::toLong() const
{
    return ossie::any::toLong(*this);
}

CORBA::ULong Value::toULong() const
{
    return ossie::any::toULong(*this);
}

CORBA::LongLong Value::toLongLong() const
{
    return ossie::any::toLongLong(*this);
}

CORBA::ULongLong Value::toULongLong() const
{
    return ossie::any::toULongLong(*this);
}

bool Value::isNil() const
{
    return ossie::any::isNull(*this);
}

PropertyMap& Value::asProperties()
{
    CF::Properties* props = 0;
    (*this) >>= props;
    return PropertyMap::cast(*props);
}

const PropertyMap& Value::asProperties() const
{
    const CF::Properties* props = 0;
    (*this) >>= props;
    return PropertyMap::cast(*props);
}

ValueSequence& Value::asSequence()
{
    CORBA::AnySeq* prop_seq;
    (*this) >>= prop_seq;
    return ValueSequence::cast(*prop_seq);
}

const ValueSequence& Value::asSequence() const
{
    CORBA::AnySeq* prop_seq;
    (*this) >>= prop_seq;
    return ValueSequence::cast(*prop_seq);
}


ValueSequence::ValueSequence() :
    CORBA::AnySeq()
{
}

ValueSequence::ValueSequence(const CORBA::AnySeq& sequence) :
    CORBA::AnySeq(sequence)
{
}

ValueSequence::ValueSequence(const ValueSequence& sequence) :
    CORBA::AnySeq(sequence)
{
}

bool ValueSequence::empty() const
{
    return length() == 0;
}

size_t ValueSequence::size() const
{
    return length();
}

Value& ValueSequence::operator[] (size_t index)
{
    return *(begin() + index);
}

const Value& ValueSequence::operator[] (size_t index) const
{
    return *(begin() + index);
}

void ValueSequence::push_back(const Value& value)
{
    ossie::corba::push_back(*this, value);
}

void ValueSequence::push_back(const CORBA::Any& value)
{
    ossie::corba::push_back(*this, value);
}

ValueSequence::iterator ValueSequence::begin()
{
    return static_cast<iterator>(this->get_buffer());
}

ValueSequence::iterator ValueSequence::end()
{
    return begin() + size();
}

ValueSequence::const_iterator ValueSequence::begin() const
{
    return static_cast<const_iterator>(this->get_buffer());
}

ValueSequence::const_iterator ValueSequence::end() const
{
    return begin() + size();
}
