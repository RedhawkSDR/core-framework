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

#include "ProcessThread.h"
#include "CF/DataType.h"

enum {
    NOOP   = 0,
    FINISH = -1,
    NORMAL = 1
};

//
// Mix-in class for threaded components and devices
//
class ThreadedComponent {
    friend class ossie::ProcessThread;
    ENABLE_LOGGING;
public:
    virtual ~ThreadedComponent ();

    // Main work function (to be implemented by subclass)
    virtual int serviceFunction () = 0;

    /**
     * Return the time at which ProcessThread::run() processed FINISH.
     */
    CF::UTCTime getFinishedTime ();

    /** About isFinished(), isRunning(), and wasStopCalled():
     * 
     * More than one underlying thread, but only one at a time, can be
     * created from startThread().
     * 
     * Before FINISH is processed, this sequence is possble:
     * - user calls startThread()
     * - thread-A is created and starts processing
     * - user calls stopThread()
     * - thread-A is interrupted and deleted
     * - user calls startThread()
     * - thread-B is created and starts processing
     * 
     * Once FINISH is processed, startThread() will not create more threads.
     */

    /**
     * True means that ProcessThread::run() processed a FINISH return
     * value from serviceFunction().
     * 
     * Return false before ProcessThread::run() processes FINISH.
     * Return true after ProcessThread::run() processes FINISH.
     */
    bool isFinished ();

     /**
     * For each underlying thread, isRunning() returns false before
     * startThread() is called. It returns true after startThread()
     * returns. Then it returns false after ProcessThread::run()
     * returns, which happens in these cases:
     *   a. It processes FINISH.
     *   b. It processes any return from serviceFunction() after
     *      stopThread() has been called.
     *   c. serviceFunction() throws thread_interrupted exception
     *   d. serviceThread is interrupted while it sleeps after a NOOP
     * 
     * In the case that a previous thread was destroyed and startThread()
     * starts a new thread, then isRunning() returns true after
     * startThread() returns.
     * 
     * stopThread() causes a thread->interrupt(), but that may not
     * terminate the thread.  As a result, isRunning() may not change
     * to false as a result of the stopThread().
     */
    bool isRunning ();

    /** 
     * True means stopThread() was called, before FINISH was processed,
     * for the latest ProcessThread::_thread, regardless of whether that
     * thread is actually running.
     * 
     * For the latest ProcessThread::_thread:
     * - return false before stopThread() is called.
     * - return true after stopThread() returns, if stopThread() returned
     *       before FINISH was processed.
     */
    bool wasStopCalled ();

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

    ossie::ProcessThread* serviceThread;
    boost::mutex serviceThreadLock;

    void setThreadName(const std::string& name);

private:
    void setFinished(bool val);
    void setRunning(bool val);
    void setFinishedTime();

    std::string _threadName;
    float _defaultDelay;

    bool _finished;
    bool _running;
    bool _stopped;

    CF::UTCTime _finishedTime;
    boost::mutex _finishedTimeMutex;
};

#endif // OSSIE_THREADEDCOMPONENT_H
