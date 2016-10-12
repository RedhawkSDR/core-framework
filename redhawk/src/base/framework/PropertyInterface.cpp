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

#include "ossie/PropertyInterface.h"


PropertyInterface::PropertyInterface (CORBA::TypeCode_ptr _type) :
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

bool PropertyInterface::isNilEnabled ()
{
        return enableNil_;
}

void PropertyInterface::enableNil (bool enable)
{
    enableNil_ = enable;
}

bool PropertyInterface::isQueryable () const
{
  if (mode != std::string("writeonly")) {
    std::vector<std::string>::const_iterator p = kinds.begin();
    while (p != kinds.end()) {
      if ((*p) == std::string("property"))
        return true;
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


bool PropertyInterface::isProperty () const
{
  std::vector<std::string>::const_iterator p = kinds.begin();
  while (p != kinds.end()) {
    if ((*p) == std::string("property"))
      return true;
    p++;
  }
  return false;
}

bool PropertyInterface::isConfigurable () const
{
  if (mode != std::string("readonly")) {
    std::vector<std::string>::const_iterator p = kinds.begin();
    while (p != kinds.end()) {
      if ((*p) == std::string("configure"))
        return true;
      if ((*p) == std::string("property"))
        return true;
      p++;
    }
  }
  return false;
  
}

bool PropertyInterface::isAllocatable () const
{
    return (std::find(kinds.begin(), kinds.end(), "allocation") != kinds.end());
}

bool PropertyInterface::isNil ()
{
    return isNil_;
}

void PropertyInterface::isNil (bool nil)
{
    isNil_ = nil;
}

void PropertyInterface::configure(const std::string& _id, const std::string& _name, const std::string& _mode,
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


template <typename T>
class SimplePropertyWrapper : public PropertyWrapper<T>
{
public:
    typedef T value_type;
    typedef PropertyWrapper<value_type> super;

    virtual short compare (const CORBA::Any& a)
    {
        return super::compare(a);
    }

protected:
    SimplePropertyWrapper (value_type& value) :
        super(value, ossie::corba::TypeCode<value_type>())
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

template <>
inline bool SimplePropertyWrapper<char>::fromAny (const CORBA::Any& a, char& v)
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
inline void SimplePropertyWrapper<char>::toAny (const char& v, CORBA::Any& a)
{
    a <<= CORBA::Any::from_char(v);
}


template <>
inline bool SimplePropertyWrapper<unsigned char>::fromAny (const CORBA::Any& a, CORBA::Octet& v)
{
    return (a >>= CORBA::Any::to_octet(v));
}

template<>
inline void SimplePropertyWrapper<unsigned char>::toAny (const CORBA::Octet& v, CORBA::Any& a)
{
    a <<= CORBA::Any::from_octet(v);
}

template <>
inline bool SimplePropertyWrapper<bool>::fromAny (const CORBA::Any& a, bool& v)
{
    return ossie::any::toNumber(a, v);
}

template <>
inline short SimplePropertyWrapper<std::string>::compare (const CORBA::Any& a)
{
    const char* tmp;
    if (a >>= tmp) {
        return strcmp(tmp, value_.c_str());
    } else {
        return 1;
    }
}

/*
 *
 */
template <typename T>
class NumericPropertyWrapper : public SimplePropertyWrapper<T>
{
public:
    typedef T value_type;
    typedef SimplePropertyWrapper<T> super;

    virtual short compare (const CORBA::Any& a)
    {
        if (super::isNil_) {
            CORBA::TypeCode_var aType = a.type();
            if (aType->kind() == (CORBA::tk_null)) {
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

    virtual bool fromAny (const CORBA::Any& any, value_type& value)
    {
        return ossie::any::toNumber(any, value);
    }

    virtual bool allocate (const value_type& capacity)
    {
        if (capacity > this->value_) {
            return false;
        } else {
            this->value_ -= capacity;
            return true;
        }
    }

    virtual void deallocate (const value_type& capacity)
    {
        this->value_ += capacity;
    }

    friend class PropertyWrapperFactory;

};

template <typename T>
class SimpleSequenceProperty : public SequenceProperty<T>
{
public:
    typedef T elem_type;
    typedef std::vector<elem_type> value_type;
    typedef SequenceProperty<elem_type> super;

protected:
    SimpleSequenceProperty(value_type& value) :
        super(value, ossie::corba::TypeCode<value_type>())
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

#define SIMPLE_FACTORY_CREATE(N,T)                     \
N##Property* PropertyWrapperFactory::Create (T& value) \
{                                                      \
    return new SimplePropertyWrapper< T >(value);      \
}

SIMPLE_FACTORY_CREATE(String, std::string);
SIMPLE_FACTORY_CREATE(Boolean, bool);
SIMPLE_FACTORY_CREATE(Char, char);

#define NUMERIC_FACTORY_CREATE(N, T)                   \
N##Property* PropertyWrapperFactory::Create (T& value) \
{                                                      \
    return new NumericPropertyWrapper<T>(value);       \
}

NUMERIC_FACTORY_CREATE(Octet, CORBA::Octet);
NUMERIC_FACTORY_CREATE(Short, CORBA::Short);
NUMERIC_FACTORY_CREATE(UShort, CORBA::UShort);
NUMERIC_FACTORY_CREATE(Long, CORBA::Long);
NUMERIC_FACTORY_CREATE(ULong, CORBA::ULong);
NUMERIC_FACTORY_CREATE(LongLong, CORBA::LongLong);
NUMERIC_FACTORY_CREATE(ULongLong, CORBA::ULongLong);
NUMERIC_FACTORY_CREATE(Float, CORBA::Float);
NUMERIC_FACTORY_CREATE(Double, CORBA::Double);

#define COMPLEX_FACTORY_CREATE(N, T) \
    SIMPLE_FACTORY_CREATE(Complex##N, std::complex<T>)

COMPLEX_FACTORY_CREATE(Boolean, bool);
COMPLEX_FACTORY_CREATE(Char, char);
COMPLEX_FACTORY_CREATE(Octet, unsigned char);
COMPLEX_FACTORY_CREATE(Short, short);
COMPLEX_FACTORY_CREATE(UShort, unsigned short);
COMPLEX_FACTORY_CREATE(Long, CORBA::Long);
COMPLEX_FACTORY_CREATE(ULong, CORBA::ULong);
COMPLEX_FACTORY_CREATE(LongLong, CORBA::LongLong);
COMPLEX_FACTORY_CREATE(ULongLong, CORBA::ULongLong);
COMPLEX_FACTORY_CREATE(Float, CORBA::Float);
COMPLEX_FACTORY_CREATE(Double, CORBA::Double);

#define SIMPLE_SEQUENCE_FACTORY_CREATE(N,T)                            \
N##SeqProperty* PropertyWrapperFactory::Create (std::vector<T>& value) \
{                                                                      \
    return new SimpleSequenceProperty< T >(value);                     \
}

SIMPLE_SEQUENCE_FACTORY_CREATE(String, std::string);
SIMPLE_SEQUENCE_FACTORY_CREATE(Boolean, bool);
SIMPLE_SEQUENCE_FACTORY_CREATE(Char, char);
SIMPLE_SEQUENCE_FACTORY_CREATE(Octet, CORBA::Octet);
SIMPLE_SEQUENCE_FACTORY_CREATE(Short, CORBA::Short);
SIMPLE_SEQUENCE_FACTORY_CREATE(UShort, CORBA::UShort);
SIMPLE_SEQUENCE_FACTORY_CREATE(Long, CORBA::Long);
SIMPLE_SEQUENCE_FACTORY_CREATE(ULong, CORBA::ULong);
SIMPLE_SEQUENCE_FACTORY_CREATE(LongLong, CORBA::LongLong);
SIMPLE_SEQUENCE_FACTORY_CREATE(ULongLong, CORBA::ULongLong);
SIMPLE_SEQUENCE_FACTORY_CREATE(Float, CORBA::Float);
SIMPLE_SEQUENCE_FACTORY_CREATE(Double, CORBA::Double);

#define COMPLEX_SEQUENCE_FACTORY_CREATE(N, T) \
    SIMPLE_SEQUENCE_FACTORY_CREATE(Complex##N, std::complex<T>)

COMPLEX_SEQUENCE_FACTORY_CREATE(Boolean, bool);
COMPLEX_SEQUENCE_FACTORY_CREATE(Char, char);
COMPLEX_SEQUENCE_FACTORY_CREATE(Octet, unsigned char);
COMPLEX_SEQUENCE_FACTORY_CREATE(Short, short);
COMPLEX_SEQUENCE_FACTORY_CREATE(UShort, unsigned short);
COMPLEX_SEQUENCE_FACTORY_CREATE(Long, CORBA::Long);
COMPLEX_SEQUENCE_FACTORY_CREATE(ULong, CORBA::ULong);
COMPLEX_SEQUENCE_FACTORY_CREATE(LongLong, CORBA::LongLong);
COMPLEX_SEQUENCE_FACTORY_CREATE(ULongLong, CORBA::ULongLong);
COMPLEX_SEQUENCE_FACTORY_CREATE(Float, CORBA::Float);
COMPLEX_SEQUENCE_FACTORY_CREATE(Double, CORBA::Double);


namespace PropertyChange {

#define SIMPLEMONITOR_FACTORY_CREATE(N,T)                     \
N##Property* MonitorFactory::Create (T& value) \
{                                                      \
    return new SimpleMonitor< T >(value);      \
}

SIMPLEMONITOR_FACTORY_CREATE(String, std::string);
SIMPLEMONITOR_FACTORY_CREATE(Boolean, bool);
SIMPLEMONITOR_FACTORY_CREATE(Char, char);

SIMPLEMONITOR_FACTORY_CREATE(Octet, CORBA::Octet);
SIMPLEMONITOR_FACTORY_CREATE(Short, CORBA::Short);
SIMPLEMONITOR_FACTORY_CREATE(UShort, CORBA::UShort);
SIMPLEMONITOR_FACTORY_CREATE(Long, CORBA::Long);
SIMPLEMONITOR_FACTORY_CREATE(ULong, CORBA::ULong);
SIMPLEMONITOR_FACTORY_CREATE(LongLong, CORBA::LongLong);
SIMPLEMONITOR_FACTORY_CREATE(ULongLong, CORBA::ULongLong);
SIMPLEMONITOR_FACTORY_CREATE(Float, CORBA::Float);
SIMPLEMONITOR_FACTORY_CREATE(Double, CORBA::Double);

#define COMPLEXMONITOR_FACTORY_CREATE(N, T) \
    SIMPLEMONITOR_FACTORY_CREATE(Complex##N, std::complex<T>)

COMPLEXMONITOR_FACTORY_CREATE(Boolean, bool);
COMPLEXMONITOR_FACTORY_CREATE(Char, char);
COMPLEXMONITOR_FACTORY_CREATE(Octet, unsigned char);
COMPLEXMONITOR_FACTORY_CREATE(Short, short);
COMPLEXMONITOR_FACTORY_CREATE(UShort, unsigned short);
COMPLEXMONITOR_FACTORY_CREATE(Long, CORBA::Long);
COMPLEXMONITOR_FACTORY_CREATE(ULong, CORBA::ULong);
COMPLEXMONITOR_FACTORY_CREATE(LongLong, CORBA::LongLong);
COMPLEXMONITOR_FACTORY_CREATE(ULongLong, CORBA::ULongLong);
COMPLEXMONITOR_FACTORY_CREATE(Float, CORBA::Float);
COMPLEXMONITOR_FACTORY_CREATE(Double, CORBA::Double);

#define SIMPLEMONITOR_SEQUENCE_FACTORY_CREATE(N,T)                            \
N##SeqProperty* MonitorFactory::Create (std::vector<T>& value) \
{                                                                      \
    return new SequenceMonitor< T >(value);                     \
}

SIMPLEMONITOR_SEQUENCE_FACTORY_CREATE(String, std::string);
SIMPLEMONITOR_SEQUENCE_FACTORY_CREATE(Boolean, bool);
SIMPLEMONITOR_SEQUENCE_FACTORY_CREATE(Char, char);
SIMPLEMONITOR_SEQUENCE_FACTORY_CREATE(Octet, CORBA::Octet);
SIMPLEMONITOR_SEQUENCE_FACTORY_CREATE(Short, CORBA::Short);
SIMPLEMONITOR_SEQUENCE_FACTORY_CREATE(UShort, CORBA::UShort);
SIMPLEMONITOR_SEQUENCE_FACTORY_CREATE(Long, CORBA::Long);
SIMPLEMONITOR_SEQUENCE_FACTORY_CREATE(ULong, CORBA::ULong);
SIMPLEMONITOR_SEQUENCE_FACTORY_CREATE(LongLong, CORBA::LongLong);
SIMPLEMONITOR_SEQUENCE_FACTORY_CREATE(ULongLong, CORBA::ULongLong);
SIMPLEMONITOR_SEQUENCE_FACTORY_CREATE(Float, CORBA::Float);
SIMPLEMONITOR_SEQUENCE_FACTORY_CREATE(Double, CORBA::Double);

#define COMPLEXMONITOR_SEQUENCE_FACTORY_CREATE(N, T) \
    SIMPLEMONITOR_SEQUENCE_FACTORY_CREATE(Complex##N, std::complex<T>)

COMPLEXMONITOR_SEQUENCE_FACTORY_CREATE(Boolean, bool);
COMPLEXMONITOR_SEQUENCE_FACTORY_CREATE(Char, char);
COMPLEXMONITOR_SEQUENCE_FACTORY_CREATE(Octet, unsigned char);
COMPLEXMONITOR_SEQUENCE_FACTORY_CREATE(Short, short);
COMPLEXMONITOR_SEQUENCE_FACTORY_CREATE(UShort, unsigned short);
COMPLEXMONITOR_SEQUENCE_FACTORY_CREATE(Long, CORBA::Long);
COMPLEXMONITOR_SEQUENCE_FACTORY_CREATE(ULong, CORBA::ULong);
COMPLEXMONITOR_SEQUENCE_FACTORY_CREATE(LongLong, CORBA::LongLong);
COMPLEXMONITOR_SEQUENCE_FACTORY_CREATE(ULongLong, CORBA::ULongLong);
COMPLEXMONITOR_SEQUENCE_FACTORY_CREATE(Float, CORBA::Float);
COMPLEXMONITOR_SEQUENCE_FACTORY_CREATE(Double, CORBA::Double);

};
