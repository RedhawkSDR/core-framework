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

#ifndef __bulkio_sri_h
#define __bulkio_sri_h

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <ossie/PropertyMap.h>

#include <BULKIO/bulkioDataTypes.h>

namespace bulkio {

    class SharedSRI {
    public:
        SharedSRI() :
            _sri()
        {
        }

        SharedSRI(const BULKIO::StreamSRI& sri) :
            _sri(boost::make_shared<BULKIO::StreamSRI>(sri))
        {
        }

        std::string streamID() const
        {
            return std::string(_sri->streamID);
        }

        bool blocking() const
        {
            return _sri->blocking;
        }

        bool complex() const
        {
            return (_sri->mode != 0);
        }

        const BULKIO::StreamSRI& sri()
        {
            return *_sri;
        }

        const BULKIO::StreamSRI& operator* () const
        {
            return *_sri;
        }

        bool operator! () const
        {
            return !_sri;
        }

    protected:
        boost::shared_ptr<BULKIO::StreamSRI> _sri;
    };

    class StreamBase {
    public:
        const std::string& streamID() const;
        const BULKIO::StreamSRI& sri() const;

        double xdelta() const;
        bool complex() const;
        bool blocking() const;

        const redhawk::PropertyMap& keywords() const;
        bool hasKeyword(const std::string& name) const;
        const redhawk::Value& getKeyword(const std::string& name) const;

        bool operator! () const;

    protected:
        class Impl : public SharedSRI {
        public:
            Impl(const SharedSRI& sri) :
                SharedSRI(sri),
                _streamID(sri.streamID())
            {
            }

            const std::string& streamID() const
            {
                return _streamID;
            }

            virtual ~Impl() { }

        protected:
            const std::string _streamID;
        };

        StreamBase();
        StreamBase(const boost::shared_ptr<Impl>& impl);

        boost::shared_ptr<Impl> _impl;
    };
}

#endif // __bulkio_sri_h
