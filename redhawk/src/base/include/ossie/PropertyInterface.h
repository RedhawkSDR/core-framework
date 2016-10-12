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

class Resource_impl;

#include "ossie/CorbaUtils.h"
#include "CF/cf.h"
#include "ossie/Port_impl.h"

#if ENABLE_EVENTS
#include "CF/ExtendedEvent.h"
#include <COS/CosEventChannelAdmin.hh>
#endif

#if not ENABLE_EVENTS
#include "CF/cf.h"
class PropertyEventSupplier : public virtual POA_CF::Port {
};
#endif

/**
 *
 */
class PropertyInterface
{
public:
    virtual ~PropertyInterface (void)
    {
    }
    
    virtual bool isNilEnabled ()
    {
        return enableNil_;
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
    virtual void enableNil (bool enableNil)
    {
        enableNil_ = enableNil;
    }

    bool isQueryable ()
    {
        if (mode != std::string("writeonly")) {
            std::vector<std::string>::iterator p = kinds.begin();
            while (p != kinds.end()) {
                if ((*p) == std::string("configure"))
                    return true;
                if ((*p) == std::string("execparam"))
                    return true;
                if ((*p) == std::string("allocation"))
                    if (action == std::string("external"))
                        return true;
                p++;
            }
        }
        return false;
    }

    bool isConfigurable ()
        {
            if (mode != std::string("readonly")) {
                std::vector<std::string>::iterator p = kinds.begin();
                while (p != kinds.end()) {
                    if ((*p) == std::string("configure"))
                        return true;
                    p++;
                }
            }
            return false;
        }

    virtual bool isNil ()
    {
        return isNil_;
    }

    virtual void isNil (bool nil)
    {
        isNil_ = nil;
    }

    void configure(const std::string& _id, const std::string& _name, const std::string& _mode,
                   const std::string& _units, const std::string& _action, const std::string& _kinds)
    {
        id = _id;
        name = _name;
        mode = _mode;
        units = _units;
        action = _action;
        std::string::size_type istart = 0;
        while (istart < _kinds.size()) {
            std::string::size_type iend = _kinds.find(',', istart);
            kinds.push_back(_kinds.substr(istart, iend));
            if (iend == std::string::npos) {
                break;
            }
            istart = iend + 1;
        }
    }

    virtual void getValue (CORBA::Any& a) = 0;
    virtual void setValue (const CORBA::Any& a) = 0;
    virtual short compare (const CORBA::Any& a) = 0;
    virtual void increment (const CORBA::Any& a) = 0;
    virtual void decrement (const CORBA::Any& a) = 0;

    std::string id;
    std::string name;
    short type;
    std::string mode;
    std::string units;
    std::string action;
    std::vector<std::string> kinds;

protected:
    PropertyInterface(short _type) :
        id(),
        name(),
        type(_type),
        mode(),
        units(),
        action(),
        kinds(),
        isNil_(false),
        enableNil_(false)
    {
    }

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

    virtual void getValue (CORBA::Any& outValue)
    {
        if (enableNil_ && isNil_) {
            outValue = CORBA::Any();
        } else {
            toAny(value_, outValue);
        }
    }

    virtual void setValue (const CORBA::Any& newValue)
    {
        if (this->fromAny(newValue, value_)) {
            isNil_ = false;
        } else {
            isNil_ = enableNil_;
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

    virtual short compare (const CORBA::Any& a)
    {
        if (super::isNil_) {
            if (a.type()->kind() == (CORBA::tk_null)) {
                return 0;
            }
            return 1;
        }

        value_type tmp;
        if (this->fromAny(a, tmp)) {
            if (tmp != this->value_) {
                return 1;
            }
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

protected:
    PropertyWrapper (value_type& value) :
        super(ossie::corba::TypeCode<value_type>()),
        value_(value)
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

    value_type& value_;
};


template <>
inline bool PropertyWrapper<char>::fromAny (const CORBA::Any& a, char& v)
{
    CORBA::Char c;
    if (a >>= CORBA::Any::to_char(c)) {
        v = c;
        return true;
    } else {
        return false;
    }
}

template<>
inline void PropertyWrapper<char>::toAny (const char& v, CORBA::Any& a)
{
    a <<= CORBA::Any::from_char(v);
}


template <>
inline bool PropertyWrapper<unsigned char>::fromAny (const CORBA::Any& a, CORBA::Octet& v)
{
    return (a >>= CORBA::Any::to_octet(v));
}

template<>
inline void PropertyWrapper<unsigned char>::toAny (const CORBA::Octet& v, CORBA::Any& a)
{
    a <<= CORBA::Any::from_octet(v);
}

template <>
inline short PropertyWrapper<std::string>::compare (const CORBA::Any& a)
{
    const char* tmp;
    if (a >>= tmp) {
        return strcmp(tmp, value_.c_str());
    } else {
        return 1;
    }
}


template <>
inline short PropertyWrapper<bool>::compare (const CORBA::Any& a)
{
    bool tmp;
    if ((a >>= tmp) && (value_ == tmp)) {
        return 0;
    } else {
        return 1;
    }
}

/**
 *
 */
template <typename T>
class NumericPropertyWrapper : public PropertyWrapper<T>
{
public:
    typedef T value_type;
    typedef PropertyWrapper<T> super;

    virtual short compare (const CORBA::Any& a)
    {
        if (super::isNil_) {
            if (a.type()->kind() == (CORBA::tk_null)) {
                return 0;
            }
            return 1;
        }

        value_type tmp;
        if (this->fromAny(a, tmp)) {
            if (tmp < super::value_) {
                return -1;
            }
            if (tmp == super::value_) {
                return 0;
            }
            return 1;
        } else {
            return 1;
        }
    }

    virtual void increment (const CORBA::Any& a)
    {
        if (!super::isNil_) {
            value_type tmp;
            if (this->fromAny(a, tmp)) {
                super::value_ += tmp;
            }
        }
    }

    virtual void decrement (const CORBA::Any& a)
    {
        if (!super::isNil_) {
            value_type tmp;
            if (this->fromAny(a, tmp)) {
                super::value_ -= tmp;
            }
        }
    }

protected:
    NumericPropertyWrapper (value_type& value) :
        super(value)
    {
    }

    friend class PropertyWrapperFactory;

};


// Convenience typedefs for simple property types.
typedef PropertyWrapper<std::string> StringProperty;
typedef PropertyWrapper<bool> BooleanProperty;
typedef PropertyWrapper<char> CharProperty;
typedef NumericPropertyWrapper<CORBA::Octet> OctetProperty;
typedef NumericPropertyWrapper<CORBA::Short> ShortProperty;
typedef NumericPropertyWrapper<CORBA::UShort> UShortProperty;
typedef NumericPropertyWrapper<CORBA::Long> LongProperty;
typedef NumericPropertyWrapper<CORBA::ULong> ULongProperty;
typedef NumericPropertyWrapper<CORBA::ULongLong> ULongLongProperty;
typedef NumericPropertyWrapper<CORBA::LongLong> LongLongProperty;
typedef NumericPropertyWrapper<CORBA::Float> FloatProperty;
typedef NumericPropertyWrapper<CORBA::Double> DoubleProperty;

/**
 * 
 */
// There is probably a more elegant wait (i.e. a traits class) to
// get SEQ_T and SEQ_VAR_T, but this is a quick hack to get ready for
// the next release
template <typename T, typename SEQ_T>
class SeqPropertyWrapper : public PropertyInterface
{
public:
    typedef std::vector<T> value_type;
    typedef SEQ_T seq_type;
    typedef typename SEQ_T::_var_type seq_var_type;

    typedef PropertyInterface super;

    virtual void getValue (CORBA::Any& outValue)
    {
        if (enableNil_ && isNil_) {
            outValue = CORBA::Any();
        } else {
            toAny(value_, outValue);
        }
    }

    virtual void setValue (const CORBA::Any& newValue)
    {
        this->fromAny(newValue, value_);
    }

    virtual void isNil (bool nil)
    {
        isNil_ = false;
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

    virtual short compare (const CORBA::Any& a)
    {
        if (super::isNil_) {
            if (a.type()->kind() == (CORBA::tk_null)) {
                return 0;
            }
            return 1;
        }

        value_type tmp;
        if (this->fromAny(a, tmp)) {
            if (tmp.size() != this->value_.size()) {
                return 1;
            }
            for (unsigned int i = 0; i < tmp.size(); i++) {
                if (tmp[i] != this->value_[i]) {
                    return 1;
                }
            }
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

protected:
    SeqPropertyWrapper (value_type& value) :
        super(ossie::corba::TypeCode<value_type>()),
        value_(value)
    {
    }

    virtual bool fromAny (const CORBA::Any& a, value_type& v)
    {
        SEQ_T* seq_p;
        if (a >>= seq_p) {
            v.resize(seq_p->length());
	    if ( v.size() > 0 )  {
                memcpy(&v[0], &(*seq_p)[0], seq_p->length()*sizeof(T));
            }
            return true;
        } else {
            return false;
        }
    }

    virtual void toAny (const value_type& v, CORBA::Any& a)
    {
        seq_var_type seq = new SEQ_T(v.size(), v.size(), (T*)&v[0], 0);
        a <<= seq;
    }

    friend class PropertyWrapperFactory;

    value_type& value_;
};

template<>
inline bool SeqPropertyWrapper<bool, CORBA::BooleanSeq>::fromAny (const CORBA::Any& a, std::vector<bool>& v) {
    CORBA::BooleanSeq* seq_p = new CORBA::BooleanSeq();
    CORBA::BooleanSeq_var seq(seq_p); // Use a var to cleanup the memory when we are done
    if (a >>= seq_p) {
        v.resize(seq_p->length());
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = (*seq_p)[i];
        }
        return true;
    } else {
        return false;
    }
}

template<>
inline void SeqPropertyWrapper<bool, CORBA::BooleanSeq>::toAny (const std::vector<bool>& v, CORBA::Any& a) {
    CORBA::BooleanSeq_var seq = new CORBA::BooleanSeq();
    seq->length(v.size());
    for (size_t i = 0; i < seq->length(); i++) {
        seq[i] = v[i];
    }
    a <<= seq;
}

template<>
inline bool SeqPropertyWrapper<char, CORBA::CharSeq>::fromAny (const CORBA::Any& a, std::vector<char>& v) {
    CORBA::CharSeq* seq_p = new CORBA::CharSeq();
    
    CORBA::CharSeq_var seq(seq_p); // Use a var to cleanup the memory when we are done
    if (a >>= seq_p) {
        v.resize(seq_p->length());
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = (*seq_p)[i];
        }
        return true;
    } else {
        return false;
    }
}

template<>
inline void SeqPropertyWrapper<char, CORBA::CharSeq>::toAny (const std::vector<char>& v, CORBA::Any& a) {
    CORBA::CharSeq_var seq = new CORBA::CharSeq();
    seq->length(v.size());
    for (size_t i = 0; i < seq->length(); i++) {
        seq[i] = v[i];
    }
    a <<= seq;
}


template<>
inline bool SeqPropertyWrapper<std::string, CORBA::StringSeq>::fromAny (const CORBA::Any& a, std::vector<std::string>& v) {
    CORBA::StringSeq* seq_p = new CORBA::StringSeq();
    CORBA::StringSeq_var seq(seq_p); // Use a var to cleanup the memory when we are done
    if (a >>= seq_p) {
        v.resize(seq_p->length());
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = std::string(static_cast<const char*>((*seq_p)[i]));
        }
        return true;
    } else {
        return false;
    }
}

template<>
inline void SeqPropertyWrapper<std::string, CORBA::StringSeq>::toAny (const std::vector<std::string>& v, CORBA::Any& a) {
    CORBA::StringSeq_var seq = new CORBA::StringSeq();
    seq->length(v.size());
    for (size_t i = 0; i < seq->length(); i++) {
        seq[i] = CORBA::string_dup(v[i].c_str());
    }
    a <<= seq;
}

typedef SeqPropertyWrapper<std::string, CORBA::StringSeq> StringSeqProperty;
typedef SeqPropertyWrapper<char, CORBA::CharSeq> CharSeqProperty;
typedef SeqPropertyWrapper<bool, CORBA::BooleanSeq> BooleanSeqProperty;
typedef SeqPropertyWrapper<CORBA::Octet, CORBA::OctetSeq> OctetSeqProperty;
typedef SeqPropertyWrapper<CORBA::Short, CORBA::ShortSeq> ShortSeqProperty;
typedef SeqPropertyWrapper<CORBA::UShort, CORBA::UShortSeq> UShortSeqProperty;
typedef SeqPropertyWrapper<CORBA::Long, CORBA::LongSeq > LongSeqProperty;
typedef SeqPropertyWrapper<CORBA::ULong, CORBA::ULongSeq > ULongSeqProperty;
typedef SeqPropertyWrapper<CORBA::LongLong, CORBA::LongLongSeq > LongLongSeqProperty;
typedef SeqPropertyWrapper<CORBA::ULongLong, CORBA::ULongLongSeq > ULongLongSeqProperty;
typedef SeqPropertyWrapper<CORBA::Float, CORBA::FloatSeq> FloatSeqProperty;
typedef SeqPropertyWrapper<CORBA::Double, CORBA::DoubleSeq> DoubleSeqProperty;


template <typename T>
class StructProperty : public PropertyWrapper<T>
{
public:
    typedef T value_type;
    typedef PropertyWrapper<T> super;

    virtual short compare (const CORBA::Any& a)
    {
        if (super::isNil_) {
            if (a.type()->kind() == (CORBA::tk_null)) {
                return 0;
            }
            return 1;
        }

        value_type tmp;
        if (this->fromAny(a, tmp)) {
            if (tmp != super::value_) {
                return 1;
            }
            return 0;
        } else {
            return 1;
        }
    }
protected:
    StructProperty (value_type& value) :
        super(value)
    {
        super::type = CORBA::tk_struct;
    }

    friend class PropertyWrapperFactory;
};


template <typename T>
class StructSequenceProperty : public PropertyInterface
{
public:
    typedef std::vector<T> value_type;

    typedef PropertyInterface super;

    virtual void getValue (CORBA::Any& outValue)
    {
        if (enableNil_ && isNil_) {
            outValue = CORBA::Any();
        } else {
            toAny(value_, outValue);
        }
    }

    virtual void isNil (bool nil)
    {
        isNil_ = false;
    }

    virtual void setValue (const CORBA::Any& newValue)
    {
        this->fromAny(newValue, value_);
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

    virtual short compare (const CORBA::Any& a)
    {
        return 1;
    }

    virtual void increment (const CORBA::Any& a)
    {
    }

    virtual void decrement (const CORBA::Any& a)
    {
    }

    StructSequenceProperty (value_type& value) :
        super(CORBA::tk_sequence),
        value_(value)
    {
    }

protected:
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

    value_type& value_;
};


class PropertyWrapperFactory
{
public:
    template <typename T>
    static PropertyInterface* Create (T& value)
    {
        return new StructProperty<T>(value);
    }

    template <typename T>
    static PropertyInterface* Create (std::vector<T>& value)
    {
        return new StructSequenceProperty<T>(value);
    }

private:
    // This class should never be instantiated.
    PropertyWrapperFactory();

};


template <>
inline PropertyInterface* PropertyWrapperFactory::Create<std::string> (std::string& value)
{
    return new StringProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<bool> (bool& value)
{
    return new BooleanProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<char> (char& value)
{
    return new CharProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<CORBA::Octet> (CORBA::Octet& value)
{
    return new OctetProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<CORBA::Short> (CORBA::Short& value)
{
    return new ShortProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<CORBA::UShort> (CORBA::UShort& value)
{
    return new UShortProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<CORBA::Long> (CORBA::Long& value)
{
    return new LongProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<CORBA::ULong> (CORBA::ULong& value)
{
    return new ULongProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<CORBA::Float> (CORBA::Float& value)
{
    return new FloatProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<CORBA::Double> (CORBA::Double& value)
{
    return new DoubleProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<std::string> (std::vector<std::string>& value)
{
    return new StringSeqProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<bool> (std::vector<bool>& value)
{
    return new BooleanSeqProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<char> (std::vector<char>& value)
{
    return new CharSeqProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<CORBA::Octet> (std::vector<CORBA::Octet>& value)
{
    return new OctetSeqProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<CORBA::Short> (std::vector<CORBA::Short>& value)
{
    return new ShortSeqProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<CORBA::UShort> (std::vector<CORBA::UShort>& value)
{
    return new UShortSeqProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<CORBA::Long> (std::vector<CORBA::Long>& value)
{
    return new LongSeqProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<CORBA::ULong> (std::vector<CORBA::ULong>& value)
{
    return new ULongSeqProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<CORBA::LongLong> (std::vector<CORBA::LongLong>& value)
{
    return new LongLongSeqProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<CORBA::ULongLong> (std::vector<CORBA::ULongLong>& value)
{
    return new ULongLongSeqProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<CORBA::Float> (std::vector<CORBA::Float>& value)
{
    return new FloatSeqProperty(value);
}

template <>
inline PropertyInterface* PropertyWrapperFactory::Create<CORBA::Double> (std::vector<CORBA::Double>& value)
{
    return new DoubleSeqProperty(value);
}

#if ENABLE_EVENTS

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

#endif // ENABLE_EVENTS

#endif // PROPERTYINTERFACE_H
