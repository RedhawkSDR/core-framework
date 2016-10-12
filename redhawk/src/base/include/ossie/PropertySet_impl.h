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


#ifndef PROPERTYSET_IMPL_H
#define PROPERTYSET_IMPL_H

#include <vector>
#include <string>
#include <sstream>
#include <map>

#include "ossiecf.h"
#include "ossie/debug.h"
#include "ossie/PropertyInterface.h"

#include "CF/cf.h"

// DEPRECATED: Old property interface, now descends from new-style base class to
// support the new interface while preserving the behavior of existing code.
class propertyContainer : public PropertyInterface
{

#define COMPAREPROPS tmp1, tmp2; \
    if ((a >>= tmp1) and (baseProperty.value >>= tmp2)) { \
        if (tmp1 < tmp2) \
            return -1; \
        if (tmp1 == tmp2) \
            return 0; \
        return 1; \
    } else { \
        return 1; \
    }

#define INCREMENTPROPS tmp1, tmp2; \
    if ((a >>= tmp1) and (baseProperty.value >>= tmp2)) { \
        tmp2 = tmp2 + tmp1; \
        baseProperty.value <<= tmp2; \
    }

#define DECREMENTPROPS tmp1, tmp2; \
    if ((a >>= tmp1) and (baseProperty.value >>= tmp2)) { \
        tmp2 = tmp2 - tmp1; \
        baseProperty.value <<= tmp2; \
    }

public:

    CF::DataType baseProperty;
    std::string string_val;

    propertyContainer(std::string _id, std::string _name, short _type,
                      std::string _mode, CORBA::Any& _initial_value, std::string _units,
                      std::string _action, std::vector<std::string> _kinds) :
        PropertyInterface(_type)
    {
        id = _id;
        name = _name;
        mode = _mode;
        units = _units;
        action = _action;
        baseProperty.id = id.c_str();
        baseProperty.value = _initial_value;
        kinds.swap(_kinds);
    }

    propertyContainer() :
        PropertyInterface(0)
    {
    }

    bool isNil ()
    {
        if (isNilEnabled()) {
            return false;
        }
        return CORBA::_tc_null->equal(baseProperty.value.type());
    }

    void getValue (CORBA::Any& a)
    {
        a = baseProperty.value;
    }

    void setValue (const CORBA::Any& a)
    {
        baseProperty.value = a;
    }

    void getValue(short& val) {
        CORBA::Short tmp;
        if(baseProperty.value >>= tmp) {
            val = tmp;
        }
    }
    void getValue(unsigned short& val) {
        CORBA::UShort tmp;
        if(baseProperty.value >>= tmp) {
            val = tmp;
        }
    }
    void getValue(long& val) {
        CORBA::Long tmp;
        if(baseProperty.value >>= tmp) {
            val = tmp;
        }
    }
    void getValue(unsigned long& val) {
        CORBA::ULong tmp;
        if(baseProperty.value >>= tmp) {
            val = tmp;
        }
    }
    void getValue(float& val) {
        CORBA::Float tmp;
        if(baseProperty.value >>= tmp) {
            val = tmp;
        }
    }
    void getValue(double& val) {
        CORBA::Double tmp;
        if(baseProperty.value >>= tmp) {
            val = tmp;
        }
    }
    void getValue(bool& val) {
        CORBA::Boolean tmp;
        if(baseProperty.value >>= CORBA::Any::to_boolean(tmp)) {
            val = tmp;
        }
    }
    void getValue(std::string& val) {
        const char* tmp;
        if(baseProperty.value >>= tmp) {
            val = tmp;
        }
    }

    void setValue(short val) {
        baseProperty.value <<= val;
        std::stringstream tmp;
        tmp << val;
        tmp >> string_val;
    }
    void setValue(unsigned short val) {
        baseProperty.value <<= val;
        std::stringstream tmp;
        tmp << val;
        tmp >> string_val;
    }
    void setValue(long val) {
        baseProperty.value <<= val;
        std::stringstream tmp;
        tmp << val;
        tmp >> string_val;
    }
    void setValue(unsigned long val) {
        baseProperty.value <<= val;
        std::stringstream tmp;
        tmp << val;
        tmp >> string_val;
    }
    void setValue(float val) {
        baseProperty.value <<= val;
        std::stringstream tmp;
        tmp << val;
        tmp >> string_val;
    }
    void setValue(double val) {
        baseProperty.value <<= val;
        std::stringstream tmp;
        tmp << val;
        tmp >> string_val;
    }
    void setValue(bool val) {
        baseProperty.value <<= (short)val;
        std::stringstream tmp;
        tmp << val;
        tmp >> string_val;
    }
    void setValue(std::string val) {
        baseProperty.value <<= val.c_str();
        std::stringstream tmp;
        tmp << val;
        tmp >> string_val;
    }

    std::string getStringValue() {
        return string_val;
    }

    short compare(CORBA::Any& a) {
        switch(type) {
        case 2:
        {CORBA::Short COMPAREPROPS}
        case 3:
        {CORBA::Long COMPAREPROPS}
        case 4:
        {CORBA::UShort COMPAREPROPS}
        case 5:
        {CORBA::ULong COMPAREPROPS}
        case 6:
        {CORBA::Float COMPAREPROPS}
        case 7:
        {CORBA::Double COMPAREPROPS}
        case 18: {
            const char* tmp1, *tmp2;
            if ((a >>= tmp1) and (baseProperty.value >>= tmp2)) {
                int retval = strcmp(tmp1, tmp2);
                return retval;
            } else {
                return 1;
            }
        }
        default:
            return 1;
        }
    }

    short compare(const CORBA::Any& a) {
        switch(type) {
        case 2:
        {CORBA::Short COMPAREPROPS}
        case 3:
        {CORBA::Long COMPAREPROPS}
        case 4:
        {CORBA::UShort COMPAREPROPS}
        case 5:
        {CORBA::ULong COMPAREPROPS}
        case 6:
        {CORBA::Float COMPAREPROPS}
        case 7:
        {CORBA::Double COMPAREPROPS}
        case 18: {
            const char* tmp1, *tmp2;
            if ((a >>= tmp1) and (baseProperty.value >>= tmp2)) {
                int retval = strcmp(tmp1, tmp2);
                return retval;
            } else {
                return 1;
            }
        }
        default:
            return 1;
        }
    }

    void increment(CORBA::Any& a) {
        switch(type) {
        case 2:
        {CORBA::Short INCREMENTPROPS}
        case 3:
        {CORBA::Long INCREMENTPROPS}
        case 4:
        {CORBA::UShort INCREMENTPROPS}
        case 5:
        {CORBA::ULong INCREMENTPROPS}
        case 6:
        {CORBA::Float INCREMENTPROPS}
        case 7:
        {CORBA::Double INCREMENTPROPS}
        default:
            return;
        }
    }

    void increment(const CORBA::Any& a) {
        switch(type) {
        case 2:
        {CORBA::Short INCREMENTPROPS}
        case 3:
        {CORBA::Long INCREMENTPROPS}
        case 4:
        {CORBA::UShort INCREMENTPROPS}
        case 5:
        {CORBA::ULong INCREMENTPROPS}
        case 6:
        {CORBA::Float INCREMENTPROPS}
        case 7:
        {CORBA::Double INCREMENTPROPS}
        default:
            return;
        }
    }

    void decrement(CORBA::Any& a) {
        switch(type) {
        case 2:
        {CORBA::Short DECREMENTPROPS}
        case 3:
        {CORBA::Long DECREMENTPROPS}
        case 4:
        {CORBA::UShort DECREMENTPROPS}
        case 5:
        {CORBA::ULong DECREMENTPROPS}
        case 6:
        {CORBA::Float DECREMENTPROPS}
        case 7:
        {CORBA::Double DECREMENTPROPS}
        default:
            return;
        }
    }

    void decrement(const CORBA::Any& a) {
        switch(type) {
        case 2:
        {CORBA::Short DECREMENTPROPS}
        case 3:
        {CORBA::Long DECREMENTPROPS}
        case 4:
        {CORBA::UShort DECREMENTPROPS}
        case 5:
        {CORBA::ULong DECREMENTPROPS}
        case 6:
        {CORBA::Float DECREMENTPROPS}
        case 7:
        {CORBA::Double DECREMENTPROPS}
        default:
            return;
        }
    }
};


/**
Figure out how to describe this interface.
*/

///\todo Why can't I use CF::PropertySet???
class OSSIECF_API PropertySet_impl: public virtual POA_CF::PropertySet
{
    ENABLE_LOGGING;

public:

    PropertySet_impl () {
        propertyChangePort = NULL;
    };
    ~PropertySet_impl ();

    /**
     * Sets all the execparams passed in runtime
     */
    void setExecparamProperties(std::map<std::string, char*>&);

    /// The core framework provides an implementation for this method.
    void
    configure (const CF::Properties& configProperties)
    throw (CF::PropertySet::PartialConfiguration,
           CF::PropertySet::InvalidConfiguration, CORBA::SystemException);

    /// The core framework provides an implementation for this method.
    void
    query (CF::Properties& configProperties)
    throw (CF::UnknownProperties, CORBA::SystemException);


    // Preferred new-style properties.
    PropertyInterface* getPropertyFromId (const std::string&);
    PropertyInterface* getPropertyFromName (const std::string&);

    // DEPRECATED: Old-style properties.
    std::map<std::string, propertyContainer> propSet;
    typedef std::map<std::string, propertyContainer>::iterator propSet_iterator;

    propertyContainer* getPropFromId(std::string&);
    
    void registerPropertyChangePort(PropertyEventSupplier* _propertyChangePort) {
        propertyChangePort = _propertyChangePort;
    };

protected:

    /*CF::Properties
            propertySet;*/
    CF::DataType
    getProperty (CORBA::String_var id);
    void
    validate (CF::Properties property, CF::Properties& validProps,
              CF::Properties& invalidProps);

    /**
     * Adds a property with no initial value.
     */
    template <typename T>
    PropertyInterface* addProperty (T& value, const std::string& id, const std::string& name, const std::string& mode,
                                    const std::string& units, const std::string& action, const std::string& kinds)
    {
        PropertyInterface* wrapper = PropertyWrapperFactory::Create(value);
        wrapper->configure(id, name, mode, units, action, kinds);
        wrapper->isNil(true);
        ownedWrappers.push_back(wrapper);
        propTable[wrapper->id] = wrapper;
        return wrapper;
    }

    /**
     * Adds a property with an initial value.
     */
    template <typename T, typename T2>
    PropertyInterface* addProperty (T& value, const T2& initial_value, const std::string& id, const std::string& name,
                                    const std::string& mode, const std::string& units, const std::string& action,
                                    const std::string& kinds)
    {
        PropertyInterface* wrapper = addProperty(value, id, name, mode, units, action, kinds);
        value = initial_value;
        wrapper->isNil(false);
        return wrapper;
    }

    /**
     * Set the callback for changes to a property to a member function on a class instance.
     * The member function must take a string argument (the identifier of the property that changed)
     * and return void.
     */
    template <class T>
    void setPropertyChangeListener (const std::string& id, T& target, void (T::*func)(const std::string&))
    {
        setPropertyCallback(id, new MemberCallback<T>(target, func));
    }

    template <class T>
    void setPropertyChangeListener (const std::string& id, T* target, void (T::*func)(const std::string&))
    {
        setPropertyCallback(id, new MemberCallback<T>(*target, func));
    }

    /**
     * Set the callback for changes to a property to a global function.
     * The function must take a string argument (the identifier of the property that changed)
     * and return void.
     */
    typedef void (*PropertyCallbackFn)(const std::string&);
    void setPropertyChangeListener (const std::string& id, PropertyCallbackFn func);

    /**
     * Call the property change callback for the given identifier.
     */
    void executePropertyCallback (const std::string& id);
    
    // This mutex is used to deal with configure/query concurrency
    boost::mutex propertySetAccess;
    
    PropertyEventSupplier* propertyChangePort;

    std::vector<PropertyInterface*> ownedWrappers;

    // Preferred new-style properties.
    typedef std::map<std::string, PropertyInterface*> PropertyMap;
    PropertyMap propTable;


private:
    /**
     * Abstract interface for property change callbacks.
     */
    class PropertyCallback
    {
    public:
        virtual void operator() (const std::string& value) = 0;
        virtual ~PropertyCallback (void) { }

    protected:
        PropertyCallback () { }
    };


    /**
     * Concrete class for static (i.e. non-member function) property change callbacks.
     */
    class StaticCallback : public PropertyCallback
    {
    public:
        virtual void operator() (const std::string& value)
        {
            (*func_)(value);
        }

    private:
        // Only allow PropertySet_impl to instantiate this class.
        StaticCallback (PropertyCallbackFn func) :
            func_(func)
        {
        }

        friend class PropertySet_impl;

        PropertyCallbackFn func_;
    };


    /**
     * Concrete class for member function property change callbacks.
     */
    template <class T>
    class MemberCallback : public PropertyCallback
    {
    public:
        typedef void (T::*MemberFn)(const std::string&);

        virtual void operator() (const std::string& value)
        {
            (target_.*func_)(value);
        }

    private:
        // Only allow PropertySet_impl to instantiate this class.
        MemberCallback (T& target, MemberFn func) :
            target_(target),
            func_(func)
        {
        }

        friend class PropertySet_impl;

        T& target_;
        MemberFn func_;
    };

    void setPropertyCallback (const std::string& id, PropertyCallback* callback);

    typedef std::map<std::string, PropertyCallback*> PropertyCallbackMap;
    PropertyCallbackMap propCallbacks;

    // For maintaining backwards compatibility with old-style properties.
    void remapProperties (void);

};
#endif                                            /*  */
