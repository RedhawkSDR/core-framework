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

#include "bulkio_typetraits.h"
#include "bulkio_datablock.h"
#include "bulkio_stream.h"

namespace bulkio {

    template <class PortType>
    class InPort;

    template <class PortType>
    struct BlockTraits {
        typedef SampleDataBlock<typename NativeTraits<PortType>::NativeType> DataBlockType;
    };

    template <>
    struct BlockTraits<BULKIO::dataXML> {
        typedef StringDataBlock DataBlockType;
    };

    template <>
    struct BlockTraits<BULKIO::dataFile> {
        typedef StringDataBlock DataBlockType;
    };

    template <class PortType>
    class InputStream : public StreamBase {
    public:
        typedef typename BlockTraits<PortType>::DataBlockType DataBlockType;

        DataBlockType read();

        DataBlockType tryread();

        bool enabled() const;
        void enable();
        void disable();

        bool eos();

    protected:
        typedef InPort<PortType> InPortType;

        class Impl;
        Impl& impl();
        const Impl& impl() const;

        typedef const Impl& (InputStream::*unspecified_bool_type)() const;

        InputStream();
        InputStream(const boost::shared_ptr<Impl>& impl);

        // Allow matching InPort class to create instances of this stream type
        friend class InPort<PortType>;
        InputStream(const StreamDescriptor& sri, InPortType* port);

        bool hasBufferedData();

    public:
        operator unspecified_bool_type() const;
    };


    template <class PortType>
    class BufferedInputStream : public InputStream<PortType> {
    public:
        BufferedInputStream();

        typedef typename InputStream<PortType>::DataBlockType DataBlockType;

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
        typedef InputStream<PortType> Base;
        using Base::_impl;

        friend class InPort<PortType>;
        typedef InPort<PortType> InPortType;
        BufferedInputStream(const StreamDescriptor&, InPortType*);

        class Impl;
        Impl& impl();
        const Impl& impl() const;
    };

    typedef BufferedInputStream<BULKIO::dataChar>      InCharStream;
    typedef BufferedInputStream<BULKIO::dataOctet>     InOctetStream;
    typedef BufferedInputStream<BULKIO::dataShort>     InShortStream;
    typedef BufferedInputStream<BULKIO::dataUshort>    InUShortStream;
    typedef BufferedInputStream<BULKIO::dataLong>      InLongStream;
    typedef BufferedInputStream<BULKIO::dataUlong>     InULongStream;
    typedef BufferedInputStream<BULKIO::dataLongLong>  InLongLongStream;
    typedef BufferedInputStream<BULKIO::dataUlongLong> InULongLongStream;
    typedef BufferedInputStream<BULKIO::dataFloat>     InFloatStream;
    typedef BufferedInputStream<BULKIO::dataDouble>    InDoubleStream;
    typedef InputStream<BULKIO::dataXML>               InXMLStream;
    typedef InputStream<BULKIO::dataFile>              InFileStream;

} // end of bulkio namespace

#endif
