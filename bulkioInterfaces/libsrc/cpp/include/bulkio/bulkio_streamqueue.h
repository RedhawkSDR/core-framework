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

#ifndef BULKIO_STREAMQUEUE_H
#define BULKIO_STREAMQUEUE_H

#include "bulkio_base.h"
#include "bulkio_time_operators.h"
#include <ossie/CorbaUtils.h>
#include <ossie/PropertyMap.h>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <deque>
#include <boost/thread/shared_mutex.hpp>

namespace bulkio {

    class StreamStatus {
    public:
        std::string stream_id;
        CF::DeviceStatusCodeType code;
        std::string message;
    };

    template <class T_port>
    class StreamQueue
    {
    public:
        typedef typename T_port::StreamType::DataBlockType DataBlockType;

    protected:
        class Record {
        public:
            Record() {
            };

            Record(std::string _s, short _p, BULKIO::PrecisionUTCTime _t, DataBlockType &_b) {
                Stream_id = _s;
                Priority = _p;
                Timestamp = _t;
                Block = _b;
            };

            std::string Stream_id;
            short Priority;
            BULKIO::PrecisionUTCTime Timestamp;
            DataBlockType Block;
        };

    public:
        StreamQueue();

        void update_port(T_port* port);

        void update_ignore_timestamp(bool ignore_timestamp);

        void update_ignore_error(bool ignore_error);

        double getCenterFrequency(DataBlockType &block);

        std::vector<std::string> held();

        bool hold(std::string stream_id);

        bool allow(const std::string &stream_id);

        void reset();

        void reset(const std::string &stream_id);

        DataBlockType getNextBlock(BULKIO::PrecisionUTCTime &now, std::vector<bulkio::StreamStatus> &error_status, double timeout=0, double future_window=1e-3);

    protected:
        void verifyTimes(BULKIO::PrecisionUTCTime now);

        void ingestStreams(BULKIO::PrecisionUTCTime& now, double timeout=0);

        static bool TimeZero(const BULKIO::PrecisionUTCTime& a) {
            if ((a.tfsec == 0) and (a.twsec == 0))
                return true;
            return false;
        }

        static bool CompareRecords(const Record& a, const Record& b) {
            // if either is zero, ignore and move to priority
            // moving zero to the front could block non-zero
            // leaving zero in the order in which it arrives means that
            //  zero could be sent but isn't
            if (not (TimeZero(a.Timestamp) or TimeZero(b.Timestamp))) {
                if (a.Timestamp < b.Timestamp) // low timestamp vs high
                    return true;
                else if (a.Timestamp > b.Timestamp)
                    return false;
            }

            if (a.Priority > b.Priority) // high priority before low
                return true;
            else if (a.Priority < b.Priority)
                return false;

            return false;
        }

        bool _ignore_timestamp, _ignore_error;
        T_port* _port;
        boost::mutex queuesMutex;
        std::deque<Record> transmit_queue;
        std::map<std::string, std::deque<Record> > held_queue;
        std::map<std::string, StreamStatus> error_streams;
        BULKIO::PrecisionUTCTime before_ingest_time;
        double sleep_overhead;
    };
}

#endif
