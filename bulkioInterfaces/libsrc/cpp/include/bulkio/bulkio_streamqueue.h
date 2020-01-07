#include <algorithm>

namespace bulkio {
    template <class T_port, typename T_data>
    class streamQueue
    {
    public:

        class txBlock : public T_port::StreamType::DataBlockType {
        public:
            explicit txBlock(const bulkio::StreamDescriptor& sri, const redhawk::shared_buffer<T_data>& buffer) : T_port::StreamType::DataBlockType(sri, buffer){
                delegated_constructor();
            };
            txBlock() {
                delegated_constructor();
            }
            double getCenterFrequency() {
                return center_frequency;
            }
        protected:
            void delegated_constructor(bool valid=false) {
                center_frequency = 0;
            }
            double center_frequency;
        };

    protected:
        class Record {
        public:
            Record(std::string _s, short _p, BULKIO::PrecisionUTCTime _t, txBlock _b) {
                Stream_id = _s;
                Priority = _p;
                Timestamp = _t;
                Block = _b;
            }
            std::string Stream_id;
            short Priority;
            BULKIO::PrecisionUTCTime Timestamp;
            txBlock Block;
        };

    public:
        streamQueue(bool ignore_timestamp=false, bool ignore_error=false) {
            _ignore_timestamp = false;
            _ignore_error = false;
            _port = NULL;
        };

        void update_port(T_port* port) {
            _port = port;
        };

        void update_ignore_timestamp(bool ignore_timestamp) {
            _ignore_timestamp = ignore_timestamp;
        };

        void update_ignore_error(bool ignore_error) {
            _ignore_error = ignore_error;
        };

        void throw_stream_error(std::string stream_id, std::string message) {
        };

        std::vector<std::string> held() {
            boost::mutex::scoped_lock lock(queuesMutex);
            std::vector<std::string> stream_ids;
            typename std::map<std::string, std::deque<Record> >::iterator it;
            for(it = held_queue.begin(); it != held_queue.end(); ++it) {
                stream_ids.push_back(it->first);
            }
            return stream_ids;
        }

        bool hold(std::string stream_id) {
            boost::mutex::scoped_lock lock(queuesMutex);
            bool retval = false;
            if (held_queue.find(stream_id) == held_queue.end()) {
                held_queue.insert(std::pair<std::string, std::vector<Record> >(stream_id, std::vector<Record>()));
            }
            typename std::deque<Record>::iterator it = transmit_queue.end();
            it--;
            while (it > transmit_queue.begin()) {
                if (it->Stream_id == stream_id) {
                    retval = true;
                    held_queue[stream_id].push_front(*it);
                    it = transmit_queue.erase(it);
                } else {
                    it--;
                }
            }
            return retval;
        }

        bool allow(std::string stream_id) {
            boost::mutex::scoped_lock lock(queuesMutex);
            bool retval = false;
            if (held_queue.find(stream_id) != held_queue.end()) {
                transmit_queue.insert(transmit_queue.begin(), held_queue[stream_id].begin(), held_queue[stream_id].end());
                held_queue.erase(stream_id);
                retval = true;
                std::sort(transmit_queue.begin(), transmit_queue.end(), CompareRecords);
            }
            return retval;
        }

        void reset() {
            boost::mutex::scoped_lock lock(queuesMutex);
            transmit_queue.clear();
            held_queue.clear();
        }

        void verifyTimes(BULKIO::PrecisionUTCTime now) {
            if (TimeZero(now))
                return;
            typename std::deque<Record>::iterator it = transmit_queue.begin();
            std::vector<std::string> error_stream_ids;
            while (it != transmit_queue.end()) {
                if (TimeZero(it->Timestamp)) {
                    continue;
                }
                // no sense in checking the timestamp if the stream id has already been flagged
                if (std::find(error_stream_ids.begin(), error_stream_ids.end(), it->Stream_id) == error_stream_ids.end()) {
                    if (it->Timestamp < now) {
                        error_stream_ids.push_back(it->Stream_id);
                    }
                }
                it++;
            }
            for (std::vector<std::string>::iterator str_id=error_stream_ids.begin(); str_id!=error_stream_ids.end(); str_id++) {
                if (held_queue.erase(*str_id) == 0) {   // if nothing was erased from held_queue, there might be something in transmit_queue
                    it = transmit_queue.end();
                    it--;
                    while (it > transmit_queue.begin()) {
                        if (it->Stream_id == *str_id) {
                            it = transmit_queue.erase(it);
                        } else {
                            it--;
                        }
                    }
                }
            }
        }

        void ingestStreams(BULKIO::PrecisionUTCTime now) {
            boost::mutex::scoped_lock lock(queuesMutex);
            while (true) {  // loop until no streams are available
                typename T_port::StreamType inputStream = _port->getCurrentStream(0);
                if (!inputStream) { // No streams are available
                    break;
                }
                std::string stream_id = ossie::corba::returnString(inputStream.sri().streamID);
                short priority = 0;
                typename T_port::StreamType::DataBlockType block = inputStream.read();
                txBlock tx_block(block.sri(), block.buffer());
                if (!block) { // No data available
                    if (inputStream.eos()) {
                    }
                    continue;
                }
                if (held_queue.find(stream_id) == held_queue.end()) {
                    transmit_queue.push_back(Record(stream_id, priority, block.getStartTime(), tx_block));
                } else {
                    held_queue[stream_id].push_back(Record(stream_id, priority, block.getStartTime(), tx_block));
                }
            }
            std::sort(transmit_queue.begin(), transmit_queue.end(), CompareRecords);
            verifyTimes(now);
        };

        txBlock getNextBlock(BULKIO::PrecisionUTCTime now, bool blocking=false) {
            verifyTimes(now);
            txBlock retval;
            if (not transmit_queue.empty()) {
                retval = transmit_queue.front().Block;
                transmit_queue.pop_front();
            }
            return retval;
        };

    protected:
        static bool TimeZero(const BULKIO::PrecisionUTCTime& a)
        {
            if ((a.tfsec == 0) and (a.twsec == 0))
                return true;
            return false;
        }
        static bool CompareRecords(const Record& a, const Record& b)
        {
            if (a.Timestamp < b.Timestamp) // low timestamp vs high
                return true;
            else if (a.Timestamp > b.Timestamp)
                return false;

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

    };
}

