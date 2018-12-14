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

#ifndef REDHAWK_SHARED_MUTEX_H
#define REDHAWK_SHARED_MUTEX_H

#include <pthread.h>

namespace redhawk {

    class shared_mutex {
    public:
        shared_mutex()
        {
            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
            pthread_mutex_init(&_mutex, &attr);
            pthread_mutexattr_destroy(&attr);
        }

        ~shared_mutex()
        {
            pthread_mutex_destroy(&_mutex);
        }

        void lock()
        {
            pthread_mutex_lock(&_mutex);
        }

        bool trylock()
        {
            if (pthread_mutex_trylock(&_mutex)) {
                return false;
            }
            return true;
        }

        void unlock()
        {
            pthread_mutex_unlock(&_mutex);
        }

    private:
        pthread_mutex_t _mutex;
    };

    class scoped_lock {
    public:
        scoped_lock(shared_mutex& mutex, bool acquire=true) :
            _mutex(&mutex),
            _locked(false)
        {
            if (acquire) {
                lock();
            }
        }

        ~scoped_lock()
        {
            unlock();
        }

        bool trylock()
        {
            if (!_locked) {
                _locked = _mutex->trylock();
            }
            return _locked;
        }

        void lock()
        {
            if (!_locked) {
                _mutex->lock();
                _locked = true;
            }
        }

        void unlock()
        {
            if (_locked) {
                _mutex->unlock();
                _locked = false;
            }
        }

        shared_mutex* mutex()
        {
            return _mutex;
        }

    private:
        shared_mutex* _mutex;
        bool _locked;
    };
}

#endif // SHARED_MUTEX_H
