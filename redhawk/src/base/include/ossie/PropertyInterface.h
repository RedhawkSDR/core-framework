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

#ifndef PROPERTYINTERFACE_H
#define PROPERTYINTERFACE_H

#include <string>
#include <complex>
#include <iostream>
#include <list>

#include <boost/function.hpp>

#include "ossie/AnyUtils.h"
#include "ossie/CorbaUtils.h"
#include "CF/cf.h"
#include "ossie/Port_impl.h"

#include "ossie/ComplexProperties.h"
#include "internal/equals.h"
#include "type_traits.h"
#include "exceptions.h"
#include "callback.h"

#include "CF/ExtendedEvent.h"
#include <COS/CosEventChannelAdmin.hh>
#include "PropertyMonitor.h"


/*
 *
 */
class PropertyInterface
{
public:
    virtual ~PropertyInterface (void)
    {
    }
    
    /*
     * By default, the PropertyWrapper will ignore Nil values set via setValue()
     * and will never return Nil values in response to getValue()
     *
     * If you enableNil on a property it is your job to understand the consequences.  
     * Most notably: 
     *  1. If isNil() is true, then a call to getValue() will 
     *     return a  CORBA::Nil until explicitly call isNil(false).
     *  2. If setValue() was used to set the value to Nil, the underlying
     *     value managed by the wrapper will not be changed.  You should
     *     be checking isNil() before accessing this value.
     */
    virtual bool isNilEnabled ();
    virtual void enableNil (bool enableNil);

    bool isQueryable () const;
    bool isProperty () const;
    bool isConfigurable () const;
    bool isAllocatable () const;

    virtual bool isNil ();
    virtual void isNil (bool nil);

    void configure(const std::string& _id, const std::string& _name, const std::string& _mode,
                   const std::string& _units, const std::string& _action, const std::string& _kinds);

    virtual void getValue (CORBA::Any& a) = 0;
    virtual void setValue (const CORBA::Any& a, bool callbacks=true) = 0;
    virtual short compare (const CORBA::Any& a) = 0;
    virtual void increment (const CORBA::Any& a) = 0;
    virtual void decrement (const CORBA::Any& a) = 0;

    virtual bool allocate (const CORBA::Any& a) = 0;
    virtual void deallocate (const CORBA::Any& a) = 0;

    virtual const std::string getNativeType () const = 0;

    std::string id;
    std::string name;
    CORBA::TypeCode_ptr type;
    std::string mode;
    std::string units;
    std::string action;
    std::vector<std::string> kinds;



protected:
    PropertyInterface(CORBA::TypeCode_ptr _type);

    // change listener registration for internal notification support classes
    template <class Target, class Base >
    void addChangeListener (Target target, void (Base::*func)())
    {
        voidListeners_.add(target, func);
    }

    template <class Target, class Base >
    void removeChangeListener (Target target, void (Base::*func)())
    {
        voidListeners_.remove(target, func);
    }


    virtual bool matchesAddress(const void* address) = 0;

    friend class PropertySet_impl;
    
    bool isNil_;
    bool enableNil_;

    // change listener registration for internal notification support classes
    ossie::notification<void (void)>                            voidListeners_;

};

/*
 * 
 */
template <typename T>
class PropertyWrapper : public PropertyInterface
{
public:
    typedef T value_type;
    typedef PropertyInterface super;

    virtual void enableNil(bool enable)
    {
        if (enable && !valueListeners_.empty()) {
            throw std::logic_error("cannot enable nil values while using by-value change listeners");
        }
        super::enableNil(enable);
    }

    virtual void getValue (CORBA::Any& outValue)
    {
        if (enableNil_ && isNil_) {
            outValue = CORBA::Any();
        } else {
            toAny(getValue(), outValue);
        }
    }

    virtual void setValue (const CORBA::Any& any, bool callbacks=true)
    {
        // Save existing value and create a pointer to it, which accounts for
        // nil values
        value_type savedValue = value_;
        const value_type* oldValue = toPointer(savedValue);

        // Convert value from Any to natural type (or nil)
        value_type temp;
        if (this->fromAny(any, temp)) {
            setValue(temp);
        } else if (ossie::any::isNull(any)) {
            // NB: For backwards compatibility, silently ignore null values if
            //     nil is not enabled; in the future, consider throwing an
            //     exception
            isNil_ = enableNil_;
        } else {
            throw std::invalid_argument("unable to set value");
        }

        // Create a pointer to the new value, again accounting for nil
        const value_type* newValue = toPointer(value_);

        // Check if the value has changed; if it has, fire the callback(s).
        if (callbacks) {
            if (!this->equals(oldValue, newValue)) {
                valueChanged(oldValue, newValue);
            }
        }
    }

    virtual const value_type& getValue (void)
    {
        if (query_) {
            value_ = query_();
        }
        return value_;
    }

    virtual void setValue (const value_type& newValue, bool callbacks=true)
    {
        if (configure_) {
            configure_(newValue);
        } else {
            value_ = newValue;
            isNil_ = false;
        }
    }

    virtual short compare (const CORBA::Any& any)
    {
        // Account for the possibility of nil in the current value
        const value_type* current = toPointer(value_);
        
        // Extract the any value, also accounting for the possibility of nil
        value_type temp;
        value_type* other;
        if (ossie::any::isNull(any)) {
            other = 0;
        } else if (this->fromAny(any, temp)) {
            other = &temp;
        } else {
            // Extraction failed, assume unequal
            return 1;
        }

        // Compare the effective values, converting boolean return into short
        if (this->equals(current, other)) {
            return 0;
        } else {
            return 1;
        }
    }

    virtual void increment (const CORBA::Any& a)
    {
    }

    virtual void decrement (const CORBA::Any& a)
    {
    }

    virtual bool allocate (const CORBA::Any& any)
    {
        value_type capacity;
        if (!fromAny(any, capacity)) {
            // Could not extract value
            return false;
        }
        if (allocator_) {
            return allocator_(capacity);
        } else {
            return allocate(capacity);
        }
    }

    virtual void deallocate (const CORBA::Any& any)
    {
        value_type capacity;
        if (!fromAny(any, capacity)) {
            // Could not extract value
            return;
        }
        if (deallocator_) {
            deallocator_(capacity);
        } else {
            deallocate(capacity);
        }
    }

    template <class Func>
    void setQuery (Func func)
    {
        if (!isQueryable()) {
            throw std::logic_error("property '" + id + "' is not queryable");
        }
        query_ = func;
    }

    template <class Target, class Func>
    void setQuery (Target target, Func func)
    {
        if (!isQueryable()) {
            throw std::logic_error("property '" + id + "' is not queryable");
        }
        ossie::bind(query_, target, func);
    }

    template <class Func>
    void setConfigure (Func func)
    {
        if (!isConfigurable()) {
            throw std::logic_error("property '" + id + "' is not configurable");
        }
        configure_ = func;
    }

    template <class Target, class Func>
    void setConfigure (Target target, Func func)
    {
        if (!isConfigurable()) {
            throw std::logic_error("property '" + id + "' is not configurable");
        }
        ossie::bind(configure_, target, func);
    }

    template <class R, class A1, class A2>
    void addChangeListener (R (*func)(A1*, A2*))
    {
        pointerListeners_.add(func);
    }


    template <class Target, class Base, class R, class A1, class A2>
    void addChangeListener (Target target, R (Base::*func)(A1*, A2*))
    {
        pointerListeners_.add(target, func);
    }

    template <class Target, class Base, class R, class A1, class A2>
    void addChangeListener (Target target, R (Base::*func)(A1*, A2*) const)
    {
        pointerListeners_.add(target, func);
    }

    template <class Func>
    void addChangeListener (Func func)
    {
        if (enableNil_) {
            throw std::logic_error("cannot set by-value change listener with nil values enabled");
        }
        valueListeners_.add(func);
    }

    template <class Target, class Func>
    void addChangeListener (Target target, Func func)
    {
        if (enableNil_) {
            throw std::logic_error("cannot set by-value change listener with nil values enabled");
        }
        valueListeners_.add(target, func);
    }

    template <class Func>
    void setAllocator (Func func)
    {
        if (!isAllocatable()) {
            throw std::logic_error("property '" + id + "' is not allocatable");
        }
        allocator_ = func;
    }

    template <class Target, class Func>
    void setAllocator (Target target, Func func)
    {
        if (!isAllocatable()) {
            throw std::logic_error("property '" + id + "' is not allocatable");
        }
        ossie::bind(allocator_, target, func);
    }

    template <class Func>
    void setDeallocator (Func func)
    {
        if (!isAllocatable()) {
            throw std::logic_error("property '" + id + "' is not allocatable");
        }
        deallocator_ = func;
    }

    template <class Target, class Func>
    void setDeallocator (Target target, Func func)
    {
        if (!isAllocatable()) {
            throw std::logic_error("property '" + id + "' is not allocatable");
        }
        ossie::bind(deallocator_, target, func);
    }

    const std::string getNativeType () const
    {
	return ossie::traits<T>::name();
    }

protected:
    PropertyWrapper (value_type& value, CORBA::TypeCode_ptr typecode) :
        super(typecode),
        value_(value)
    {
    }

    inline const value_type* toPointer(const value_type& value)
    {
        if (enableNil_ && isNil_) {
            return 0;
        } else {
            return &value;
        }
    }

    void valueChanged (const value_type* oldValue, const value_type* newValue)
    {
        pointerListeners_(oldValue, newValue);
        valueListeners_(*oldValue, *newValue);
        voidListeners_();
    }

    bool equals (const value_type* oldValue, const value_type* newValue)
    {
        if (oldValue && newValue) {
            // Both old and new are non-nil, check value equality
            // NB: Prior to 1.8, the code generators did not define an
            //     operator== for struct properties. For 1.X, an internal
            //     function abstracts the complexity; in 2.0, this can be
            //     replaced by a regular comparison.
            return ossie::internal::equals(*oldValue, *newValue);
        } else {
            // One or both values is nil, so determine equality by comparing
            // the pointers--they will only be the same if both are nil (i.e. a
            // null pointer).
            return (oldValue == newValue);
        }
    }

    virtual bool fromAny (const CORBA::Any& a, value_type& v) = 0;
    virtual void toAny (const value_type& v, CORBA::Any& a) = 0;

    // Default allocation implemenation always fails; subclasses may override
    // to provide different default behavior
    virtual bool allocate (const value_type& capacity)
    {
        throw ossie::not_implemented_error("allocate");
    }

    // Default deallocation implemenation does nothing; subclasses may override
    // to provide different default behavior
    virtual void deallocate (const value_type& capacity)
    {
        throw ossie::not_implemented_error("deallocate");
    }

    virtual bool matchesAddress(const void* address)
    {
        return (address == &value_);
    }
    
    value_type& value_;

private:
    // Delegate function types
    typedef boost::function<value_type()> QueryFunc;
    typedef boost::function<void (const value_type&)> ConfigureFunc;
    typedef boost::function<bool (const value_type&)> AllocateFunc;
    typedef boost::function<void (const value_type&)> DeallocateFunc;

    QueryFunc query_;
    ConfigureFunc configure_;
    AllocateFunc allocator_;
    DeallocateFunc deallocator_;

    // By-pointer and by-value change listeners
    ossie::notification<void (const value_type*, const value_type*)> pointerListeners_;
    ossie::notification<void (const value_type&, const value_type&)> valueListeners_;
};

// Convenience typedefs for simple property types.
typedef PropertyWrapper<std::string> StringProperty;
typedef PropertyWrapper<bool> BooleanProperty;
typedef PropertyWrapper<char> CharProperty;
typedef PropertyWrapper<CORBA::Octet> OctetProperty;
typedef PropertyWrapper<CORBA::Short> ShortProperty;
typedef PropertyWrapper<CORBA::UShort> UShortProperty;
typedef PropertyWrapper<CORBA::Long> LongProperty;
typedef PropertyWrapper<CORBA::ULong> ULongProperty;
typedef PropertyWrapper<CORBA::ULongLong> ULongLongProperty;
typedef PropertyWrapper<CORBA::LongLong> LongLongProperty;
typedef PropertyWrapper<CORBA::Float> FloatProperty;
typedef PropertyWrapper<CORBA::Double> DoubleProperty;

typedef PropertyWrapper<std::complex<float> >            ComplexFloatProperty;
typedef PropertyWrapper<std::complex<bool> >             ComplexBooleanProperty;
typedef PropertyWrapper<std::complex<CORBA::ULong> >     ComplexULongProperty;
typedef PropertyWrapper<std::complex<short> >            ComplexShortProperty;
typedef PropertyWrapper<std::complex<unsigned char> >    ComplexOctetProperty;
typedef PropertyWrapper<std::complex<char> >             ComplexCharProperty;
typedef PropertyWrapper<std::complex<unsigned short> >   ComplexUShortProperty;
typedef PropertyWrapper<std::complex<double> >           ComplexDoubleProperty;
typedef PropertyWrapper<std::complex<CORBA::Long> >      ComplexLongProperty;
typedef PropertyWrapper<std::complex<CORBA::LongLong> >  ComplexLongLongProperty;
typedef PropertyWrapper<std::complex<CORBA::ULongLong> > ComplexULongLongProperty;

/*
 * 
 */
template <typename T>
class SequenceProperty : public PropertyWrapper<std::vector<T> >
{
public:
    typedef T elem_type;
    typedef std::vector<T> value_type;
    typedef PropertyWrapper<value_type> super;

    virtual void setValue (const CORBA::Any& newValue, bool callbacks=true)
    {
        if (ossie::any::isNull(newValue)) {
            // Nil values should clear the sequence
            super::value_.clear();
        } else {
            super::setValue(newValue, callbacks);
        }
    }

    virtual void isNil (bool nil)
    {
        super::isNil_ = false;
    }

protected:
    SequenceProperty(value_type& value, CORBA::TypeCode_ptr typecode) :
        super(value, typecode)
    {
    }
};

typedef SequenceProperty<std::string>      StringSeqProperty;
typedef SequenceProperty<char>             CharSeqProperty;
typedef SequenceProperty<bool>             BooleanSeqProperty;
typedef SequenceProperty<CORBA::Octet>     OctetSeqProperty;
typedef SequenceProperty<CORBA::Short>     ShortSeqProperty;
typedef SequenceProperty<CORBA::UShort>    UShortSeqProperty;
typedef SequenceProperty<CORBA::Long>      LongSeqProperty;
typedef SequenceProperty<CORBA::ULong>     ULongSeqProperty;
typedef SequenceProperty<CORBA::LongLong>  LongLongSeqProperty;
typedef SequenceProperty<CORBA::ULongLong> ULongLongSeqProperty;
typedef SequenceProperty<CORBA::Float>     FloatSeqProperty;
typedef SequenceProperty<CORBA::Double>    DoubleSeqProperty;

typedef SequenceProperty<std::complex<float> >            ComplexFloatSeqProperty;
typedef SequenceProperty<std::complex<double> >           ComplexDoubleSeqProperty;
typedef SequenceProperty<std::complex<char> >             ComplexCharSeqProperty;
typedef SequenceProperty<std::complex<bool> >             ComplexBooleanSeqProperty;
typedef SequenceProperty<std::complex<unsigned char> >    ComplexOctetSeqProperty;
typedef SequenceProperty<std::complex<short> >            ComplexShortSeqProperty;
typedef SequenceProperty<std::complex<unsigned short> >   ComplexUShortSeqProperty;
typedef SequenceProperty<std::complex<CORBA::Long> >      ComplexLongSeqProperty;
typedef SequenceProperty<std::complex<CORBA::ULong> >     ComplexULongSeqProperty;
typedef SequenceProperty<std::complex<CORBA::LongLong> >  ComplexLongLongSeqProperty;
typedef SequenceProperty<std::complex<CORBA::ULongLong> > ComplexULongLongSeqProperty;

template <typename T>
class StructProperty : public PropertyWrapper<T>
{
public:
    typedef T value_type;
    typedef PropertyWrapper<T> super;

    // This definition exists strictly because pre-1.10 code generators define
    // an explicit specialization of this method; it may be removed when source
    // compatibility with 1.9 and older is no longer required.
    virtual short compare (const CORBA::Any& a)
    {
        return super::compare(a);
    }

protected:
    StructProperty (value_type& value) :
        super(value, CORBA::_tc_TypeCode)
    {
    }

    virtual bool fromAny (const CORBA::Any& a, value_type& v)
    {
        return (a >>= v);
    }

    virtual void toAny (const value_type& v, CORBA::Any& a)
    {
        a <<= v;
    }

    friend class PropertyWrapperFactory;
};

template <typename T>
class StructSequenceProperty : public SequenceProperty<T>
{
public:
    typedef T elem_type;
    typedef std::vector<elem_type> value_type;
    typedef SequenceProperty<elem_type> super;

    // This definition exists strictly because pre-1.10 code generators define
    // an explicit specialization of this method; it may be removed when source
    // compatibility with 1.9 and older is no longer required.
    virtual short compare (const CORBA::Any& a)
    {
        return super::compare(a);
    }

protected:
    StructSequenceProperty (value_type& value) :
        super(value, CORBA::_tc_TypeCode)
    {
    }

    virtual bool fromAny (const CORBA::Any& a, value_type& v)
    {
        CORBA::AnySeq* anySeqPtr;
        if (!(a >>= anySeqPtr)) {
            return false;
        }
        CORBA::AnySeq& anySeq = *anySeqPtr;
        v.resize(anySeq.length());
        for (CORBA::ULong ii = 0; ii < anySeq.length(); ++ii) {
            if (!(anySeq[ii] >>= v[ii])) {
                return false;
            }
        }
        return true;
    }

    virtual void toAny (const value_type& v, CORBA::Any& a)
    {
        CORBA::AnySeq anySeq;
        anySeq.length(v.size());
        for (CORBA::ULong ii = 0; ii < anySeq.length(); ++ii) {
            anySeq[ii] <<= v[ii];
        }
        a <<= anySeq;
    }

    friend class PropertyWrapperFactory;
};

class PropertyWrapperFactory
{
public:
    template <typename T>
    static PropertyWrapper<T>* Create (T& value)
    {
        return new StructProperty<T>(value);
    }

    template <typename T>
    static PropertyWrapper<std::vector<T> >* Create (std::vector<T>& value)
    {
        return new StructSequenceProperty<T>(value);
    }

    static StringProperty* Create (std::string&);
    static BooleanProperty* Create (bool&);
    static CharProperty* Create (char&);
    static OctetProperty* Create (CORBA::Octet&);
    static ShortProperty* Create (CORBA::Short&);
    static UShortProperty* Create (CORBA::UShort&);
    static LongProperty* Create (CORBA::Long&);
    static ULongProperty* Create (CORBA::ULong&);
    static LongLongProperty* Create (CORBA::LongLong&);
    static ULongLongProperty* Create (CORBA::ULongLong&);
    static FloatProperty* Create (CORBA::Float&);
    static DoubleProperty* Create (CORBA::Double&);

    static ComplexBooleanProperty* Create (std::complex<bool>&);
    static ComplexCharProperty* Create (std::complex<char>&);
    static ComplexOctetProperty* Create (std::complex<unsigned char>&);
    static ComplexShortProperty* Create (std::complex<short>&);
    static ComplexUShortProperty* Create (std::complex<unsigned short>&);
    static ComplexLongProperty* Create (std::complex<CORBA::Long>&);
    static ComplexULongProperty* Create (std::complex<CORBA::ULong>&);
    static ComplexLongLongProperty* Create (std::complex<CORBA::LongLong>&);
    static ComplexULongLongProperty* Create (std::complex<CORBA::ULongLong>&);
    static ComplexFloatProperty* Create (std::complex<float>&);
    static ComplexDoubleProperty* Create (std::complex<double>&);

    static StringSeqProperty* Create (std::vector<std::string>&);
    static BooleanSeqProperty* Create (std::vector<bool>&);
    static CharSeqProperty* Create (std::vector<char>&);
    static OctetSeqProperty* Create (std::vector<CORBA::Octet>&);
    static ShortSeqProperty* Create (std::vector<CORBA::Short>&);
    static UShortSeqProperty* Create (std::vector<CORBA::UShort>&);
    static LongSeqProperty* Create (std::vector<CORBA::Long>&);
    static ULongSeqProperty* Create (std::vector<CORBA::ULong>&);
    static LongLongSeqProperty* Create (std::vector<CORBA::LongLong>&);
    static ULongLongSeqProperty* Create (std::vector<CORBA::ULongLong>&);
    static FloatSeqProperty* Create (std::vector<CORBA::Float>&);
    static DoubleSeqProperty* Create (std::vector<CORBA::Double>&);

    static ComplexBooleanSeqProperty* Create (std::vector<std::complex<bool> >&);
    static ComplexCharSeqProperty* Create (std::vector<std::complex<char> >&);
    static ComplexOctetSeqProperty* Create (std::vector<std::complex<unsigned char> >&);
    static ComplexShortSeqProperty* Create (std::vector<std::complex<short> >&);
    static ComplexUShortSeqProperty* Create (std::vector<std::complex<unsigned short> >&);
    static ComplexLongSeqProperty* Create (std::vector<std::complex<CORBA::Long> >&);
    static ComplexULongSeqProperty* Create (std::vector<std::complex<CORBA::ULong> >&);
    static ComplexLongLongSeqProperty* Create (std::vector<std::complex<CORBA::LongLong> >&);
    static ComplexULongLongSeqProperty* Create (std::vector<std::complex<CORBA::ULongLong> >&);
    static ComplexFloatSeqProperty* Create (std::vector<std::complex<float> >&);
    static ComplexDoubleSeqProperty* Create (std::vector<std::complex<double> >&);

private:
    // This class should never be instantiated.
    PropertyWrapperFactory();

};



class PropertyEventSupplier : public Port_Uses_base_impl, public virtual POA_CF::Port {

public:

    struct propertyInfo {
        std::string component_id;
        std::string component_name;
        PropertyInterface *property;
    };
    PropertyEventSupplier (std::string port_name) : Port_Uses_base_impl(port_name) { };
    virtual ~PropertyEventSupplier (void) { };

    // CF::Port methods
    void connectPort(CORBA::Object_ptr connection, const char* connectionId) {
        CosEventChannelAdmin::EventChannel_var channel = ossie::corba::_narrowSafe<CosEventChannelAdmin::EventChannel>(connection);
        if (CORBA::is_nil(channel)) {
            throw CF::Port::InvalidPort(0, "The object provided did not narrow to a CosEventChannelAdmin::EventChannel type");
        }
        CosEventChannelAdmin::SupplierAdmin_var supplier_admin = channel->for_suppliers();
        CosEventChannelAdmin::ProxyPushConsumer_ptr proxy_consumer = supplier_admin->obtain_push_consumer();
        extendConsumers(connectionId, proxy_consumer);
    };

    void disconnectPort(const char* connectionId) {
    };

    void push(const CORBA::Any& data) {
        std::map<std::string, CosEventChannelAdmin::ProxyPushConsumer_var>::iterator connection = consumers.begin();
        while (connection != consumers.end()) {
            try {
                (connection->second)->push(data);
            } catch ( ... ) {
            }
            connection++;
        }
    };
    
    void sendPropertyEvent(const std::string &id) {
        if (propertyMap.find(id) == propertyMap.end()) {
            return;
        }
        // populate event type
        ExtendedEvent::PropertySetChangeEventType_var event = new ExtendedEvent::PropertySetChangeEventType();
        event->sourceId = CORBA::string_dup(propertyMap[id].component_id.c_str());
        event->sourceName = CORBA::string_dup(propertyMap[id].component_name.c_str());
        event->properties.length(1);
        event->properties[0].id = CORBA::string_dup(propertyMap[id].property->id.c_str());
        // load the any onto the event's any
        propertyMap[id].property->getValue(event->properties[0].value);
        CORBA::Any eventAny;
        eventAny <<= event;
        // send the event
        this->push(eventAny);
    };
    
    void registerProperty(const std::string &component_id, const std::string &component_name, PropertyInterface *property) {
        // add a property to the port's map
        propertyInfo tmp;
        tmp.component_id = component_id;
        tmp.component_name = component_name;
        tmp.property = property;
        propertyMap[property->id]=tmp;
    };

    void extendConsumers(std::string consumer_id, CosEventChannelAdmin::ProxyPushConsumer_ptr proxy_consumer) {
        consumers[std::string(consumer_id)] = proxy_consumer;
    };


protected:
    
    std::map<std::string, CosEventChannelAdmin::ProxyPushConsumer_var> consumers;
    std::map<std::string, CosEventChannelAdmin::EventChannel_ptr> _connections;
    // map of properties that the port has access to
    std::map<std::string, propertyInfo> propertyMap;
    
    
};

#endif // PROPERTYINTERFACE_H
