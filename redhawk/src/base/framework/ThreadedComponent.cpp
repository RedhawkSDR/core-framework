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

#include <ossie/ThreadedComponent.h>
#include <ossie/CorbaUtils.h>

namespace ossie {

PREPARE_CF_LOGGING(ProcessThread);

ProcessThread::ProcessThread(ThreadedComponent *target, float delay, const std::string& name) :
    _thread(0),
    _running(false),
    _target(target),
    _name(name),
    _mythread(_thread)
{
    updateDelay(delay);
}

void ProcessThread::start()
{
    if (!_thread) {
        _running = true;
        _thread = new boost::thread(&ProcessThread::run, this);
    }
}

void ProcessThread::run()
{
    // If a name was given, set it on the current thread
    // NB: On RHEL/CentOS 6, the name is limited to 15 characters, and the call
    //     fails if the name exceeds that limit
    if (!_name.empty()) {
        std::string name = _name.substr(0, 15);
        pthread_setname_np(pthread_self(), name.c_str());
    }

    while (_running) {
        int state;
        try {
            state = _target->serviceFunction();
        } catch (const std::exception& exc) {
            LOG_FATAL(ProcessThread, "Unhandled exception in service function: " << exc.what());
            exit(-1);
        } catch (const CORBA::Exception& exc) {
            LOG_FATAL(ProcessThread, "Unhandled exception in service function: "
                      << ossie::corba::describeException(exc));
            exit(-1);
        } catch (boost::thread_interrupted &) {
            break;
        } catch (...) {
            LOG_FATAL(ProcessThread, "Unhandled exception in service function");
            exit(-1);
        }
        if (state == FINISH) {
            return;
        } else if (state == NOOP) {
            try {
                boost::posix_time::time_duration boost_delay = boost::posix_time::microseconds(_delay.tv_sec*1e6 + _delay.tv_nsec*1e-3);
                boost::this_thread::sleep(boost_delay);
            } catch (boost::thread_interrupted &) {
                break;
            } catch (...) {
                throw;
            }
        }
        else {
            boost::this_thread::yield();
        }
    }
}

bool ProcessThread::release(unsigned long secs, unsigned long usecs)
{
    _running = false;
    this->stop();
    if (_thread)  {
        if ((secs == 0) && (usecs == 0)){
            _thread->join();
        } else {
            boost::system_time waitime = boost::get_system_time() + boost::posix_time::seconds(secs) +  boost::posix_time::microseconds(usecs);
            if (!_thread->timed_join(waitime)) {
                return false;
            }
        }
        delete _thread;
        _thread = 0;
    }
    
    return true;
}

void ProcessThread::stop() {
    _running = false;
    if ( _thread ) _thread->interrupt();
}

ProcessThread::~ProcessThread()
{
    if (_thread) {
        release(0);
        _thread = 0;
    }
}

void ProcessThread::updateDelay(float delay)
{
    _delay.tv_sec = (time_t)delay;
    _delay.tv_nsec = (delay-_delay.tv_sec)*1e9;
}

bool ProcessThread::threadRunning()
{
    return _running;
}

};   //end of ossie namespace

ThreadedComponent::ThreadedComponent() :
    serviceThread(0),
    serviceThreadLock(),
    _threadName(),
    _defaultDelay(0.1)
{
}

ThreadedComponent::~ThreadedComponent()
{
}

void ThreadedComponent::startThread ()
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (!serviceThread) {
        serviceThread = new ossie::ProcessThread(this, _defaultDelay, _threadName);
        serviceThread->start();
    }
}

bool ThreadedComponent::stopThread ()
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (serviceThread) {
        if (!serviceThread->release(2)) {
            return false;
        }
        delete  serviceThread;
        serviceThread = 0;
    }
    return true;
}

float ThreadedComponent::getThreadDelay ()
{
    return _defaultDelay;
}

void ThreadedComponent::setThreadDelay (float delay)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    _defaultDelay = delay;
    if (serviceThread) {
        serviceThread->updateDelay(delay);
    }
}

void ThreadedComponent::setThreadName (const std::string& name)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    _threadName = name;
}
