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

#include <ossie/ExecutorService.h>

using namespace redhawk;

ExecutorService::ExecutorService() :
    _thread(0),
    _running(false)
{
}

ExecutorService::~ExecutorService()
{
    stop();
}

void ExecutorService::start ()
{
    boost::mutex::scoped_lock lock(_mutex);
    if (_running) {
        return;
    }

    _running = true;
    _thread = new boost::thread(&ExecutorService::_run, this);
}

void ExecutorService::stop ()
{
    boost::thread* old_thread = 0;
    {
        boost::mutex::scoped_lock lock(_mutex);
        _running = false;
        old_thread = _thread;
        _thread = 0;
        _cond.notify_all();
    }
    if (old_thread) {
        old_thread->join();
        delete old_thread;
    }
}

void ExecutorService::clear ()
{
    boost::mutex::scoped_lock lock(_mutex);
    _queue.clear();
    _cond.notify_all();
}

size_t ExecutorService::pending ()
{
    boost::mutex::scoped_lock lock(_mutex);
    return _queue.size();
}

void ExecutorService::_run ()
{
    boost::mutex::scoped_lock lock(_mutex);
    while (_running) {
        if (_queue.empty()) {
            _cond.wait(lock);
        } else {
            task_queue::iterator task = _queue.begin();
            if (task->first > boost::get_system_time()) {
                // Head of queue is scheduled in the future
                boost::system_time when = task->first;
                _cond.timed_wait(lock, when);
            } else {
                // Copy the task's function and remove it from the queue
                func_type func = task->second;
                _queue.erase(task);

                // Run task with the lock released
                lock.unlock();
                func();
                lock.lock();
            }
        }
    }
}

void ExecutorService::_insertSorted (func_type func, boost::system_time when)
{
    boost::mutex::scoped_lock lock(_mutex);
    task_queue::iterator pos = _queue.begin();
    while ((pos != _queue.end()) && (when > pos->first)) {
        ++pos;
    }
    _queue.insert(pos, std::make_pair(when, func));
    _cond.notify_all();
}
