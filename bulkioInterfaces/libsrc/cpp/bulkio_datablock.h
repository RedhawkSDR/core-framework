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

#ifndef __bulkio_datablock_h
#define __bulkio_datablock_h

#include <list>
#include <complex>

#include <boost/shared_ptr.hpp>

#include <BULKIO/bulkioDataTypes.h>

namespace bulkio {

  struct SampleTimestamp
  {
    SampleTimestamp(const BULKIO::PrecisionUTCTime& time, size_t offset=0, bool synthetic=false) :
      time(time),
      offset(offset),
      synthetic(synthetic)
    {
    }

    BULKIO::PrecisionUTCTime time;
    size_t offset;
    bool synthetic;
  };

  template <class T>
  class DataBlock
  {
  public:
    typedef T ScalarType;
    typedef std::complex<T> ComplexType;

    DataBlock();
    DataBlock(const BULKIO::StreamSRI& sri, size_t size=0);

    DataBlock copy() const;

    const BULKIO::StreamSRI& sri() const;
    double xdelta() const;

    ScalarType* data();
    const ScalarType* data() const;
    size_t size() const;
    void resize(size_t count);

    bool complex() const;
    ComplexType* cxdata();
    const ComplexType* cxdata() const;
    size_t cxsize() const;

    bool sriChanged() const;
    int sriChangeFlags() const;
    void sriChangeFlags(int flags);

    bool inputQueueFlushed() const;
    void inputQueueFlushed(bool flush);

    void addTimestamp(const SampleTimestamp& timestamp);
    std::list<SampleTimestamp> getTimestamps() const;
    double getNetTimeDrift() const;
    double getMaxTimeDrift() const;

    bool operator! () const
    {
      return !_impl;
    }

    void swap(std::vector<ScalarType>& other);
  private:
    struct Impl;
    boost::shared_ptr<Impl> _impl;
  };

  typedef DataBlock<int8_t>           CharDataBlock;
  typedef DataBlock<CORBA::Octet>     OctetDataBlock;
  typedef DataBlock<CORBA::Short>     ShortDataBlock;
  typedef DataBlock<CORBA::UShort>    UShortDataBlock;
  typedef DataBlock<CORBA::Long>      LongDataBlock;
  typedef DataBlock<CORBA::ULong>     ULongDataBlock;
  typedef DataBlock<CORBA::LongLong>  LongLongDataBlock;
  typedef DataBlock<CORBA::ULongLong> ULongLongDataBlock;
  typedef DataBlock<CORBA::Float>     FloatDataBlock;
  typedef DataBlock<CORBA::Double>    DoubleDataBlock;

}  // end of bulkio namespace

#endif
