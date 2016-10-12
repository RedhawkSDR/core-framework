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

#ifndef OSSIE_THREADEDCOMPONENT_H
#define OSSIE_THREADEDCOMPONENT_H

#include <time.h>
#include <boost/thread.hpp>

enum {
    NOOP   = 0,
    FINISH = -1,
    NORMAL = 1
};

class ThreadedComponent;

class ProcessThread
{
public:
    ProcessThread(ThreadedComponent* target, float delay);
    ~ProcessThread();

    // Kicks off the thread
    void start ();

    // Manages calls to target's service function
    void run ();

    // Stops thread and wait for termination
    bool release (unsigned long secs = 0, unsigned long usecs = 0);

    void stop();

    // Changes the delay between calls to service function after a NOOP
    void updateDelay (float delay);

    bool threadRunning();

private:
    boost::thread* _thread;
    volatile bool _running;
    ThreadedComponent* _target;
    struct timespec _delay;

public: 
    boost::thread*& _mythread;
};

//
// Mix-in class for threaded components and devices
//
class ThreadedComponent {
public:
    virtual ~ThreadedComponent ();

    // Main work function (to be implemented by subclass)
    virtual int serviceFunction () = 0;

protected:
    ThreadedComponent ();

    // Starts the processing thread, if necessary
    void startThread ();

    // Stops the processing thread, if necessary
    bool stopThread ();

    // Returns the current delay between calls to service function after a NOOP
    float getThreadDelay ();

    // Changes the delay between calls to service function after a NOOP
    void setThreadDelay (float delay);

    ProcessThread* serviceThread;
    boost::mutex serviceThreadLock;

private:
    float _defaultDelay;
};

#endif // OSSIE_THREADEDCOMPONENT_H
