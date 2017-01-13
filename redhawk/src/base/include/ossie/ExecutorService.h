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

#ifndef REDHAWK_EXECUTORSERVICE_H
#define REDHAWK_EXECUTORSERVICE_H

#include <list>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>

namespace redhawk {

    /**
     * @brief  A class for scheduling functions to run at a later time.
     *
     * %ExecutorService provides an interface for queueing functions to run on
     * another thread at a specified time. This can be used for implementing
     * periodic monitors, executing deferred callbacks, or other operations
     * that do not need to be performed immediately (and do not require a
     * return value).
     */
    class ExecutorService {
    public:
        /**
         * @brief  Construct an %ExecutorService.
         *
         * The %ExecutorService is created in a stopped state.  To begin
         * executing scheduled functions, call start().
         */
        ExecutorService();

        /**
         * @brief  Destroys the %ExecutorService.
         *
         * The executor thread is stopped and all queued functions are purged.
         */
        ~ExecutorService();

        /**
         * @brief  Starts executing scheduled functions.
         *
         * If the executor thread is not running, it is started. Any functions
         * scheduled for the current time (or earlier) will be run at the next
         * possible time.
         */
        void start ();

        /**
         * @brief  Stops executing scheduled functions.
         *
         * If the executor thread is running, it is stopped. Any remaining
         * scheduled functions will not be run until the %ExecutorService is
         * started again.
         */
        void stop ();

        /**
         * @brief  Calls a function on the executor thread.
         * @param func  Callable object.
         *
         * Queues the callable object @a func to be called on the executor
         * thread at the next possible time. This function does not wait for
         * @a func to execute.
         *
         * If the %ExecutorService is not running, @a func will run after the
         * next call to start().
         */
        template <class F>
        void execute (F func)
        {
            _insertSorted(func);
        }

        /**
         * @brief  Calls a function on the executor thread.
         * @param func  Callable object.
         * @param A1  Argument to pass to callable object.
         *
         * Queues the callable object @a func to be called with the single
         * argument @a A1 on the executor thread at the next possible time.
         * This function does not wait for @a func to execute.
         *
         * If @a func is a class member function, the class instance should be
         * passed as @a A1.
         *
         * If the %ExecutorService is not running, @a func will run after the
         * next call to start().
         */
        template <class F, class A1>
        void execute (F func, A1 arg1)
        {
            _insertSorted(boost::bind(func, arg1));
        }

        /**
         * @brief  Calls a function on the executor thread.
         * @param func  Callable object.
         * @param A1  First argument to pass to callable object.
         * @param A2  Second argument to pass to callable object.
         *
         * Queues the callable object @a func to be called with the arguments
         * @a A1 and @a A2 on the executor thread at the next possible time.
         * This function does not wait for @a func to execute.
         *
         * If @a func is a class member function, the class instance should be
         * passed as @a A1.
         *
         * If the %ExecutorService is not running, @a func will run after the
         * next call to start().
         */
        template <class F, class A1, class A2>
        void execute (F func, A1 arg1, A2 arg2)
        {
            _insertSorted(boost::bind(func, arg1, arg2));
        }

        /**
         * @brief  Schedules a function on the executor thread.
         * @param when  The time at which to run the function.
         * @param func  Callable object.
         *
         * Queues the callable object @a func to be called on the executor
         * thread at @a when. The actual time at which it runs is guaranteed to
         * be at least @a when, but may be later, depending on the system and
         * what the %ExecutorService is doing at the time.
         *
         * If the %ExecutorService is not running at the scheduled time,
         * @a func will run after the next call to start().
         */
        template <class F>
        void schedule (boost::system_time when, F func)
        {
            _insertSorted(func, when);
        }

        /**
         * @brief  Schedules a function on the executor thread.
         * @param when  The time at which to run the function.
         * @param func  Callable object.
         * @param A1  Argument to pass to callable object.
         *
         * Queues the callable object @a func to be called with the single
         * argument @a A1 on the executor thread at @a when. The actual time
         * at which it runs is guaranteed to be at least @a when, but may be
         * later, depending on the system and what the %ExecutorService is
         * doing at the time.
         *
         * If @a func is a class member function, the class instance should be
         * passed as @a A1.
         *
         * If the %ExecutorService is not running at the scheduled time,
         * @a func will run after the next call to start().
         */
        template <class F, class A1>
        void schedule (boost::system_time when, F func, A1 arg1)
        {
            _insertSorted(boost::bind(func, arg1), when);
        }

        /**
         * @brief  Schedules a function on the executor thread.
         * @param when  The time at which to run the function.
         * @param func  Callable object.
         * @param A1  First argument to pass to callable object.
         * @param A2  Second argument to pass to callable object.
         *
         * Queues the callable object @a func to be called with the arguments
         * @a A1 and @a A2 on the executor thread at @a when. The actual time
         * at which it runs is guaranteed to be at least @a when, but may be
         * later, depending on the system and what the %ExecutorService is
         * doing at the time.
         *
         * If @a func is a class member function, the class instance should be
         * passed as @a A1.
         *
         * If the %ExecutorService is not running at the scheduled time,
         * @a func will run after the next call to start().
         */
        template <class F, class A1, class A2>
        void schedule (boost::system_time when, F func, A1 arg1, A2 arg2)
        {
            _insertSorted(boost::bind(func, arg1, arg2), when);
        }

        /**
         * @brief  Discards all pending functions.
         */
        void clear ();

        /**
         * @brief  Returns the number of queued functions to be run.
         */
        size_t pending ();

    private:
        /// @cond IMPL

        typedef boost::function<void ()> func_type;
        typedef std::pair<boost::system_time,func_type> task_type;
        typedef std::list<task_type> task_queue;

        // Thread main function.
        void _run ();

        // Inserts a callable object into the task queue at the given time,
        // defaulting to now (i.e., run at the next possible time).
        void _insertSorted (func_type func, boost::system_time when=boost::get_system_time());

        // Mutex/condvar pair to synchronize access and handle waiting for the
        // next scheduled event.
        boost::mutex _mutex;
        boost::condition_variable _cond;
        
        // Executor thread and control flag.
        boost::thread* _thread;
        volatile bool _running;

        // Function queue, sorted by scheduled time such that the first item on
        // the queue is the next function to call.
        task_queue _queue;

        /// @endcond
    };

}

#endif // REDHAWK_EXECUTORSERVICE_H
