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
  struct BlockTraits {
    typedef DataBlock<typename PortTraits::DataTransferTraits::NativeDataType> DataBlockType;
  };

  template <>
  struct BlockTraits<XMLPortTraits> {
    typedef XMLDataBlock DataBlockType;
  };

  template <>
  struct BlockTraits<FilePortTraits> {
    typedef FileDataBlock DataBlockType;
  };

  template <class PortTraits>
  class InputStreamBase {
  public:
    typedef typename BlockTraits<PortTraits>::DataBlockType DataBlockType;

    const std::string& streamID() const;
    const BULKIO::StreamSRI& sri() const;

    DataBlockType read();

    DataBlockType tryread();

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

    // Allow matching InPort class to create instances of this stream type
    friend class InPort<PortTraits>;
    InputStreamBase(const boost::shared_ptr<BULKIO::StreamSRI>& sri, InPortType* port);

    bool hasBufferedData();

    boost::shared_ptr<Impl> _impl;

  public:
    operator unspecified_bool_type() const;
  };


  template <class PortTraits>
  class InputStream : public InputStreamBase<PortTraits> {
  public:
    InputStream();

    typedef typename InputStreamBase<PortTraits>::DataBlockType DataBlockType;

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
  typedef InputStreamBase<bulkio::XMLPortTraits>   InXMLStream;
  typedef InputStreamBase<bulkio::FilePortTraits>  InFileStream;

  template <class PortTraits>
  struct StreamTraits {
    typedef InputStream<PortTraits> InStreamType;
  };

  template <>
  struct StreamTraits<XMLPortTraits> {
    typedef InXMLStream InStreamType;
  };

  template <>
  struct StreamTraits<FilePortTraits> {
    typedef InFileStream InStreamType;
  };

} // end of bulkio namespace

#endif
