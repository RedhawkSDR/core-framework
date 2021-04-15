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

#ifndef PROGRAMMABLEDEVICE_IMPL_BASE_H
#define PROGRAMMABLEDEVICE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/ExecutableDevice_impl.h>
#include <ossie/CF/AggregateDevices.h>
#include <ossie/AggregateDevice_impl.h>

#include "struct_props.h"

#define NOOP 0
#define FINISH -1
#define NORMAL 1

class ProgrammableDevice_base;

template < typename TargetClass >
class ProcessThread
{
    public:
        ProcessThread(TargetClass *_target, float _delay) :
            target(_target)
        {
            _mythread = 0;
            _thread_running = false;
            _udelay = (__useconds_t)(_delay * 1000000);
        };

        // kick off the thread
        void start() {
            if (_mythread == 0) {
                _thread_running = true;
                _mythread = new boost::thread(&ProcessThread::run, this);
            }
        };

        // manage calls to target's service function
        void run() {
            int state = NORMAL;
            while (_thread_running and (state != FINISH)) {
                state = target->serviceFunction();
                if (state == NOOP) usleep(_udelay);
            }
        };

        // stop thread and wait for termination
        bool release(unsigned long secs = 0, unsigned long usecs = 0) {
            _thread_running = false;
            if (_mythread)  {
                if ((secs == 0) and (usecs == 0)){
                    _mythread->join();
                } else {
                    boost::system_time waitime= boost::get_system_time() + boost::posix_time::seconds(secs) +  boost::posix_time::microseconds(usecs) ;
                    if (!_mythread->timed_join(waitime)) {
                        return 0;
                    }
                }
                delete _mythread;
                _mythread = 0;
            }
    
            return 1;
        };

        virtual ~ProcessThread(){
            if (_mythread != 0) {
                release(0);
                _mythread = 0;
            }
        };

        void updateDelay(float _delay) { _udelay = (__useconds_t)(_delay * 1000000); };


    private:
        boost::thread *_mythread;
        bool _thread_running;
        TargetClass *target;
        __useconds_t _udelay;
        boost::condition_variable _end_of_run;
        boost::mutex _eor_mutex;
};

class ProgrammableDevice_base : public ExecutableDevice_impl, public virtual POA_CF::AggregateExecutableDevice, public AggregateDevice_impl
{
    public:
        ProgrammableDevice_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        ProgrammableDevice_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        ProgrammableDevice_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        ProgrammableDevice_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);

        void start();

        void stop();

        void releaseObject();

        void initialize();

        void loadProperties();

        virtual int serviceFunction() = 0;

    protected:
        ProcessThread<ProgrammableDevice_base> *serviceThread; 
        boost::mutex serviceThreadLock;

        // Member variables exposed as properties
        std::string device_kind;
        std::string device_model;
        std::vector<hw_load_status_struct> hw_load_statuses;
        std::vector<hw_load_request_struct> hw_load_requests;


    private:
        void construct();


};
#endif
