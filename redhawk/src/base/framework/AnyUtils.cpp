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
#include <boost/lexical_cast.hpp>

#include <ossie/AnyUtils.h>
#include <ossie/CorbaUtils.h>

namespace {
    template <typename T>
    inline T extractAny (const CORBA::Any& any)
    {
        T value = 0;
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

    template <typename Tin, typename Tout>
    inline void extractAndConvert (const CORBA::Any& any, Tout& out)
    {
        typedef boost::numeric::converter<Tout,Tin> Converter; 
        Tin value = extractAny<Tin>(any);
        if (Converter::out_of_range(value)) {
            throw std::range_error("value out of range");
        }
        out = Converter::convert(value);
    }

    template <typename Tin>
    inline void extractAndConvert (const CORBA::Any& any, bool& out)
    {
        Tin value = extractAny<Tin>(any);
        out = value;
    }

    template <typename T>
    inline T stringToNumber (const std::string& str)
    {
        if (str.find_first_not_of("-0123456789") != std::string::npos) {
            // String has non-decimal digit, non-sign characters, so try going
            // through double in case it's a decimal point or exponent
            double temp = boost::lexical_cast<double>(str);
            typedef boost::numeric::converter<T,double> Converter;
            if (Converter::out_of_range(temp)) {
                throw std::range_error("value out of range");
            }
            return Converter::convert(temp);
        } else {
            // invalid cast to wrong type will throw bad_lexical_cast
            return boost::lexical_cast<T>(str);
        }
    }

    // Specialization for CORBA::Octet (unsigned char), always converting via
    // double because lexical_cast throws a bad_lexical_cast exception with
    // CORBA::Octet
    template<>
    inline CORBA::Octet stringToNumber (const std::string& str)
    {
        double temp = boost::lexical_cast<double>(str);
        typedef boost::numeric::converter<CORBA::Octet,double> Converter;
        if (Converter::out_of_range(temp)) {
            throw std::range_error("value out of range");
        }
        return Converter::convert(temp);
    }

    template <typename T>
    inline bool anyToNumber (const CORBA::Any& any, T& value)
    {
        CORBA::TypeCode_var anyType = any.type();
        switch (anyType->kind()) {
        case CORBA::tk_boolean:
            extractAndConvert<bool>(any, value);
            break;
        case CORBA::tk_octet:
            extractAndConvert<CORBA::Octet>(any, value);
            break;
        case CORBA::tk_short:
            extractAndConvert<CORBA::Short>(any, value);
            break;
        case CORBA::tk_ushort:
            extractAndConvert<CORBA::UShort>(any, value);
            break;
        case CORBA::tk_long:
            extractAndConvert<CORBA::Long>(any, value);
            break;
        case CORBA::tk_ulong:
            extractAndConvert<CORBA::ULong>(any, value);
            break;
        case CORBA::tk_longlong:
            extractAndConvert<CORBA::LongLong>(any, value);
            break;
        case CORBA::tk_ulonglong:
            extractAndConvert<CORBA::ULongLong>(any, value);
            break;
        case CORBA::tk_float:
            extractAndConvert<CORBA::Float>(any, value);
            break;
        case CORBA::tk_double:
            extractAndConvert<CORBA::Double>(any, value);
            break;
        case CORBA::tk_string:
            {
                std::string str;
                any >>= str;
                value = stringToNumber<T>(str);
            }
            break;
        default:
            return false;
        }
        return true;
    }
}

#define ANY_TO_NUMERIC_TYPE(T,N)                                    \
bool ossie::any::toNumber (const CORBA::Any& any, T& value)         \
{                                                                   \
    return anyToNumber(any, value);                                 \
}                                                                   \
T ossie::any::to##N (const CORBA::Any& any)                         \
{                                                                   \
    T value;                                                        \
    if (ossie::any::toNumber(any, value)) {                         \
        return value;                                               \
    } else {                                                        \
        throw std::invalid_argument("Non-numeric Any type");        \
    }                                                               \
}

#define ANY_TO_NUMERIC(N) ANY_TO_NUMERIC_TYPE(CORBA::N, N)

ANY_TO_NUMERIC_TYPE(bool, Boolean);
ANY_TO_NUMERIC(Octet);
ANY_TO_NUMERIC(Short);
ANY_TO_NUMERIC(UShort);
ANY_TO_NUMERIC(Long);
ANY_TO_NUMERIC(ULong);
ANY_TO_NUMERIC(LongLong);
ANY_TO_NUMERIC(ULongLong);
ANY_TO_NUMERIC(Float);
ANY_TO_NUMERIC(Double);
