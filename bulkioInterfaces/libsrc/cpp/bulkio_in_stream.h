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
#include "bulkio_stream.h"

namespace bulkio {

    template <class PortTraits>
    class InPort;

    template <class PortTraits>
    struct BlockTraits {
        typedef SampleDataBlock<typename PortTraits::DataTransferTraits::NativeDataType> DataBlockType;
    };

    template <>
    struct BlockTraits<XMLPortTraits> {
        typedef StringDataBlock DataBlockType;
    };

    template <>
    struct BlockTraits<FilePortTraits> {
        typedef StringDataBlock DataBlockType;
    };

    template <class PortTraits>
    class InputStream : public StreamBase {
    public:
        typedef typename BlockTraits<PortTraits>::DataBlockType DataBlockType;

        DataBlockType read();

        DataBlockType tryread();

        bool enabled() const;
        void enable();
        void disable();

        bool eos();

    protected:
        typedef InPort<PortTraits> InPortType;

        class Impl;
        Impl& impl();
        const Impl& impl() const;

        typedef const Impl& (InputStream::*unspecified_bool_type)() const;

        InputStream();
        InputStream(const boost::shared_ptr<Impl>& impl);

        // Allow matching InPort class to create instances of this stream type
        friend class InPort<PortTraits>;
        InputStream(const StreamDescriptor& sri, InPortType* port);

        bool hasBufferedData();

    public:
        operator unspecified_bool_type() const;
    };


    template <class PortTraits>
    class BufferedInputStream : public InputStream<PortTraits> {
    public:
        BufferedInputStream();

        typedef typename InputStream<PortTraits>::DataBlockType DataBlockType;

        DataBlockType read();
        DataBlockType read(size_t count);
        DataBlockType read(size_t count, size_t consume);

        DataBlockType tryread();
        DataBlockType tryread(size_t count);
        DataBlockType tryread(size_t count, size_t consume);

        size_t skip(size_t count);

        size_t samplesAvailable();

        bool operator== (const BufferedInputStream& other) const;

        bool ready();

    private:
        typedef InputStream<PortTraits> Base;
        using Base::_impl;

        friend class InPort<PortTraits>;
        typedef InPort<PortTraits> InPortType;
        BufferedInputStream(const StreamDescriptor&, InPortType*);

        class Impl;
        Impl& impl();
        const Impl& impl() const;
    };

    typedef BufferedInputStream<bulkio::CharPortTraits>      InCharStream;
    typedef BufferedInputStream<bulkio::OctetPortTraits>     InOctetStream;
    typedef BufferedInputStream<bulkio::ShortPortTraits>     InShortStream;
    typedef BufferedInputStream<bulkio::UShortPortTraits>    InUShortStream;
    typedef BufferedInputStream<bulkio::LongPortTraits>      InLongStream;
    typedef BufferedInputStream<bulkio::ULongPortTraits>     InULongStream;
    typedef BufferedInputStream<bulkio::LongLongPortTraits>  InLongLongStream;
    typedef BufferedInputStream<bulkio::ULongLongPortTraits> InULongLongStream;
    typedef BufferedInputStream<bulkio::FloatPortTraits>     InFloatStream;
    typedef BufferedInputStream<bulkio::DoublePortTraits>    InDoubleStream;
    typedef InputStream<bulkio::XMLPortTraits>               InXMLStream;
    typedef InputStream<bulkio::FilePortTraits>              InFileStream;

    template <class PortTraits>
    struct StreamTraits {
        typedef BufferedInputStream<PortTraits> InStreamType;
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
