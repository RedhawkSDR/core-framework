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
#ifndef BULKIO_MULTIOUT_PORT_H
#define BULKIO_MULTIOUT_PORT_H

#include <cppunit/extensions/HelperMacros.h>
#include<omniORB4/CORBA.h>
#include<ossie/debug.h>
#include<bulkio.h>

template< typename OUT_PORT, typename IN_PORT >
class Bulkio_MultiOut_Port : public CppUnit::TestFixture
{

public:
  void setUp();
  void tearDown();

  template < typename T > void test_port_api( T *port );

  rh_logger::LoggerPtr logger;

  CORBA::ORB_ptr orb;

  std::string  lname;

  IN_PORT *ip1; 
  IN_PORT *ip2;
  IN_PORT *ip3;
  IN_PORT *ip4;
  OUT_PORT *port;
  PortableServer::ObjectId_var ip1_oid;
  PortableServer::ObjectId_var ip2_oid;
  PortableServer::ObjectId_var ip3_oid; 
  PortableServer::ObjectId_var ip4_oid;
  PortableServer::ObjectId_var port_oid;

  std::vector< bulkio::connection_descriptor_struct >  desc_list;
};


template< typename OUT_PORT, typename IN_PORT >
class Bulkio_MultiOut_Data_Port : public Bulkio_MultiOut_Port<OUT_PORT,IN_PORT>
{
  CPPUNIT_TEST_SUITE( Bulkio_MultiOut_Data_Port );
  CPPUNIT_TEST( test_multiout_sri_filtered ); 
  CPPUNIT_TEST( test_multiout_sri_eos_filtered );
  CPPUNIT_TEST( test_multiout_data_filtered );
  CPPUNIT_TEST( test_multiout_data_sri_filtered );
  CPPUNIT_TEST_SUITE_END();

public:
  virtual void test_multiout_sri_filtered();
  virtual void test_multiout_sri_eos_filtered();
  virtual void test_multiout_data_filtered();
  virtual void test_multiout_data_sri_filtered();
};


template< typename OUT_PORT, typename IN_PORT, typename STREAM_DEF >
class Bulkio_MultiOut_Attachable_Port : public Bulkio_MultiOut_Port<OUT_PORT,IN_PORT>
{
  CPPUNIT_TEST_SUITE( Bulkio_MultiOut_Attachable_Port );
  CPPUNIT_TEST( test_multiout_sri ); 
  CPPUNIT_TEST( test_multiout_sri_filtered ); 
  CPPUNIT_TEST( test_multiout_attach ); 
  CPPUNIT_TEST_SUITE_END();

public:
  Bulkio_MultiOut_Attachable_Port() :
    Bulkio_MultiOut_Port<OUT_PORT,IN_PORT>(),
    newSriEvents(0),
    updateSriEvents(0)
  {}

  virtual void test_multiout_sri();
  virtual void test_multiout_sri_filtered();
  virtual void test_multiout_attach();

  void newSriCallback(const BULKIO::StreamSRI& sri) {
    newSriEvents++;
  }

  void sriUpdateCallback(const BULKIO::StreamSRI& sri) {
    updateSriEvents++;
  }

  int newSriEvents;
  int updateSriEvents;

};


class DefinitionGenerator {
public:
  static void generateDefinition(const std::string& id, long port, BULKIO::VITA49StreamDefinition& newStream) {
    newStream.id = id.c_str();
    newStream.ip_address = "0.0.0.1";
    newStream.port = port;
    newStream.protocol = BULKIO::VITA49_TCP_TRANSPORT;
    newStream.valid_data_format = false;
    newStream.vlan = 12345;
    newStream.data_format.channel_tag_size = 0;
    newStream.data_format.complexity = BULKIO::VITA49_COMPLEX_CARTESIAN;
    newStream.data_format.data_item_format = BULKIO::VITA49_32F;
    newStream.data_format.data_item_size = 0;
    newStream.data_format.event_tag_size = 0;
    newStream.data_format.item_packing_field_size = 0;
    newStream.data_format.packing_method_processing_efficient = false;
    newStream.data_format.repeat_count = 0;
    newStream.data_format.repeating = false;
    newStream.data_format.vector_size = 0;
  }
  static void generateDefinition(const std::string& id, long port, BULKIO::SDDSStreamDefinition& newStream) {
    newStream.id = id.c_str();
    newStream.port = port;
    newStream.vlan = 12345;
    newStream.dataFormat = BULKIO::SDDS_CF;
    newStream.multicastAddress = "1.2.3.4";
    newStream.privateInfo = "Some private info";
    newStream.sampleRate = 12345;
    newStream.timeTagValid = false;
  }
};

/*class SddsDefGenerator {
  static BULKIO::SDDSStreamDefinition* generateDefinition(const std::string& id, long port) {
    BULKIO::SDDSStreamDefinition* newStream = new BULKIO::SDDSStreamDefinition();
    newStream->id = id.c_str();
    newStream->port = port;
    newStream->vlan = 12345;
    newStream->dataFormat = BULKIO::SDDS_CF;
    newStream->multicastAddress = "1.2.3.4";
    newStream->privateInfo = "Some private info";
    newStream->sampleRate = 12345;
    newStream->timeTagValid = false;
    return newStream;
  }
};
*/

typedef Bulkio_MultiOut_Data_Port< bulkio::OutOctetPort, bulkio::InUInt8Port >   MultiOutUInt8;
typedef Bulkio_MultiOut_Data_Port< bulkio::OutInt16Port, bulkio::InInt16Port >   MultiOutInt16;
typedef Bulkio_MultiOut_Data_Port< bulkio::OutUInt16Port, bulkio::InUInt16Port >   MultiOutUInt16;
typedef Bulkio_MultiOut_Data_Port< bulkio::OutInt32Port, bulkio::InInt32Port >   MultiOutInt32;
typedef Bulkio_MultiOut_Data_Port< bulkio::OutUInt32Port, bulkio::InUInt32Port >   MultiOutUInt32;
typedef Bulkio_MultiOut_Data_Port< bulkio::OutInt64Port, bulkio::InInt64Port >   MultiOutInt64;
typedef Bulkio_MultiOut_Data_Port< bulkio::OutUInt64Port, bulkio::InUInt64Port >   MultiOutUInt64;
typedef Bulkio_MultiOut_Data_Port< bulkio::OutDoublePort, bulkio::InDoublePort >   MultiOutDouble;
typedef Bulkio_MultiOut_Data_Port< bulkio::OutFloatPort, bulkio::InFloatPort >   MultiOutFloat;

typedef Bulkio_MultiOut_Attachable_Port< bulkio::OutSDDSPort, bulkio::InSDDSPort, BULKIO::SDDSStreamDefinition >     MultiOutSDDS;
typedef Bulkio_MultiOut_Attachable_Port< bulkio::OutVITA49Port, bulkio::InVITA49Port, BULKIO::VITA49StreamDefinition> MultiOutVITA49;


#define DEF_TEST( IP, OP, NAME ) class MultiOut##IP##_Port : public  Bulkio_MultiOut_Data_Port< bulkio::Out##OP##Port, bulkio::In##IP##Port > \
{ \
  CPPUNIT_TEST_SUITE( MultiOut##IP##_Port ); \
  CPPUNIT_TEST( test_multiout_sri_filtered ); \
  CPPUNIT_TEST( test_multiout_sri_eos_filtered ); \
  CPPUNIT_TEST( test_multiout_data_filtered ); \
  CPPUNIT_TEST( test_multiout_data_sri_filtered ); \
  CPPUNIT_TEST_SUITE_END(); \
public: \
\
 MultiOut##IP##_Port() : MultiOut##IP (){ \
    this->lname=#NAME; \
  }; \
};

DEF_TEST( UInt8, Octet, UINT8 );
DEF_TEST( Int16, Int16, INT16 );
DEF_TEST( UInt16, UInt16, UINT16 );
DEF_TEST( Int32, Int32, INT32 );
DEF_TEST( UInt32, UInt32, UINT32 );
DEF_TEST( Int64, Int64, INT64 );
DEF_TEST( UInt64, UInt64, UINT64 );
DEF_TEST( Double, Double, DOUBLE );
DEF_TEST( Float, Float, FLOAT );


#define DEF_ATTACHABLE_TEST( NAME ) class MultiOut##NAME##_Port : public  Bulkio_MultiOut_Attachable_Port< bulkio::Out##NAME##Port, bulkio::In##NAME##Port, BULKIO::NAME##StreamDefinition > \
{ \
  CPPUNIT_TEST_SUITE( MultiOut##NAME##_Port ); \
  CPPUNIT_TEST( test_multiout_sri ); \
  CPPUNIT_TEST( test_multiout_sri_filtered ); \
  CPPUNIT_TEST( test_multiout_attach ); \
  CPPUNIT_TEST_SUITE_END(); \
public: \
\
 MultiOut##NAME##_Port() : MultiOut##NAME (){ \
    this->lname=#NAME; \
  }; \
};


DEF_ATTACHABLE_TEST(SDDS);
DEF_ATTACHABLE_TEST(VITA49);


#endif  // BULKIO_OutPort_PORT_H
