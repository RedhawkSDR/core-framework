#include "bulkio_stream.h"

using bulkio::StreamBase;

const std::string& StreamBase::streamID() const
{
    return _impl->streamID();
}

const BULKIO::StreamSRI& StreamBase::sri() const
{
    return _impl->sri();
}

void StreamBase::sri(const BULKIO::StreamSRI& sri)
{
    _impl->flush();
    _impl->setSRI(sri);
}

double StreamBase::xdelta() const
{
    return sri().xdelta;
}

void StreamBase::xdelta(double delta)
{
    _impl->flush();
    _impl->setXDelta(delta);
}

bool StreamBase::complex() const
{
    return (sri().mode != 0);
}

void StreamBase::complex(bool mode)
{
    _impl->flush();
    _impl->setComplex(mode);
}

bool StreamBase::blocking() const
{
    return sri().blocking;
}

void StreamBase::blocking(bool mode)
{
    _impl->flush();
    _impl->setBlocking(mode);
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

void StreamBase::keywords(const _CORBA_Unbounded_Sequence<CF::DataType>& props)
{
    _impl->flush();
    _impl->setKeywords(props);
}

void StreamBase::setKeyword(const std::string& name, const CORBA::Any& value)
{
    _impl->flush();
    _impl->setKeyword(name, value);
}

void StreamBase::setKeyword(const std::string& name, const redhawk::Value& value)
{
    _impl->flush();
    _impl->setKeyword(name, value);
}

void StreamBase::eraseKeyword(const std::string& name)
{
    _impl->flush();
    _impl->eraseKeyword(name);
}

void StreamBase::close()
{
    _impl->close();
    _impl.reset();
}

