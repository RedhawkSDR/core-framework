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

/*******************************************************************************

This file defines the structures that hold the output of the parsed XML files.

*********************************************************************************/

#ifndef DMDPARSER_H
#define DMDPARSER_H

#include <string>
#include "ossie/debug.h"

namespace ossie {
    /*
     *
     */
    class DomainManagerConfiguration
    {
        ENABLE_LOGGING
      
        public:
        DomainManagerConfiguration(std::string id, std::string name, std::string domainManagerSoftPkg) :
          _id(id), _name(name), _domainManagerSoftPkg(domainManagerSoftPkg)
        {}

        const char* getID() const {
            return _id.c_str();
        }

        const char* getName() const {
            return _name.c_str();
        }

        const char* getDomainManagerSoftPkg() {
            return _domainManagerSoftPkg.c_str();
        }
        
        protected:
        std::string _id;
        std::string _name;
        std::string _domainManagerSoftPkg;
    };

    /*
     *
     */
    class Properties
    {
        public:
        // Hide full access to the vector so that we ensure safe access
        // to the contained properties
        std::vector<Property*>::const_iterator begin() const {
            return _props.begin();
        }

        std::vector<Property*>::const_iterator end() const {
            return _props.end();
        }

        std::vector<Property*>::const_reference at(size_t n) const {
            return _props.at(n);
        }

        protected:
        std::vector<Property*> _props;
        std::vector<Test> _tests;
    };

    class Test
    {
    }

    /*
     *
     */
    class Property
    {
        public:
        Property(std::string id, 
                 std::string name, 
                 std::string mode, 
                 std::string description,
                 std::vector<std::string> kinds) :
                _id(id), _name(name), _mode(mode), _description(description), _kinds(kinds)
        {
        }

        const char* getID() const {
            return _id.c_str();
        }

        const char* getName() const {
            return _name.c_str();
        }

        const char* getMode() const {
            return _mode.c_str();
        }

        const char* getDescription() const {
            return _description.c_str();
        }

        std::vector<std::string> getKinds() const {
            return _kinds;
        };

        bool isReadable() const {
            return (_mode != "writeonly");
        }

        bool isWriteable() const {
            return (_mode != "readonly");
        };

        bool isAllocation() const {
            return (std::find(_kinds.begin(), _kinds.end(), "allocation") != _kinds.end())
        }

        bool isConfigure() const {
            return (std::find(_kinds.begin(), _kinds.end(), "configure") != _kinds.end())
        }

        bool isExecParam() const {
            return (std::find(_kinds.begin(), _kinds.end(), "execparam") != _kinds.end())
        }

        bool isFactoryParam();
            return (std::find(_kinds.begin(), _kinds.end(), "factoryparam") != _kinds.end())
        }

        bool isTest();
            return (std::find(_kinds.begin(), _kinds.end(), "test") != _kinds.end())
        }
        protected:
        // These are shared by all types of properties
        std::string _id;
        std::string _name;
        std::string _mode;
        std::string _description;
        std::vector<std::string> _kinds; // For struct and structsequence this vector must be of length 1
    };

    /*
     *
     */
    class AbstractSimpleProperty : public Property
    {
        public:
        // Preserve the old OSSIE API
        bool isBoolean() const {
        return (_type == "boolean");
        }

        bool isChar() const {
            return (_type == "char");
        }

        bool isDouble() const {
            return (_type == "double");
        }

        bool isFloat() const {
            return (_type == "float");
        }

        bool isShort() const {
            return (_type == "short");
        }

        bool isUShort() const {
            return (_type == "ushort");
        }
    
        bool isLong() const {
            return (_type == "long");
        }

        bool isObjref() const {
            return (_type == "objref");
        }

        bool isOctet() const {
            return (_type == "octet");
        }

        bool isString() const {
            return (_type == "string");
        }

        bool isULong() const {
            return (_type == "ulong");
        }

        bool isUshort() const {
            return (_type == "ushort");
        }

        bool isEqual() const {
            return (_action == "eq");
        }

        bool isNotEqual() const {
            return (_action == "ne");
        }

        bool isGreaterThan() const {
            return (_action == "gt");
        }

        bool isLessThan() const {
            return (_action == "lt");
        }

        bool isGreaterThanOrEqual() const {
            return (_action == "ge");
        }

        bool isLessThanOrEqual() const {
            return (_action == "le");
        }

        bool isExternal() const {
            return (_action == "external");
        }

        const char* getPropType() const {
            return _type.c_str();
        }

        const char* getType() const {
            return _type.c_str();
        }

        const char* getAction() const {
            return _action.c_str();
        }

        protected:
        std::string _type;
        std::string _action;
        std::string _units;
    };

    class SimpleProperty : public AbstractSimpleProperty
    {
        public:
        std::string getValue() const {
            return _value;
        }
        protected:
        std::string _value;
        std::map<std::string, std::string> _enumerations; 
    };

    template<class T>
    class SimpleSequenceProperty<T> : public AbstractSimpleProperty<T>
    {
        public::
        std::vector<std::string> getValues() const {
            return _values;
        }
        protected:
        std::vector<T> _values; 
    };

}
#endif
