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
  class InputStream {
  public:
    InputStream();

    typedef typename PortTraits::DataTransferTraits::NativeDataType NativeType;
    typedef DataBlock<NativeType> DataBlockType;

    const std::string& streamID() const;
    const BULKIO::StreamSRI& sri() const;

    bool eos();

    DataBlockType read();
    DataBlockType read(size_t count);
    DataBlockType read(size_t count, size_t consume);

    DataBlockType tryread();
    DataBlockType tryread(size_t count);
    DataBlockType tryread(size_t count, size_t consume);

    size_t skip(size_t count);

    bool enabled() const;
    void enable();
    void disable();

    size_t samplesAvailable();

    bool operator! () const;
    bool operator== (const InputStream& other) const;

    bool ready();

  private:
    friend class InPort<PortTraits>;
    InputStream(const BULKIO::StreamSRI&, InPort<PortTraits>*);

    bool hasBufferedData();

    class Impl;
    boost::shared_ptr<Impl> _impl;
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

} // end of bulkio namespace

#endif
