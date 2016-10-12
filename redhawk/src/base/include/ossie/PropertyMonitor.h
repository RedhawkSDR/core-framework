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

#ifndef _RH_PROPERTY_MONITOR_H
#define _RH_PROPERTY_MONITOR_H

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

/************************************************************************************
  PropertyMonitors
************************************************************************************/

namespace PropertyChange {


  class Monitor {

  public:

    virtual ~Monitor() {};
    virtual bool isChanged() const =0;
    virtual void reset() = 0;
  };


  template< class T  >
    class SimpleMonitor : public Monitor
    {
    public:
      typedef T value_type;
      typedef Monitor  super;

      virtual ~SimpleMonitor() {};

      virtual bool isChanged() const { 
	if ( tested_ ) return diff_;
	if ( old_ != ref_ ) {
	  diff_=true;
	}
	tested_=1;
	return diff_;
      };

      virtual void reset() {
	old_ = ref_;
	tested_=0;
	diff_=false;
      };


      value_type& getPropertyValue() const { return ref_; };
      value_type& getCachedValue() const { return old_; };

    protected:

      SimpleMonitor( value_type& ref ): 
      super(),
	ref_(ref), 
	old_(ref), 
	tested_(0), 
	diff_(false) 
	{};

      value_type  &ref_; 
      mutable value_type old_;
      mutable uint8_t    tested_;
      mutable bool       diff_;

      friend class MonitorFactory;

    };

// Convenience typedefs for simple property types.
typedef SimpleMonitor<std::string> StringProperty;
typedef SimpleMonitor<bool> BooleanProperty;
typedef SimpleMonitor<char> CharProperty;
typedef SimpleMonitor<CORBA::Octet> OctetProperty;
typedef SimpleMonitor<CORBA::Short> ShortProperty;
typedef SimpleMonitor<CORBA::UShort> UShortProperty;
typedef SimpleMonitor<CORBA::Long> LongProperty;
typedef SimpleMonitor<CORBA::ULong> ULongProperty;
typedef SimpleMonitor<CORBA::ULongLong> ULongLongProperty;
typedef SimpleMonitor<CORBA::LongLong> LongLongProperty;
typedef SimpleMonitor<CORBA::Float> FloatProperty;
typedef SimpleMonitor<CORBA::Double> DoubleProperty;

typedef SimpleMonitor<std::complex<float> >            ComplexFloatProperty;
typedef SimpleMonitor<std::complex<bool> >             ComplexBooleanProperty;
typedef SimpleMonitor<std::complex<CORBA::ULong> >     ComplexULongProperty;
typedef SimpleMonitor<std::complex<short> >            ComplexShortProperty;
typedef SimpleMonitor<std::complex<unsigned char> >    ComplexOctetProperty;
typedef SimpleMonitor<std::complex<char> >             ComplexCharProperty;
typedef SimpleMonitor<std::complex<unsigned short> >   ComplexUShortProperty;
typedef SimpleMonitor<std::complex<double> >           ComplexDoubleProperty;
typedef SimpleMonitor<std::complex<CORBA::Long> >      ComplexLongProperty;
typedef SimpleMonitor<std::complex<CORBA::LongLong> >  ComplexLongLongProperty;
typedef SimpleMonitor<std::complex<CORBA::ULongLong> > ComplexULongLongProperty;




  template< typename T >
  class SequenceMonitor : public Monitor
    {
    public:
      typedef T elem_type;
      typedef std::vector< T > value_type;
      typedef Monitor    super;

      virtual ~SequenceMonitor() {};

      virtual bool isChanged() const {
	if ( this->tested_ ) return this->diff_;
	if ( this->ref_.size() != this->old_.size() ){
	  this->diff_=true;
	}
	this->tested_=1;
	return this->diff_;
      };

      virtual void reset() {
	this->old_ = this->ref_;
	this->tested_=0;
	this->diff_=false;
      };

    protected:

       SequenceMonitor( value_type& ref ): 
      super(),
	ref_(ref), old_(ref), tested_(0), diff_(false) 
	{};


      value_type  &ref_; 
      mutable value_type old_;
      mutable uint8_t    tested_;
      mutable bool       diff_;

      friend class MonitorFactory;
    
    };

typedef SequenceMonitor<std::string>      StringSeqProperty;
typedef SequenceMonitor<char>             CharSeqProperty;
typedef SequenceMonitor<bool>             BooleanSeqProperty;
typedef SequenceMonitor<CORBA::Octet>     OctetSeqProperty;
typedef SequenceMonitor<CORBA::Short>     ShortSeqProperty;
typedef SequenceMonitor<CORBA::UShort>    UShortSeqProperty;
typedef SequenceMonitor<CORBA::Long>      LongSeqProperty;
typedef SequenceMonitor<CORBA::ULong>     ULongSeqProperty;
typedef SequenceMonitor<CORBA::LongLong>  LongLongSeqProperty;
typedef SequenceMonitor<CORBA::ULongLong> ULongLongSeqProperty;
typedef SequenceMonitor<CORBA::Float>     FloatSeqProperty;
typedef SequenceMonitor<CORBA::Double>    DoubleSeqProperty;

typedef SequenceMonitor<std::complex<float> >            ComplexFloatSeqProperty;
typedef SequenceMonitor<std::complex<double> >           ComplexDoubleSeqProperty;
typedef SequenceMonitor<std::complex<char> >             ComplexCharSeqProperty;
typedef SequenceMonitor<std::complex<bool> >             ComplexBooleanSeqProperty;
typedef SequenceMonitor<std::complex<unsigned char> >    ComplexOctetSeqProperty;
typedef SequenceMonitor<std::complex<short> >            ComplexShortSeqProperty;
typedef SequenceMonitor<std::complex<unsigned short> >   ComplexUShortSeqProperty;
typedef SequenceMonitor<std::complex<CORBA::Long> >      ComplexLongSeqProperty;
typedef SequenceMonitor<std::complex<CORBA::ULong> >     ComplexULongSeqProperty;
typedef SequenceMonitor<std::complex<CORBA::LongLong> >  ComplexLongLongSeqProperty;
typedef SequenceMonitor<std::complex<CORBA::ULongLong> > ComplexULongLongSeqProperty;


  template< class T >
    class StructMonitor : public SimpleMonitor< T >
    {
    public:
      typedef T value_type;
      typedef SimpleMonitor< value_type > super;

      virtual bool isChanged() const {
	if ( this->tested_ ) return this->diff_;
	if ( this->ref_ != this->old_ ){
	  this->diff_=true;
	}
	this->tested_=1;
	return this->diff_;
      };

    protected:
    StructMonitor(value_type& value) :
      super(value)
      {
      } 

      friend class MonitorFactory;
    
    };



  template< typename T >
    class StructSequenceMonitor : public SequenceMonitor< T >
    {
    public:
      typedef T elem_type;
      typedef std::vector< elem_type > value_type;
      typedef SequenceMonitor< elem_type > super;

      virtual bool isChanged() const {
	if ( this->tested_ ) return this->diff_;
	if ( this->ref_.size() != this->old_.size() ){
	  this->diff_=true;
	}
	this->tested_=1;
	return this->diff_;
      };

    protected:
    StructSequenceMonitor(value_type& value) :
      super(value)
      {
      } 

      friend class MonitorFactory;
    
    };

  class MonitorFactory
  {
  public:

    template <typename T >
      static Monitor *Create (T& value)
      {
        return new StructMonitor< T >(value);
      }
    //static SimpleMonitor< std::vector< T > >* Create (std::vector< T >& value)

    template <typename T >
      static Monitor * Create (std::vector< T >& value)
      {
        return new StructSequenceMonitor< T >(value);
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
    MonitorFactory();

  };


};  // end of PropertyMonitor namespace

#endif // PROPERTYINTERFACE_H
