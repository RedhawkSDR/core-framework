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

};
#endif                                            /*  */
