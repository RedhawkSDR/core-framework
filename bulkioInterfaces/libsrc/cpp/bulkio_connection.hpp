namespace bulkio {

    template <typename PortTraits>
    class PortTransport : public redhawk::BasicTransport
    {
    public:
        typedef typename PortTraits::PortType PortType;
        typedef typename PortType::_var_type VarType;
        typedef typename PortType::_ptr_type PtrType;
        typedef typename PortTraits::NativeType NativeType;
        typedef typename PortTraits::SharedBufferType SharedBufferType;

        PortTransport(const std::string& connectionId, const std::string& name, PtrType objref) :
            redhawk::BasicTransport(connectionId, objref),
            stats(name, sizeof(NativeType)),
            _port(PortType::_duplicate(objref))
        {
        }

        virtual ~PortTransport() { };

        virtual void disconnect()
        {
            // Send an end-of-stream for all active streams
            for (std::set<std::string>::iterator stream = _streams.begin(); stream != _streams.end(); ++stream) {
                this->_pushPacket(SharedBufferType(), bulkio::time::utils::notSet(), true, *stream);
            }
            _streams.clear();
        }

        void pushSRI(const std::string& streamID, const BULKIO::StreamSRI& sri)
        {
            _streams.insert(streamID);
            this->_pushSRI(sri);
        }

        void pushPacket(const SharedBufferType& data,
                        const BULKIO::PrecisionUTCTime& T,
                        bool EOS,
                        const std::string& streamID,
                        const BULKIO::StreamSRI& sri)
        {
            this->_sendPacket(data, T, EOS, streamID, sri);
            if (EOS) {
                _streams.erase(streamID);
            }
        }

        PtrType objref()
        {
            if (isAlive()) {
                return PortType::_duplicate(_port);
            } else {
                return PortType::_nil();
            }
        }

        linkStatistics stats;

    protected:
        virtual void _pushSRI(const BULKIO::StreamSRI& sri) = 0;

        virtual void _sendPacket(const SharedBufferType& data,
                                 const BULKIO::PrecisionUTCTime& T,
                                 bool EOS,
                                 const std::string& streamID,
                                 const BULKIO::StreamSRI& sri)
        {
            this->_pushPacket(data, T, EOS, streamID);
            stats.update(this->_dataLength(data), 0, EOS, streamID);
        }

        virtual void _pushPacket(const SharedBufferType& data,
                                 const BULKIO::PrecisionUTCTime& T,
                                 bool EOS,
                                 const std::string& streamID) = 0;

        //
        // Returns the total number of elements of data in a pushPacket call, for
        // statistical tracking; enables XML and File specialization, which have
        // different notions of size
        //
        size_t _dataLength(const SharedBufferType& data)
        {
            return data.size();
        }

        VarType _port;
        std::set<std::string> _streams;
    };

    template <>
    size_t PortTransport<FilePortTraits>::_dataLength(const std::string& /*unused*/)
    {
        return 1;
    }

    template <typename PortTraits>
    class RemoteTransport : public PortTransport<PortTraits>
    {
    public:
        typedef typename PortTraits::PortType PortType;
        typedef typename PortType::_ptr_type PtrType;
        typedef typename PortTraits::SharedBufferType SharedBufferType;
        typedef typename PortTraits::SequenceType PortSequenceType;
        typedef typename PortTraits::TransportType TransportType;

        RemoteTransport(const std::string& connectionId, const std::string& name, PtrType port) :
            PortTransport<PortTraits>(connectionId, name, port)
        {
        }

    protected:
        virtual void _pushSRI(const BULKIO::StreamSRI& sri)
        {
            try {
                this->_port->pushSRI(sri);
            } catch (const CORBA::SystemException& exc) {
                throw redhawk::FatalTransportError(ossie::corba::describeException(exc));
            }
        }

        virtual void _pushPacket(const SharedBufferType& data,
                                 const BULKIO::PrecisionUTCTime& T,
                                 bool EOS,
                                 const std::string& streamID)
        {
            try {
                _pushPacketImpl(data, T, EOS, streamID.c_str());
            } catch (const CORBA::SystemException& exc) {
                throw redhawk::FatalTransportError(ossie::corba::describeException(exc));
            }
        }

    private:
        void _pushPacketImpl(const SharedBufferType& data,
                             const BULKIO::PrecisionUTCTime& T,
                             bool EOS,
                             const char* streamID)
        {
            const TransportType* ptr = reinterpret_cast<const TransportType*>(data.data());
            const PortSequenceType buffer(data.size(), data.size(), const_cast<TransportType*>(ptr), false);
            this->_port->pushPacket(buffer, T, EOS, streamID);
        }
    };

    template <>
    void RemoteTransport<FilePortTraits>::_pushPacketImpl(const std::string& data,
                                                          const BULKIO::PrecisionUTCTime& T,
                                                          bool EOS,
                                                          const char* streamID)
    {
        _port->pushPacket(data.c_str(), T, EOS, streamID);
    }

    template <>
    void RemoteTransport<XMLPortTraits>::_pushPacketImpl(const std::string& data,
                                                         const BULKIO::PrecisionUTCTime& /* unused */,
                                                         bool EOS,
                                                         const char* streamID)
    {
        _port->pushPacket(data.c_str(), EOS, streamID);
    }

    template <typename PortTraits>
    class ChunkingTransport : public RemoteTransport<PortTraits>
    {
    public:
        typedef typename PortTraits::PortType PortType;
        typedef typename PortType::_ptr_type PtrType;
        typedef typename PortTraits::TransportType TransportType;
        typedef typename PortTraits::SharedBufferType SharedBufferType;

        ChunkingTransport(const std::string& connectionId, const std::string& name, PtrType port) :
            RemoteTransport<PortTraits>(connectionId, name, port)      
        {
            // Multiply by some number < 1 to leave some margin for the CORBA header
            const size_t maxPayloadSize = (size_t) (bulkio::Const::MaxTransferBytes() * .9);
            maxSamplesPerPush = maxPayloadSize/sizeof(TransportType);
        }

        /*
         * Push a packet whose payload may not fit within the CORBA limit. The
         * packet is broken down into sub-packets and sent via multiple pushPacket
         * calls.  The EOS is set to false for all of the sub-packets, except for
         * the last sub-packet, which uses the input EOS argument.
         */
        virtual void _sendPacket(const SharedBufferType& data,
                                 const BULKIO::PrecisionUTCTime& T,
                                 bool EOS,
                                 const std::string& streamID,
                                 const BULKIO::StreamSRI& sri)
        {
            double xdelta = sri.xdelta;
            size_t itemSize = sri.mode?2:1;
            size_t frameSize = itemSize;
            if (sri.subsize > 0) {
                frameSize *= sri.subsize;
            }
            // Quantize the push size (in terms of scalars) to the nearest frame,
            // which takes both the complex mode and subsize into account
            const size_t maxPushSize = (maxSamplesPerPush/frameSize) * frameSize;

            // Always do at least one push (may be empty), ensuring that all samples
            // are pushed
            size_t first = 0;
            size_t samplesRemaining = data.size();

            // Initialize time of first subpacket
            BULKIO::PrecisionUTCTime packetTime = T;
      
            do {
                // Don't send more samples than are remaining
                const size_t pushSize = std::min(samplesRemaining, maxPushSize);
                samplesRemaining -= pushSize;

                // Send end-of-stream as false for all sub-packets except for the
                // last one (when there are no samples remaining after this push),
                // which gets the input EOS.
                bool packetEOS = false;
                if (samplesRemaining == 0) {
                    packetEOS = EOS;
                }

                // Take the next slice of the input buffer.
                SharedBufferType subPacket = data.slice(first, first + pushSize);
                RemoteTransport<PortTraits>::_sendPacket(subPacket, packetTime, packetEOS, streamID, sri);

                // Synthesize the next packet timestamp
                if (packetTime.tcstatus == BULKIO::TCS_VALID) {
                    packetTime += (pushSize/itemSize)* xdelta;
                }

                // Advance buffer to next sub-packet boundary
                first += pushSize;
            } while (samplesRemaining > 0);
        }

    private:
        size_t maxSamplesPerPush;
    };


    template <typename PortTraits>
    class LocalTransport : public PortTransport<PortTraits>
    {
    public:
        typedef typename PortTraits::PortType PortType;
        typedef typename PortType::_ptr_type PtrType;
        typedef typename LocalTraits<PortTraits>::InPortType LocalPortType;
        typedef typename PortTraits::SharedBufferType SharedBufferType;

        LocalTransport(const std::string& connectionId, const std::string& name,
                       LocalPortType* localPort, PtrType port) :
            PortTransport<PortTraits>(connectionId, name, port),
            _localPort(localPort)
        {
            _localPort->_add_ref();
        }

        ~LocalTransport()
        {
            _localPort->_remove_ref();
        }

    protected:
        virtual void _pushSRI(const BULKIO::StreamSRI& sri)
        {
            _localPort->pushSRI(sri);
        }

        virtual void _pushPacket(const SharedBufferType& data,
                                 const BULKIO::PrecisionUTCTime& T,
                                 bool EOS,
                                 const std::string& streamID)
        {
            if (data.transient()) {
                // The data comes from a non-shared source (a vector or raw pointer),
                // so we need to make a copy. This could be optimized for the fanout
                // case by making the copy at a higher level, but only if there's at
                // least one local connection.
                _localPort->pushPacket(data.copy(), T, EOS, streamID);
            } else {
                _localPort->pushPacket(data, T, EOS, streamID);
            }
        }

        LocalPortType* _localPort;
    };

    template <>
    void LocalTransport<FilePortTraits>::_pushPacket(const std::string& data,
                                                     const BULKIO::PrecisionUTCTime& T,
                                                     bool EOS,
                                                     const std::string& streamID)
    {
        _localPort->pushPacket(data, T, EOS, streamID);
    }

    template <>
    void LocalTransport<XMLPortTraits>::_pushPacket(const std::string& data,
                                                    const BULKIO::PrecisionUTCTime& /*unused*/,
                                                    bool EOS,
                                                    const std::string& streamID)
    {
        _localPort->pushPacket(data, EOS, streamID);
    }

}
