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
#include <deque>

#include <omniORB4/CORBA.h>

#include <timing.h>
#include <threaded_deleter.h>

#include "rawdata.h"

class Reader : public virtual POA_rawdata::reader {
public:
    Reader() :
        _received(0),
        _lastPacketSize(0),
        _packetCount(0),
        _totalTime(0.0),
        _averageTime(0.0)
    {
    }

    void push_octet(const rawdata::octet_sequence& data)
    {
        double start = get_time();
        _received += data.length();
        _deleter.deallocate_array(const_cast<rawdata::octet_sequence&>(data).get_buffer(1));
        double end = get_time();
        record_time(end-start, data.length());
    }

    void push_short(const rawdata::short_sequence& data)
    {
        _received += data.length() * sizeof(CORBA::Short);
        _deleter.deallocate_array(const_cast<rawdata::short_sequence&>(data).get_buffer(1));
    }

    void push_float(const rawdata::float_sequence& data)
    {
        _received += data.length() * sizeof(CORBA::Float);
        _deleter.deallocate_array(const_cast<rawdata::float_sequence&>(data).get_buffer(1));
    }

    CORBA::LongLong received()
    {
        return _received;
    }

    double average_time()
    {
        return _averageTime;
    }

    void record_time(double elapsed, size_t length) {
        if (_lastPacketSize != length) {
            _lastPacketSize = length;
            _packetCount = 1;
            _totalTime = elapsed;
            _averageTime = _totalTime;
        } else {
            _packetCount++;
            _totalTime += elapsed;
            _averageTime = _totalTime / _packetCount;
        }
    }

private:
    threaded_deleter _deleter;
    size_t _received;

    size_t _lastPacketSize;
    size_t _packetCount;
    double _totalTime;
    double _averageTime;
};

int main (int argc, char* argv[])
{
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj);
    PortableServer::POAManager_var manager = root_poa->the_POAManager();
    manager->activate();

    Reader* reader = new Reader();
    PortableServer::ObjectId_var oid = root_poa->activate_object(reader);
    rawdata::reader_var ref = reader->_this();
    CORBA::String_var ior = orb->object_to_string(ref);
    std::cout << ior << std::endl;

    orb->run();

    orb->shutdown(true);

    orb->destroy();
}
