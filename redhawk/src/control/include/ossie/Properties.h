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

#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__
#include<vector>
#include<map>
#include<string>
#include<istream>
#include<memory>
#include<ostream>
#include<cassert>
#include<sstream>
#include <boost/shared_ptr.hpp>

#include "ossie/CF/cf.h"
#include "ossie/debug.h"
#include "ossie/exceptions.h"
#include "ossie/ossieparser.h"

namespace ossie {

    class ComponentProperty;

    /*
     * Abstract data structure that represents a property. 
     */
    class Property
    {
        ENABLE_LOGGING

    public:
        friend class Properties; 

        enum KindType {
            KIND_CONFIGURE    = 1<<1,
            KIND_EXECPARAM    = 1<<2,
            KIND_ALLOCATION   = 1<<3,
            KIND_FACTORYPARAM = 1<<4,
            KIND_TEST         = 1<<5,
            KIND_EVENT        = 1<<6,
            KIND_MESSAGE      = 1<<7,
            KIND_PROPERTY     = 1<<8,
            KIND_DEFAULT      = KIND_CONFIGURE
        };

        struct Kinds
        {
        public:
            Kinds() :
                kinds(0)
            {
            }

            Kinds(KindType kind) :
                kinds(kind)
            {
            }

            Kinds& operator|= (KindType kind)
            {
                kinds |= kind;
                return *this;
            }

            bool operator& (KindType kind) const
            {
                return (kinds & kind);
            }

            Kinds operator| (KindType kind)
            {
                Kinds result(*this);
                result |= kind;
                return result;
            }

            bool operator! () const
            {
                return (kinds == 0);
            }

        private:
            int kinds;
        };

        enum ActionType {
            ACTION_EXTERNAL,
            ACTION_EQ,
            ACTION_NE,
            ACTION_GT,
            ACTION_LT,
            ACTION_GE,
            ACTION_LE,
            ACTION_DEFAULT = ACTION_EXTERNAL
        };

        enum AccessType {
            MODE_READWRITE,
            MODE_READONLY,
            MODE_WRITEONLY,
            MODE_DEFAULT = MODE_READWRITE
        };

        Property() {}

        Property(const std::string& id, 
                 const std::string& name, 
                 AccessType mode, 
                 ActionType action, 
                 Kinds kinds);

        virtual ~Property();

        bool isReadOnly() const;
        bool isReadWrite() const;
        bool isWriteOnly() const;
        bool canOverride() const;
        bool isAllocation() const;
        bool isConfigure() const;
        bool isProperty() const;
        bool isTest() const;
        virtual bool isCommandLine() const;
        bool isExecParam() const;
        bool isFactoryParam() const;
        bool isEqual() const;
        bool isNotEqual() const;
        bool isGreaterThan() const;
        bool isLessThan() const;
        bool isGreaterThanOrEqual() const;
        bool isLessThanOrEqual() const;
        bool isExternal() const;

        const char* getID() const;
        const char* getName() const;
        AccessType getMode() const;
        // NB: getAction() should return an ActionType; however, there are
        // several places that use its return as a string for an argument to
        // property helper functions in the base library. Before the parsers
        // become "public" API, this should be revisited.
        std::string getAction() const;
        Kinds getKinds() const;

        std::string mapPrimitiveToComplex(const std::string& type) const;

        // Pure virtual functions
        virtual bool isNone() const = 0;
        virtual const std::string asString() const = 0;
        virtual Property* clone() const = 0;

        virtual void override(const Property* otherProp) = 0;
        virtual void override(const ComponentProperty* newValue) = 0;

    protected:
        // Common across all property types
        std::string id;
        std::string name;
        AccessType mode;
        ActionType action;
        Kinds kinds;
    };

    inline Property* new_clone(const Property& property) {
        return property.clone();
    }

    inline Property::Kinds operator|(Property::KindType a, Property::KindType b)
    {
        return Property::Kinds(a) | b;
    }

    /*
     * 
     */
    class SimpleProperty : public Property
    {
        friend class Properties; 
        ENABLE_LOGGING

    public:
        SimpleProperty() {}

        SimpleProperty(const std::string& id, 
                       const std::string& name, 
                       const std::string& type, 
                       AccessType mode, 
                       ActionType action, 
                       Kinds kinds,
                       const optional_value<std::string>& value,
                       bool complex=false,
                       bool commandline=false,
                       bool optional=false);

        virtual ~SimpleProperty();

        // SimpleProperty specific functions
        const char* getValue() const;
        const std::string& getType() const;
        bool isComplex() const;
        bool isOptional() const;

        // Implementation of virtual functions
        virtual bool isCommandLine() const;
        virtual bool isNone() const;
        virtual const std::string asString() const;
        virtual Property* clone() const;

        virtual void override(const Property* otherProp);
        virtual void override(const ComponentProperty* newValue);

    private:
        std::string type;
        optional_value<std::string> value;
        bool complex;
        bool commandline;
        bool optional;
    };

    /*
     *
     */
    class SimpleSequenceProperty : public Property
    {
        friend class Properties; 
        ENABLE_LOGGING

    public:
        SimpleSequenceProperty() {}

        SimpleSequenceProperty(const std::string&              id, 
                               const std::string&              name, 
                               const std::string&              type, 
                               AccessType                      mode, 
                               ActionType                      action,
                               Kinds                           kinds,
                               const std::vector<std::string>& values,
                               bool                            complex=false,
                               bool                            optional=false);

        virtual ~SimpleSequenceProperty();

        const std::vector<std::string>& getValues() const;
        const std::string& getType() const;
        bool isComplex() const;
        bool isOptional() const;

        virtual bool isNone() const;
        virtual const std::string asString() const;
        virtual Property* clone() const;

        virtual void override(const Property* otherProp);
        virtual void override(const ComponentProperty* newValue);

    private:
        std::string type;
        std::vector<std::string> values;
        bool complex;
        bool optional;
    };

    /*
     *
     */
    class StructProperty : public Property
    {
        friend class Properties; 
        ENABLE_LOGGING

    public:
        StructProperty() {}

        StructProperty(const std::string& id, 
                       const std::string& name,
                       AccessType mode,
                       Kinds configurationkinds,
                       const ossie::PropertyList& value) :
            Property(id, name, mode, Property::ACTION_EXTERNAL, configurationkinds),
            value(value)
        {
	}

        virtual ~StructProperty();
        virtual bool isNone() const;
        virtual const std::string asString() const;
        virtual Property* clone() const;

        const PropertyList& getValue() const;

        const Property* getField(const std::string& id) const;

        virtual void override(const Property* otherProp);
        virtual void override(const ComponentProperty* newValue);

    private:
        PropertyList value;
    };

    /*
     *
     */
    class StructSequenceProperty : public Property
    {
        friend class Properties; 
        ENABLE_LOGGING

    public:
        StructSequenceProperty() {}

        StructSequenceProperty(const std::string& id,
                               const std::string& name,
                               AccessType mode,
                               const StructProperty& structdef,
                               Kinds configurationkinds,
                               const std::vector<StructProperty>& values) :
            Property(id, name, mode, Property::ACTION_EXTERNAL, configurationkinds),
            structdef(structdef),
            values(values)
        {
        }

        virtual ~StructSequenceProperty();

        virtual bool isNone() const;
        virtual const std::string asString() const;
        virtual Property* clone() const;

        const StructProperty& getStruct() const;
        const std::vector<StructProperty>& getValues() const;

        virtual void override(const Property* otherProp);
        virtual void override(const ComponentProperty* newValue);

    private:
        StructProperty structdef;
        std::vector<StructProperty> values;
    };

    std::ostream& operator<<(std::ostream& out, const Property& prop);
    std::ostream& operator<<(std::ostream& stream, Property::KindType kind);
    std::ostream& operator<<(std::ostream& stream, Property::Kinds kinds);
    std::ostream& operator<<(std::ostream& stream, Property::ActionType action);
    std::ostream& operator<<(std::ostream& stream, Property::AccessType mode);

    /*
     *
     */
    class PRF
    {
        ENABLE_LOGGING

    public:
        std::map<std::string, const ossie::Property*> _properties;
        std::vector<const Property*> _allProperties;
        std::vector<const Property*> _configProperties;
        std::vector<const Property*> _ctorProperties;
        std::vector<const Property*> _allocationProperties;
        std::vector<const Property*> _execProperties;
        std::vector<const Property*> _factoryProperties;

        PRF() : 
        _properties(), 
        _allProperties(), 
        _configProperties(), 
        _ctorProperties(), 
        _allocationProperties(), 
        _execProperties(), 
        _factoryProperties() 
        {}

        ~PRF() {
        LOG_TRACE(PRF, "Deleting PRF containing " << _allProperties.size() << " properties");
        // Clear all vectors
        std::vector<const Property*>::iterator p;
        for (p = _allProperties.begin(); p != _allProperties.end(); ++p) {
            delete *p;
        }

        _allProperties.clear();
        _configProperties.clear();
        _ctorProperties.clear();
        _allocationProperties.clear();
        _execProperties.clear();
        _factoryProperties.clear();

        // Clear the property map
        _properties.clear();
        }

        void addProperty(const Property* p);

    protected:
        PRF(const PRF&) // Hide copy constructor
        {}

    };

    /*
     * Helper class that deals with PRF files.
     */
    class Properties
    {
        ENABLE_LOGGING

    public:
        Properties();
        
        Properties(std::istream& input);

        virtual ~Properties();

        const std::vector<const Property*>& getProperties() const;

        const Property* getProperty(const std::string& id) const;

        const std::vector<const Property*>& getConfigureProperties() const;

        const std::vector<const Property*>& getConstructProperties() const;

        const std::vector<const Property*>& getAllocationProperties() const;

        const Property* getAllocationProperty(const std::string& id);

        const std::vector<const Property*>& getExecParamProperties() const;

        const std::vector<const Property*>& getFactoryParamProperties() const;

        /*
         * Load the contents of a PRF from an input stream
         */
        void load(std::istream& input);

        /*
         * Join the contents of another PRF along with this one.
         * This will override values for any properties with the same id
         */
        void join(std::istream& input);
        void join(ossie::Properties& props);

        /*
         * Overrides properties with values from another source
         */
        void override(const ossie::ComponentPropertyList & values);

    protected:
        Properties(const Properties&) // Hide copy constructor
        {}

    private:
	boost::shared_ptr<ossie::PRF> _prf;
    };

}
#endif
