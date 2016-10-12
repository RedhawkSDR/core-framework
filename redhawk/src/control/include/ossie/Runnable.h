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

#ifndef OSSIE_RUNNABLE_H
#define OSSIE_RUNNABLE_H

#include <omniORB4/CORBA.h>

namespace ossie {

    class Runnable {
    public:
        void run (void)
        {
            mutex_.lock();
            while (isRunning_) {
                cond_.wait();
            }
            mutex_.unlock();
        }

        void halt (void)
        {
            mutex_.lock();
            isRunning_ = false;
            cond_.signal();
            mutex_.unlock();
        }

    protected:
        Runnable () :
            mutex_(),
            cond_(&mutex_),
            isRunning_(true)
        {
        }

    private:
        omni_mutex mutex_;
        omni_condition cond_;
        volatile bool isRunning_;
    };

};

#endif // OSSIE_RUNNABLE_H
