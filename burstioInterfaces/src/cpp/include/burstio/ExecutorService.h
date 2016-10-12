/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK burstioInterfaces.
 *
 * REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef BURSTIO_EXECUTORSERVICE_H
#define BURSTIO_EXECUTORSERVICE_H

#include <list>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>

namespace burstio {

    class ExecutorService {
    public:
        ExecutorService() :
            thread_(0),
            running_(false)
        {
        }

        void start ()
        {
            boost::mutex::scoped_lock lock(mutex_);
            if (running_) {
                return;
            }

            running_ = true;
            thread_ = new boost::thread(&ExecutorService::run, this);
        }

        void stop ()
        {
            boost::thread* old_thread = 0;
            {
                boost::mutex::scoped_lock lock(mutex_);
                running_ = false;
                old_thread = thread_;
                thread_ = 0;
                cond_.notify_all();
            }
            if (old_thread) {
                old_thread->join();
                delete old_thread;
            }
        }

        template <class F>
        void execute (F func)
        {
            insert_sorted(func);
        }

        template <class F, class A1>
        void execute (F func, A1 arg1)
        {
            insert_sorted(boost::bind(func, arg1));
        }

        template <class F>
        void schedule (boost::system_time when, F func)
        {
            insert_sorted(func, when);
        }

        template <class F, class A1>
        void schedule (boost::system_time when, F func, A1 arg1)
        {
            insert_sorted(boost::bind(func, arg1), when);
        }

        void clear ()
        {
            boost::mutex::scoped_lock lock(mutex_);
            queue_.clear();
            cond_.notify_all();
        }

    private:
        typedef boost::function<void ()> func_type;
        typedef std::pair<boost::system_time,func_type> task_type;
        typedef std::list<task_type> task_queue;

        void run ()
        {
            boost::mutex::scoped_lock lock(mutex_);
            while (running_) {
                while (!queue_.empty()) {
                    // Start at the front of the queue every time--a task may
                    // have been added while the lock was released to service
                    // the last task
                    task_queue::iterator task = queue_.begin();
                    if (task->first > boost::get_system_time()) {
                        // Head of queue is scheduled in the future
                        break;
                    }

                    // Copy the task's function and remove it from the queue
                    func_type func = task->second;
                    queue_.erase(task);

                    // Run task with the lock released
                    lock.unlock();
                    func();
                    lock.lock();
                }

                if (queue_.empty()) {
                    cond_.wait(lock);
                } else {
                    boost::system_time when = queue_.front().first;
                    cond_.timed_wait(lock, when);
                }
            }
        }

        void insert_sorted (func_type func, boost::system_time when=boost::get_system_time())
        {
            boost::mutex::scoped_lock lock(mutex_);
            task_queue::iterator pos = queue_.begin();
            while ((pos != queue_.end()) && (when > pos->first)) {
                ++pos;
            }
            queue_.insert(pos, std::make_pair(when, func));
            cond_.notify_all();
        }


        boost::mutex mutex_;
        boost::condition_variable cond_;
        
        boost::thread* thread_;
        task_queue queue_;
        bool running_;
    };

}

#endif // BURSTIO_EXECUTORSERVICE_H
