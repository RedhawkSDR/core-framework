/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK throughput.
 *
 * REDHAWK throughput is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK throughput is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include <iostream>

#include <omniORB4/CORBA.h>

#include <timing.h>

#include "rawdata.h"

class Writer : public virtual POA_rawdata::writer {
public:
    Writer() :
        _thread(0),
        _running(true),
        _length(1024),
        _totalPackets(0),
        _totalSeconds(0.0),
        _averageTime(0.0)
    {
        _thread = new omni_thread(&Writer::thread_start, this);
    }

    void connect(rawdata::reader_ptr reader, const char* format)
    {
        _format = format;
        _reader = rawdata::reader::_duplicate(reader);
    }

    void transfer_length(CORBA::Long length)
    {
        _length = length;
        _totalPackets = 0;
        _totalSeconds = 0.0;
        _averageTime = 0.0;
    }

    void start()
    {
        _thread->start();
    }

    void stop()
    {
        _running = false;
    }

    double average_time()
    {
        return _averageTime;
    }

private:
    void thread_run()
    {
        if (_format == "float") {
            rawdata::float_sequence data;
            data.length(_length);
            while (_running) {
                if (data.length() != _length) {
                    data.length(_length);
                }
                _reader->push_float(data);
            }
        } else if (_format == "short") {
            rawdata::short_sequence data;
            data.length(_length);
            while (_running) {
                if (data.length() != _length) {
                    data.length(_length);
                }
                _reader->push_short(data);
            }
        } else {
            rawdata::octet_sequence data;
            data.length(_length);
            while (_running) {
                if (data.length() != _length) {
                    data.length(_length);
                }

                double start = get_time();
                _reader->push_octet(data);
                double end = get_time();

                _totalPackets++;
                _totalSeconds += end-start;
                _averageTime = _totalSeconds/_totalPackets;
            }
        }
    }


    static void* thread_start(void* arg)
    {
        Writer* writer = (Writer*)arg;
        writer->thread_run();
        return 0;
    }

    omni_thread* _thread;
    rawdata::reader_var _reader;
    volatile bool _running;
    std::string _format;
    int _length;

    size_t _totalPackets;
    double _totalSeconds;
    double _averageTime;
};

int main (int argc, char* argv[])
{
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj);
    PortableServer::POAManager_var manager = root_poa->the_POAManager();
    manager->activate();

    Writer* writer = new Writer();
    PortableServer::ObjectId_var oid = root_poa->activate_object(writer);
    rawdata::writer_var ref = writer->_this();
    CORBA::String_var ior = orb->object_to_string(ref);
    std::cout << ior << std::endl;

    orb->run();

    orb->shutdown(true);

    orb->destroy();
}
