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


#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <map>
#if HAVE_OMNIORB4
#include "omniORB4/CORBA.h"
#endif

#include <ossie/CorbaUtils.h>
#include <ossie/prop_helpers.h>
#include <ossie/debug.h>

using namespace ossie;

CREATE_LOGGER(prop_helpers)

bool ossie::compare_anys(CORBA::Any& a, CORBA::Any& b, std::string& action)
{
    CORBA::TypeCode_var typeA = a.type();
    CORBA::TypeCode_var typeB = b.type();

    // If the types don't match, the comparison will always be false
    if (typeA->kind() != typeB->kind()) {
        return false;
    }
    bool result = false;
    switch (typeA->kind()) {
    case CORBA::tk_boolean: {
        CORBA::Boolean tmp1;
        CORBA::Boolean tmp2;
        a >>= CORBA::Any::to_boolean(tmp1);
        b >>= CORBA::Any::to_boolean(tmp2);
        result = perform_action(tmp1, tmp2, action);
        break;
    }

    case CORBA::tk_char: {
        CORBA::Char tmp1;
        CORBA::Char tmp2;
        a >>= CORBA::Any::to_char(tmp1);
        b >>= CORBA::Any::to_char(tmp2);
        result = perform_action(tmp1, tmp2, action);
        break;
    }


    case CORBA::tk_octet: {
        CORBA::Octet tmp1;
        CORBA::Octet tmp2;
        a >>= CORBA::Any::to_octet(tmp1);
        b >>= CORBA::Any::to_octet(tmp2);
        result = perform_action(tmp1, tmp2, action);
        break;
    }

    case CORBA::tk_ushort: {
        CORBA::UShort tmp1;
        CORBA::UShort tmp2;
        a >>= tmp1;
        b >>= tmp2;
        result = perform_action(tmp1, tmp2, action);
        break;
    }

    case CORBA::tk_short: {
        CORBA::Short tmp1;
        CORBA::Short tmp2;
        a >>= tmp1;
        b >>= tmp2;
        result = perform_action(tmp1, tmp2, action);
        break;
    }

    case CORBA::tk_float: {
        CORBA::Float tmp1;
        CORBA::Float tmp2;
        a >>= tmp1;
        b >>= tmp2;
        result = perform_action(tmp1, tmp2, action);
        break;
    }

    case CORBA::tk_double: {
        CORBA::Double tmp1;
        CORBA::Double tmp2;
        a >>= tmp1;
        b >>= tmp2;
        result = perform_action(tmp1, tmp2, action);
        break;
    }

    case CORBA::tk_ulong: {
        CORBA::ULong tmp1;
        CORBA::ULong tmp2;
        a >>= tmp1;
        b >>= tmp2;
        result = perform_action(tmp1, tmp2, action);
        break;
    }

    case CORBA::tk_long: {
        CORBA::Long tmp1;
        CORBA::Long tmp2;
        a >>= tmp1;
        b >>= tmp2;
        result = perform_action(tmp1, tmp2, action);
        break;
    }

    case CORBA::tk_longlong: {
        CORBA::LongLong tmp1;
        CORBA::LongLong tmp2;
        a >>= tmp1;
        b >>= tmp2;
        result = perform_action(tmp1, tmp2, action);
        break;
    }

    case CORBA::tk_string: {
        const char* tmp1;
        const char* tmp2;
        a >>= tmp1;
        b >>= tmp2;
        result = perform_action(std::string(tmp1), std::string(tmp2), action);
        break;
    }
    default:
        result = false;
    }
    return result;
}

CORBA::Any ossie::calculateDynamicProp(double operand, CORBA::Any& prop, std::string& math, CORBA::TCKind resultKind)
{
    double tmp_result = 0.0;
    CORBA::TypeCode_var typeProp = prop.type();

    switch (typeProp->kind()) {
    case CORBA::tk_ushort: {
        CORBA::UShort tmp1;
        prop >>= tmp1;
        LOG_TRACE(prop_helpers, "Performing math " << operand << " " << math << " " << tmp1);
        tmp_result = perform_math(operand, tmp1, math);
        break;
    }

    case CORBA::tk_short: {
        CORBA::Short tmp1;
        prop >>= tmp1;
        LOG_TRACE(prop_helpers, "Performing math " << operand << " " << math << " " << tmp1);
        tmp_result = perform_math(operand, tmp1, math);
        break;
    }

    case CORBA::tk_float: {
        CORBA::Float tmp1;
        prop >>= tmp1;
        LOG_TRACE(prop_helpers, "Performing math " << operand << " " << math << " " << tmp1);
        tmp_result = perform_math(operand, tmp1, math);
        break;
    }

    case CORBA::tk_double: {
        CORBA::Double tmp1;
        prop >>= tmp1;
        LOG_TRACE(prop_helpers, "Performing math " << operand << " " << math << " " << tmp1);
        tmp_result = perform_math(operand, tmp1, math);
        break;
    }

    case CORBA::tk_ulong: {
        CORBA::ULong tmp1;
        prop >>= tmp1;
        tmp_result = perform_math(operand, tmp1, math);
        break;
    }

    case CORBA::tk_long: {
        CORBA::Long tmp1;
        prop >>= tmp1;
        tmp_result = perform_math(operand, tmp1, math);
        break;
    }

    case CORBA::tk_longlong: {
        CORBA::LongLong tmp1;
        prop >>= tmp1;
        tmp_result = perform_math(operand, tmp1, math);
        break;
    }

    case CORBA::tk_string: {
        // Do nothing
        break;
    }

    default:
        break;
    }

    LOG_TRACE(prop_helpers, "calculated dynamic prop " << tmp_result);

    CORBA::Any result;
    switch (resultKind) {
    case CORBA::tk_ushort: {
        result <<= static_cast<CORBA::UShort>(tmp_result);
        break;
    }

    case CORBA::tk_short: {
        result <<= static_cast<CORBA::Short>(tmp_result);
        break;
    }

    case CORBA::tk_float: {
        result <<= static_cast<CORBA::Float>(tmp_result);
        break;
    }

    case CORBA::tk_double: {
        result <<= static_cast<CORBA::Double>(tmp_result);
        break;
    }

    case CORBA::tk_ulong: {
        result <<= static_cast<CORBA::ULong>(tmp_result);
        break;
    }

    case CORBA::tk_long: {
        result <<= static_cast<CORBA::Long>(tmp_result);
        break;
    }

    case CORBA::tk_longlong: {
        result <<= static_cast<CORBA::LongLong>(tmp_result);
        break;
    }

    case CORBA::tk_string: {
        result <<= any_to_string(prop);
        break;
    }

    default:
        break;
    }
    return result;
}

CORBA::Any ossie::string_to_any(std::string value, CORBA::TCKind kind)
{
    CORBA::Any result;
    switch (kind) {
    case CORBA::tk_boolean:
        if ((value == "true") || (value == "True") || (value == "TRUE") || (value == "1")) {
            result <<= CORBA::Any::from_boolean(CORBA::Boolean(true));
        } else {
            result <<= CORBA::Any::from_boolean(CORBA::Boolean(false));
        }
        break;

    case CORBA::tk_char:
        result <<= CORBA::Any::from_char(CORBA::Char(value[0]));
        break;

    case CORBA::tk_double:
        result <<= CORBA::Double(strtod(value.c_str(), NULL));
        break;

    case CORBA::tk_octet:
        result <<= CORBA::Any::from_octet(CORBA::Octet(strtol(value.c_str(), NULL, 0)));
        break;

    case CORBA::tk_ushort:
        result <<= CORBA::UShort(strtol(value.c_str(), NULL, 0));
        break;

    case CORBA::tk_short:
        result <<= CORBA::Short(strtol(value.c_str(), NULL, 0));
        break;

    case CORBA::tk_float:
        result <<= CORBA::Float(strtof(value.c_str(), NULL));
        break;

    case CORBA::tk_ulong:
        result <<= CORBA::ULong(strtol(value.c_str(), NULL, 0));
        break;

    case CORBA::tk_long:
        result <<= CORBA::Long(strtol(value.c_str(), NULL, 0));
        break;

    case CORBA::tk_longlong:
        result <<= CORBA::LongLong(strtoll(value.c_str(), NULL, 0));
        break;

    case CORBA::tk_ulonglong:
        result <<= CORBA::ULongLong(strtoll(value.c_str(), NULL, 0));
        break;

    case CORBA::tk_string:
        result <<= value.c_str();
        break;
    default:
        result = CORBA::Any();
    }
    return result;
}

CORBA::Any ossie::strings_to_any(const std::vector<std::string>& values, CORBA::TCKind kind)
{
    CORBA::Any result;
    switch (kind) {
    case CORBA::tk_boolean:
        result <<= strings_to_boolean_sequence(values);
        break;

    case CORBA::tk_char:
        result <<= strings_to_char_sequence(values);
        break;

    case CORBA::tk_double:
        result <<= strings_to_double_sequence(values);
        break;

    case CORBA::tk_octet:
        result <<= strings_to_octet_sequence(values);
        break;

    case CORBA::tk_ushort:
        result <<= strings_to_unsigned_short_sequence(values);
        break;

    case CORBA::tk_short:
        result <<= strings_to_short_sequence(values);
        break;

    case CORBA::tk_float:
        result <<= strings_to_float_sequence(values);
        break;

    case CORBA::tk_ulong:
        result <<= strings_to_unsigned_long_sequence(values);
        break;

    case CORBA::tk_long:
        result <<= strings_to_long_sequence(values);
        break;

    case CORBA::tk_longlong:
        result <<= strings_to_long_long_sequence(values);
        break;

    case CORBA::tk_ulonglong:
        result <<= strings_to_unsigned_long_long_sequence(values);
        break;

    case CORBA::tk_string:
        result <<= strings_to_string_sequence(values);
        break;
    default:
        result = CORBA::Any();
    }
    return result;
}

std::string ossie::any_to_string(const CORBA::Any& value)
{
    std::ostringstream result;
    CORBA::TypeCode_var typeValue = value.type();

    switch (typeValue->kind()) {
    case CORBA::tk_boolean: {
        CORBA::Boolean tmp;
        value >>= CORBA::Any::to_boolean(tmp);
        result << tmp;
    }
    break;

    case CORBA::tk_char: {
        CORBA::Char tmp;
        value >>= CORBA::Any::to_char(tmp);
        result << tmp;
    }
    break;


    case CORBA::tk_octet: {
        CORBA::Octet tmp;
        value >>= CORBA::Any::to_octet(tmp);
        /*
         * NOTE:  Octet is an unsigned char and hence is stored as an ASCII
         *        character rather than the number itself so it needs to be
         *        casted into its numerical representation
         */
        result << (int)tmp;
    }
    break;

    case CORBA::tk_ushort: {
        CORBA::UShort tmp;
        value >>= tmp;
        result << tmp;
        break;
    }

    case CORBA::tk_short: {
        CORBA::Short tmp;
        value >>= tmp;
        result << tmp;
        break;
    }

    case CORBA::tk_float: {
        CORBA::Float tmp;
        value >>= tmp;
        result << tmp;
        break;
    }

    case CORBA::tk_double: {
        CORBA::Double tmp;
        value >>= tmp;
        result << tmp;
        break;
    }

    case CORBA::tk_ulong: {
        CORBA::ULong tmp;
        value >>= tmp;
        result << tmp;
        break;
    }

    case CORBA::tk_long: {
        CORBA::Long tmp;
        value >>= tmp;
        result << tmp;
        break;
    }

    case CORBA::tk_longlong: {
        CORBA::LongLong tmp;
        value >>= tmp;
        result << tmp;
        break;
    }

    case CORBA::tk_string: {
        const char* tmp;
        value >>= tmp;
        result << tmp;
        break;
    }
    default:
        result << "Kind: " << typeValue->kind();
    }
    return result.str();
}


CORBA::Any::from_boolean ossie::strings_to_boolean(const std::vector<std::string> &values)
{
    CORBA::Boolean result(false);

    if ((values[0] == "true") || (values[0] == "True") || (values[0] == "TRUE") || (values[0] == "1")) {
        result = true;
    } else if ((values[0] == "false") || (values[0] == "False") || (values[0] == "FALSE") || (values[0] == "0")) {
        result = false;
    }

    return CORBA::Any::from_boolean(result);
}

CORBA::Any::from_char ossie::strings_to_char(const std::vector<std::string> &values)
{
    CORBA::Char result(' ');

    result = values[0][0];

    return CORBA::Any::from_char(result);
}

CORBA::Double ossie::strings_to_double(const std::vector<std::string> &values)
{
    CORBA::Double result(0);

    result = strtod(values[0].c_str(), NULL);

    return result;
}

CORBA::Float ossie::strings_to_float(const std::vector<std::string> &values)
{
    CORBA::Float result(0);

    result = strtof(values[0].c_str(), NULL);

    return result;
}

CORBA::Short ossie::strings_to_short(const std::vector<std::string> &values)
{
    CORBA::Short result(0);

    result = (short) strtol(values[0].c_str(), NULL, 0);

    return result;
}

CORBA::Long ossie::strings_to_long(const std::vector<std::string> &values)
{
    CORBA::Long result(0);

    result = strtol(values[0].c_str(), NULL, 0);

    return result;
}

CORBA::LongLong ossie::strings_to_long_long(const std::vector<std::string> &values)
{
    CORBA::LongLong result(0);

    result = strtoll(values[0].c_str(), NULL, 0);

    return result;
}

CORBA::Any::from_octet ossie::strings_to_octet(const std::vector<std::string> &values)
{
    CORBA::Octet result(0);

    result = (short) strtol(values[0].c_str(), NULL, 0);

    return CORBA::Any::from_octet(result);
}

CORBA::UShort ossie::strings_to_unsigned_short(const std::vector<std::string> &values)
{
    CORBA::UShort result(0);

    result = (unsigned short) strtol(values[0].c_str(), NULL, 0);

    return result;
}

CORBA::ULong ossie::strings_to_unsigned_long(const std::vector<std::string> &values)
{
    CORBA::ULong result(0);

    result = (unsigned long) atoll(values[0].c_str());

    return result;
}

CORBA::String_var ossie::strings_to_string(const std::vector<std::string> &values)
{
    CORBA::String_var result;

    result = CORBA::string_dup(values[0].c_str());

    return result;
}

CORBA::BooleanSeq* ossie::strings_to_boolean_sequence(const std::vector<std::string> &values)
{
    CORBA::BooleanSeq_var result = new CORBA::BooleanSeq;

    result->length(values.size());
    for (unsigned int i = 0; i < values.size(); ++i) {

        if ((values[i] == "true") || (values[i] == "True") || (values[i] == "TRUE") || (values[i] == "1")) {
            result[i] = true;
        } else if ((values[i] == "false") || (values[i] == "False") || (values[i] == "FALSE") || (values[i] == "0")) {
            result[i] = false;
        }
    }
    return result._retn();
}

CORBA::CharSeq* ossie::strings_to_char_sequence(const std::vector<std::string> &values)
{
    CORBA::CharSeq_var result = new CORBA::CharSeq;

    result->length(values.size());
    for (unsigned int i = 0; i < values.size(); ++i) {

        result[i] = values[i][0];
    }
    return result._retn();
}

CORBA::DoubleSeq* ossie::strings_to_double_sequence(const std::vector<std::string> &values)
{
    CORBA::DoubleSeq_var result = new CORBA::DoubleSeq;

    result->length(values.size());
    for (unsigned int i = 0; i < values.size(); ++i) {
        result[i] = strtod(values[i].c_str(), NULL);
    }

    return result._retn();
}

CORBA::FloatSeq* ossie::strings_to_float_sequence(const std::vector<std::string> &values)
{
    CORBA::FloatSeq_var result = new CORBA::FloatSeq;

    result->length(values.size());

    for (unsigned int i = 0; i < values.size(); ++i) {
        result[i] = strtof(values[i].c_str(), NULL);
    }

    return result._retn();
}

CORBA::ShortSeq* ossie::strings_to_short_sequence(const std::vector<std::string> &values)
{
    CORBA::ShortSeq_var result = new CORBA::ShortSeq;

    result->length(values.size());

    for (unsigned int i = 0; i < values.size(); ++i) {
        result[i] = (short) strtol(values[i].c_str(), NULL, 0);
    }

    return result._retn();
}

CORBA::LongSeq* ossie::strings_to_long_sequence(const std::vector<std::string> &values)
{
    CORBA::LongSeq_var result = new CORBA::LongSeq;

    result->length(values.size());

    for (unsigned int i = 0; i < values.size(); ++i) {
        result[i] = strtol(values[i].c_str(), NULL, 0);
    }

    return result._retn();
}

CORBA::LongLongSeq* ossie::strings_to_long_long_sequence(const std::vector<std::string> &values)
{
    CORBA::LongLongSeq_var result = new CORBA::LongLongSeq;

    result->length(values.size());

    for (unsigned int i = 0; i < values.size(); ++i) {
        result[i] = strtoll(values[i].c_str(), NULL, 0);
    }

    return result._retn();
}

CORBA::ULongLongSeq* ossie::strings_to_unsigned_long_long_sequence(const std::vector<std::string> &values)
{
    CORBA::ULongLongSeq_var result = new CORBA::ULongLongSeq;

    result->length(values.size());

    for (unsigned int i = 0; i < values.size(); ++i) {
        result[i] = strtoll(values[i].c_str(), NULL, 0);
    }

    return result._retn();
}

CORBA::OctetSeq* ossie::strings_to_octet_sequence(const std::vector<std::string> &values)
{
    CORBA::OctetSeq_var result = new CORBA::OctetSeq;

    result->length(values.size());

    for (unsigned int i = 0; i < values.size(); ++i) {
        result[i] = (short) strtol(values[i].c_str(), NULL, 0);
    }

    return result._retn();
}

CORBA::UShortSeq* ossie::strings_to_unsigned_short_sequence(const std::vector<std::string> &values)
{
    CORBA::UShortSeq_var result = new CORBA::UShortSeq;

    result->length(values.size());

    for (unsigned int i = 0; i < values.size(); ++i) {
        result[i] = (unsigned short) strtol(values[i].c_str(), NULL, 0);
    }

    return result._retn();
}

CORBA::ULongSeq* ossie::strings_to_unsigned_long_sequence(const std::vector<std::string> &values)
{
    CORBA::ULongSeq_var result = new CORBA::ULongSeq;

    result->length(values.size());

    for (unsigned int i = 0; i < values.size(); ++i) {
        result[i] = (unsigned long) atoll(values[i].c_str());
    }

    return result._retn();
}

CORBA::StringSeq* ossie::strings_to_string_sequence(const std::vector<std::string> &values)
{
    CORBA::StringSeq_var result = new CORBA::StringSeq;

    result->length(values.size());

    for (unsigned int i = 0; i < values.size(); ++i) {
        result[i] = CORBA::string_dup(values[i].c_str());
    }

    return result._retn();
}

CF::DataType ossie::convertPropertyToDataType(const Property* prop) {
    if (dynamic_cast<const SimpleProperty*>(prop) != NULL) {
        const SimpleProperty* simp = dynamic_cast<const SimpleProperty*>(prop);
        return convertPropertyToDataType(simp);
    } else if (dynamic_cast<const SimpleSequenceProperty*>(prop) != NULL) {
        const SimpleSequenceProperty* simpseq = dynamic_cast<const SimpleSequenceProperty*>(prop);
        return convertPropertyToDataType(simpseq);
    } else if (dynamic_cast<const StructProperty*>(prop) != NULL) {
        const StructProperty* stru = dynamic_cast<const StructProperty*>(prop);
        return convertPropertyToDataType(stru);
    } else if (dynamic_cast<const StructSequenceProperty*>(prop) != NULL) {
        const StructSequenceProperty* struseq = dynamic_cast<const StructSequenceProperty*>(prop);
        return convertPropertyToDataType(struseq);
    }
    return CF::DataType();
}

CF::DataType ossie::overridePropertyValue(const Property* prop, const ComponentProperty* compprop) {
    if (dynamic_cast<const SimpleProperty*>(prop) != NULL) {
        const SimpleProperty* simp = dynamic_cast<const SimpleProperty*>(prop);
        return overridePropertyValue(simp, compprop);
    } else if (dynamic_cast<const SimpleSequenceProperty*>(prop) != NULL) {
        const SimpleSequenceProperty* simpseq = dynamic_cast<const SimpleSequenceProperty*>(prop);
        return overridePropertyValue(simpseq, compprop);
    } else if (dynamic_cast<const StructProperty*>(prop) != NULL) {
        const StructProperty* stru = dynamic_cast<const StructProperty*>(prop);
        return overridePropertyValue(stru, compprop);
    } else if (dynamic_cast<const StructSequenceProperty*>(prop) != NULL) {
        const StructSequenceProperty* struseq = dynamic_cast<const StructSequenceProperty*>(prop);
        return overridePropertyValue(struseq, compprop);
    }
    return CF::DataType();
}

CF::DataType ossie::overridePropertyValue(const Property* prop, const ComponentProperty* compprop, const CF::Properties& configureProperties) {
    if (dynamic_cast<const StructProperty*>(prop) != NULL) {
        const StructProperty* stru = dynamic_cast<const StructProperty*>(prop);
        return overridePropertyValue(stru, compprop, configureProperties);
    }
    return CF::DataType();
}

CORBA::TCKind ossie::getTypeKind(std::string type) {
    CORBA::TCKind kind;
    if (type == "boolean") {
        kind = CORBA::tk_boolean;
    } else if (type == "char") {
        kind = CORBA::tk_char;
    } else if (type == "double") {
        kind = CORBA::tk_double;
    } else if (type == "float") {
        kind = CORBA::tk_float;
    } else if (type == "short") {
        kind = CORBA::tk_short;
    } else if (type == "long") {
        kind = CORBA::tk_long;
    } else if (type == "longlong") {
        kind = CORBA::tk_longlong;
    } else if (type == "ulonglong") {
        kind = CORBA::tk_longlong;
    } else if (type == "octet") {
        kind = CORBA::tk_octet;
    } else if (type == "ushort") {
        kind = CORBA::tk_ushort;
    } else if (type == "ulong") {
        kind = CORBA::tk_ulong;
    } else if (type == "string") {
        kind = CORBA::tk_string;
    } else {
        kind = CORBA::tk_null;
    }
    return kind;
}

CF::DataType ossie::convertPropertyToDataType(const SimpleProperty* prop) {
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(prop->getID());

    if (prop->getValue() != NULL) {
        std::string value(prop->getValue());
        CORBA::TCKind kind = ossie::getTypeKind(static_cast<std::string>(prop->getType()));
        dataType.value = ossie::string_to_any(value, kind);
    }
    return dataType;
}

CF::DataType ossie::convertPropertyToDataType(const SimpleSequenceProperty* prop) {
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(prop->getID());
    if (!prop->isNone()) {
        CORBA::TCKind kind = ossie::getTypeKind(static_cast<std::string>(prop->getType()));
        dataType.value = ossie::strings_to_any(prop->getValues(), kind);
    }
    return dataType;
}

CF::DataType ossie::convertPropertyToDataType(const StructProperty* prop) {
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(prop->getID());

    if (prop->isNone()) {
        return dataType;
    }

    CF::Properties structval_;
    for (std::vector<SimpleProperty>::const_iterator i = prop->getValue().begin();
         i != prop->getValue().end();
         ++i) {

        CF::DataType dt;
        dt.id = CORBA::string_dup(i->getID());
        CORBA::TCKind kind = ossie::getTypeKind(static_cast<std::string>(i->getType()));
        if (i->getValue()) {
            dt.value = ossie::string_to_any(i->getValue(), kind);
            LOG_TRACE(prop_helpers, "setting struct item " << i->getID());
        } else {
            LOG_TRACE(prop_helpers, "struct item " << i->getID() << " has no default value");
        }
        structval_.length(structval_.length() + 1);
        structval_[structval_.length() - 1] = dt;
    }
    dataType.value <<= structval_;
    return dataType;
}

CF::DataType ossie::convertPropertyToDataType(const StructSequenceProperty* prop) {
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(prop->getID());

    if (prop->isNone()) {
        return dataType;
    }

    const std::vector<StructProperty>& propValues = prop->getValues();
    CORBA::AnySeq values;
    values.length(propValues.size());
    for (CORBA::ULong ii = 0; ii < values.length(); ++ii) {
        CF::DataType val = convertPropertyToDataType(&propValues[ii]);
        values[ii] = val.value;
    }
    dataType.value <<= values;
    return dataType;
}

static CF::DataType overrideSimpleValue(const SimpleProperty* prop, const std::string& value)
{
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(prop->getID());
    CORBA::TCKind kind = ossie::getTypeKind(prop->getType());
    dataType.value = ossie::string_to_any(value, kind);
    return dataType;
}

CF::DataType ossie::overridePropertyValue(const SimpleProperty* prop, const ComponentProperty* compprop) {
    const SimplePropertyRef* simpleref = dynamic_cast<const SimplePropertyRef*>(compprop);
    if (!simpleref) {
        LOG_WARN(prop_helpers, "ignoring attempt to override simple property " << prop->getID() << " because override definition is not a simpleref");
        return convertPropertyToDataType(prop);
    }

    LOG_TRACE(prop_helpers, "overriding simple property id " << prop->getID());
    return overrideSimpleValue(prop, simpleref->getValue());
}

CF::DataType ossie::overridePropertyValue(const SimpleSequenceProperty* prop, const ComponentProperty* compprop) {
    CF::DataType dataType = convertPropertyToDataType(prop);
    if (dynamic_cast<const SimpleSequencePropertyRef*>(compprop) != NULL) {
        const SimpleSequencePropertyRef* simpleseqref = dynamic_cast<const SimpleSequencePropertyRef*>(compprop);
        LOG_TRACE(prop_helpers, "overriding simpleseq property id " << dataType.id);
        CORBA::TCKind kind = ossie::getTypeKind(static_cast<std::string>(prop->getType()));
        dataType.value = ossie::strings_to_any(simpleseqref->getValues(), kind);
    } else {
        LOG_WARN(prop_helpers, "ignoring attempt to override simple sequence property " << dataType.id << " because override definition is not a simpleseqref");
    }
    return dataType;
}

static CF::Properties overrideStructValues(const StructProperty* prop, const std::map<std::string, std::string>& values)
{
    const std::vector<SimpleProperty>& simpleProps = prop->getValue();
    LOG_TRACE(prop_helpers, "structure has " << simpleProps.size() << " elements");
    CF::Properties structval;
    structval.length(simpleProps.size());
    for (CORBA::ULong ii = 0; ii < structval.length(); ++ii) {
        const SimpleProperty& simple = simpleProps[ii];
        const std::string id = simple.getID();
        std::map<std::string, std::string>::const_iterator itemoverride = values.find(id);
        if (itemoverride == values.end()) {
            LOG_TRACE(prop_helpers, "using default value for struct element " << id);
            structval[ii] = convertPropertyToDataType(&simple);
        } else {
            LOG_TRACE(prop_helpers, "setting structure element " << id << " to " << itemoverride->second);
            structval[ii] = overrideSimpleValue(&simple, itemoverride->second);
        }
    }

    return structval;
}

static CF::Properties overrideStructValues(const StructProperty* prop, const std::map<std::string, std::string>& values, const CF::Properties& configureProperties)
{
    const std::vector<SimpleProperty>& simpleProps = prop->getValue();
    LOG_TRACE(prop_helpers, "structure has " << simpleProps.size() << " elements");
    CF::Properties structval;
    structval.length(simpleProps.size());
    for (CORBA::ULong ii = 0; ii < structval.length(); ++ii) {
        const SimpleProperty& simple = simpleProps[ii];
        const std::string id = simple.getID();
        std::map<std::string, std::string>::const_iterator itemoverride = values.find(id);
        if (itemoverride == values.end()) {
            LOG_TRACE(prop_helpers, "using default value for struct element " << id);
            structval[ii] = convertPropertyToDataType(&simple);
        } else {
            LOG_TRACE(prop_helpers, "setting structure element " << id << " to " << itemoverride->second);
            std::string value = itemoverride->second;
            if (strncmp(value.c_str(), "__MATH__", 8) == 0) {
                CF::DataType dataType;
                dataType.id = CORBA::string_dup(simple.getID());
                CORBA::TCKind kind = ossie::getTypeKind(simple.getType());
                LOG_TRACE(prop_helpers, "Invoking custom OSSIE dynamic allocation property support")
                // Turn propvalue into a string for easy parsing
                std::string mathStatement = value.substr(8);
                if ((*mathStatement.begin() == '(') && (*mathStatement.rbegin() == ')')) {
                    // TODO - implement a more relaxed parser
                    mathStatement.erase(mathStatement.begin(), mathStatement.begin() + 1);
                    mathStatement.erase(mathStatement.end() - 1, mathStatement.end());
                    std::vector<std::string> args;
                    while ((mathStatement.length() > 0) && (mathStatement.find(',') != std::string::npos)) {
                        args.push_back(mathStatement.substr(0, mathStatement.find(',')));
                        LOG_TRACE(prop_helpers, "ARG " << args.back())
                        mathStatement.erase(0, mathStatement.find(',') + 1);
                    }
                    args.push_back(mathStatement);
                    LOG_TRACE(prop_helpers, "ARG " << args.back())

                    if (args.size() != 3) {
                        std::ostringstream eout;
                        eout << " invalid __MATH__ statement; '" << mathStatement << "'";
                        throw ossie::PropertyMatchingError(eout.str());
                    }

                    LOG_TRACE(prop_helpers, "__MATH__ " << args[0] << " " << args[1] << " " << args[2])

                    double operand;
                    operand = strtod(args[0].c_str(), NULL);

                    // See if there is a property in the component
                    LOG_TRACE(prop_helpers, "Attempting to find matching property for " << args[1])
                    const CF::DataType* matchingCompProp = 0;
                    for (unsigned int j = 0; j < configureProperties.length(); j++) {
                        if (strcmp(configureProperties[j].id, args[1].c_str()) == 0) {
                            LOG_TRACE(prop_helpers, "Matched property for " << args[1])
                            matchingCompProp = &configureProperties[j];
                        }
                        // See if the property we're looking for is a member of a struct
                        //  **note: this only works because the dtd states that each
                        //          property id is globally unique within the prf
                        const CF::Properties* tmp_ref = NULL;
                        configureProperties[j].value >>= tmp_ref;
                        if (tmp_ref != NULL) {
                            for (unsigned prop_idx = 0; prop_idx<tmp_ref->length(); prop_idx++) {
                                if (strcmp((*tmp_ref)[prop_idx].id, args[1].c_str()) == 0) {
                                    LOG_TRACE(prop_helpers, "Matched property for " << args[1])
                                    matchingCompProp = &(*tmp_ref)[prop_idx];
                                }
                            }
                        }
                    }

                    if (matchingCompProp == 0) {
                        std::ostringstream eout;
                        eout << " failed to match component property in __MATH__ statement; property id = " << args[1] << " does not exist in component as a configure property";
                        throw ossie::PropertyMatchingError(eout.str());
                    }

                    std::string math = args[2];
                    CORBA::Any compValue = matchingCompProp->value;
                    dataType.value = ossie::calculateDynamicProp(operand, compValue, math, kind);
                } else {
                    std::ostringstream eout;
                    eout << " invalid __MATH__ statement; '" << mathStatement << "'";
                    throw ossie::PropertyMatchingError(eout.str());
                }
                structval[ii] = dataType;
            } else {
                structval[ii] = overrideSimpleValue(&simple, itemoverride->second);
            }
        }
    }

    return structval;
}

CF::DataType ossie::overridePropertyValue(const StructProperty* prop, const ComponentProperty* compprop) {
    CF::DataType dataType = convertPropertyToDataType(prop);

    const StructPropertyRef* structref = dynamic_cast<const StructPropertyRef*>(compprop);
    if (structref) {
        LOG_TRACE(prop_helpers, "overriding struct property id " << dataType.id);
        dataType.value <<= overrideStructValues(prop, structref->getValue());
    } else {
        LOG_WARN(prop_helpers, "ignoring attempt to override struct property " << dataType.id << " because override definition is not a structref");
    }
    return dataType;
}

CF::DataType ossie::overridePropertyValue(const StructProperty* prop, const ComponentProperty* compprop, const CF::Properties& configureProperties) {
    CF::DataType dataType = convertPropertyToDataType(prop);

    const StructPropertyRef* structref = dynamic_cast<const StructPropertyRef*>(compprop);
    if (structref) {
        LOG_TRACE(prop_helpers, "overriding struct property id " << dataType.id << " (supports __MATH__)");
        dataType.value <<= overrideStructValues(prop, structref->getValue(), configureProperties);
    } else {
        LOG_WARN(prop_helpers, "ignoring attempt to override struct property " << dataType.id << " because override definition is not a structref");
    }
    return dataType;
}

CF::DataType ossie::overridePropertyValue(const StructSequenceProperty* prop, const ComponentProperty* compprop) {
    CF::DataType dataType = convertPropertyToDataType(prop);

    const StructSequencePropertyRef* structsequenceref = dynamic_cast<const StructSequencePropertyRef*>(compprop);
    if (structsequenceref) {
        LOG_TRACE(prop_helpers, "overriding structsequence property id " << dataType.id);

        const std::vector<std::map<std::string, std::string> >& overrideValues = structsequenceref->getValues();
        LOG_TRACE(prop_helpers, "structsequence has " << overrideValues.size() << " values");

        CORBA::AnySeq values;
        values.length(overrideValues.size());
        const std::vector<StructProperty>default_values = prop->getValues();
        for (CORBA::ULong ii = 0; ii < values.length(); ++ii) {
            StructProperty tmp = prop->getStruct();
            if (ii < default_values.size()) {
                tmp = default_values[ii];
            }
            values[ii] <<= overrideStructValues(&tmp, overrideValues[ii]);
        }
        dataType.value <<= values;
    } else {
        LOG_WARN(prop_helpers, "ignoring attempt to override structsequence property " << dataType.id << " because override definition is not a structsequenceref");
    }
    return dataType;
}

bool ossie::checkProcessor(const std::vector<std::string>& processorDeps, const std::vector<const Property*>& props)
{
    if (processorDeps.size() == 0) {
        return true;
    }

    // Interpreting D.2.1.6.8, if more than one processor element is provided assume that the implementation
    // can run on any of those...so once we find one match we are successful
    for (unsigned int j = 0; j < processorDeps.size(); j++) {
        bool matchProcessor = true;

        std::string processor = processorDeps[j];
        if (processor != "") {
            matchProcessor = false;
            LOG_TRACE(prop_helpers, "Attempting to match processor " << processor << " against " << props.size() << " properties")
            for (unsigned int i = 0; i < props.size(); i++) {
                if (dynamic_cast<const SimpleProperty*>(props[i]) != NULL) {
                    const SimpleProperty* matchingProp = dynamic_cast<const SimpleProperty*>(props[i]);
                    std::string action = matchingProp->getAction();
                    LOG_TRACE(prop_helpers, "Checking property " << matchingProp->getID() << " " << matchingProp->getName())
                    if (strcmp(matchingProp->getName(), "processor_name") == 0) {
                        const char *tmp_value = matchingProp->getValue();
                        std::string dev_processor_name("");
                        if (tmp_value != NULL) {
                            dev_processor_name = tmp_value;
                        }
                        LOG_TRACE(prop_helpers, "Performing comparison operation '" << dev_processor_name << "' " << action << " '" << processor << "'")
                        matchProcessor = ossie::perform_action(dev_processor_name, processor, action);
                        if (matchProcessor) break;
                    }
                }
            }
        }
    
        if (matchProcessor) {
                return true;
        }
    }
    return false;
}

bool ossie::checkOs(const std::vector<ossie::SPD::NameVersionPair>& osDeps, const std::vector<const Property*>& props)
{
    if (osDeps.size() == 0) {
        return true;
    }

    // Interpreting D.2.1.6.7, if more than one processor element is provided assume that the implementation
    // can run on any of those...so once we find one match we are successful
    for (unsigned int j = 0; j < osDeps.size(); j++) {
        bool matchOs = true;
        bool matchOsVersion = true;

        std::string os = osDeps[j].first;
        std::string osVersion = osDeps[j].second;

        if (os != "") {
            matchOs = false;
            LOG_TRACE(prop_helpers, "Attempting to match os " << os)
            for (unsigned int i = 0; i < props.size(); i++) {
                if (dynamic_cast<const SimpleProperty*>(props[i]) != NULL) {
                    const SimpleProperty* matchingProp = dynamic_cast<const SimpleProperty*>(props[i]);
                    std::string action = matchingProp->getAction();
                    if (strcmp(matchingProp->getName(), "os_name") == 0) {
                        const char *tmp_dev_os_name = matchingProp->getValue();
                        std::string dev_os_name("");
                        if (tmp_dev_os_name != NULL) {
                            dev_os_name = tmp_dev_os_name;
                        }
                        LOG_TRACE(prop_helpers, "Performing comparison operation " << dev_os_name << " " << action << " " << os)
                        matchOs = ossie::perform_action(dev_os_name, os, action);
                        if (matchOs) break;
                    }
                }
            }
        }

        LOG_TRACE(prop_helpers, "Attempting to match os version")
        if (osVersion != "") {
            matchOsVersion = false;
            LOG_TRACE(prop_helpers, "Attempting to match os version" << osVersion)
            for (unsigned int i = 0; i < props.size(); i++) {
                if (dynamic_cast<const SimpleProperty*>(props[i]) != NULL) {
                    const SimpleProperty* matchingProp = dynamic_cast<const SimpleProperty*>(props[i]);
                    std::string action = matchingProp->getAction();
                    if (strcmp(matchingProp->getName(), "os_version") == 0) {
                        const char *tmp_dev_os_version = matchingProp->getValue();
                        std::string dev_os_version("");
                        if (tmp_dev_os_version != NULL) {
                            dev_os_version = tmp_dev_os_version;
                        }
                        LOG_TRACE(prop_helpers, "Performing comparison operation " << dev_os_version << " " << action << " " << osVersion)
                        matchOsVersion = ossie::perform_action(dev_os_version, osVersion, action);
                        if (matchOsVersion) break;
                    }
                }
            }
        }

        if (matchOs && matchOsVersion) {
            return true;
        }
    }

    return false;
}

CF::Properties ossie::getNonNilConfigureProperties(CF::Properties& originalProperties)
{
    CF::Properties nonNilProperties;
    CORBA::TypeCode_var typeProp;

    for (unsigned int i = 0; i < originalProperties.length(); i++) {
        CF::DataType prop = originalProperties[i];
        typeProp = prop.value.type();
        if (typeProp->kind() != CORBA::tk_null) {
            nonNilProperties.length(nonNilProperties.length() + 1);
            nonNilProperties[nonNilProperties.length()-1] = prop;
        }
    }
    return nonNilProperties;
}
