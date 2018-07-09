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


#ifndef PROPHELPERS_H
#define PROPHELPERS_H

#include <string>
#include <vector>

#if HAVE_OMNIORB4
#include <omniORB4/CORBA.h>
#endif

#include "CF/cf.h"
#include <stdexcept>
#include <sstream>

namespace redhawk {
  namespace time {
    namespace utils {
        CF::UTCTime create( const double wholeSecs=-1.0, const double fractionalSecs=-1.0 );
        
        // year:month:day::hour:minute:second
        CF::UTCTime convert( const std::string formatted );

        /*
         * Create a time stamp object from the current time of day reported by the system
         */
        CF::UTCTime now();
      
        /*
         * Create a time stamp object from the current time of day reported by the system
         */
        CF::UTCTime notSet();

        /*
         * Adjust the whole and fractional portions of a time stamp object to
         * ensure there is no fraction in the whole seconds, and vice-versa
         */
        void normalize(CF::UTCTime& time);
    }
  }
}

namespace ossie
{
    template<class T>
    bool perform_action(T val1, T val2, std::string& action)
    {
        if (action == "eq") {
            return (val1 == val2);
        } else if (action == "ne") {
            return (val1 != val2);
        } else if (action == "gt") {
            return (val1 > val2);
        } else if (action == "lt") {
            return (val1 < val2);
        } else if (action == "ge") {
            return (val1 >= val2);
        } else if (action == "le") {
            return (val1 <= val2);
        } else {
            return false;
        }
    }

    class badConversion : public std::runtime_error {
    public:
        badConversion(std::string value, std::string type) : std::runtime_error("Unable to perform conversion"), _value(value), _type(type) {};
        ~badConversion() throw() {};
        virtual const char* what() const throw()
        {
            std::ostringstream _msg;
            _msg << std::runtime_error::what() << ": '"<<_value<<"' to type '"<<_type << "'";
            return _msg.str().c_str();
        };
    private:
        std::string _value, _type;
    };

    template<class T>
    double perform_math(double operand, T propval, std::string& math)
    {
        if (math == "*") {
            return static_cast<double>(operand * static_cast<double>(propval));
        } else if (math == "-") {
            return static_cast<double>(operand - static_cast<double>(propval));
        } else if (math == "+") {
            return static_cast<double>(operand + static_cast<double>(propval));
        } else if (math == "/") {
            return static_cast<double>(operand / static_cast<double>(propval));
        } else if (math == "=") {
            return propval;
        } else {
            return propval;
        }
    }

    bool compare_anys(const CORBA::Any& a, const CORBA::Any& b, std::string& action);
    CORBA::Any calculateDynamicProp(double operand, CORBA::Any& prop, std::string& math, CORBA::TCKind resultKind);
    CORBA::Any string_to_any(std::string value, CORBA::TypeCode_ptr type);
    CORBA::Any stringToSimpleAny(std::string value, CORBA::TCKind kind);
    CORBA::Any stringToComplexAny(std::string value, std::string structName);

    template <typename Type, class CFComplexType>
        CORBA::Any convertComplexStringToAny(std::string value);
    template <class CFComplexType>
        std::string convertComplexAnyToString(const CORBA::Any&);

    std::string complexAnyToString(const CORBA::Any& value);
    std::string simpleAnyToString(const CORBA::Any& value);
    std::string any_to_string(const CORBA::Any& value);
    std::vector<std::string> any_to_strings(const CORBA::Any& value);

    CORBA::Any::from_boolean strings_to_boolean(const std::vector<std::string> &values);
    CORBA::Any::from_char strings_to_char(const std::vector<std::string> &values);
    CORBA::Double strings_to_double(const std::vector<std::string> &values);
    CORBA::Float strings_to_float(const std::vector<std::string> &values);
    CORBA::Short strings_to_short(const std::vector<std::string> &values);
    CORBA::Long strings_to_long(const std::vector<std::string> &values);
    CORBA::LongLong strings_to_long_long(const std::vector<std::string> &values);
    CORBA::Any::from_octet strings_to_octet(const std::vector<std::string> &values);
    CORBA::UShort strings_to_unsigned_short(const std::vector<std::string> &values);
    CORBA::ULong strings_to_unsigned_long(const std::vector<std::string> &values);
    CORBA::String_var strings_to_string(const std::vector<std::string> &values);
    std::string any_to_string(const CORBA::Any& value);

    CORBA::Any strings_to_any(const std::vector<std::string>& values, CORBA::TCKind kind);
    CORBA::Any strings_to_any(const std::vector<std::string>& values, CORBA::TCKind kind, CORBA::TypeCode_ptr type);

    CORBA::BooleanSeq* strings_to_boolean_sequence(const std::vector<std::string> &values);
    CORBA::CharSeq* strings_to_char_sequence(const std::vector<std::string> &values);
    CORBA::DoubleSeq* strings_to_double_sequence(const std::vector<std::string> &values);
    CORBA::FloatSeq* strings_to_float_sequence(const std::vector<std::string> &values);
    CORBA::ShortSeq* strings_to_short_sequence(const std::vector<std::string> &values);
    CORBA::LongSeq* strings_to_long_sequence(const std::vector<std::string> &values);
    CORBA::LongLongSeq* strings_to_long_long_sequence(const std::vector<std::string> &values);
    CORBA::OctetSeq* strings_to_octet_sequence(const std::vector<std::string> &values);
    CORBA::UShortSeq* strings_to_unsigned_short_sequence(const std::vector<std::string> &values);
    CORBA::ULongSeq* strings_to_unsigned_long_sequence(const std::vector<std::string> &values);
    CORBA::ULongLongSeq* strings_to_unsigned_long_long_sequence(const std::vector<std::string> &values);
    CORBA::StringSeq* strings_to_string_sequence(const std::vector<std::string> &values);
    CF::UTCTimeSequence* strings_to_utctime_sequence(const std::vector<std::string> &values);

    CF::complexBooleanSeq* strings_to_complex_boolean_sequence(const std::vector<std::string> &values);
    CF::complexCharSeq* strings_to_complex_char_sequence(const std::vector<std::string> &values);
    CF::complexDoubleSeq* strings_to_complex_double_sequence(const std::vector<std::string> &values);
    CF::complexFloatSeq* strings_to_complex_float_sequence(const std::vector<std::string> &values);
    CF::complexShortSeq* strings_to_complex_short_sequence(const std::vector<std::string> &values);
    CF::complexLongSeq* strings_to_complex_long_sequence(const std::vector<std::string> &values);
    CF::complexLongLongSeq* strings_to_complex_long_long_sequence(const std::vector<std::string> &values);
    CF::complexOctetSeq* strings_to_complex_octet_sequence(const std::vector<std::string> &values);
    CF::complexUShortSeq* strings_to_complex_unsigned_short_sequence(const std::vector<std::string> &values);
    CF::complexULongSeq* strings_to_complex_unsigned_long_sequence(const std::vector<std::string> &values);
    CF::complexULongLongSeq* strings_to_complex_unsigned_long_long_sequence(const std::vector<std::string> &values);

    CORBA::TCKind       getTypeKind(std::string type);
    CORBA::TypeCode_ptr getTypeCode(std::string type);
    CORBA::TypeCode_ptr getTypeCode(CORBA::TCKind kind, std::string structName);
    CF::Properties getNonNilProperties(const CF::Properties& originalProperties);
    CF::Properties getNonNilConfigureProperties(const CF::Properties& originalProperties);
}

#endif
