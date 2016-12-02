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

    class SRI : public BULKIO::StreamSRI {
    public:
        static SRI& cast(BULKIO::StreamSRI& sri)
        {
            return static_cast<SRI&>(sri);
        }

        static const SRI& cast(const BULKIO::StreamSRI& sri)
        {
            return static_cast<const SRI&>(sri);
        }

        explicit SRI(const BULKIO::StreamSRI& sri) :
            BULKIO::StreamSRI(sri)
        {
        }

        std::string getStreamID() const
        {
            return std::string(this->streamID);
        }

        bool complex() const
        {
            return (this->mode != 0);
        }

        const redhawk::PropertyMap& getKeywords() const
        {
            return redhawk::PropertyMap::cast(this->keywords);
        }
    };

    class SharedSRI {
    public:
        SharedSRI() :
            _sri()
        {
        }

        SharedSRI(const BULKIO::StreamSRI& sri) :
            _sri(boost::make_shared<SRI>(sri))
        {
        }

        const SRI* get() const
        {
            return _sri.get();
        }

        const SRI& operator* () const
        {
            return *_sri;
        }

        const SRI* operator-> () const
        {
            return _sri.get();
        }

        bool operator! () const
        {
            return !_sri;
        }

    private:
        boost::shared_ptr<SRI> _sri;
    };
}

#endif // __bulkio_sri_h
