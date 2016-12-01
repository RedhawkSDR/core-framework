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

#ifndef __bulkio_in_stream_h
#define __bulkio_in_stream_h

#include <string>
#include <boost/shared_ptr.hpp>

#include <BULKIO/bulkioDataTypes.h>

#include "bulkio_traits.h"
#include "bulkio_datablock.h"

namespace bulkio {

  template <class PortTraits>
  class InPort;

  template <class PortTraits>
  class InputStreamBase {
  public:
    const std::string& streamID() const;
    const BULKIO::StreamSRI& sri() const;

    bool enabled() const;
    void enable();
    void disable();

    bool eos();

    bool operator! () const;

  protected:
    typedef InPort<PortTraits> InPortType;

    class Impl;
    typedef boost::shared_ptr<Impl> InputStreamBase::*unspecified_bool_type;

    InputStreamBase();
    InputStreamBase(const boost::shared_ptr<Impl>& impl);

    bool hasBufferedData();

    boost::shared_ptr<Impl> _impl;

  public:
    operator unspecified_bool_type() const;
  };


  template <class PortTraits>
  class InputStream : public InputStreamBase<PortTraits> {
  public:
    InputStream();

    typedef typename PortTraits::DataTransferTraits::NativeDataType NativeType;
    typedef DataBlock<NativeType> DataBlockType;

    DataBlockType read();
    DataBlockType read(size_t count);
    DataBlockType read(size_t count, size_t consume);

    DataBlockType tryread();
    DataBlockType tryread(size_t count);
    DataBlockType tryread(size_t count, size_t consume);

    size_t skip(size_t count);

    size_t samplesAvailable();

    bool operator== (const InputStream& other) const;

    bool ready();

  private:
    typedef InputStreamBase<PortTraits> Base;
    using Base::_impl;

    friend class InPort<PortTraits>;
    typedef InPort<PortTraits> InPortType;
    InputStream(const boost::shared_ptr<BULKIO::StreamSRI>&, InPortType*);

    class Impl;
    Impl& impl();
    const Impl& impl() const;
  };

  template <>
  class InputStream<XMLPortTraits> : public InputStreamBase<XMLPortTraits> {
  public:
    typedef XMLDataBlock DataBlockType;

    InputStream();

    XMLDataBlock read();

  private:
    friend class InPort<XMLPortTraits>;
    typedef InPort<XMLPortTraits> InPortType;
    InputStream(const boost::shared_ptr<BULKIO::StreamSRI>&, InPortType*);

    class Impl;
    Impl& impl();
    const Impl& impl() const;
  };

  template <>
  class InputStream<FilePortTraits> : public InputStreamBase<FilePortTraits> {
  public:
    typedef FileDataBlock DataBlockType;

    InputStream();

    FileDataBlock read();

  private:
    friend class InPort<FilePortTraits>;
    typedef InPort<FilePortTraits> InPortType;
    InputStream(const boost::shared_ptr<BULKIO::StreamSRI>&, InPortType*);

    class Impl;
    Impl& impl();
    const Impl& impl() const;
  };

  typedef InputStream<bulkio::CharPortTraits>      InCharStream;
  typedef InputStream<bulkio::OctetPortTraits>     InOctetStream;
  typedef InputStream<bulkio::ShortPortTraits>     InShortStream;
  typedef InputStream<bulkio::UShortPortTraits>    InUShortStream;
  typedef InputStream<bulkio::LongPortTraits>      InLongStream;
  typedef InputStream<bulkio::ULongPortTraits>     InULongStream;
  typedef InputStream<bulkio::LongLongPortTraits>  InLongLongStream;
  typedef InputStream<bulkio::ULongLongPortTraits> InULongLongStream;
  typedef InputStream<bulkio::FloatPortTraits>     InFloatStream;
  typedef InputStream<bulkio::DoublePortTraits>    InDoubleStream;
  typedef InputStream<bulkio::XMLPortTraits>       InXMLStream;
  typedef InputStream<bulkio::FilePortTraits>      InFileStream;

} // end of bulkio namespace

#endif
