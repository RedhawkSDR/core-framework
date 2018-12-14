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
#include <iomanip>
#if HAVE_OMNIORB4
#include "omniORB4/CORBA.h"
#endif

#include <ossie/CorbaUtils.h>
#include <ossie/prop_helpers.h>
#include <ossie/debug.h>
#include <ossie/PropertyMap.h>

using namespace ossie;

CREATE_LOGGER(prop_helpers)

bool ossie::compare_anys(const CORBA::Any& a, const CORBA::Any& b, std::string& action) {
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

    case CORBA::tk_ulonglong: {
        CORBA::ULongLong tmp1;
        CORBA::ULongLong tmp2;
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

/*
 * Convert a string in the format A+jB to a CORBA::Any.
 *
 * Type corresponds to the type of the real/imag members of the CF::complex
 * struct (e.g., CORBA::Long or float).
 *
 * CFComplexType corresponds to the type of the CF::complexStruct (e.g.,
 * CF::complexLong or CF::complexFloat).
 */
template <typename  Type, class CFComplexType>
CORBA::Any ossie::convertComplexStringToAny(std::string value) {
    
    char sign = '+'; // sign represents + or -
    char j = 0; // j represents the letter j in the string
    Type A, B;     // A is the real value, B is the complex value
    A = 0;
    B = 0;

    CORBA::Any result;

    // Assuming a string of the format A+jB, parse out A and B.
    std::stringstream stream(value);
    stream >> A >> sign >> j >> B;

    if (value.size() > 1) {
        if (value[0] == 'j') {
            std::stringstream stream(value);
            stream >> j >> B;
        } else if ((value[0] == '-') and (value[1] == 'j')) {
            std::stringstream stream(value);
            stream >> sign >> j >> B;
        }
    }

    // if A-jB instead of A+jB, flip the sign of B
    if (sign == '-') {
        B *= -1;
    }

    if (value.find('j') == std::string::npos)
        B = 0;

    // Create a complex representation and convert it to a CORBA::any.
    CFComplexType cfComplex;
    cfComplex.real = A;
    cfComplex.imag = B;
    result <<= cfComplex;

    return result;
}

/*
 * Performs __MATH__ operations.
 *
 * Note: not supported for tk_struct, including complex values.
 */
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

    case CORBA::tk_ulonglong: {
        CORBA::ULongLong tmp1;
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

    case CORBA::tk_ulonglong: {
        result <<= static_cast<CORBA::ULongLong>(tmp_result);
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

/*
 * Convert a string in the form of A+jB to a CORBA::Any.
 *
 * This method differs from stringToSimpleAny in that the data type to
 * use is looked-up using the struct name (parameter(0)) of the tc_struct.
 *
 * Struct types that are not explicitly supported will cause the function to
 * return a new, empty CORBA::Any.
 */
CORBA::Any ossie::stringToComplexAny(std::string value, std::string structName) {
    CORBA::Any result;

    if (structName == "complexFloat"){
        result = convertComplexStringToAny<float, CF::complexFloat>(value);
    } else if (structName == "complexBoolean"){
        result = convertComplexStringToAny<bool, CF::complexBoolean>(value);
    } else if (structName == "complexULong"){
        result = convertComplexStringToAny<CORBA::ULong, CF::complexULong>(value);
    } else if (structName == "complexShort"){
        result = convertComplexStringToAny<short, CF::complexShort>(value);
    } else if (structName == "complexOctet"){
        result = convertComplexStringToAny<unsigned int, CF::complexOctet>(value);
    } else if (structName == "complexChar"){
        result = convertComplexStringToAny<int, CF::complexChar>(value);
    } else if (structName == "complexUShort"){
        result = convertComplexStringToAny<unsigned short, CF::complexUShort>(value);
    } else if (structName == "complexDouble"){
        result = convertComplexStringToAny<double, CF::complexDouble>(value);
    } else if (structName == "complexLong"){
        result = convertComplexStringToAny<CORBA::Long, CF::complexLong>(value);
    } else if (structName == "complexLongLong"){
        result = convertComplexStringToAny<CORBA::LongLong, CF::complexLongLong>(value);
    } else if (structName == "complexULongLong"){
        result = convertComplexStringToAny<CORBA::ULongLong, CF::complexULongLong>(value);
    } else {
        result = CORBA::Any();
    }
    return result;
}

namespace redhawk {
  namespace time {
    namespace utils {
        CF::UTCTime create( const double wholeSecs, const double fractionalSecs ) {
            double wsec = wholeSecs;
            double fsec = fractionalSecs;
            if ( wsec < 0.0 || fsec < 0.0 ) {
                struct timeval tmp_time;
                struct timezone tmp_tz;
                gettimeofday(&tmp_time, &tmp_tz);
                wsec = tmp_time.tv_sec;
                fsec = tmp_time.tv_usec / 1e6;
            }
            CF::UTCTime tstamp = CF::UTCTime();
            tstamp.tcstatus = 1;
            struct tm t = {0};
            tstamp.twsec = wsec + t.tm_gmtoff;
            tstamp.tfsec = fsec;
            return tstamp;
        }
        
        CF::UTCTime convert( const std::string formatted ) {
            if (formatted == "now") {
                return now();
            }
            unsigned int year;
            unsigned int month;
            unsigned int day;
            unsigned int hour;
            unsigned int minute;
            double second;
            int retval = sscanf(formatted.c_str(), "%d:%d:%d::%d:%d:%lf",&year,&month,&day,&hour,&minute,&second);
            if (retval != 6) {
                return notSet();
            }
            CF::UTCTime utctime;
            utctime.tcstatus=1;
            struct tm t = {0};
            t.tm_year = year - 1900;
            t.tm_mon = month - 1;
            t.tm_mday = day;
            t.tm_hour = hour;
            t.tm_min = minute;
            t.tm_sec = (int)second;
            utctime.twsec = mktime(&t) - timezone;
            utctime.tfsec = second - (int)second;
            return utctime;
        }
        
        std::string toString( const CF::UTCTime utc ) {
            struct tm time;
            time_t seconds = utc.twsec;
            std::ostringstream stream;
            gmtime_r(&seconds, &time);
            stream << (1900+time.tm_year) << ':';
            stream << std::setw(2) << std::setfill('0') << (time.tm_mon+1) << ':';
            stream << std::setw(2) << time.tm_mday << "::";
            stream << std::setw(2) << time.tm_hour << ":";
            stream << std::setw(2) << time.tm_min << ":";
            stream << std::setw(2) << time.tm_sec;
            int usec = round(utc.tfsec * 1000000.0);
            stream << "." << std::setw(6) << usec;
            return stream.str();
        }

        /*
         * Create a time stamp object from the current time of day reported by the system
         */
        CF::UTCTime now() {
            return create();
        }
      
        /*
         * Create a time stamp object from the current time of day reported by the system
         */
        CF::UTCTime notSet() {
            CF::UTCTime tstamp = CF::UTCTime();
            tstamp.tcstatus = 0;
            tstamp.twsec = 0.0;
            tstamp.tfsec = 0.0;
            return tstamp;
        }

        /*
         * Adjust the whole and fractional portions of a time stamp object to
         * ensure there is no fraction in the whole seconds, and vice-versa
         */
        void normalize(CF::UTCTime& time) {
            // Get fractional adjustment from whole seconds
            double fadj = std::modf(time.twsec, &time.twsec);

            // Adjust fractional seconds and get whole seconds adjustment
            double wadj = 0;
            time.tfsec = std::modf(time.tfsec + fadj, &wadj);

            // If fractional seconds are negative, borrow a second from the whole
            // seconds to make it positive, normalizing to [0,1)
            if (time.tfsec < 0.0) {
                time.tfsec += 1.0;
                wadj -= 1.0;
            }
            time.twsec += wadj;
        }
    }
  }
}

/*
 * Convert from a string to a simple CORBA::Any.
 *
 * The data type must be one of the simple, built-in CORBA::TCKinds (e.g., _tk_float).
 *
 * Simple types that are not explicitly supported will return an empty CORBA::Any.
 */
CORBA::Any ossie::stringToSimpleAny(std::string value, CORBA::TCKind kind) {
    CORBA::Any result;
    char *endptr = NULL;
    std::string _type;
    bool fixed_point = false;
    if (kind == CORBA::tk_boolean){
        _type = "boolean";
        if ((value == "true") || (value == "True") || (value == "TRUE") || (value == "1")) {
            result <<= CORBA::Any::from_boolean(CORBA::Boolean(true));
        } else {
            result <<= CORBA::Any::from_boolean(CORBA::Boolean(false));
        }
    } else if (kind == CORBA::tk_char){
        _type = "char";
        result <<= CORBA::Any::from_char(CORBA::Char(value[0]));
    } else if (kind == CORBA::tk_double){
        _type = "double";
        result <<= CORBA::Double(strtod(value.c_str(), &endptr));
    } else if (kind == CORBA::tk_octet){
        fixed_point = true;
        _type = "octet";
        result <<= CORBA::Any::from_octet(CORBA::Octet(strtol(value.c_str(), &endptr, 0)));
    } else if (kind == CORBA::tk_ushort){
        fixed_point = true;
        _type = "ushort";
        result <<= CORBA::UShort(strtol(value.c_str(), &endptr, 0));
    } else if (kind == CORBA::tk_short){
        fixed_point = true;
        _type = "short";
        result <<= CORBA::Short(strtol(value.c_str(), &endptr, 0));
    } else if (kind == CORBA::tk_float){
        _type = "float";
        result <<= CORBA::Float(strtof(value.c_str(), &endptr));
    } else if (kind == CORBA::tk_ulong){
        fixed_point = true;
        _type = "ulong";
        result <<= CORBA::ULong(strtol(value.c_str(), &endptr, 0));
    } else if (kind == CORBA::tk_long){
        fixed_point = true;
        _type = "long";
        result <<= CORBA::Long(strtol(value.c_str(), &endptr, 0));
    } else if (kind == CORBA::tk_longlong){
        fixed_point = true;
        _type = "longlong";
        result <<= CORBA::LongLong(strtoll(value.c_str(), &endptr, 0));
    } else if (kind == CORBA::tk_ulonglong){
        fixed_point = true;
        _type = "ulonglong";
        result <<= CORBA::ULongLong(strtoll(value.c_str(), &endptr, 0));
    } else if (kind == CORBA::tk_string){
        _type = "string";
        result <<= value.c_str();
    } else {
        _type = "any";
        result = CORBA::Any();
    } // end of outer switch statement

    if (endptr != NULL) {
        if ((value != "(null)") and ((endptr == value.c_str()) or (*endptr != '\0'))) {
            if (fixed_point) {
                strtod(value.c_str(), &endptr);
                if (*endptr == '\0')
                    return result;
            }
            throw ossie::badConversion(value, _type);
        }
    }
    return result;
}

/*
 * Convert from a string to a CORBA::Any for simple and complex types.
 *
 * Simple types are indexed by typeCode kind, while complex types are indexed
 * by struct name.
 */
CORBA::Any ossie::string_to_any(std::string value, CORBA::TypeCode_ptr type)
{
    CORBA::Any result;

    if (type->kind() == CORBA::tk_struct) {
        // Struct types are assumed to be complex values.  Struct types
        // that are not explicitly supported will cause the function to
        // return a new, empty CORBA::Any.
        std::string structName = any_to_string(*(type->parameter(0)));
        if (structName == std::string("UTCTime")) {
            result <<= redhawk::time::utils::convert(value);
        } else {
            result = stringToComplexAny(value, structName);
        }
    }
    else {
        result = stringToSimpleAny(value, type->kind());
    }

    return result;
}

CORBA::Any ossie::strings_to_any(const std::vector<std::string>& values, CORBA::TCKind kind)
{
    return strings_to_any(values, kind, NULL);
}

CORBA::Any ossie::strings_to_any(const std::vector<std::string>& values, CORBA::TCKind kind, CORBA::TypeCode_ptr type)
{
    CORBA::Any result;
    if (type != NULL) {
        if (type->kind() == CORBA::tk_struct) {
            std::string structName = any_to_string(*(type->parameter(0)));
            if (structName == std::string("UTCTime")) {
                result <<= strings_to_utctime_sequence(values);
                return result;
            }
        }
    }
    switch (kind) {
    case CORBA::tk_boolean:
        result <<= strings_to_boolean_sequence(values);
        return result;
    case CORBA::tk_char:
        result <<= strings_to_char_sequence(values);
        return result;
    case CORBA::tk_double:
        result <<= strings_to_double_sequence(values);
        return result;
    case CORBA::tk_octet:
        result <<= strings_to_octet_sequence(values);
        return result;
    case CORBA::tk_ushort:
        result <<= strings_to_unsigned_short_sequence(values);
        return result;
    case CORBA::tk_short:
        result <<= strings_to_short_sequence(values);
        return result;
    case CORBA::tk_float:
        result <<= strings_to_float_sequence(values);
        return result;
    case CORBA::tk_ulong:
        result <<= strings_to_unsigned_long_sequence(values);
        return result;
    case CORBA::tk_long:
        result <<= strings_to_long_sequence(values);
        return result;
    case CORBA::tk_longlong:
        result <<= strings_to_long_long_sequence(values);
        return result;
    case CORBA::tk_ulonglong:
        result <<= strings_to_unsigned_long_long_sequence(values);
        return result;
    case CORBA::tk_string:
        result <<= strings_to_string_sequence(values);
        break;

    default:
        result = CORBA::Any();
    }
    if (type != NULL) {
        std::string structName = any_to_string(*(type->parameter(0)));
        if (structName == "complexFloat"){
            result <<= strings_to_complex_float_sequence(values);
        } else if (structName == "complexBoolean"){
            result <<= strings_to_complex_boolean_sequence(values);
        } else if (structName == "complexULong"){
            result <<= strings_to_complex_unsigned_long_sequence(values);
        } else if (structName == "complexShort"){
            result <<= strings_to_complex_short_sequence(values);
        } else if (structName == "complexOctet"){
            result <<= strings_to_complex_octet_sequence(values);
        } else if (structName == "complexChar"){
            result <<= strings_to_complex_char_sequence(values);
        } else if (structName == "complexUShort"){
            result <<= strings_to_complex_unsigned_short_sequence(values);
        } else if (structName == "complexDouble"){
            result <<= strings_to_complex_double_sequence(values);
        } else if (structName == "complexLong"){
            result <<= strings_to_complex_long_sequence(values);
        } else if (structName == "complexLongLong"){
            result <<= strings_to_complex_long_long_sequence(values);
        } else if (structName == "complexULongLong"){
            result <<= strings_to_complex_unsigned_long_long_sequence(values);
        }
    }
    return result;
}

/*CORBA::Any ossie::strings_to_any(const std::vector<std::string>& values, CORBA::TypeCode_ptr type)
{
    CORBA::Any result;
    std::string structName = any_to_string(*(type->parameter(0)));
    if (structName == "complexFloat"){
        result <<= strings_to_complex_float_sequence(values);
    } else if (structName == "complexBoolean"){
        result <<= strings_to_complex_boolean_sequence(values);
    } else if (structName == "complexULong"){
        result <<= strings_to_complex_unsigned_long_sequence(values);
    } else if (structName == "complexShort"){
        result <<= strings_to_complex_short_sequence(values);
    } else if (structName == "complexOctet"){
        result <<= strings_to_complex_octet_sequence(values);
    } else if (structName == "complexChar"){
        result <<= strings_to_complex_char_sequence(values);
    } else if (structName == "complexUShort"){
        result <<= strings_to_complex_unsigned_short_sequence(values);
    } else if (structName == "complexDouble"){
        result <<= strings_to_complex_double_sequence(values);
    } else if (structName == "complexLong"){
        result <<= strings_to_complex_long_sequence(values);
    } else if (structName == "complexLongLong"){
        result <<= strings_to_complex_long_long_sequence(values);
    } else if (structName == "complexULongLong"){
        result <<= strings_to_complex_unsigned_long_long_sequence(values);
    } else {
        result = CORBA::Any();
    }

    return result;
}*/

/*
 * Convert a CORBA::Any to a string in the format A+jB.
 *
 * CFComplexType corresponds to the type of the CF::complexStruct (e.g.,
 * CF::complexLong or CF::complexFloat).
 */
template <class CFComplexType>
std::string ossie::convertComplexAnyToString(const CORBA::Any& value){

    std::ostringstream result;

    CFComplexType *tmpCFComplexType;
    value >>= tmpCFComplexType;

    if (tmpCFComplexType->imag < 0) {
        // format result as A-jB instead of a+j-B
        result << tmpCFComplexType->real << "-j" << tmpCFComplexType->imag * -1;
    }
    else {
        result << tmpCFComplexType->real << "+j" << tmpCFComplexType->imag;
    }

    return result.str();
}

std::string ossie::complexAnyToString(const CORBA::Any& value)
{
    std::string result;
    CORBA::TypeCode_var valueType = value.type();
    std::string structName = any_to_string(*(valueType->parameter(0)));
    if (structName == "complexFloat"){
         result = convertComplexAnyToString<CF::complexFloat>(value);
    } else if (structName == "complexBoolean"){
        result = convertComplexAnyToString<CF::complexBoolean>(value);
    } else if (structName == "complexULong"){
        result = convertComplexAnyToString<CF::complexULong>(value);
    } else if (structName == "complexShort"){
        result = convertComplexAnyToString<CF::complexShort>(value);
    } else if (structName == "complexOctet"){
        result = convertComplexAnyToString<CF::complexOctet>(value);
    } else if (structName == "complexChar"){
        result = convertComplexAnyToString<CF::complexChar>(value);
    } else if (structName == "complexUShort"){
        result = convertComplexAnyToString<CF::complexUShort>(value);
    } else if (structName == "complexDouble"){
        result = convertComplexAnyToString<CF::complexDouble>(value);
    } else if (structName == "complexLong"){
        result = convertComplexAnyToString<CF::complexLong>(value);
    } else if (structName == "complexLongLong"){
        result = convertComplexAnyToString<CF::complexLongLong>(value);
    } else if (structName == "complexULongLong"){
        result = convertComplexAnyToString<CF::complexULongLong>(value);
    } else {
        std::ostringstream tmp;
        tmp << "Kind: " << valueType;
        result = tmp.str();
    }
    return result;
}

std::string ossie::simpleAnyToString(const CORBA::Any& value)
{
    std::ostringstream result;
    CORBA::TypeCode_var typeValue = value.type();

    switch (typeValue->kind()) {
    case CORBA::tk_null:
        result << "(null)";
        break;

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

    case CORBA::tk_ulonglong: {
        CORBA::ULongLong tmp;
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

std::string ossie::any_to_string(const CORBA::Any& value)
{
    std::string result;

    CORBA::TypeCode_var valueType = value.type();
    if (valueType->equivalent(CF::_tc_Properties)) {
        result = redhawk::Value::cast(value).asProperties().toString();
    } else if (valueType->kind() == CORBA::tk_struct) {
        result = complexAnyToString(value);
    }
    else {
        result = simpleAnyToString(value);
    }
    return result;
}

std::vector<std::string> ossie::any_to_strings(const CORBA::Any& value)
{
    std::vector<std::string> retval;
    value >>= retval;
    return retval;
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

CF::complexBooleanSeq* ossie::strings_to_complex_boolean_sequence(const std::vector<std::string> &values)
{
    CF::complexBooleanSeq_var result = new CF::complexBooleanSeq;

    result->length(values.size());
    CF::complexBoolean *vtmp;
    CORBA::Any tmp;
    for (unsigned int i = 0; i < values.size(); ++i) {
        tmp = convertComplexStringToAny<bool, CF::complexBoolean>(values[i]);
        tmp >>= vtmp;
        result[i].imag = vtmp->imag;
        result[i].real = vtmp->real;
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

CF::complexCharSeq* ossie::strings_to_complex_char_sequence(const std::vector<std::string> &values)
{
    CF::complexCharSeq_var result = new CF::complexCharSeq;

    result->length(values.size());
    CF::complexChar *vtmp;
    CORBA::Any tmp;
    for (unsigned int i = 0; i < values.size(); ++i) {
        tmp = convertComplexStringToAny<int, CF::complexChar>(values[i]);
        tmp >>= vtmp;
        result[i].imag = vtmp->imag;
        result[i].real = vtmp->real;
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

CF::complexDoubleSeq* ossie::strings_to_complex_double_sequence(const std::vector<std::string> &values)
{
    CF::complexDoubleSeq_var result = new CF::complexDoubleSeq;

    result->length(values.size());
    CF::complexDouble *vtmp;
    CORBA::Any tmp;
    for (unsigned int i = 0; i < values.size(); ++i) {
        tmp = convertComplexStringToAny<double, CF::complexDouble>(values[i]);
        tmp >>= vtmp;
        result[i].imag = vtmp->imag;
        result[i].real = vtmp->real;
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

CF::complexFloatSeq* ossie::strings_to_complex_float_sequence(const std::vector<std::string> &values)
{
    CF::complexFloatSeq_var result = new CF::complexFloatSeq;

    result->length(values.size());

    CF::complexFloat *vtmp;
    CORBA::Any tmp;
    for (unsigned int i = 0; i < values.size(); ++i) {
        tmp = convertComplexStringToAny<float, CF::complexFloat>(values[i]);
        tmp >>= vtmp;
        result[i].imag = vtmp->imag;
        result[i].real = vtmp->real;
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

CF::complexShortSeq* ossie::strings_to_complex_short_sequence(const std::vector<std::string> &values)
{
    CF::complexShortSeq_var result = new CF::complexShortSeq;

    result->length(values.size());

    CF::complexShort*vtmp;
    CORBA::Any tmp;
    for (unsigned int i = 0; i < values.size(); ++i) {
        tmp = convertComplexStringToAny<short, CF::complexShort>(values[i]);
        tmp >>= vtmp;
        result[i].imag = vtmp->imag;
        result[i].real = vtmp->real;
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

CF::complexLongSeq* ossie::strings_to_complex_long_sequence(const std::vector<std::string> &values)
{
    CF::complexLongSeq_var result = new CF::complexLongSeq;

    result->length(values.size());

    CF::complexLong *vtmp;
    CORBA::Any tmp;
    for (unsigned int i = 0; i < values.size(); ++i) {
        tmp = convertComplexStringToAny<CORBA::Long, CF::complexLong>(values[i]);
        tmp >>= vtmp;
        result[i].imag = vtmp->imag;
        result[i].real = vtmp->real;
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

CF::complexLongLongSeq* ossie::strings_to_complex_long_long_sequence(const std::vector<std::string> &values)
{
    CF::complexLongLongSeq_var result = new CF::complexLongLongSeq;

    result->length(values.size());

    CF::complexLongLong *vtmp;
    CORBA::Any tmp;
    for (unsigned int i = 0; i < values.size(); ++i) {
        tmp = convertComplexStringToAny<CORBA::LongLong, CF::complexLongLong>(values[i]);
        tmp >>= vtmp;
        result[i].imag = vtmp->imag;
        result[i].real = vtmp->real;
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

CF::complexULongLongSeq* ossie::strings_to_complex_unsigned_long_long_sequence(const std::vector<std::string> &values)
{
    CF::complexULongLongSeq_var result = new CF::complexULongLongSeq;

    result->length(values.size());

    CF::complexULongLong *vtmp;
    CORBA::Any tmp;
    for (unsigned int i = 0; i < values.size(); ++i) {
        tmp = convertComplexStringToAny<CORBA::ULongLong, CF::complexULongLong>(values[i]);
        tmp >>= vtmp;
        result[i].imag = vtmp->imag;
        result[i].real = vtmp->real;
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

CF::complexOctetSeq* ossie::strings_to_complex_octet_sequence(const std::vector<std::string> &values)
{
    CF::complexOctetSeq_var result = new CF::complexOctetSeq;

    result->length(values.size());

    CF::complexOctet *vtmp;
    CORBA::Any tmp;
    for (unsigned int i = 0; i < values.size(); ++i) {
        tmp = convertComplexStringToAny<unsigned int, CF::complexOctet>(values[i]);
        tmp >>= vtmp;
        result[i].imag = vtmp->imag;
        result[i].real = vtmp->real;
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

CF::complexUShortSeq* ossie::strings_to_complex_unsigned_short_sequence(const std::vector<std::string> &values)
{
    CF::complexUShortSeq_var result = new CF::complexUShortSeq;

    result->length(values.size());

    CF::complexUShort *vtmp;
    CORBA::Any tmp;
    for (unsigned int i = 0; i < values.size(); ++i) {
        tmp = convertComplexStringToAny<unsigned short, CF::complexUShort>(values[i]);
        tmp >>= vtmp;
        result[i].imag = vtmp->imag;
        result[i].real = vtmp->real;
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

CF::complexULongSeq* ossie::strings_to_complex_unsigned_long_sequence(const std::vector<std::string> &values)
{
    CF::complexULongSeq_var result = new CF::complexULongSeq;

    result->length(values.size());

    CF::complexULong *vtmp;
    CORBA::Any tmp;
    for (unsigned int i = 0; i < values.size(); ++i) {
        tmp = convertComplexStringToAny<CORBA::ULong, CF::complexULong>(values[i]);
        tmp >>= vtmp;
        result[i].imag = vtmp->imag;
        result[i].real = vtmp->real;
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

CF::UTCTimeSequence* ossie::strings_to_utctime_sequence(const std::vector<std::string> &values)
{
    CF::UTCTimeSequence_var result = new CF::UTCTimeSequence;

    result->length(values.size());

    for (unsigned int i = 0; i < values.size(); ++i) {
        result[i] = redhawk::time::utils::convert(values[i].c_str());
    }

    return result._retn();
}

/*
 * Get the TypeCode kind based on a string.  Note that complex
 * types are not supported, as the TypeCode kind for all complex
 * types is tk_struct.  For complex types, use getTypeCode and
 * compare the StructName (any_to_string(TypeCode->parameter(0))).
 */
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
    } else if (type == "octet") {
        kind = CORBA::tk_octet;
    } else if (type == "ushort") {
        kind = CORBA::tk_ushort;
    } else if (type == "ulong") {
        kind = CORBA::tk_ulong;
    } else if (type == "ulonglong"){
        kind = CORBA::tk_ulonglong;
    } else if (type == "string") {
        kind = CORBA::tk_string;
    } else {
        kind = CORBA::tk_null;
    }
    return kind;
}

CORBA::TypeCode_ptr ossie::getTypeCode(std::string type) {

    CORBA::TypeCode_ptr kind;
    if (type == "boolean") {
        kind = CORBA::_tc_boolean;
    } else if (type == "char") {
        kind = CORBA::_tc_char;
    } else if (type == "double") {
        kind = CORBA::_tc_double;
    } else if (type == "float") {
        kind = CORBA::_tc_float;
    } else if (type == "short") {
        kind = CORBA::_tc_short;
    } else if (type == "long") {
        kind = CORBA::_tc_long;
    } else if (type == "longlong") {
        kind = CORBA::_tc_longlong;
    } else if (type == "octet") {
        kind = CORBA::_tc_octet;
    } else if (type == "ushort") {
        kind = CORBA::_tc_ushort;
    } else if (type == "ulong") {
        kind = CORBA::_tc_ulong;
    } else if (type == "ulonglong"){
        kind = CORBA::_tc_ulonglong;
    } else if (type == "string"){
        kind = CORBA::_tc_string;
    } else if (type == "utctime"){
        kind = CF::_tc_UTCTime;
    } else if (type == "complexDouble") {
        kind = CF::_tc_complexDouble;
    } else if (type == "complexFloat") {
        kind = CF::_tc_complexFloat;
    } else if (type == "complexShort") {
        kind = CF::_tc_complexShort;
    } else if (type == "complexUShort") {
        kind = CF::_tc_complexUShort;
    } else if (type == "complexOctet") {
        kind = CF::_tc_complexOctet;
    } else if (type == "complexChar") {
        kind = CF::_tc_complexChar;
    } else if (type == "complexBoolean") {
        kind = CF::_tc_complexBoolean;
    } else if (type == "complexLong") {
        kind = CF::_tc_complexLong;
    } else if (type == "complexLongLong") {
        kind = CF::_tc_complexLongLong;
    } else if (type == "complexULong") {
        kind = CF::_tc_complexULong;
    } else if (type == "complexULongLong") {
        kind = CF::_tc_complexULongLong;
    } else {
        kind = CORBA::_tc_null;
    }
    return kind;
}

/*
 * Convert from a kind to a CORBA TypeCode.
 *
 * The structName argument is provided because kind needs to be further described to map to a specific type code
 */
CORBA::TypeCode_ptr ossie::getTypeCode(CORBA::TCKind kind, std::string structName) {

    CORBA::TypeCode_ptr typecode;
    if (kind == CORBA::tk_boolean) {
    	typecode = CORBA::_tc_boolean;
    } else if (kind == CORBA::tk_char) {
    	typecode = CORBA::_tc_char;
    } else if (kind == CORBA::tk_double) {
    	typecode = CORBA::_tc_double;
    } else if (kind == CORBA::tk_float) {
    	typecode = CORBA::_tc_float;
    } else if (kind == CORBA::tk_short) {
    	typecode = CORBA::_tc_short;
    } else if (kind == CORBA::tk_long) {
    	typecode = CORBA::_tc_long;
    } else if (kind == CORBA::tk_longlong) {
    	typecode = CORBA::_tc_longlong;
    } else if (kind == CORBA::tk_octet) {
    	typecode = CORBA::_tc_octet;
    } else if (kind == CORBA::tk_ushort) {
    	typecode = CORBA::_tc_ushort;
    } else if (kind == CORBA::tk_ulong) {
    	typecode = CORBA::_tc_ulong;
    } else if (kind == CORBA::tk_ulonglong){
    	typecode = CORBA::_tc_ulonglong;
    } else if (kind == CORBA::tk_string){
    	typecode = CORBA::_tc_string;
    } else if (kind == CORBA::tk_struct) {
        if (structName == "complexDouble") {
            typecode = CF::_tc_complexDouble;
        } else if (structName == "complexFloat") {
            typecode = CF::_tc_complexFloat;
        } else if (structName == "complexShort") {
            typecode = CF::_tc_complexShort;
        } else if (structName == "complexUShort") {
            typecode = CF::_tc_complexUShort;
        } else if (structName == "complexOctet") {
            typecode = CF::_tc_complexOctet;
        } else if (structName == "complexChar") {
            typecode = CF::_tc_complexChar;
        } else if (structName == "complexBoolean") {
            typecode = CF::_tc_complexBoolean;
        } else if (structName == "complexLong") {
            typecode = CF::_tc_complexLong;
        } else if (structName == "complexLongLong") {
            typecode = CF::_tc_complexLongLong;
        } else if (structName == "complexULong") {
            typecode = CF::_tc_complexULong;
        } else if (structName == "complexULongLong") {
            typecode = CF::_tc_complexULongLong;
        } else {
        	typecode = CORBA::_tc_null;
        }
    } else {
    	typecode = CORBA::_tc_null;
	}

    return typecode;
}


CF::Properties ossie::getNonNilProperties(const CF::Properties& originalProperties)
{
    redhawk::PropertyMap nonNilProperties;
    const redhawk::PropertyMap& properties = redhawk::PropertyMap::cast(originalProperties);

    for (redhawk::PropertyMap::const_iterator prop = properties.begin(); prop != properties.end(); ++prop) {
        if (!prop->getValue().isNil()) {
            nonNilProperties.push_back(*prop);
        }
    }
    return nonNilProperties;
}

CF::Properties ossie::getNonNilConfigureProperties(const CF::Properties& originalProperties)
{
  return getNonNilProperties(originalProperties);
}
