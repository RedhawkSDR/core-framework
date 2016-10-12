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
/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "TestLargePush.h"

PREPARE_LOGGING(TestLargePush_i)

TestLargePush_i::TestLargePush_i(const char *uuid, const char *label) :
    TestLargePush_base(uuid, label)
{
}

TestLargePush_i::~TestLargePush_i()
{
}

void TestLargePush_i::serviceAsBuffers() 
{
    
    BULKIO::PrecisionUTCTime timestamp = BULKIO::PrecisionUTCTime();
    std::string streamID = "test";
    bool EOS = true;
    
    float* outputBufferFloat = new float[numSamples];
    for (size_t i = 0; i < numSamples; i++) outputBufferFloat[i] = 0.0;
    dataFloat->pushPacket(
            outputBufferFloat,  /* data buffer */
            numSamples,         /* buffer size */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */
    
    double* outputBufferDouble = new double[numSamples];
    for (size_t i = 0; i < numSamples; i++) outputBufferDouble[i] = 0.0;
    dataDouble->pushPacket(
            outputBufferDouble, /* data buffer */
            numSamples,         /* buffer size */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    unsigned short* outputBufferUshort = new unsigned short[numSamples];
    for (size_t i = 0; i < numSamples; i++) outputBufferUshort[i] = 0;
    dataUshort->pushPacket(
            outputBufferUshort, /* data buffer */
            numSamples,         /* buffer size */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    CORBA::ULong* outputBufferUlong = new CORBA::ULong[numSamples];
    for (size_t i = 0; i < numSamples; i++) outputBufferUlong[i] = 0;
    dataUlong->pushPacket(
            outputBufferUlong,  /* data buffer */
            numSamples,         /* buffer size */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */
    
    CORBA::Long* outputBufferLong = new CORBA::Long[numSamples];
    for (size_t i = 0; i < numSamples; i++) outputBufferLong[i] = 0;
    dataLong->pushPacket(
            outputBufferLong,   /* data buffer */
            numSamples,         /* buffer size */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */
    
    CORBA::ULongLong* outputBufferULL = new CORBA::ULongLong[numSamples];
    for (size_t i = 0; i < numSamples; i++) outputBufferULL[i] = 0;
    dataUlongLong->pushPacket(
            outputBufferULL,    /* data buffer */
            numSamples,         /* buffer size */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */
    
    CORBA::LongLong* outputBufferLL = new CORBA::LongLong[numSamples];
    for (size_t i = 0; i < numSamples; i++) outputBufferLL[i] = 0;
    dataLongLong->pushPacket(
            outputBufferLL,     /* data buffer */
            numSamples,         /* buffer size */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */
    
    short* outputBufferShort = new short[numSamples];
    for (size_t i = 0; i < numSamples; i++) outputBufferShort[i] = 0;
    dataShort->pushPacket(
            outputBufferShort,  /* data buffer */
            numSamples,         /* buffer size */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */
    
    CORBA::Octet* outputBufferOctet = new CORBA::Octet[numSamples];
    for (size_t i = 0; i < numSamples; i++) outputBufferOctet[i] = 0;
    dataOctet->pushPacket(
            outputBufferOctet,  /* data buffer */
            numSamples,         /* buffer size */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */


    delete[] outputBufferFloat;
    delete[] outputBufferDouble;
    delete[] outputBufferUshort;
    delete[] outputBufferUlong;
    delete[] outputBufferLong;
    delete[] outputBufferULL;
    delete[] outputBufferLL;
    delete[] outputBufferShort;
    delete[] outputBufferOctet;
}

void TestLargePush_i::serviceAsVectors() 
{

    BULKIO::PrecisionUTCTime timestamp = bulkio::time::utils::now();
    std::string streamID = "test";
    bool EOS = true;

    BULKIO::StreamSRI sri = bulkio::sri::create(streamID);
    sri.xdelta = 0.001;

    std::vector<float> outputDataFloat;
    outputDataFloat.resize(numSamples);
    dataFloat->pushSRI(sri);
    dataFloat->pushPacket(
            outputDataFloat,    /* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */
    
    std::vector<double> outputDataDouble;
    outputDataDouble.resize(numSamples);
    dataDouble->pushSRI(sri);
    dataDouble->pushPacket(
            outputDataDouble,   /* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    std::vector<unsigned short> outputDataUshort;
    outputDataUshort.resize(numSamples);
    dataUshort->pushSRI(sri);
    dataUshort->pushPacket(
            outputDataUshort,   /* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    std::vector<CORBA::ULong> outputDataUlong;
    outputDataUlong.resize(numSamples);
    dataUlong->pushSRI(sri);
    dataUlong->pushPacket(
            outputDataUlong,    /* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    std::vector<CORBA::Long> outputDataLong;
    outputDataLong.resize(numSamples);
    dataLong->pushSRI(sri);
    dataLong->pushPacket(
            outputDataLong,    /* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    std::vector<CORBA::ULongLong> outputDataUlongLong;
    outputDataUlongLong.resize(numSamples);
    dataUlongLong->pushSRI(sri);
    dataUlongLong->pushPacket(
            outputDataUlongLong,/* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    std::vector<CORBA::LongLong> outputDataLongLong;
    outputDataLongLong.resize(numSamples);
    dataLongLong->pushSRI(sri);
    dataLongLong->pushPacket(
            outputDataLongLong,    /* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    std::vector<short> outputDataShort;
    outputDataShort.resize(numSamples);
    dataShort->pushSRI(sri);
    dataShort->pushPacket(
            outputDataShort,    /* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    std::vector<CORBA::Octet> outputDataOctet;
    outputDataOctet.resize(numSamples);
    dataOctet->pushSRI(sri);
    dataOctet->pushPacket(
            outputDataOctet,    /* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

}

int TestLargePush_i::serviceFunction()
{
    serviceAsBuffers();
    return FINISH;
}
