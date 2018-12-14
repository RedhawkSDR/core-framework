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

#include<sstream>
#include"ossie/Properties.h"
#include"internal/prf-parser.h"
#include"ossie/ossieparser.h"
#include <ossie/componentProfile.h>

using namespace ossie;

PREPARE_CF_LOGGING(Properties)
PREPARE_CF_LOGGING(PRF)
PREPARE_CF_LOGGING(Property)
PREPARE_CF_LOGGING(SimpleProperty)
PREPARE_CF_LOGGING(SimpleSequenceProperty)
PREPARE_CF_LOGGING(StructProperty)
PREPARE_CF_LOGGING(StructSequenceProperty)

/*
 * PRF class
 */
void PRF::addProperty(const Property* p) throw (ossie::parser_error)
{
    LOG_TRACE(PRF, "Adding property " << p->getID() << " " << p);
    assert(p != 0);
    std::map<std::string, const Property*>::iterator v = _properties.end(); //_properties.find(p->getID());
    if (v == _properties.end()) {
        // Add the property
        _properties[p->getID()] = p;
        _allProperties.push_back(p);

        if (p->isAllocation ()) {
            _allocationProperties.push_back(p);
        }
        if (p->isConfigure ()) {
            _configProperties.push_back(p);
        }
        if (p->isProperty ()) {
            _ctorProperties.push_back(p);
        }
        if (p->isExecParam ()) {
            _execProperties.push_back(p);
        }
        if (p->isFactoryParam ()) {
            _factoryProperties.push_back(p);
        }
    } else {
        std::ostringstream emsg;
        emsg << "Duplicate property id " << p->getID();
        throw ossie::parser_error(emsg.str()); 
    }
}

/*
 * Properties class
 */
Properties::Properties()
{
    _prf.reset(new ossie::PRF());
}

Properties::Properties(std::istream& input) throw(ossie::parser_error) 
{
    LOG_TRACE(Properties, "Constructing properties")
    load(input);
}

Properties::~Properties()
{
    LOG_TRACE(Properties, "Destruction for properties")
}

void Properties::load(std::istream& input) throw (ossie::parser_error) {
  std::auto_ptr<ossie::PRF> t = ossie::internalparser::parsePRF(input);
  _prf.reset(t.release());
}

void Properties::join(std::istream& input) throw (ossie::parser_error) {
    LOG_TRACE(Properties, "Loading property set")
      std::auto_ptr<ossie::PRF> _joinedprf = ossie::internalparser::parsePRF(input);
    if (_prf.get() == 0) {
        LOG_TRACE(Properties, "No initial load, using join set for properties")
        _prf.reset(_joinedprf.release());
    } else {
        LOG_TRACE(Properties, "Merging property sets")
        std::vector<const Property*>::iterator jp_iter;
        for (jp_iter = _joinedprf->_allProperties.begin(); jp_iter != _joinedprf->_allProperties.end(); ++jp_iter) {
            std::map<std::string, const Property*>::iterator p;
            assert(*jp_iter != 0);
            const Property* jp = *jp_iter;

            p = _prf->_properties.find(jp->getID());
            LOG_TRACE(Properties, "Merging '" << *jp << "'") 

            // The property exists so override it's value
            if (p != _prf->_properties.end()) {
                Property* thep = const_cast<Property*>(p->second);
                assert(thep != 0);
                // override the value
                if (thep->isReadOnly()) {
                    LOG_WARN(Properties, "ignoring attempt to override readonly property with id: " << thep->getID());
                }
                LOG_TRACE(Properties, "Overriding '" << *thep << "' with '" << *jp << "'") 
                thep->override(jp);
                LOG_TRACE(Properties, "New value '" << *thep << "' with '") 
            } else {
                LOG_TRACE(Properties, "Adding '" << *jp << "'") 
                _prf->addProperty(jp->clone());
            }
        }
        LOG_TRACE(Properties, "Done merging property sets, cleaning up")
    }
}

void Properties::join(ossie::Properties& props) throw (ossie::parser_error) {
    LOG_TRACE(Properties, "Loading property set")
    LOG_TRACE(Properties, "Merging property sets")
    std::vector<const Property*>::iterator jp_iter;
    for (jp_iter = props._prf->_allProperties.begin(); jp_iter != props._prf->_allProperties.end(); ++jp_iter) {
        std::map<std::string, const Property*>::iterator p;
        assert(*jp_iter != 0);
        const Property* jp = *jp_iter;

        p = _prf->_properties.find(jp->getID());
        LOG_TRACE(Properties, "Merging '" << *jp << "'") 

        // The property exists so override it's value
        if (p != _prf->_properties.end()) {
            Property* thep = const_cast<Property*>(p->second);
            assert(thep != 0);
            // override the value
            if (thep->isReadOnly()) {
                LOG_WARN(Properties, "ignoring attempt to override readonly property with id: " << thep->getID());
            }
            LOG_TRACE(Properties, "Overriding '" << *thep << "' with '" << *jp << "'") 
            thep->override(jp);
            LOG_TRACE(Properties, "New value '" << *thep << "' with '") 
        } else {
            LOG_TRACE(Properties, "Adding '" << *jp << "'") 
            _prf->addProperty(jp->clone());
        }
    }
    LOG_TRACE(Properties, "Done merging property sets, cleaning up")
}

void Properties::override(const ossie::ComponentPropertyList & values)
{
    for (ossie::ComponentPropertyList::const_iterator iter = values.begin(); iter != values.end(); ++iter) {
        const ComponentProperty* new_value = &(*iter);
        const Property* property = const_cast<Properties*>(this)->getProperty(new_value->getID());
        if (!property) {
            LOG_TRACE(Properties, "Skipping override of non-existent property " << new_value->getID());
        } else {
            const_cast<Property*>(property)->override(new_value);
        }
    }
}

const std::vector<const Property*>& Properties::getProperties() const
{
    assert(_prf.get() != 0);
    return _prf->_allProperties;
}

const Property* Properties::getProperty(const std::string& id) const
{
    assert(_prf.get() != 0);
    std::map<std::string, const Property*>::iterator p = _prf->_properties.find(id);
    if (p != _prf->_properties.end()) {
        return p->second;
    } else {
        return 0;
    }
}

const std::vector<const Property*>& Properties::getConfigureProperties() const
{
    assert(_prf.get() != 0);
    return _prf->_configProperties;
}


const std::vector<const Property*>& Properties::getConstructProperties() const
{
    assert(_prf.get() != 0);
    return _prf->_ctorProperties;
}

const std::vector<const Property*>& Properties::getAllocationProperties() const
{
    assert(_prf.get() != 0);
    return _prf->_allocationProperties;
}

const Property* Properties::getAllocationProperty(const std::string& id)
{
    const Property* property = getProperty(id);
    if (!property || !property->isAllocation()) {
        return 0;
    }
    return property;
}

const std::vector<const Property*>& Properties::getExecParamProperties() const
{
    assert(_prf.get() != 0);
    return _prf->_execProperties;
}

const std::vector<const Property*>& Properties::getFactoryParamProperties() const
{
    assert(_prf.get() != 0);
    return _prf->_factoryProperties;
}

/*
 * Property class
 */

std::ostream& ossie::operator<<(std::ostream& stream, const ossie::Property& property)
{
    stream << property.asString();
    return stream;
}

std::ostream& ossie::operator<<(std::ostream& stream, ossie::Property::KindType kind)
{
    switch (kind) {
    case Property::KIND_CONFIGURE:
        stream << "configure";
        break;
    case Property::KIND_EXECPARAM:
        stream << "execparam";
        break;
    case Property::KIND_ALLOCATION:
        stream << "allocation";
        break;
    case Property::KIND_FACTORYPARAM:
        stream << "factoryparam";
        break;
    case Property::KIND_TEST:
        stream << "test";
        break;
    case Property::KIND_EVENT:
        stream << "event";
        break;
    case Property::KIND_MESSAGE:
        stream << "message";
        break;
    case Property::KIND_PROPERTY:
        stream << "property";
        break;
    default:
        break;
    }
    return stream;
}

std::ostream& ossie::operator<<(std::ostream& stream, ossie::Property::Kinds kinds)
{
    for (int bit = 1; bit <= Property::KIND_PROPERTY; bit <<= 1) {
        Property::KindType flag = static_cast<Property::KindType>(bit);
        if (kinds & flag) {
            stream << flag << ",";
        }
    }
    return stream;
}

std::ostream& ossie::operator<<(std::ostream& stream, ossie::Property::ActionType action)
{
    switch (action) {
    case ossie::Property::ACTION_GE:
        stream << "ge";
        break;
    case ossie::Property::ACTION_GT:
        stream << "gt";
        break;
    case ossie::Property::ACTION_LE:
        stream << "le";
        break;
    case ossie::Property::ACTION_LT:
        stream << "lt";
        break;
    case ossie::Property::ACTION_NE:
        stream << "ne";
        break;
    case ossie::Property::ACTION_EQ:
        stream << "eq";
        break;
    case ossie::Property::ACTION_EXTERNAL:
        stream << "external";
        break;
    }
    return stream;
}

std::ostream& ossie::operator<<(std::ostream& stream, ossie::Property::AccessType mode)
{
    switch (mode) {
    case ossie::Property::MODE_READWRITE:
        stream << "readwrite";
        break;
    case ossie::Property::MODE_READONLY:
        stream << "readonly";
        break;
    case ossie::Property::MODE_WRITEONLY:
        stream << "writeonly";
        break;
    }
    return stream;
}

Property::Property(const std::string& id, 
                   const std::string& name, 
                   AccessType mode, 
                   ActionType action, 
                   Kinds kinds):
    id(id),
    name(name),
    mode(mode),
    action(action),
    kinds((!kinds)?KIND_DEFAULT:kinds)
{
}

Property::~Property()
{
}

bool Property::isAllocation() const
{
    return kinds & KIND_ALLOCATION;
}

bool Property::isConfigure() const
{
    return kinds & KIND_CONFIGURE;
}

bool Property::isProperty() const
{
    return kinds & KIND_PROPERTY;
}

bool Property::isTest() const
{
    return kinds & KIND_TEST;
}

bool Property::isExecParam() const
{
    return kinds & KIND_EXECPARAM;
}

bool Property::isFactoryParam() const
{
    return kinds & KIND_FACTORYPARAM;
}

const char* Property::getID() const
{
    return id.c_str();
}

const char* Property::getName() const
{
    return name.c_str();
}

Property::AccessType Property::getMode() const
{
    return mode;
}

std::string Property::getAction() const
{
    std::ostringstream out;
    out << action;
    return out.str();
}

Property::Kinds Property::getKinds() const
{
    return kinds;
}

bool Property::isReadOnly() const
{
    return (mode == MODE_READONLY);
}

bool Property::isCommandLine() const
{
    return false;
}

bool Property::isReadWrite() const
{
    return (mode == MODE_READWRITE);
}

bool Property::isWriteOnly() const
{
    return (mode == MODE_WRITEONLY);
}

bool Property::canOverride() const
{
    // Only allow overrides for writable or 'property' kind properties
    if (isProperty()) {
        return true;
    } else {
        return !isReadOnly();
    }
}

bool Property::isEqual() const
{
    return (action == ACTION_EQ);
}

bool Property::isNotEqual() const
{
    return (action == ACTION_NE);
}

bool Property::isGreaterThan() const
{
    return (action == ACTION_GT);
}

bool Property::isLessThan() const
{
    return (action == ACTION_LT);
}

bool Property::isGreaterThanOrEqual() const
{
    return (action == ACTION_GE);
}

bool Property::isLessThanOrEqual() const
{
    return (action == ACTION_LE);
}

bool Property::isExternal() const
{
    return (action == ACTION_EXTERNAL);
}

std::string Property::mapPrimitiveToComplex(const std::string& type) const 
{
    std::string newType = type;
    if (type.compare("float") == 0) {
        newType = "complexFloat";
    } else if (type.compare("double") == 0) {
        newType = "complexDouble";
    } else if (type.compare("char") == 0) {
        newType = "complexChar";
    } else if (type.compare("octet") == 0) {
        newType = "complexOctet";
    } else if (type.compare("boolean") == 0) {
        newType = "complexBoolean";
    } else if (type.compare("short") == 0) {
        newType = "complexShort";
    } else if (type.compare("ushort") == 0) {
        newType = "complexUShort";
    } else if (type.compare("long") == 0) {
        newType = "complexLong";
    } else if (type.compare("longlong") == 0) {
        newType = "complexLongLong";
    } else if (type.compare("ulong") == 0) {
        newType = "complexULong";
    } else if (type.compare("ulonglong") == 0) {
        newType = "complexULongLong";
    }
    /*
     * else do nothing.  Either the primitive to complex
     * conversion has already been completed, or there
     * is an invalid type.  In the event of an invalid"const ossie::SPD::SoftPkgRef&"; 
     * type, let downstream checks catch the error.
     */ 

    return newType;
}

/*
 * SimpleProperty class
 */
SimpleProperty::SimpleProperty(const std::string& id, 
                               const std::string& name, 
                               const std::string& type, 
                               AccessType mode, 
                               ActionType action, 
                               Kinds kinds,
                               const optional_value<std::string>& value, 
                               bool complex,
                               bool commandline,
                               bool optional) :
  Property(id, name, mode, action, kinds),
  value(value), 
  complex(complex),
  commandline(commandline),
  optional(optional)
{
    if (complex) {
        /* 
         * Downstream processing expects complex types
         * (e.g., complexLong) rather than primitive 
         * types (e.g., long).
         */
        if (type.find("complex") != std::string::npos) {
            this->type = type;
        } else {
            this->type = mapPrimitiveToComplex(type);
        }
    } else {
        this->type = type;
    }
}

SimpleProperty::~SimpleProperty()
{
}


bool SimpleProperty::isCommandLine() const
{
    return commandline;
}

bool SimpleProperty::isNone() const {
    return !value.isSet();
}

void SimpleProperty::override(const Property* otherProp) {
    SimpleProperty* otherSimpleProp = const_cast<SimpleProperty*>(dynamic_cast<const SimpleProperty*>(otherProp));
    if (otherSimpleProp != NULL) {
        value = otherSimpleProp->value;
    } else {
        LOG_WARN(SimpleProperty, "Ignoring override request, simple properties can only override simple properties")
    }
}

void SimpleProperty::override(const ComponentProperty* newValue) {
    const SimplePropertyRef* simpleRef = dynamic_cast<const SimplePropertyRef*>(newValue);
    if (simpleRef) {
        value = simpleRef->getValue();
    } else {
        LOG_WARN(SimpleProperty, "Ignoring override request, simple properties can only override simple properties");
    }
}

const std::string& SimpleProperty::getType() const
{
    return type;
}

bool SimpleProperty::isComplex() const
{
    return complex;
}

bool SimpleProperty::isOptional() const
{
    return optional;
}

const char* SimpleProperty::getValue() const
{
    if (value.isSet()) {
        return value->c_str();
    } else {
        return NULL;
    }
}

const std::string SimpleProperty::asString() const {
    std::ostringstream out;
    out << "Simple Property: <'" << this->id << "' '" << this->name << " " << this->mode << " " << this->type << " '";
    out << kinds << "' ";
    if (value.isSet()) {
        out << " = '" << *(this->value) << "'>";
    }
    return out.str();
}

Property* SimpleProperty::clone() const {
    return new SimpleProperty(*this);
}


/*
 * SimpleSequenceProperty class
 */
SimpleSequenceProperty::SimpleSequenceProperty(const std::string&              id, 
                                               const std::string&              name, 
                                               const std::string&              type, 
                                               AccessType                      mode, 
                                               ActionType                      action, 
                                               Kinds                           kinds,
                                               const std::vector<std::string>& values,
                                               bool                            complex,
                                               bool                            optional) :
    Property(id, name, mode, action, kinds), 
    type(type), 
    values(values),
    complex(complex),
    optional(optional)
{
    if (complex) {
        /* 
         * Downstream processing expects complex types
         * (e.g., complexLong) rather than primitive 
         * types (e.g., long).
         */
         this->type = mapPrimitiveToComplex(type);
    } else {
        this->type = type;
    }
}

SimpleSequenceProperty::~SimpleSequenceProperty()
{
}

bool SimpleSequenceProperty::isNone() const {
    return (values.size() == 0);
}

void SimpleSequenceProperty::override(const Property* otherProp) {
    const SimpleSequenceProperty* otherSimpleSequenceProp = dynamic_cast<const SimpleSequenceProperty*>(otherProp);
    if (otherSimpleSequenceProp != NULL) {
        values.clear();
        std::copy(otherSimpleSequenceProp->values.begin(), otherSimpleSequenceProp->values.end(), std::back_inserter(values));
    } else {
        LOG_WARN(SimpleSequenceProperty, "Ignoring override request")
    }
}

void SimpleSequenceProperty::override(const ComponentProperty* newValue) {
    const SimpleSequencePropertyRef* simpleSequenceRef = dynamic_cast<const SimpleSequencePropertyRef*>(newValue);
    if (simpleSequenceRef) {
        values = simpleSequenceRef->getValues();
    } else {
        LOG_WARN(SimpleSequenceProperty, "Ignoring override request");
    }
}

const std::string& SimpleSequenceProperty::getType() const
{
    return type;
}

bool SimpleSequenceProperty::isComplex() const
{
    return complex;
}

bool SimpleSequenceProperty::isOptional() const
{
    return optional;
}

const std::vector<std::string>& SimpleSequenceProperty::getValues() const
{
    return values;
}

const std::string SimpleSequenceProperty::asString() const {
    std::ostringstream out;
    out << "Simple Sequence Property: '" << this->id << "' '" << this->name << "'";
    std::vector<std::string>::const_iterator i;
    for (i = values.begin(); i != values.end(); ++i) {
        out << " '" << *i << "' ";
    }
    return out.str();
}

Property* SimpleSequenceProperty::clone() const {
    return new SimpleSequenceProperty(*this);
}

/*
 * StructProperty class
 */
StructProperty::~StructProperty()
{
}

bool StructProperty::isNone() const {
    // it is not possible to set only one of the structure values
    if (value.size() > 0)
        return (value[0].isNone());
    else
        return true;
}

void StructProperty::override(const Property* otherProp) {
    const StructProperty* otherStructProp = dynamic_cast<const StructProperty*>(otherProp);
    if (otherStructProp != NULL) {
        value = otherStructProp->getValue();
    } else {
        LOG_WARN(StructProperty, "Ignoring override request")
    }
}

void StructProperty::override(const ComponentProperty* newValue) {
    const StructPropertyRef* structRef = dynamic_cast<const StructPropertyRef*>(newValue);
    if (structRef) {
    } else {
        LOG_WARN(StructProperty, "Ignoring override request");
    }
}

const std::string StructProperty::asString() const {
    std::ostringstream out;
    out << "'" << this->id << "' '" << this->name;
    for (PropertyList::const_iterator i = value.begin(); i != value.end(); ++i) {
        out << "   " << *i << std::endl;
    }
    return out.str();
}

Property* StructProperty::clone() const {
    return new StructProperty(*this);
};

const ossie::PropertyList& StructProperty::getValue() const {
    return value;
}

const Property* StructProperty::getField(const std::string& fieldId) const {
    for (PropertyList::const_iterator field = value.begin(); field !=value.end(); ++field) {
        if (fieldId == field->getID()) {
            return &(*field);
        }
    }
    return 0;
}

/*
 * StructSequenceProperty class
 */
StructSequenceProperty::~StructSequenceProperty()
{
}

bool StructSequenceProperty::isNone() const {
    return (values.size() == 0);
}

void StructSequenceProperty::override(const Property* otherProp) {
    const StructSequenceProperty* otherStructSeqProp = dynamic_cast<const StructSequenceProperty*>(otherProp);
    if (otherStructSeqProp) {
        values = otherStructSeqProp->values;
    } else {
        LOG_WARN(StructSequenceProperty, "Ignoring override request");
    }
}

void StructSequenceProperty::override(const ComponentProperty* newValue) {
    const StructSequencePropertyRef* structSeqRef = dynamic_cast<const StructSequencePropertyRef*>(newValue);
    if (structSeqRef) {
    } else {
        LOG_WARN(StructSequenceProperty, "Ignoring override request");
    }
}

const std::string StructSequenceProperty::asString() const {
    std::ostringstream out;
    out << "'" << this->id << "' '" << this->name;
    for (std::vector<StructProperty>::const_iterator ii = values.begin(); ii != values.end(); ++ii) {
        out << "{" << std::endl;
        out << ii->asString();
        out << "}" << std::endl;
    }
    return out.str();
}

Property* StructSequenceProperty::clone() const {
    return new StructSequenceProperty(id, name, mode, structdef, kinds, values);
}

const StructProperty& StructSequenceProperty::getStruct() const {
    return structdef;
}

const std::vector<StructProperty>& StructSequenceProperty::getValues() const {
    return values;
}
