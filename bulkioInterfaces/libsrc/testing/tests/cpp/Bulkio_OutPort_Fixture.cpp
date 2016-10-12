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
#include "Bulkio_OutPort_Fixture.h"
#include "bulkio.h"


// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Bulkio_OutPort_Fixture );


class MyOutFloatPort : public bulkio::OutFloatPort {

public:

  MyOutFloatPort( std::string pname, bulkio::LOGGER_PTR logger ) :
    bulkio::OutFloatPort( pname, logger ) {};


  void pushPacket( bulkio::OutFloatPort::NativeSequenceType & data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {

    stats[streamID].update( 1, 1.0, false, "testing" );
    bulkio::OutFloatPort::pushPacket( data, T, EOS, streamID );
  }

};

// Global connection/disconnection callbacks
static void port_connected( const char* connectionId ) {

}

static void port_disconnected( const char* connectionId ) {

}


void
Bulkio_OutPort_Fixture::setUp()
{
   logger = rh_logger::Logger::getLogger("BulkioOutPort");
   logger->setLevel( rh_logger::Level::getInfo());
   orb = ossie::corba::CorbaInit(0,NULL);
}


void
Bulkio_OutPort_Fixture::tearDown()
{
}

template<  typename T, typename IP >
void  Bulkio_OutPort_Fixture::test_port_api( T *port  ) {

  RH_DEBUG(logger, "Running tests port:" << port->getName() );

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  port->setNewConnectListener(&port_connected);
  port->setNewDisconnectListener(&port_disconnected);

  {
     CORBA::Object_ptr p = CORBA::Object::_nil();
    // narrowing exception expected here
    // set logging level to Fatal to ignore port Error message
    // and then set back to Info
    logger->setLevel( rh_logger::Level::getFatal());
    CPPUNIT_ASSERT_THROW(port->connectPort( p, "connection_1"), CF::Port::InvalidPort );
    logger->setLevel( rh_logger::Level::getInfo());
  }

  IP *p  = new IP("sink_1", logger );
  //PortableServer::ObjectId_var p_oid = ossie::corba::RootPOA()->activate_object(p);
  port->connectPort( p->_this(), "connection_1");

  port->disconnectPort( "connection_1");
  port->disconnectPort( "connection_1");
  //ossie::corba::RootPOA()->deactivate_object(p_oid);

  BULKIO::StreamSRI sri;
  port->pushSRI( sri );

  // Push using sequences
  typename T::NativeSequenceType v;
  BULKIO::PrecisionUTCTime TS;
  port->pushPacket( v, TS, false, "test_port_api" );

  port->pushPacket( v, TS, true, "test_port_api" );

  port->pushPacket( v, TS, true, "unknown_stream_id" );
 
  // Push using pointers
  size_t size = 100; 
  typename T::TransportType* buff = new typename T::TransportType[size];
  port->pushPacket( buff, size, TS, false, "test_port_api" );

  port->pushPacket( buff, size, TS, true, "test_port_api" );

  port->pushPacket( buff, size, TS, true, "unknown_stream_id" );
  delete[] buff;

  BULKIO::UsesPortStatisticsSequence *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT( rt == BULKIO::IDLE );


  typename T::ConnectionsList cl =  port->_getConnections();
  std::string sid="none";
  int cnt= port->getCurrentSRI().count(sid);
  CPPUNIT_ASSERT( cnt == 0 );

  port->enableStats( false );

  port->setLogger(logger);
}


template< >
void  Bulkio_OutPort_Fixture::test_port_api< bulkio::OutCharPort, bulkio::InCharPort  >( bulkio::OutCharPort *port  ) {

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  port->setNewConnectListener(&port_connected);
  port->setNewDisconnectListener(&port_disconnected);

  bulkio::InCharPort *p  = new bulkio::InCharPort("sink_1", logger );
  PortableServer::ObjectId_var p_oid = ossie::corba::RootPOA()->activate_object(p);
  port->connectPort( p->_this(), "connection_1");

  port->disconnectPort( "connection_1");
  port->disconnectPort( "connection_1");
  ossie::corba::RootPOA()->deactivate_object(p_oid);

  BULKIO::StreamSRI sri;
  port->pushSRI( sri );

  // Push packets using sequence
  std::vector< bulkio::OutCharPort::NativeType > v;
  BULKIO::PrecisionUTCTime TS;
  port->pushPacket( v, TS, false, "test_port_api" );

  std::vector< bulkio::Char > v1;
  port->pushPacket( v1, TS, false, "test_port_api" );
  
  // Push packets using pointers
  size_t size = 100; 
  char* buff = new char[size];
  port->pushPacket( buff, size, TS, false, "test_port_api" );
  delete[] buff;

  bulkio::Int8* buff1 = new bulkio::Int8[size];
  port->pushPacket( buff1, size, TS, false, "test_port_api" );
  delete[] buff1;

  BULKIO::UsesPortStatisticsSequence *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

    BULKIO::PortUsageType rt = port->state();
    CPPUNIT_ASSERT( rt == BULKIO::IDLE );

    port->enableStats( false );

    port->setLogger(logger);
}


template< >
void  Bulkio_OutPort_Fixture::test_port_api< bulkio::OutFilePort, bulkio::InFilePort >( bulkio::OutFilePort *port  ) {

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  port->setNewConnectListener(&port_connected);
  port->setNewDisconnectListener(&port_disconnected);

  bulkio::InFilePort *p  = new bulkio::InFilePort("sink_1", logger );
  PortableServer::ObjectId_var p_oid = ossie::corba::RootPOA()->activate_object(p);
  port->connectPort( p->_this(), "connection_1");

  port->disconnectPort( "connection_1");
  port->disconnectPort( "connection_1");
  ossie::corba::RootPOA()->deactivate_object(p_oid);

  BULKIO::StreamSRI sri;
  port->pushSRI( sri );

  bulkio::OutFilePort::NativeSequenceType v;
  BULKIO::PrecisionUTCTime TS;
  port->pushPacket( v, TS, false, "test_port_api" );

  port->pushPacket( v, TS, true, "test_port_api" );

  BULKIO::UsesPortStatisticsSequence *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT( rt == BULKIO::IDLE );

  port->enableStats( false );

  port->setLogger(logger);
}



template<>
void  Bulkio_OutPort_Fixture::test_port_api< bulkio::OutXMLPort, bulkio::InXMLPort >( bulkio::OutXMLPort *port  ) {

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  port->setNewConnectListener(&port_connected);
  port->setNewDisconnectListener(&port_disconnected);

  bulkio::InXMLPort *p  = new bulkio::InXMLPort("sink_1", logger );
  PortableServer::ObjectId_var p_oid = ossie::corba::RootPOA()->activate_object(p);
  port->connectPort( p->_this(), "connection_1");

  port->disconnectPort( "connection_1");
  port->disconnectPort( "connection_1");
  ossie::corba::RootPOA()->deactivate_object(p_oid);

  BULKIO::StreamSRI sri;
  port->pushSRI( sri );

  bulkio::OutXMLPort::NativeSequenceType v;
  BULKIO::PrecisionUTCTime TS;
  port->pushPacket( v, false, "test_port_api" );

  port->pushPacket( v, TS, true, "test_port_api" );

  BULKIO::UsesPortStatisticsSequence *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT( rt == BULKIO::IDLE );

  port->enableStats( false );

  port->setLogger(logger);
}


template<  >
void  Bulkio_OutPort_Fixture::test_port_api< bulkio::OutSDDSPort, bulkio::InSDDSPort >( bulkio::OutSDDSPort *port  ) {

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  port->setNewConnectListener(&port_connected);
  port->setNewDisconnectListener(&port_disconnected);


  bulkio::InSDDSPort *p  = new bulkio::InSDDSPort("sink_1", logger );
  PortableServer::ObjectId_var p_oid = ossie::corba::RootPOA()->activate_object(p);
  port->connectPort( p->_this(), "connection_1");

  port->disconnectPort( "connection_1");
  port->disconnectPort( "connection_1");
  ossie::corba::RootPOA()->deactivate_object(p_oid);


  BULKIO::StreamSRI sri;
  BULKIO::PrecisionUTCTime TS;
  port->pushSRI( sri, TS );

  BULKIO::UsesPortStatisticsSequence *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT( rt == BULKIO::IDLE );

  port->enableStats( false );

  // create a connection
  port->connectPort( p->_this(), "connection_1");
  port->enableStats( true );
  port->setBitSize(10);
  port->updateStats( 12, 1, false, "stream1");

  stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  int slen =   stats->length();
  //std::cout << " slen :" << slen << std::endl;
  CPPUNIT_ASSERT( slen == 1 ) ;
  CPPUNIT_ASSERT( strcmp((*stats)[0].connectionId, "connection_1") == 0 );
  delete stats;

  port->setLogger(logger);
}





void
Bulkio_OutPort_Fixture::test_create_int8()
{
  bulkio::OutCharPort *port = new bulkio::OutCharPort("test_int8", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_int8()
{
  bulkio::OutCharPort *port = new bulkio::OutCharPort("test_api_int8", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api<bulkio::OutCharPort, bulkio::InCharPort>( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void
Bulkio_OutPort_Fixture::test_create_int16()
{
  bulkio::OutInt16Port *port = new bulkio::OutInt16Port("test_ctor_int16", logger );
  CPPUNIT_ASSERT( port != NULL );
}


void
Bulkio_OutPort_Fixture::test_int16()
{
  bulkio::OutInt16Port *port = new bulkio::OutInt16Port("test_api_int16", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api<bulkio::OutInt16Port, bulkio::InInt16Port>( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void
Bulkio_OutPort_Fixture::test_create_int32()
{
  bulkio::OutInt32Port *port = new bulkio::OutInt32Port("test_ctor_int32", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_int32()
{
  bulkio::OutInt32Port *port = new bulkio::OutInt32Port("test_api_int32", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api<bulkio::OutInt32Port,bulkio::InInt32Port>( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}



void
Bulkio_OutPort_Fixture::test_create_int64()
{
  bulkio::OutInt64Port *port = new bulkio::OutInt64Port("test_ctor_int64", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_int64()
{
  bulkio::OutInt64Port *port = new bulkio::OutInt64Port("test_api_int64", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api<bulkio::OutInt64Port,bulkio::InInt64Port>( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}



void
Bulkio_OutPort_Fixture::test_create_uint8()
{
  bulkio::OutOctetPort *port = new bulkio::OutOctetPort("test_ctor_uint8", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_create_uint16()
{
  bulkio::OutUInt16Port *port = new bulkio::OutUInt16Port("test_ctor_uint16", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_uint16()
{
  bulkio::OutUInt16Port *port = new bulkio::OutUInt16Port("test_api_uint16", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api<bulkio::OutUInt16Port,bulkio::InUInt16Port>( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void
Bulkio_OutPort_Fixture::test_create_uint32()
{
  bulkio::OutUInt32Port *port = new bulkio::OutUInt32Port("test_ctor_uint32", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_uint32()
{
  bulkio::OutUInt32Port *port = new bulkio::OutUInt32Port("test_api_uint32", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api<bulkio::OutUInt32Port, bulkio::InUInt32Port>( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void
Bulkio_OutPort_Fixture::test_create_uint64()
{
  bulkio::OutUInt64Port *port = new bulkio::OutUInt64Port("test_ctor_uint64", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_uint64()
{
  bulkio::OutUInt64Port *port = new bulkio::OutUInt64Port("test_api_uint64", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api<bulkio::OutUInt64Port,bulkio::InUInt64Port>( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void
Bulkio_OutPort_Fixture::test_create_float()
{
  bulkio::OutFloatPort *port = new bulkio::OutFloatPort("test_ctor_float", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_create_double()
{
  bulkio::OutDoublePort *port = new bulkio::OutDoublePort("test_ctor_double", logger );
  CPPUNIT_ASSERT( port != NULL );
}


void
Bulkio_OutPort_Fixture::test_create_file()
{
  bulkio::OutFilePort *port = new bulkio::OutFilePort("test_ctor_file", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_file()
{
  bulkio::OutFilePort *port = new bulkio::OutFilePort("test_api_file", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api< bulkio::OutFilePort, bulkio::InFilePort >( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void
Bulkio_OutPort_Fixture::test_create_xml()
{
  bulkio::OutXMLPort *port = new bulkio::OutXMLPort("test_ctor_xml", logger );
  CPPUNIT_ASSERT( port != NULL );
}




void
Bulkio_OutPort_Fixture::test_xml()
{
  bulkio::OutXMLPort *port = new bulkio::OutXMLPort("test_api_xml", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api< bulkio::OutXMLPort, bulkio::InXMLPort >( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void
Bulkio_OutPort_Fixture::test_create_sdds()
{
  bulkio::OutSDDSPort *port = new bulkio::OutSDDSPort("test_ctor_sdds", logger);
  CPPUNIT_ASSERT( port != NULL );
}




void
Bulkio_OutPort_Fixture::test_sdds()
{
  bulkio::OutSDDSPort *port = new bulkio::OutSDDSPort("test_api_sdds", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api< bulkio::OutSDDSPort, bulkio::InSDDSPort > ( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}



void
Bulkio_OutPort_Fixture::test_subclass()
{
  bulkio::OutFloatPort *port = new MyOutFloatPort("test_api_subclass", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api<bulkio::OutFloatPort,bulkio::InFloatPort >( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}

