#ifndef BULKIO_STREAMQUEUE_H
#define BULKIO_STREAMQUEUE_H

#include <algorithm>

#define MAXIMUM_SLEEP_MICROSECONDS 10000 // 10 milliseconds

template <class T_port, typename T_data>
class streamQueue
{
public:
    typedef typename T_port::StreamType::DataBlockType DataBlockType;

protected:
    class Record {
    public:
        Record(std::string _s, short _p, BULKIO::PrecisionUTCTime _t, DataBlockType &_b) {
            Stream_id = _s;
            Priority = _p;
            Timestamp = _t;
            Block = _b;
        }
        std::string Stream_id;
        short Priority;
        BULKIO::PrecisionUTCTime Timestamp;
        DataBlockType Block;
    };

public:
    streamQueue(bool ignore_timestamp=false, bool ignore_error=false) {
        _ignore_timestamp = false;
        _ignore_error = false;
        _port = NULL;
        before_ingest_time = bulkio::time::utils::now();
        before_ingest_time.twsec = 0;
        before_ingest_time.tfsec = 0;
        sleep_overhead = 0;
        int sleep_checks = 10;
        int sleep_time = 100;

        // calibrate the amount of overhead from entering/leaving sleep
        BULKIO::PrecisionUTCTime before_time = bulkio::time::utils::now();
        for (unsigned int i=0; i<sleep_checks; i++) {
            before_time = bulkio::time::utils::now();
            boost::this_thread::sleep_for(boost::chrono::microseconds(sleep_time));
            double sleep_diff = bulkio::time::utils::now() - before_time;
            sleep_overhead += sleep_diff;
        }
        double d_sleep_time = sleep_time;
        d_sleep_time /= 1e6;
        sleep_overhead = (sleep_overhead-(d_sleep_time*sleep_checks))/sleep_checks;
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

    double getCenterFrequency(DataBlockType &block) {
        return 0;
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
                do {
                    if (it->Stream_id == *str_id) {
                        it = transmit_queue.erase(it);
                    } else {
                        it--;
                    }
                } while (it != transmit_queue.begin());
            }
        }
    }

    void ingestStreams(BULKIO::PrecisionUTCTime& now, double timeout=0) {
        bool transmit_updated = false;
        while (true) {  // loop until no streams are available
            typename T_port::StreamType inputStream = _port->getCurrentStream(timeout);
            if (!inputStream) { // No streams are available
                break;
            }
            std::string stream_id = ossie::corba::returnString(inputStream.sri().streamID);
            DataBlockType block = inputStream.read();
            if (inputStream.eos()) {
            }
            if (!block) { // No data available
                continue;
            }
            short priority = 0;
            if (held_queue.find(stream_id) == held_queue.end()) {
                transmit_queue.push_back(Record(stream_id, priority, block.getStartTime(), block));
                transmit_updated = true;
            } else {
                held_queue[stream_id].push_back(Record(stream_id, priority, block.getStartTime(), block));
            }
        }
        if (transmit_updated)
            std::sort(transmit_queue.begin(), transmit_queue.end(), CompareRecords);
        verifyTimes(now);
    };

    DataBlockType getNextBlock(BULKIO::PrecisionUTCTime now, double timeout=0, double future_window=100e-6) {
        boost::mutex::scoped_lock lock(queuesMutex);
        BULKIO::PrecisionUTCTime initial_time = bulkio::time::utils::now();
        double fixed_time_offset = now - initial_time;
        initial_time += fixed_time_offset;

        DataBlockType retval;
        double _timeout = 0;
        double orig_timeout = timeout;
        double half_future_window_us = future_window * 5e7;
        bool breakout = false;

        before_ingest_time = bulkio::time::utils::now()+fixed_time_offset;
        while (true) {
            ingestStreams(before_ingest_time, _timeout);
            double current_time_diff = (bulkio::time::utils::now()+fixed_time_offset) - initial_time;
            if ((current_time_diff > timeout) and (timeout != 0)) { // we've hit a timeout condition; leave now
                break;
            }
            if (not transmit_queue.empty()) {
                if (future_window > 0) { // future_window < 0 means "just give me the front of the queue
                    DataBlockType inspect_block = transmit_queue.front().Block;
                    double block_timediff = inspect_block.getStartTime() - (bulkio::time::utils::now()+fixed_time_offset);
                    if (block_timediff > future_window+sleep_overhead) { // the time different is not quite there yet
                        long int sleep_time = (block_timediff-future_window)*1e6;
                        if (sleep_time > half_future_window_us) {
                            sleep_time = half_future_window_us;
                            breakout = false;
                        } else {
                            breakout = true;
                            sleep_time -= sleep_overhead;
                        }
                        boost::this_thread::sleep_for(boost::chrono::microseconds(sleep_time));
                        if (not breakout) { // check for any changes in the queue
                            continue;
                        }
                    }
                }
                retval = transmit_queue.front().Block;
                transmit_queue.pop_front();
                break;
            } else {    // transmit queue is emtpy
                double ingest_time = (bulkio::time::utils::now()+fixed_time_offset) - before_ingest_time;
                _timeout = (ingest_time > future_window*0.75)?future_window*0.5:ingest_time*0.5;
                if (_timeout < 1e9) {
                    _timeout = future_window*0.75;
                }
                double current_time_diff = (bulkio::time::utils::now()+fixed_time_offset) - initial_time;
                if (current_time_diff > (orig_timeout-_timeout)) {
                    _timeout = orig_timeout-current_time_diff-future_window;
                    if (_timeout<0) {
                        _timeout = 0;
                    }
                }
                before_ingest_time = (bulkio::time::utils::now()+fixed_time_offset);
            }
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
    BULKIO::PrecisionUTCTime before_ingest_time;
    double sleep_overhead;
};

class data_snk_i : public data_snk_base
{
    ENABLE_LOGGING
public:
    data_snk_i(const char *uuid, const char *label);
    ~data_snk_i();

    void constructor();

    int serviceFunction();

    streamQueue<bulkio::InShortPort, short> queue;
};

#endif
