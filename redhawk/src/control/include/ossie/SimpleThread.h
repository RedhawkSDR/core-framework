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
#ifndef __RH_CNTRL_SIMPLE_THREAD_H__
#define __RH_CNTRL_SIMPLE_THREAD_H__

#include <iostream>
#include <string>
#include <cctype>
#include <exception>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/resource.h>
#include <sys/time.h>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/thread.hpp>

class SimpleThread {

public:

  enum {
    NOOP   = 0,
    FINISH = -1,
    NORMAL = 1
  };

  typedef  int (*PFunc)(void);

private:
  boost::thread* _thread;
  volatile bool _running;
  PFunc          _target;
  struct timespec _delay;

public: 
  boost::thread*& _mythread;

public:
  SimpleThread( PFunc target, float delay=0.5) :
    _thread(0),
    _running(false),
    _target(target),
    _mythread(_thread)
  {
    updateDelay(delay);
  }

  void start() {
    if (!_thread) {
      _running = true;
      _thread = new boost::thread(&SimpleThread::run, this);
    }
  }

  void run()
  {
    while (_running) {
      int state = _target();
      if (state == FINISH) {
        return;
      } else if (state == NOOP) {
        nanosleep(&_delay, NULL);
      }
      else {
        boost::this_thread::yield();
      }
    }
  }

  bool release(unsigned long secs=0, unsigned long usecs=0) {

    _running = false;
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

  void stop() {
    _running = false;
    if ( _thread ) _thread->interrupt();
  }

  ~SimpleThread()
  {
    if (_thread) {
      release(0);
      _thread = 0;
    }
  }

  void updateDelay(float delay)
  {
    _delay.tv_sec = (time_t)delay;
    _delay.tv_nsec = (delay-_delay.tv_sec)*1e9;
  }

  bool threadRunning()
  {
    return _running;
  }

};

#endif
