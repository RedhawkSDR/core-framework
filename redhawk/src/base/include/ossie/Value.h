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

#ifndef REDHAWK_VALUE_H
#define REDHAWK_VALUE_H

#include <ossie/CorbaUtils.h>
#include <ossie/AnyUtils.h>

namespace redhawk {

    class PropertyMap;
    class ValueSequence;

    class Value : public CORBA::Any {
    public:

        Value();
        explicit Value(const CORBA::Any& any);
        Value(const Value& any);

        template <typename T>
        explicit Value(const T& value)
        {
            setValue(value);
        }

        static inline Value& cast(CORBA::Any& dt)
        {
            return static_cast<Value&>(dt);
        }

        static inline const Value& cast(const CORBA::Any& dt)
        {
            return static_cast<const Value&>(dt);
        }

        Value& operator=(const CORBA::Any& any);
        Value& operator=(const Value& any);

        template <typename T>
        Value& operator=(const T& value)
        {
            setValue(value);
            return *this;
        }

        std::string toString() const;
        bool toBoolean() const;
        float toFloat() const;
        double toDouble() const;
        CORBA::Octet toOctet() const;
        CORBA::Short toShort() const;
        CORBA::UShort toUShort() const;
        CORBA::Long toLong() const;
        CORBA::ULong toULong() const;
        CORBA::LongLong toLongLong() const;
        CORBA::ULongLong toULongLong() const;
        bool isNil() const;
        
        redhawk::PropertyMap& asProperties();
        const redhawk::PropertyMap& asProperties() const;

        redhawk::ValueSequence& asSequence();
        const redhawk::ValueSequence& asSequence() const;

        template <typename T>
        bool getValue(T& value) const
        {
            return (*this)>>=value;
        }

        template <typename T>
        void setValue(const T& value)
        {
            (*this)<<=value;
        }
    };


    class ValueSequence : public CORBA::AnySeq {
    public:
        typedef Value* iterator;
        typedef const Value* const_iterator;

        static inline ValueSequence& cast(CORBA::AnySeq& properties)
        {
            return static_cast<ValueSequence&>(properties);
        }

        static inline const ValueSequence& cast(const CORBA::AnySeq& properties)
        {
            return static_cast<const ValueSequence&>(properties);
        }

        ValueSequence();
        ValueSequence(const CORBA::AnySeq& sequence);
        ValueSequence(const ValueSequence& sequence);

        bool empty() const;

        size_t size() const;

        Value& operator[] (size_t index);
        const Value& operator[] (size_t index) const;

        void push_back(const CORBA::Any& value);
        void push_back(const Value& value);

        template <class T>
        void push_back(const T& value)
        {
            push_back(Value(value));
        }

        iterator begin();
        iterator end();

        const_iterator begin() const;
        const_iterator end() const;
    };
}

#endif // REDHAWK_VALUE_H
