/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef __bulkio_out_stream_h
#define __bulkio_out_stream_h

#include <string>
#include <complex>
#include <boost/shared_ptr.hpp>

#include <ossie/PropertyMap.h>
#include <BULKIO/bulkioDataTypes.h>

#include "bulkio_traits.h"
#include "bulkio_datablock.h"

namespace bulkio {

  template <class PortTraits>
  class OutPort;

  template <class PortTraits>
  class OutputStream {
  public:
    typedef typename PortTraits::DataTransferTraits::NativeDataType ScalarType;
    typedef std::complex<ScalarType> ComplexType;
        
    OutputStream();

    const std::string& streamID() const;

    const BULKIO::StreamSRI& sri() const;
    void sri(const BULKIO::StreamSRI& sri);

    double xdelta() const;
    void xdelta(double delta);

    bool complex() const;
    void complex(bool mode);

    bool blocking() const;
    void blocking(bool mode);

    const redhawk::PropertyMap& keywords() const;
    void keywords(const _CORBA_Unbounded_Sequence<CF::DataType>& props);

    bool hasKeyword(const std::string& name) const;
    const redhawk::Value& getKeyword(const std::string& name) const;
    void setKeyword(const std::string& name, const CORBA::Any& value);
    void setKeyword(const std::string& name, const redhawk::Value& value);
    template <typename T>
    void setKeyword(const std::string& name, const T& value)
    {
      setKeyword(name, redhawk::Value(value));
    }
    void eraseKeyword(const std::string& name);

    template <class T>
    void write(const std::vector<T>& data, const BULKIO::PrecisionUTCTime& time)
    {
      write(&data[0], data.size(), time);
    }

    template <class T>
    void write(const std::vector<T>& data, const std::list<bulkio::SampleTimestamp>& times)
    {
      write(&data[0], data.size(), times);
    }

    void write(const ScalarType* data, size_t count, const BULKIO::PrecisionUTCTime& time);
    void write(const ScalarType* data, size_t count, const std::list<bulkio::SampleTimestamp>& times);

    void write(const ComplexType* data, size_t count, const BULKIO::PrecisionUTCTime& time);
    void write(const ComplexType* data, size_t count, const std::list<bulkio::SampleTimestamp>& times);

    void close();

    bool operator! () const
    {
      return !_impl;
    }

  private:
    friend class OutPort<PortTraits>;
    OutputStream(const BULKIO::StreamSRI& sri, OutPort<PortTraits>* port);

    class Impl;
    boost::shared_ptr<Impl> _impl;
  };

  typedef OutputStream<bulkio::CharPortTraits>      OutCharStream;
  typedef OutputStream<bulkio::OctetPortTraits>     OutOctetStream;
  typedef OutputStream<bulkio::ShortPortTraits>     OutShortStream;
  typedef OutputStream<bulkio::UShortPortTraits>    OutUShortStream;
  typedef OutputStream<bulkio::LongPortTraits>      OutLongStream;
  typedef OutputStream<bulkio::ULongPortTraits>     OutULongStream;
  typedef OutputStream<bulkio::LongLongPortTraits>  OutLongLongStream;
  typedef OutputStream<bulkio::ULongLongPortTraits> OutULongLongStream;
  typedef OutputStream<bulkio::FloatPortTraits>     OutFloatStream;
  typedef OutputStream<bulkio::DoublePortTraits>    OutDoubleStream;

} // end of bulkio namespace

#endif
