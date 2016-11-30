#ifndef __bulkio_stream_h
#define __bulkio_stream_h

#include <string>

#include <boost/shared_ptr.hpp>

#include <ossie/PropertyMap.h>

#include "bulkio_base.h"

namespace bulkio {

    class StreamBase {
    public:
        const std::string& streamID() const;

        const BULKIO::StreamSRI& sri() const;
        void sri(const BULKIO::StreamSRI& sri);

        double xdelta() const;
        void xdelta(double xdelta);

        bool complex() const;
        void complex(bool mode);

        bool blocking() const;
        void blocking(bool mode);

        const redhawk::PropertyMap& keywords() const;
        bool hasKeyword(const std::string& name) const;
        const redhawk::Value& getKeyword(const std::string& name) const;

        void keywords(const _CORBA_Unbounded_Sequence<CF::DataType>& props);
        void setKeyword(const std::string& name, const CORBA::Any& value);
        void setKeyword(const std::string& name, const redhawk::Value& value);
        template <typename T>
        void setKeyword(const std::string& name, const T& value)
        {
            setKeyword(name, redhawk::Value(value));
        }
        void eraseKeyword(const std::string& name);

        void close();

        bool operator! () const
        {
            return !_impl;
        }

        int modcount() const
        {
            return _impl->modcount();
        }

    protected:
        class Impl {
        public:
            Impl(const BULKIO::StreamSRI& sri) :
                _streamID(sri.streamID),
                _sri(sri),
                _modcount(0)
            {
            }

            virtual ~Impl()
            {
            }

            virtual void flush()
            {
            }

            virtual void close()
            {
            }

            const std::string& streamID() const
            {
                return _streamID;
            }

            const BULKIO::StreamSRI& sri() const
            {
                return _sri;
            }

            void setXDelta(double delta)
            {
                _setStreamMetadata(_sri.xdelta, delta);
            }

            void setComplex(bool mode)
            {
                _setStreamMetadata(_sri.mode, mode?1:0);
            }

            void setBlocking(bool mode)
            {
                _setStreamMetadata(_sri.blocking, mode?1:0);
            }

            void setKeywords(const _CORBA_Unbounded_Sequence<CF::DataType>& properties)
            {
                _sri.keywords = properties;
                ++_modcount;
            }

            void setKeyword(const std::string& name, const CORBA::Any& value)
            {
                redhawk::PropertyMap::cast(_sri.keywords)[name] = value;
                ++_modcount;
            }

            void setKeyword(const std::string& name, const redhawk::Value& value)
            {
                setKeyword(name, static_cast<const CORBA::Any&>(value));
            }

            void eraseKeyword(const std::string& name)
            {
                redhawk::PropertyMap::cast(_sri.keywords).erase(name);
                ++_modcount;
            }

            void setSRI(const BULKIO::StreamSRI& sri)
            {
                // Copy the new SRI, except for the stream ID, which is immutable
                _sri = sri;
                _sri.streamID = _streamID.c_str();
                ++_modcount;
            }

            int modcount() const
            {
                return _modcount;
            }

        protected:
            template <typename Field, typename Value>
            void _setStreamMetadata(Field& field, Value value)
            {
                if (field != value) {
                    field = value;
                    ++_modcount;
                }
            }

            const std::string _streamID;
            BULKIO::StreamSRI _sri;
            int _modcount;
        };

        StreamBase() :
            _impl()
        {
        }

        StreamBase(boost::shared_ptr<Impl> impl) :
            _impl(impl)
        {
        }

        boost::shared_ptr<Impl> _impl;
    };

}

#endif // __bulkio_stream_h
