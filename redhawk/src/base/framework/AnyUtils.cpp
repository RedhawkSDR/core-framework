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

#include <stdexcept>

#include <boost/numeric/conversion/converter.hpp>

#include <ossie/AnyUtils.h>

namespace {
    template <typename T>
    inline T extractAny (const CORBA::Any& any)
    {
        T value;
        any >>= value;
        return value;
    }

    template <>
    inline CORBA::Octet extractAny (const CORBA::Any& any)
    {
        CORBA::Octet value;
        any >>= CORBA::Any::to_octet(value);
        return value;
    }

    template <typename Tout, typename Tin>
    inline Tout extractAndConvert (const CORBA::Any& any)
    {
        typedef boost::numeric::converter<Tout,Tin> Converter; 
        Tin value = extractAny<Tin>(any);
        if (Converter::out_of_range(value)) {
            throw std::range_error("value out of range");
        }
        return Converter::convert(value);
    }

    template <typename T>
    inline bool anyToNumber (const CORBA::Any& any, T& value)
    {
        CORBA::TypeCode_var anyType = any.type();
        switch (anyType->kind()) {
        case CORBA::tk_octet:
            value = extractAndConvert<T, CORBA::Octet>(any);
            break;
        case CORBA::tk_short:
            value = extractAndConvert<T, CORBA::Short>(any);
            break;
        case CORBA::tk_ushort:
            value = extractAndConvert<T, CORBA::UShort>(any);
            break;
        case CORBA::tk_long:
            value = extractAndConvert<T, CORBA::Long>(any);
            break;
        case CORBA::tk_ulong:
            value = extractAndConvert<T, CORBA::ULong>(any);
            break;
        case CORBA::tk_longlong:
            value = extractAndConvert<T, CORBA::LongLong>(any);
            break;
        case CORBA::tk_ulonglong:
            value = extractAndConvert<T, CORBA::ULongLong>(any);
            break;
        case CORBA::tk_float:
            value = extractAndConvert<T, CORBA::Float>(any);
            break;
        case CORBA::tk_double:
            value = extractAndConvert<T, CORBA::Double>(any);
            break;
        default:
            return false;
        }
        return true;
    }
}

#define ANY_TO_NUMERIC(N)                                           \
bool ossie::any::toNumber (const CORBA::Any& any, CORBA::N& value)  \
{                                                                   \
    return anyToNumber(any, value);                                 \
}                                                                   \
CORBA::N ossie::any::to##N (const CORBA::Any& any)                  \
{                                                                   \
    CORBA::N value;                                                 \
    if (ossie::any::toNumber(any, value)) {                         \
        return value;                                               \
    } else {                                                        \
        throw std::invalid_argument("Non-numeric Any type");        \
    }                                                               \
}

ANY_TO_NUMERIC(Octet);
ANY_TO_NUMERIC(Short);
ANY_TO_NUMERIC(UShort);
ANY_TO_NUMERIC(Long);
ANY_TO_NUMERIC(ULong);
ANY_TO_NUMERIC(LongLong);
ANY_TO_NUMERIC(ULongLong);
ANY_TO_NUMERIC(Float);
ANY_TO_NUMERIC(Double);
