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


#ifndef TESTALLPROPTYPES_IMPL_BASE_H
#define TESTALLPROPTYPES_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Resource_impl.h>

#include "struct_props.h"

#define NOOP 0
#define FINISH -1
#define NORMAL 1

class TestAllPropTypes_base;


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

class TestAllPropTypes_base : public Resource_impl
{

    public: 
        TestAllPropTypes_base(const char *uuid, const char *label);

        void start();

        void stop();

        void releaseObject();

        void initialize();

        void loadProperties();

        virtual int serviceFunction() = 0;

    protected:
        ProcessThread<TestAllPropTypes_base> *serviceThread; 
        boost::mutex serviceThreadLock;  

        // Member variables exposed as properties
        std::string simple_string;
        bool simple_boolean;
        CORBA::ULong simple_ulong;
        std::string simple_objref;
        short simple_short;
        float simple_float;
        unsigned char simple_octet;
        char simple_char;
        unsigned short simple_ushort;
        double simple_double;
        CORBA::Long simple_long;
        CORBA::LongLong simple_longlong;
        CORBA::ULongLong simple_ulonglong;
        CF::UTCTime simple_utctime;
        std::vector<std::string> simple_sequence_string;
        std::vector<bool> simple_sequence_boolean;
        std::vector<CORBA::ULong> simple_sequence_ulong;
        std::vector<std::string> simple_sequence_objref;
        std::vector<short> simple_sequence_short;
        std::vector<float> simple_sequence_float;
        std::vector<unsigned char> simple_sequence_octet;
        std::vector<char> simple_sequence_char;
        std::vector<unsigned short> simple_sequence_ushort;
        std::vector<double> simple_sequence_double;
        std::vector<CORBA::Long> simple_sequence_long;
        std::vector<CORBA::LongLong> simple_sequence_longlong;
        std::vector<CORBA::ULongLong> simple_sequence_ulonglong;
        std::vector<CF::UTCTime> simple_sequence_utctime;
        struct_vars_struct struct_vars;
        std::vector<struct_seq_vars_struct> struct_seq;
    
    private:
        void construct();

};
#endif
