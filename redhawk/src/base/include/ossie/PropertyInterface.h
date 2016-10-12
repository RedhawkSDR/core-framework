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

#include "CF/ExtendedEvent.h"
#include <COS/CosEventChannelAdmin.hh>

/**
 *
 */
class PropertyInterface
{
public:
    virtual ~PropertyInterface (void)
    {
    }
    
    /**
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
    bool isConfigurable () const;
    bool isAllocatable () const;

    virtual bool isNil ();
    virtual void isNil (bool nil);

    void configure(const std::string& _id, const std::string& _name, const std::string& _mode,
                   const std::string& _units, const std::string& _action, const std::string& _kinds);

    virtual void getValue (CORBA::Any& a) = 0;
    virtual void setValue (const CORBA::Any& a) = 0;
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

    friend class PropertySet_impl;
    
    bool isNil_;
    bool enableNil_;
};

/**
 * 
 */
template <typename T>
class PropertyWrapper : public PropertyInterface
{
public:
    typedef T value_type;
    typedef PropertyInterface super;
    typedef boost::function<void (const value_type*, const value_type*)> Callback;
    typedef boost::function<bool (const value_type&)> Allocator;
    typedef boost::function<void (const value_type&)> Deallocator;

    virtual void getValue (CORBA::Any& outValue)
    {
        if (enableNil_ && isNil_) {
            outValue = CORBA::Any();
        } else {
            toAny(value_, outValue);
        }
    }

    virtual void setValue (const CORBA::Any& any)
    {
        // Save existing value and create a pointer to it, representing nil as
        // a null pointer.
        value_type savedValue = value_;
        value_type* oldValue;
        if (enableNil_ && isNil_) {
            oldValue = 0;
        } else {
            oldValue = &savedValue;
        }

        // Convert value from Any to natural type, and create a pointer to the
        // new value, representing nil as a null pointer.
        value_type* newValue;
        if (this->fromAny(any, value_)) {
            isNil_ = false;
            newValue = &value_;
        } else if (ossie::any::isNull(any)) {
            isNil_ = enableNil_;
            newValue = 0;
        } else {
            throw std::invalid_argument("Unable to set value");
        }

        // Check if the value has changed; if it has, fire the callback(s).
        if (!this->equals(oldValue, newValue)) {
            valueChanged(oldValue, newValue);
        }
    }

    virtual const value_type& getValue (void)
    {
        return value_;
    }

    virtual void setValue (const value_type& newValue)
    {
        value_ = newValue;
        isNil_ = false;
    }

    virtual short compare (const CORBA::Any& any)
    {
        // Account for the possibility of nil in the current value
        value_type* current;
        if (enableNil_ && isNil_) {
            current = 0;
        } else {
            current = &value_;
        }
        
        // Extract the any value, also accounting for the possibility of nil
        value_type temp;
        value_type* other;
        if (ossie::any::isNull(any)) {
            other = 0;
        } else {
            if (!this->fromAny(any, temp)) {
                // Extraction failed, assume unequal
                return 1;
            }
            other = &temp;
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

    void addChangeListener (Callback listener)
    {
        changeListeners_.push_back(listener);
    }

    void setAllocator (Allocator allocator)
    {
        allocator_ = allocator;
    }

    void setDeallocator (Deallocator deallocator)
    {
        deallocator_ = deallocator;
    }

    const std::string getNativeType () const
    {
	return ossie::traits<T>::name();
    }

protected:
    typedef std::list<Callback> CallbackList;
    CallbackList changeListeners_;

    Allocator allocator_;
    Deallocator deallocator_;

    PropertyWrapper (value_type& value, CORBA::TypeCode_ptr typecode) :
        super(typecode),
        value_(value)
    {
    }

    void valueChanged (const value_type* oldValue, const value_type* newValue)
    {
        for (typename CallbackList::iterator ii = changeListeners_.begin(); ii != changeListeners_.end(); ++ii) {
            (*ii)(oldValue, newValue);
        }
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
    
    value_type& value_;
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

/**
 * 
 */
template <typename T>
class SequenceProperty : public PropertyWrapper<std::vector<T> >
{
public:
    typedef T elem_type;
    typedef std::vector<T> value_type;
    typedef PropertyWrapper<value_type> super;

    virtual void setValue (const CORBA::Any& newValue)
    {
        if (ossie::any::isNull(newValue)) {
            // Nil values should clear the sequence
            super::value_.clear();
        } else {
            super::setValue(newValue);
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

/************************************************************************************
  PropertyChange producer
************************************************************************************/

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
