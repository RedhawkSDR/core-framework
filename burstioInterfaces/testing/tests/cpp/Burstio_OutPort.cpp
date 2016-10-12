/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK burstioInterfaces.
 *
 * REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include "Burstio_OutPort.h"
#include "bulkio.h"
#include "burstio.h"


// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Burstio_OutPort );

class MyFloatPort : public burstio::BurstFloatOut {

public:

  MyFloatPort( std::string pname ):
    burstio::BurstFloatOut( pname ) {};

  //
  // over ride default behavior for pushPacket and pushSRI
  //
  void pushBursts(const burstio::BurstFloatOut::BurstSequenceType & bursts) {
    burstio::BurstFloatOut::pushBursts( bursts );
  }

};


void 
Burstio_OutPort::setUp()
{
   logger = rh_logger::Logger::getLogger("BurstioOutPort");
   logger->setLevel( rh_logger::Level::getError());
   orb = ossie::corba::CorbaInit(0,NULL);
   conn_cnt=0;
   disconn_cnt=0;
}


void 
Burstio_OutPort::tearDown()
{
}


// Global connection/disconnection callbacks
void Burstio_OutPort::connectCB( const std::string &connectionId ) {
  conn_cnt++;
}

void Burstio_OutPort::disconnectCB( const std::string &connectionId ) {
  disconn_cnt++;
}

BURSTIO::BurstSRI Burstio_OutPort::make_sri_test(const  std::string &sid, const std::string & id) {
  BURSTIO::BurstSRI sri;
  sri.streamID = sid.c_str();
  sri.id = id.c_str();
  sri.xdelta = 1.0;
  sri.mode = 0;       /* 0-Scalar, 1-Complex */
  sri.flags = 0;
  sri.tau = 1.1;
  sri.theta = 1.2f;
  sri.gain = 1.3f;
  sri.uwlength = 128;
  sri.bursttype = 2;
  sri.burstLength = 1024;
  sri.CHAN_RF = 1e6;
  sri.baudestimate = 2.0f;
  sri.carrieroffset = 2.1;
  sri.SNR = 2.3;
  sri.modulation = "mod";
  sri.baudrate = 56000.0;
  sri.fec = "vit";       /* fec type */
  sri.fecrate = "7/8";   /* not a number for TPC rates like (64,57)x(64,57) */
  sri.randomizer="R20";
  sri.overhead="unknown";
  sri.expectedStartOfBurstTime=burstio::utils::now();
  return sri;
}


template< typename T, typename IP >
void  Burstio_OutPort::test_setget_api( T *port  ) 
{
  RH_DEBUG(logger, "BURSTIO OUT-PORT API BEGIN: " << port->getName() );

  // try and assign logger to port
  port->setLogger(logger);

  BULKIO::UsesPortStatisticsSequence *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_OUT_PORT_TEST Invalid Port State", rt == BULKIO::IDLE );

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_OUT_PORT_TEST Connection list", clist != NULL );
  delete clist;
	
  //
  // test callback feature
  //
  port->addConnectListener( this, &Burstio_OutPort::connectCB );
  port->addDisconnectListener( this, &Burstio_OutPort::disconnectCB );

  {
    CORBA::Object_ptr p = CORBA::Object::_nil();
    // narrowing exception expected here
    // set logging level to Fatal to ignore port Error message
    // and then set back to Info
    CPPUNIT_ASSERT_THROW(port->connectPort( p, "connection_1"), CF::Port::InvalidPort );
  }

  IP *ip1  = new IP("sink_1" );
  PortableServer::ObjectId_var ip1_oid = ossie::corba::RootPOA()->activate_object(ip1);
  port->connectPort( ip1->_this(), "connection_1" );
  port->disconnectPort( "connection_1" );
  CPPUNIT_ASSERT_THROW(port->disconnectPort( "connection_1"), CF::Port::InvalidPort );


  clist = port->connections();
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_OUT_PORT_TEST Connection list - after connection/disconnect", clist != NULL );
  delete clist;

  // clear callback and reconnect
  port->removeConnectListener( this, &Burstio_OutPort::connectCB );
  port->removeDisconnectListener( this, &Burstio_OutPort::disconnectCB );

  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_OUT_PORT_TEST Connection Callback Fail", conn_cnt == 1 );
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_OUT_PORT_TEST Disconnection Callback Fail", disconn_cnt == 1 );


  // set to default queue threshold
  size_t tmp = port->getMaxBursts();
  CPPUNIT_ASSERT_MESSAGE("BURSTIO_OUT_PORT_TEST Get Max Bursts Failed", tmp == 100 );

  //
  // test max bursts state
  //
  tmp=22;
  port->setMaxBursts(tmp);
  tmp = port->getMaxBursts();
  CPPUNIT_ASSERT_MESSAGE("BURSTIO_OUT_PORT_TEST Get Max Bursts Failed", tmp == 22 );

  //
  // test byte threshold state
  //
  tmp=0xdeedbeef;
  port->setByteThreshold(tmp);
  tmp = port->getByteThreshold();
  CPPUNIT_ASSERT_MESSAGE("BURSTIO_OUT_PORT_TEST Get Byte Threshold Failed", tmp == 0xdeedbeef );

  //
  // test latency threshold
  //
  tmp=123456789;
  port->setLatencyThreshold(tmp);
  tmp = port->getLatencyThreshold();
  CPPUNIT_ASSERT_MESSAGE("BURSTIO_OUT_PORT_TEST Get Byte Threshold Failed", tmp == 123456789 );

  try {
    port->connectPort( ip1->_this(), "connection_1" );
  }
  catch(...) {
  }

  BULKIO::PrecisionUTCTime TS = burstio::utils::now();
  std::string sid = "test_port_api";
  std::string id = "id-1";
  BURSTIO::BurstSRI sri =  make_sri_test( sid, id );

  // push Bursts
  typename T::BurstSequenceType bursts;
  bursts.length(1);
  typename T::BurstType burst;
  burst.SRI = sri;
  burst.EOS = false;
  burst.T = burstio::utils::now();
  burst.data.length(50);
  bursts[0] = burst;
  std::vector< typename T::NativeType > data;

  // push vector of bursts
   port->pushBursts( bursts );

   // test single burst push
   port->pushBurst( burst );

   // test single burst push
    port->pushBurst( data, sri );

  // test start/stop sequence
  port->start();
  port->stop();

  port->start();
  port->stop();


  port->start();
  port->flush();
  port->flush();
  port->flush();
  port->stop();

  //
  // try and grab a statistics object... should be null
  //
  stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

}

struct conn_filter {
  std::string port_name;
  std::string connection_id;
  std::string stream_id;
  //  conn_filt( std::string &p, std::string &c, std::string &s ) : port_name(p), connection_id(c), stream_id(s) {};
  conn_filter( std::string &p, const char *c, const char *s ) : port_name(p), connection_id(c), stream_id(s) {};
};



template< typename T >
void  Burstio_OutPort::test_connection_filter( T *port  ) 
{

  RH_DEBUG(logger, "BURSTIO CONNECTION FILTER TEST: " << port->getName() );
  
  std::string pname = port->getName();
  std::vector< conn_filter >  filter;
  filter.push_back( conn_filter( pname, "connection-1", "stream-1-1"));
  filter.push_back( conn_filter( pname, "connection-1", "stream-1-2"));
  filter.push_back( conn_filter( pname, "connection-2", "stream-2-1"));
  filter.push_back( conn_filter( pname, "connection-2", "stream-2-2"));
  filter.push_back( conn_filter( pname, "connection-3", "stream-3-1"));
  filter.push_back( conn_filter( pname, "connection-3", "stream-3-2"));
  filter.push_back( conn_filter( pname, "connection-4", "stream-4-1"));
  port->updateConnectionFilter( filter );

  // simple add remove sequence
  port->addConnectionFilter( "stream-4-2", "connection-4");
  port->addConnectionFilter( "stream-4-3", "connection-4");

  port->removeConnectionFilter( "stream-4-2", "connection-4");
  port->removeConnectionFilter( "stream-4-3", "connection-4");


  // simple add remove to create possible errant handling of ids
  port->addConnectionFilter( "stream-4-3", "connection-4");


  port->removeConnectionFilter( "stream-4-3", "connection-4");
  port->removeConnectionFilter( "stream-4-3", "connection-4");
  port->removeConnectionFilter( "stream-4-3", "connection-4");
  port->removeConnectionFilter( "stream-4-3", "connection-4");

}


void 
Burstio_OutPort::test_create_int8()
{
  burstio::BurstByteOut *port = new burstio::BurstByteOut("test_ctor_int8" );
  CPPUNIT_ASSERT( port != NULL );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void 
Burstio_OutPort::test_int8()
{
  burstio::BurstByteOut *port = new burstio::BurstByteOut("test_api_int8" );
  CPPUNIT_ASSERT( port != NULL );

  test_setget_api< burstio::BurstByteOut, burstio::BurstByteIn >( port );

  test_connection_filter( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Burstio_OutPort::test_create_int16()
{
  burstio::BurstShortOut *port = new burstio::BurstShortOut("test_ctor_int16");
  CPPUNIT_ASSERT( port != NULL );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void 
Burstio_OutPort::test_int16()
{
  burstio::BurstShortOut *port = new burstio::BurstShortOut("test_api_int16");
  CPPUNIT_ASSERT( port != NULL );

  test_setget_api< burstio::BurstShortOut, burstio::BurstShortIn >( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}

void 
Burstio_OutPort::test_create_int32()
{
  burstio::BurstLongOut *port = new burstio::BurstLongOut("test_ctor_int32");
  CPPUNIT_ASSERT( port != NULL );
}

void 
Burstio_OutPort::test_int32()
{
  burstio::BurstLongOut *port = new burstio::BurstLongOut("test_api_int32");
  CPPUNIT_ASSERT( port != NULL );

  test_setget_api< burstio::BurstLongOut, burstio::BurstLongIn >( port );

  //test_push_flush_sequence( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Burstio_OutPort::test_create_int64()
{
  burstio::BurstLongLongOut *port = new burstio::BurstLongLongOut("test_ctor_int64");
  CPPUNIT_ASSERT( port != NULL );
}


void 
Burstio_OutPort::test_int64()
{
  burstio::BurstLongLongOut *port = new burstio::BurstLongLongOut("test_api_int64");
  CPPUNIT_ASSERT( port != NULL );

  test_setget_api< burstio::BurstLongLongOut, burstio::BurstLongLongIn >( port );

  //test_push_flush_sequence( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Burstio_OutPort::test_create_uint8()
{
  burstio::BurstUbyteOut *port = new burstio::BurstUbyteOut("test_api_int64");
  CPPUNIT_ASSERT( port != NULL );
}

void 
Burstio_OutPort::test_uint8()
{
  burstio::BurstUbyteOut *port = new burstio::BurstUbyteOut("test_api_uint8");
  CPPUNIT_ASSERT( port != NULL );

  test_setget_api< burstio::BurstUbyteOut, burstio::BurstUbyteIn >( port );

  //test_push_flush_sequence( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}

void 
Burstio_OutPort::test_create_uint16()
{
  burstio::BurstUshortOut *port = new burstio::BurstUshortOut("test_ctor_uint16");
  CPPUNIT_ASSERT( port != NULL );
}

void 
Burstio_OutPort::test_uint16()
{
  burstio::BurstUshortOut *port = new burstio::BurstUshortOut("test_api_uint16");
  CPPUNIT_ASSERT( port != NULL );

  test_setget_api< burstio::BurstUshortOut, burstio::BurstUshortIn >( port );

  //test_push_flush_sequence( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Burstio_OutPort::test_create_uint32()
{
  burstio::BurstUlongOut *port = new burstio::BurstUlongOut("test_ctor_uint32");
  CPPUNIT_ASSERT( port != NULL );
}

void 
Burstio_OutPort::test_uint32()
{
  burstio::BurstUlongOut *port = new burstio::BurstUlongOut("test_api_uint32");
  CPPUNIT_ASSERT( port != NULL );

  test_setget_api< burstio::BurstUlongOut, burstio::BurstUlongIn >( port );

  //test_push_flush_sequence( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Burstio_OutPort::test_create_uint64()
{
  burstio::BurstUlongLongOut *port = new burstio::BurstUlongLongOut("test_ctor_uint64");
  CPPUNIT_ASSERT( port != NULL );
}

void 
Burstio_OutPort::test_uint64()
{
  burstio::BurstUlongLongOut *port = new burstio::BurstUlongLongOut("test_api_uint64");
  CPPUNIT_ASSERT( port != NULL );

  test_setget_api< burstio::BurstUlongLongOut, burstio::BurstUlongLongIn >( port );

  //test_push_flush_sequence( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}



void 
Burstio_OutPort::test_create_float()
{
  burstio::BurstFloatOut *port = new burstio::BurstFloatOut("test_ctor_float");
  CPPUNIT_ASSERT( port != NULL );
}


void 
Burstio_OutPort::test_float()
{
  burstio::BurstFloatOut *port = new burstio::BurstFloatOut("test_api_float");
  CPPUNIT_ASSERT( port != NULL );

 test_setget_api< burstio::BurstFloatOut, burstio::BurstFloatIn >( port );

  //test_push_flush_sequence( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Burstio_OutPort::test_create_double()
{
  burstio::BurstDoubleOut *port = new burstio::BurstDoubleOut("test_ctor_float");
  CPPUNIT_ASSERT( port != NULL );
}



void 
Burstio_OutPort::test_double()
{
  burstio::BurstDoubleOut *port = new burstio::BurstDoubleOut("test_api_float");
  CPPUNIT_ASSERT( port != NULL );

 test_setget_api< burstio::BurstDoubleOut, burstio::BurstDoubleIn >( port );

  //test_push_flush_sequence( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}

#if 0
void 
Burstio_OutPort::test_subclass()
{
  burstio::BurstFloatOut *port = new MyFloatPort("test_api_subclass" );
  
  CPPUNIT_ASSERT( port != NULL );

  test_setget_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}

#endif
