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
    return _impl->xdelta();
}

void StreamBase::xdelta(double delta)
{
    _impl->flush();
    _impl->xdelta(delta);
}

bool StreamBase::complex() const
{
    return _impl->complex();
}

void StreamBase::complex(bool mode)
{
    _impl->flush();
    _impl->complex(mode);
}

bool StreamBase::blocking() const
{
    return _impl->sri().blocking;
}

void StreamBase::blocking(bool mode)
{
    _impl->flush();
    _impl->blocking(mode);
}

const redhawk::PropertyMap& StreamBase::keywords() const
{
    return _impl->keywords();
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
    _impl->keywords(props);
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

