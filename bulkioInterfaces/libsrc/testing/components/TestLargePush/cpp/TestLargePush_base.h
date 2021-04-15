/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef TESTLARGEPUSH_IMPL_BASE_H
#define TESTLARGEPUSH_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Resource_impl.h>

//Need to comment this out to build locally 
//#include "bulkio/bulkio.h"
#include "bulkio.h"

#define NOOP 0
#define FINISH -1
#define NORMAL 1

class TestLargePush_base;

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

class TestLargePush_base : public Resource_impl
{
    public:
        TestLargePush_base(const char *uuid, const char *label);

        void start();

        void stop();

        CORBA::Object_ptr getPort(const char* _id);

        void releaseObject();

        void initialize();

        void loadProperties();

        virtual int serviceFunction() = 0;

    protected:
        ProcessThread<TestLargePush_base> *serviceThread; 
        boost::mutex serviceThreadLock;

        // Member variables exposed as properties
        CORBA::ULongLong numSamples;

        // Ports
        bulkio::OutCharPort *dataChar;
        bulkio::OutFilePort *dataFile;
        bulkio::OutShortPort *dataShort;
        bulkio::OutULongPort *dataUlong;
        bulkio::OutULongLongPort *dataUlongLong;
        bulkio::OutUShortPort *dataUshort;
        bulkio::OutXMLPort *dataXML;
        bulkio::OutLongPort *dataLong;
        bulkio::OutLongLongPort *dataLongLong;
        bulkio::OutOctetPort *dataOctet;
        bulkio::OutFloatPort *dataFloat;
        bulkio::OutSDDSPort *dataSDDS;
        bulkio::OutDoublePort *dataDouble;


    private:
        void construct();


};
#endif
