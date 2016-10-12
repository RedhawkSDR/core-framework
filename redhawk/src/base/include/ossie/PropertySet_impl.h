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

#include <boost/bind.hpp>

#include "ossie/debug.h"
#include "ossie/PropertyInterface.h"

#include "CF/cf.h"

/**
Figure out how to describe this interface.
*/

///\todo Why can't I use CF::PropertySet???
class PropertySet_impl: public virtual POA_CF::PropertySet
{
    ENABLE_LOGGING;

public:

    PropertySet_impl ();
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
    PropertyInterface* addProperty (T& value, 
                                    const std::string& id, 
                                    const std::string& name, 
                                    const std::string& mode,
                                    const std::string& units, 
                                    const std::string& action, 
                                    const std::string& kinds)
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
    PropertyInterface* addProperty (T& value, 
                                    const T2& initial_value, 
                                    const std::string& id, 
                                    const std::string& name,
                                    const std::string& mode, 
                                    const std::string& units, 
                                    const std::string& action,
                                    const std::string& kinds)
    {
        PropertyInterface* wrapper = addProperty(value, id, name, mode, units, action, kinds);
        value = initial_value;
        wrapper->isNil(false);
        return wrapper;
    }

    template <typename T>
    void addPropertyChangeListener (const std::string& id, typename PropertyWrapper<T>::Callback func)
    {
        try {
            PropertyWrapper<T>* wrapper = getPropertyWrapperById<T>(id);
            wrapper->addChangeListener(func);
        } catch (const std::invalid_argument& error) {
            LOG_WARN(PropertySet_impl, "Cannot add change listener: " << error.what());
        }
    }

    template <class C, typename T>
    void addPropertyChangeListener (const std::string& id, C* target, void (C::*func)(const T*, const T*))
    {
        typename PropertyWrapper<T>::Callback cb;
        cb = boost::bind(func, target, _1, _2);
        addPropertyChangeListener<T>(id, cb);
    }

    template <typename T>
    void addPropertyChangeListener (const std::string& id, void (*func)(const T*, const T*))
    {
        typename PropertyWrapper<T>::Callback cb;
        cb = func;
        addPropertyChangeListener<T>(id, cb);
    }

    /**
     * Set the callback for changes to a property to a member function on a class instance.
     * The member function must take a string argument (the identifier of the property that changed)
     * and return void.
     */
    template <class T>
    void setPropertyChangeListener (const std::string& id, T& target, void (T::*func)(const std::string&))
    {
        PropertyCallback cb;
        cb = boost::bind(func, &target, _1);
        setPropertyCallback(id, cb);
    }

    template <class T>
    void setPropertyChangeListener (const std::string& id, T* target, void (T::*func)(const std::string&))
    {
        PropertyCallback cb;
        cb = boost::bind(func, target, _1);
        setPropertyCallback(id, cb);
    }

    template <class T>
    PropertyWrapper<T>* getPropertyWrapperById (const std::string& id)
    {
        PropertyInterface* property = getPropertyFromId(id);
        if (!property) {
            throw std::invalid_argument("No property '" + id  + "'");
        }
        PropertyWrapper<T>* wrapper = dynamic_cast<PropertyWrapper<T>*>(property);
        if (!wrapper) {
            std::ostringstream message;
            message << "Property '" << id << "' is of type '" << property->getNativeType() << "'";
            throw std::invalid_argument(message.str());
        }
        return wrapper;
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
    typedef boost::function<void (const std::string&)> PropertyCallback;
    void setPropertyCallback (const std::string& id, PropertyCallback callback);

    typedef std::map<std::string, PropertyCallback> PropertyCallbackMap;
    PropertyCallbackMap propCallbacks;

};
#endif                                            /*  */
