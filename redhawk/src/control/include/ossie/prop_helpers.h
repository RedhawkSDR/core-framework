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

#include "ossie/Properties.h"
#include "ossie/SoftPkg.h"
#include "ossie/componentProfile.h"

#if HAVE_OMNIORB4
#include "omniORB4/CORBA.h"
#endif

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

    bool compare_anys(CORBA::Any& a, CORBA::Any& b, std::string& action);
    CORBA::Any calculateDynamicProp(double operand, CORBA::Any& prop, std::string& math, CORBA::TCKind resultKind);
    CORBA::Any string_to_any(std::string value, CORBA::TCKind kind);
    std::string any_to_string(const CORBA::Any& value);
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

    CORBA::TCKind getTypeKind(std::string type);
    CF::DataType convertPropertyToDataType(const Property* prop);
    CF::DataType convertPropertyToDataType(const SimpleProperty* prop);
    CF::DataType convertPropertyToDataType(const SimpleSequenceProperty* prop);
    CF::DataType convertPropertyToDataType(const StructProperty* prop);
    CF::DataType convertPropertyToDataType(const StructSequenceProperty* prop);

    CF::DataType overridePropertyValue(const Property* prop, const ossie::ComponentProperty* compprop);
    CF::DataType overridePropertyValue(const SimpleProperty* prop, const ossie::ComponentProperty* compprop);
    CF::DataType overridePropertyValue(const SimpleSequenceProperty* prop, const ossie::ComponentProperty* compprop);
    CF::DataType overridePropertyValue(const StructProperty* prop, const ossie::ComponentProperty* compprop);
    CF::DataType overridePropertyValue(const StructSequenceProperty* prop, const ossie::ComponentProperty* compprop);
    CF::DataType overridePropertyValue(const Property* prop, const ossie::ComponentProperty* compprop, const CF::Properties& configureProperties);
    CF::DataType overridePropertyValue(const StructProperty* prop, const ossie::ComponentProperty* compprop, const CF::Properties& configureProperties);

    bool checkProcessor(const std::vector<std::string>& processorDeps, const std::vector<const Property*>& props);
    bool checkOs(const std::vector<ossie::SPD::NameVersionPair>& osDeps, const std::vector<const Property*>& props);
    CF::Properties getNonNilConfigureProperties(CF::Properties& originalProperties);
}

#endif
