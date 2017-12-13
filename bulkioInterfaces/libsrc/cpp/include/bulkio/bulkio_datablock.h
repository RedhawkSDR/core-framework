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

#include <ossie/shared_buffer.h>

#include <BULKIO/bulkioDataTypes.h>
#include "bulkio_base.h"

namespace bulkio {

    class SRI;
    class StreamDescriptor;

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
        DataBlock();
        DataBlock(const StreamDescriptor& sri, const T& buffer);
      
        DataBlock copy() const;

        const BULKIO::StreamSRI& sri() const;
        double xdelta() const;

        const T& data() const;

        bool sriChanged() const;
        int sriChangeFlags() const;
        void sriChangeFlags(int flags);

        bool inputQueueFlushed() const;
        void inputQueueFlushed(bool flush);

        void addTimestamp(const SampleTimestamp& timestamp);
        const BULKIO::PrecisionUTCTime& getStartTime() const;
        std::list<SampleTimestamp> getTimestamps() const;
        double getNetTimeDrift() const;
        double getMaxTimeDrift() const;

        bool operator! () const
        {
            return !_impl;
        }

    protected:
        struct Impl;
        boost::shared_ptr<Impl> _impl;

        typedef boost::shared_ptr<Impl> DataBlock::*unspecified_bool_type;

    public:
        operator unspecified_bool_type() const;
    };

    template <class T>
    class SampleDataBlock : public DataBlock<redhawk::shared_buffer<T> >
    {
    public:
        typedef T ScalarType;
        typedef std::complex<T> ComplexType;
        typedef redhawk::shared_buffer<ScalarType> ScalarBuffer;
        typedef redhawk::shared_buffer<ComplexType> ComplexBuffer;

        SampleDataBlock();
        explicit SampleDataBlock(const StreamDescriptor& sri, const ScalarBuffer& buffer=ScalarBuffer());
        SampleDataBlock(const BULKIO::StreamSRI& sri, size_t size=0);

        SampleDataBlock copy() const;

        ScalarType* data();
        const ScalarType* data() const;
        size_t size() const;
        void resize(size_t count);

        bool complex() const;
        ComplexType* cxdata();
        const ComplexType* cxdata() const;
        size_t cxsize() const;

        void swap(std::vector<ScalarType>& other);

        ScalarBuffer buffer() const;
        ComplexBuffer cxbuffer() const;

        void buffer(const ScalarBuffer& other);

    private:
        typedef DataBlock<ScalarBuffer> Base;
        using Base::_impl;
    };

    typedef SampleDataBlock<int8_t>           CharDataBlock;
    typedef SampleDataBlock<CORBA::Octet>     OctetDataBlock;
    typedef SampleDataBlock<CORBA::Short>     ShortDataBlock;
    typedef SampleDataBlock<CORBA::UShort>    UShortDataBlock;
    typedef SampleDataBlock<CORBA::Long>      LongDataBlock;
    typedef SampleDataBlock<CORBA::ULong>     ULongDataBlock;
    typedef SampleDataBlock<CORBA::LongLong>  LongLongDataBlock;
    typedef SampleDataBlock<CORBA::ULongLong> ULongLongDataBlock;
    typedef SampleDataBlock<CORBA::Float>     FloatDataBlock;
    typedef SampleDataBlock<CORBA::Double>    DoubleDataBlock;
    typedef DataBlock<bitstring>              BitDataBlock;
    typedef DataBlock<std::string>            StringDataBlock;

}  // end of bulkio namespace

#endif
