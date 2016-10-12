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

using namespace ossie;

PREPARE_LOGGING(Properties)
PREPARE_LOGGING(PRF)
PREPARE_LOGGING(Property)
PREPARE_LOGGING(SimpleProperty)
PREPARE_LOGGING(SimpleSequenceProperty)
PREPARE_LOGGING(StructProperty)
PREPARE_LOGGING(StructSequenceProperty)

/**
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

/**
 * Properties class
 */
Properties::Properties()
{
    _prf.reset(new ossie::PRF());
}

Properties::Properties(std::istream& input) throw(ossie::parser_error) : _prf(0)
{
    LOG_TRACE(Properties, "Constructing properties")
    load(input);
}

Properties::~Properties()
{
    LOG_TRACE(Properties, "Destruction for properties")
}

Properties& Properties::operator=(Properties other) {
    // IMPORTANT, pass 'other' by value to avoid having to explictly make a copy
    this->_prf = other._prf;
    return *this;
}


void Properties::load(std::istream& input) throw (ossie::parser_error) {
    _prf = ossie::internalparser::parsePRF(input);
}

void Properties::join(std::istream& input) throw (ossie::parser_error) {
    LOG_TRACE(Properties, "Loading property set")
    std::auto_ptr<ossie::PRF> _joinedprf = ossie::internalparser::parsePRF(input);
    if (_prf.get() == 0) {
        LOG_TRACE(Properties, "No initial load, using join set for properties")
        _prf = _joinedprf;
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

const std::vector<const Property*>& Properties::getProperties() const
{
    assert(_prf.get() != 0);
    return _prf->_allProperties;
}

const Property* Properties::getProperty(const std::string& id)
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

const std::vector<const Property*>& Properties::getAllocationProperties() const
{
    assert(_prf.get() != 0);
    return _prf->_allocationProperties;
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

/**
 * Property class
 */
Property::~Property()
{
} 

bool Property::isAllocation() const
{
    TRACE_ENTER(Property);

    for (unsigned int i = 0; i < kinds.size (); i++) {
        if (kinds[i] == "allocation")
            { return true; }
    }

    return false;
}

bool Property::isConfigure() const
{
    TRACE_ENTER(Property);

    if (kinds.size() == 0) {
        return true;
    }
    for (unsigned int i = 0; i < kinds.size (); i++) {
        if (kinds[i] == "configure")
            { return true; }
    }

    return false;
}

bool Property::isTest() const
{
    TRACE_ENTER(Property);

    for (unsigned int i = 0; i < kinds.size (); i++) {
        if (kinds[i] == "test")
            { return true; }
    }

    return false;
}

bool Property::isExecParam() const
{
    TRACE_ENTER(Property);

    for (unsigned int i = 0; i < kinds.size (); i++) {
        if (kinds[i] == "execparam")
            { return true; }
    }

    return false;
}

bool Property::isFactoryParam() const
{
    TRACE_ENTER(Property);

    for (unsigned int i = 0; i < kinds.size (); i++) {
        if (kinds[i] == "factoryparam")
            { return true; }
    }

    return false;
}

const char* Property::getID() const
{
    return id.c_str();
}

const char* Property::getName() const
{
    return name.c_str();
}

const char* Property::getMode() const
{
    return mode.c_str();
}

const char* Property::getAction() const
{
    return action.c_str();
}

const std::vector <std::string>& Property::getKinds() const
{
    return kinds;
}

bool Property::isReadOnly() const
{
    return (mode == "readonly");
}

bool Property::isReadWrite() const
{
    return (mode == "readwrite");
}

bool Property::isWriteOnly() const
{
    return (mode == "writeonly");
}

bool Property::isEqual() const
{
    return (action == "eq");
}

bool Property::isNotEqual() const
{
    return (action == "ne");
}


bool Property::isGreaterThan() const
{
    return (action == "gt");
}

bool Property::isLessThan() const
{
    return (action == "lt");
}

bool Property::isGreaterThanOrEqual() const
{
    return (action == "ge");
}


bool Property::isLessThanOrEqual() const
{
    return (action == "le");
}

bool Property::isExternal() const
{
    return ((action == "external") || (action == ""));
}

/**
 * SimpleProperty class
 */
SimpleProperty::SimpleProperty(const std::string& id, 
                               const std::string& name, 
                               const std::string& type, 
                               const std::string& mode, 
                               const std::string& action, 
                               const std::vector<std::string>& kinds,
                               const optional_value<std::string>& value) :
Property(id, name, mode, action, kinds), type(type), value(value)
{
}

SimpleProperty::~SimpleProperty()
{
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

const char* SimpleProperty::getType() const
{
    return type.c_str();
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
    std::vector<std::string>::const_iterator i;
    for (i = kinds.begin(); i != kinds.end(); ++i) {
        out << *i << ", ";
    }
    out << "' ";
    if (value.isSet()) {
        out << " = '" << *(this->value) << "'>";
    }
    return out.str();
}

const Property* SimpleProperty::clone() const {
    return new SimpleProperty(id, name, type, mode, action, kinds, value);
}


/**
 * SimpleSequenceProperty class
 */
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

const char* SimpleSequenceProperty::getType() const
{
    return type.c_str();
}

const std::vector<std::string>& SimpleSequenceProperty::getValues() const
{
    return values;
}

const std::string SimpleSequenceProperty::asString() const {
    std::ostringstream out;
    out << "'" << this->id << "' '" << this->name;
    std::vector<std::string>::const_iterator i;
    for (i = values.begin(); i != values.end(); ++i) {
        out << " '" << *i << "' ";
    }
    return out.str();
}

const Property* SimpleSequenceProperty::clone() const {
    return new SimpleSequenceProperty(id, name, type, mode, action, kinds, values);
}

/**
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
        value.clear();
        std::copy(otherStructProp->value.begin(), otherStructProp->value.end(), std::back_inserter(value));
    } else {
        LOG_WARN(StructProperty, "Ignoring override request")
    }
}

const std::string StructProperty::asString() const {
    std::ostringstream out;
    out << "'" << this->id << "' '" << this->name;
    std::vector<ossie::SimpleProperty>::const_iterator i;
    for (i = value.begin(); i != value.end(); ++i) {
        out << "   " << *i << std::endl;
    }
    return out.str();
}

const Property* StructProperty::clone() const {
    return new StructProperty(id, name, mode, kinds, value);
};

const std::vector<SimpleProperty>& StructProperty::getValue() const {
    return value;
}

/**
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

const Property* StructSequenceProperty::clone() const {
    return new StructSequenceProperty(id, name, mode, structdef, kinds, values);
}

const StructProperty& StructSequenceProperty::getStruct() const {
    return structdef;
}

const std::vector<StructProperty>& StructSequenceProperty::getValues() const {
    return values;
}
