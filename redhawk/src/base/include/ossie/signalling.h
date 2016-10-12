/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef REDHAWK_SIGNALLING_H
#define REDHAWK_SIGNALLING_H

#include <list>
#include <set>
#include <string>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace redhawk {

    namespace detail {
        /*
         * Tests whether the sorted ranges [first1, last1) and [first2, last2)
         * intersect.
         */
        template <class InputIterator1, class InputIterator2>
        bool intersects(InputIterator1 first1, const InputIterator1 last1,
                        InputIterator2 first2, const InputIterator2 last2)
        {
            while ((first1 != last1) && (first2 != last2)) {
                if (*first1 < *first2) {
                    ++first1;
                } else if (*first2 < *first1) {
                    ++first2;
                } else {
                    return true;
                }
            }
            return false;
        }
    }

    /*
     * A generic class to support waiting for a set of related events, such as
     * input data becoming available.
     *
     * Typically, a class will contain a signal for a particular type of event
     * (e.g., data received), where every event has a unique id of the template
     * argument type. In member functions that wish to wait for a related
     * condition to become true (e.g., there is data available to read), an
     * instance of signal::waiter can be created on the stack to manage waiting
     * for events to occur.
     */
    template <typename IdType>
    class signal {
    public:
        typedef IdType id_type;
        typedef std::set<id_type> signal_set;

        /*
         * Create a new signal.
         */
        signal()
        {
        }

        /*
         * Scope-based class for managing waiting on events from a parent
         * signal.
         *
         * Creating a waiter atomically adds it to the waiters list for its
         * parent signal to ensure that no signals are missed. To safely check
         * for the desired condition, first create a waiter, then perform any
         * desired checks (e.g., are any inputs ready for reading). This avoids
         * a potential race condition where the condition is initially false,
         * but a signal is received before the waiter is created.
         *
         * When the waiter is destroyed, typically by going out of scope, it
         * removes itself from the parent signal's waiter list.
         *
         * Each thread that wants to wait for signals should create its own
         * waiter instance. Calling wait() on the same waiter from multiple
         * threads may result in missed signals.
         */
        class waiter {
        public:
            /*
             * Create a new waiter associated with parent, that expires after
             * timeout seconds.
             *
             * A negative timeout indicates an indefinite wait. When no signals
             * are received, wait() will not return unless this waiter is
             * interrupted.
             */
            waiter(signal* parent, float timeout) :
                _M_parent(parent),
                _M_received(),
                _M_interrupted(false),
                _M_timeout(_M_end_time(timeout))
            {
                _M_parent->_M_add_waiter(this);
            }

            ~waiter()
            {
                // Automatically unregister this waiter when out of scope
                _M_parent->_M_remove_waiter(this);
            }

            /*
             * Notify this waiter that a signal was received. If signal_id is
             * not being ignored, wait() will return true.
             */
            void notify(const id_type& signal_id)
            {
                boost::mutex::scoped_lock lock(_M_mutex);
                _M_received.insert(signal_id);
                _M_cond.notify_all();
            }

            /*
             * Wait until a signal has been received. Returns true if a signal
             * was received since the last call to wait(), or false if this
             * waiter has been interrupted or the timeout expired.
             *
             * Any received signals are cleared after this function returns, so
             * subsequent calls will block unless a new signal is received.
             *
             * Only one thread may call wait() at a time. If multiple threads
             * are waiting on signals, each thread should create its own waiter
             * instance.
             */
            bool wait()
            {
                return wait(signal_set());
            }

            /*
             * Wait until a signal from the set signal_ids has been received.
             * Returns true if such a signal was received since the last call
             * to wait(), or false if this waiter has been interrupted or the
             * timeout expired.
             *
             * If signal_ids is empty, the behavior is equivalent to wait()
             * with no arguments--all signals are accepted.
             *
             * Any received signals are cleared after this function returns, so
             * subsequent calls will block unless a new signal is received.
             *
             * Only one thread may call wait() at a time. If multiple threads
             * are waiting on signals, each thread should create its own waiter
             * instance.
             */
            bool wait(const signal_set& signal_ids)
            {
                boost::mutex::scoped_lock lock(_M_mutex);
                bool result = false;
                while (!_M_interrupted && !(result = _M_check_signals(signal_ids))) {
                    if (!_M_wait_signal(lock)) {
                        break;
                    }
                }
                _M_received.clear();
                return result;
            }

            /*
             * Wakes up this waiter. Any current or future calls to wait() will
             * immediately return false.
             */
            void interrupt()
            {
                boost::mutex::scoped_lock lock(_M_mutex);
                _M_interrupted = true;
                _M_cond.notify_all();
            }

        private:
            static boost::posix_time::ptime _M_end_time(float timeout)
            {
                if (timeout < 0.0) {
                    // Negative: waiter never expires, time is positive infinity
                    return boost::posix_time::pos_infin;
                } else if (timeout > 0.0) {
                    // Positive: waiter expires timeout seconds from now
                    return boost::get_system_time() + boost::posix_time::microseconds(timeout*1e6);
                } else {
                    // Zero: never wait, use invalid time object
                    return boost::posix_time::ptime();
                }
            }

            bool _M_check_signals(const signal_set& signal_ids) const
            {
                if (signal_ids.empty()) {
                    return !_M_received.empty();
                }
                return detail::intersects(signal_ids.begin(), signal_ids.end(),
                                          _M_received.begin(), _M_received.end());
            }

            bool _M_wait_signal(boost::mutex::scoped_lock& lock)
            {
                if (_M_timeout.is_not_a_date_time()) {
                    return false;
                } else if (_M_timeout.is_pos_infinity()) {
                    _M_cond.wait(lock);
                    return true;
                } else {
                    return _M_cond.timed_wait(lock, _M_timeout);
                }
            }

            signal* const _M_parent;
            boost::mutex _M_mutex;
            boost::condition_variable _M_cond;
            signal_set _M_received;
            volatile bool _M_interrupted;
            boost::posix_time::ptime _M_timeout;
        };

        /*
         * Notify all waiters that a signal has been received. Any waiters that
         * are blocked and not filtering signal_id will return.
         */
        void notify(const id_type& signal_id)
        {
            boost::mutex::scoped_lock lock(_M_mutex);
            for (typename waiter_list::iterator ii = _M_waiters.begin(); ii != _M_waiters.end(); ++ii) {
                (*ii)->notify(signal_id);
            }
        }

        /*
         * Interrupt all waiters, causing them to return immediately.
         */
        void interrupt()
        {
            boost::mutex::scoped_lock lock(_M_mutex);
            for (typename waiter_list::iterator ii = _M_waiters.begin(); ii != _M_waiters.end(); ++ii) {
                (*ii)->interrupt();
            }
        }

    private:
        typedef std::list<waiter*> waiter_list;

        void _M_add_waiter(waiter* w)
        {
            boost::mutex::scoped_lock lock(_M_mutex);
            _M_waiters.push_back(w);
        }
        
        void _M_remove_waiter(waiter* w)
        {
            boost::mutex::scoped_lock lock(_M_mutex);
            _M_waiters.remove(w);
        }

        friend class waiter;

        boost::mutex _M_mutex;
        std::list<waiter*> _M_waiters;
    };
}

#endif // REDHAWK_SIGNALLING_H
