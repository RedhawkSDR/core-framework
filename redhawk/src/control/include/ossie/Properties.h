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

        Property() {}

        Property(const std::string& id, 
                 const std::string& name, 
                 const std::string& mode, 
                 const std::string& action, 
                 const std::vector<std::string>& kinds);

        Property(const std::string& id, 
                 const std::string& name, 
                 const std::string& mode, 
                 const std::string& action, 
                 const std::vector<std::string>& kinds,
                 const std::string& cmdline );

        virtual ~Property();

        bool isReadOnly() const;
        bool isReadWrite() const;
        bool isWriteOnly() const;
        bool isAllocation() const;
        bool isConfigure() const;
        bool isProperty() const;
        bool isTest() const;
        bool isCommandLine() const;
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
        const char* getMode() const;
        const char* getAction() const;
        const std::vector<std::string>& getKinds() const;

        std::string mapPrimitiveToComplex(const std::string& type) const;

        // Pure virtual functions
        virtual bool isNone() const = 0;
        virtual const std::string asString() const = 0;
        virtual const Property* clone() const = 0;

    protected:
        // Common across all property types
        std::string id;
        std::string name;
        std::string mode;
        std::string action;
        std::vector <std::string> kinds;
        std::string commandline;

        
        // Pure virtual functions
        virtual void override(const Property* otherProp) = 0;
        virtual void override(const ComponentProperty* newValue) = 0;
    };

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
               const std::string& mode, 
               const std::string& action, 
               const std::vector<std::string>& kinds,
               const optional_value<std::string>& value,
               const std::string& complex_,
               const std::string& commandline,
	       const std::string& optional);

        SimpleProperty(const std::string& id, 
               const std::string& name, 
               const std::string& type, 
               const std::string& mode, 
               const std::string& action, 
               const std::vector<std::string>& kinds,
               const optional_value<std::string>& value);

        virtual ~SimpleProperty();

        // SimpleProperty specific functions
        const char* getValue() const;

        // Implementation of virtual functions
        virtual bool isNone() const;
        virtual const std::string asString() const;
        virtual const Property* clone() const;
        const char* getType() const;
        const char* getComplex() const;
        const char* getCommandLine() const;
	const char* getOptional() const;

    protected:
        virtual void override(const Property* otherProp);
        virtual void override(const ComponentProperty* newValue);

    private:
        std::string type;
        optional_value<std::string> value;
        std::string _complex;
	std::string optional;
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
                               const std::string&              mode, 
                               const std::string&              action, 
                               const std::vector<std::string>& kinds,
                               const std::vector<std::string>& values,
                               const std::string&              complex_,
			       const std::string& 	       optional);

        SimpleSequenceProperty(const std::string&              id, 
                               const std::string&              name, 
                               const std::string&              type, 
                               const std::string&              mode, 
                               const std::string&              action, 
                               const std::vector<std::string>& kinds,
                               const std::vector<std::string>& values);

        virtual ~SimpleSequenceProperty();

        const std::vector<std::string>& getValues() const;

        virtual bool isNone() const;
        virtual const std::string asString() const;
        virtual const Property* clone() const;
        const char* getType() const;
        const char* getComplex() const;
	const char* getOptional() const;

    protected:
        virtual void override(const Property* otherProp);
        virtual void override(const ComponentProperty* newValue);

    private:
        std::string type;
        std::vector<std::string> values;
        std::string _complex;
	std::string optional;
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
                   const std::string& mode, 
                   const std::vector<std::string>& configurationkinds,
                   const std::vector<Property*>& value) :
            Property(id, name, mode, "external", configurationkinds) 
        {
	    std::vector<Property*>::const_iterator it;
	    for(it=value.begin(); it != value.end(); ++it) {
                this->value.push_back(const_cast<Property*>((*it)->clone()));
	    }
	}

        StructProperty(const std::string& id, 
                   const std::string& name, 
                   const std::string& mode, 
                   const std::vector<std::string>& configurationkinds,
                       const ossie::PropertyList & value) :
            Property(id, name, mode, "external", configurationkinds) 
        {
          ossie::PropertyList::const_iterator it;
	    for(it=value.begin(); it != value.end(); ++it) {
                this->value.push_back(const_cast<Property*>(it->clone()));
	    }
	}

	StructProperty(const StructProperty& other) :
          Property(other.id, other.name, other.mode, other.action, other.kinds)
        {
	    std::vector<Property*>::const_iterator it;
	    for(it=other.value.begin(); it != other.value.end(); ++it) {
	        this->value.push_back(const_cast<Property*>((*it)->clone()));
	    }
        }

        virtual ~StructProperty();
        virtual bool isNone() const;
        virtual const std::string asString() const;
        virtual const Property* clone() const;

        StructProperty &operator=(const StructProperty& src);

        const std::vector<Property*>& getValue() const ;

        const Property* getField(const std::string& id) const;

    protected:
        virtual void override(const Property* otherProp);
        virtual void override(const ComponentProperty* newValue);

    private:
        std::vector<Property*> value;
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
                               const std::string& mode,
                               const StructProperty& structdef,
                               const std::vector<std::string>& configurationkinds,
                               const std::vector<StructProperty>& values) :
            Property(id, name, mode, "external", configurationkinds),
            structdef(structdef),
            values(values)
        {
        }

        virtual ~StructSequenceProperty();

        StructSequenceProperty(const StructSequenceProperty &src ) :
        Property(src.id, src.name, src.mode, src.action, src.kinds),
          structdef(src.structdef),
          values(src.values)
        {
        }

        StructSequenceProperty & operator=( const StructSequenceProperty &src );
        
        virtual bool isNone() const;
        virtual const std::string asString() const;
        virtual const Property* clone() const;

        const StructProperty& getStruct() const;
        const std::vector<StructProperty>& getValues() const;

    protected:
        virtual void override(const Property* otherProp);
        virtual void override(const ComponentProperty* newValue);

    private:
        StructProperty structdef;
        std::vector<StructProperty> values;
    };

    template< typename charT, typename Traits>
    std::basic_ostream<charT, Traits>& operator<<(std::basic_ostream<charT, Traits> &out, const Property& prop)
    {
        out << prop.asString();
        return out;
    }

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

        void addProperty(const Property* p) throw (ossie::parser_error);

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
        
        Properties(std::istream& input) throw(ossie::parser_error);

        Properties& operator=( const Properties &other);

        virtual ~Properties();

        const std::vector<const Property*>& getProperties() const;

        const Property* getProperty(const std::string& id);

        const std::vector<const Property*>& getConfigureProperties() const;

        const std::vector<const Property*>& getConstructProperties() const;

        const std::vector<const Property*>& getAllocationProperties() const;

        const Property* getAllocationProperty(const std::string& id);

        const std::vector<const Property*>& getExecParamProperties() const;

        const std::vector<const Property*>& getFactoryParamProperties() const;

        /*
         * Load the contents of a PRF from an input stream
         */
        void load(std::istream& input) throw (ossie::parser_error);

        /*
         * Join the contents of another PRF along with this one.
         * This will override values for any properties with the same id
         */
        void join(std::istream& input) throw (ossie::parser_error);
        void join(ossie::Properties& props) throw (ossie::parser_error);

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
