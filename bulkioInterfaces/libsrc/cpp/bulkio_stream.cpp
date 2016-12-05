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

#include "bulkio_stream.h"

using bulkio::StreamBase;

StreamBase::StreamBase() :
    _impl()
{
}

StreamBase::StreamBase(const boost::shared_ptr<Impl>& impl) :
    _impl(impl)
{
}

const std::string& StreamBase::streamID() const
{
    return _impl->streamID();
}

const BULKIO::StreamSRI& StreamBase::sri() const
{
    return _impl->sri();
}

double StreamBase::xdelta() const
{
    return sri().xdelta;
}

bool StreamBase::complex() const
{
    return _impl->complex();
}

bool StreamBase::blocking() const
{
    return _impl->blocking();
}

bool StreamBase::operator!() const
{
    return !_impl;
}

const redhawk::PropertyMap& StreamBase::keywords() const
{
    return redhawk::PropertyMap::cast(sri().keywords);
}

bool StreamBase::hasKeyword(const std::string& name) const
{
    return keywords().contains(name);
}

const redhawk::Value& StreamBase::getKeyword(const std::string& name) const
{
    return keywords()[name];
}
