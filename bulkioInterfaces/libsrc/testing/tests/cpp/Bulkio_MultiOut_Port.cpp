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
#include "bulkio.h"
#include  "Bulkio_MultiOut_Port.h"

template < typename OUT_PORT, typename IN_PORT >
void Bulkio_MultiOut_Port< OUT_PORT, IN_PORT >::setUp()
{
  logger = rh_logger::Logger::getLogger("Bulkio-MultiOutPort-" + lname );
  logger->setLevel( rh_logger::Level::getInfo());
  orb = ossie::corba::CorbaInit(0,NULL);

  RH_DEBUG(this->logger, "Setup - Multiout Create Ports Table " );

  ip1 = new IN_PORT("sink_1", logger );
  ip1_oid = ossie::corba::RootPOA()->activate_object(ip1);
  ip2 = new IN_PORT("sink_2", logger );
  ip2_oid = ossie::corba::RootPOA()->activate_object(ip2);
  ip3 = new IN_PORT("sink_3", logger );
  ip3_oid = ossie::corba::RootPOA()->activate_object(ip3);
  ip4 = new IN_PORT("sink_4", logger );
  ip4_oid = ossie::corba::RootPOA()->activate_object(ip4);
  port = new OUT_PORT("multiout_source", logger );
  port_oid = ossie::corba::RootPOA()->activate_object(port);

  desc_list.clear();
  RH_DEBUG(this->logger, "Setup - Multiout Connection Table " );
  bulkio::connection_descriptor_struct desc;
  desc.connection_id = "connection_1";
  desc.stream_id = "stream-1-1";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_1";
  desc.stream_id = "stream-1-2";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_1";
  desc.stream_id = "stream-1-3";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_2";
  desc.stream_id = "stream-2-1";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_2";
  desc.stream_id = "stream-2-2";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_2";
  desc.stream_id = "stream-2-3";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_3";
  desc.stream_id = "stream-3-1";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_3";
  desc.stream_id = "stream-3-2";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_3";
  desc.stream_id = "stream-3-3";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_4";
  desc.stream_id = "stream-4-1";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);

}


template < typename OUT_PORT, typename IN_PORT >
void Bulkio_MultiOut_Port< OUT_PORT, IN_PORT >::tearDown()
{

  RH_DEBUG(this->logger, "TearDown - Deactivate Servants " );
  ossie::corba::RootPOA()->deactivate_object(ip1_oid);
  ossie::corba::RootPOA()->deactivate_object(ip2_oid);
  ossie::corba::RootPOA()->deactivate_object(ip3_oid);
  ossie::corba::RootPOA()->deactivate_object(ip4_oid);
  ossie::corba::RootPOA()->deactivate_object(port_oid);

  RH_DEBUG(this->logger, "TearDown - Shutdown the ORB " );
  //orb->shutdown(1);
}

//
// test_multiout_sri_filtered()
//
//   Test pushing out SRI to a single port and ensure other ports did not receive the SRI data
//

template < typename OUT_PORT, typename IN_PORT >
void  Bulkio_MultiOut_Data_Port< OUT_PORT, IN_PORT >::test_multiout_sri_filtered( ) {

  RH_DEBUG(this->logger, "Multiout SRI Filtered - BEGIN " );

  ExtendedCF::UsesConnectionSequence *clist = this->port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  RH_DEBUG(this->logger, "Multiout SRI Filtered - Create Connections and Filter list " );
  this->port->connectPort( this->ip1->_this(), "connection_1");
  this->port->connectPort( this->ip2->_this(), "connection_2");
  this->port->connectPort( this->ip3->_this(), "connection_3");
  this->port->connectPort( this->ip4->_this(), "connection_4");
  this->port->updateConnectionFilter( this->desc_list );

  //
  // Push SRI for IP1
  //

  std::string  filter_stream_id( "stream-1-1" );
  double srate=11.0;
  double xdelta = 1.0/srate;
  BULKIO::StreamSRI sri;
  BULKIO::PrecisionUTCTime TS = bulkio::time::utils::now();
  typename OUT_PORT::NativeSequenceType v(91);
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri );

  BULKIO::StreamSRISequence  *streams = this->ip1->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  delete streams;

  streams = this->ip2->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  streams = this->ip3->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  streams = this->ip4->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 4, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered  - Port 4, SRI was Received, Failed", streams->length() == 0 );
  delete streams;

}



//
// test_multiout_sri_eos_filtered()
//
// Test pushing out SRI to each port and ensure other ports did not receive the SRI data,
// then terminate the data flow for each stream with EOS and then check each ports
// active SRI list is empty
//
template < typename OUT_PORT, typename IN_PORT >
void  Bulkio_MultiOut_Data_Port< OUT_PORT, IN_PORT >::test_multiout_sri_eos_filtered( ) {

  RH_DEBUG(this->logger, "Multiout SRI Filtered - BEGIN " );

  ExtendedCF::UsesConnectionSequence *clist = this->port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  RH_DEBUG(this->logger, "Multiout SRI Filtered - Create Connections and Filter list " );
  this->port->connectPort( this->ip1->_this(), "connection_1");
  this->port->connectPort( this->ip2->_this(), "connection_2");
  this->port->connectPort( this->ip3->_this(), "connection_3");
  this->port->connectPort( this->ip4->_this(), "connection_4");
  this->port->updateConnectionFilter( this->desc_list );

  //
  // Push SRI for IP1
  //

  std::string  filter_stream_id( "stream-1-1" );
  double srate=11.0;
  double xdelta = 1.0/srate;
  BULKIO::StreamSRI sri;
  BULKIO::PrecisionUTCTime TS = bulkio::time::utils::now();
  typename OUT_PORT::NativeSequenceType v(0);
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri );

  BULKIO::StreamSRISequence  *streams = this->ip1->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  BULKIO::StreamSRI asri;
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = this->ip2->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  streams = this->ip3->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  streams = this->ip4->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 4, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered  - Port 4, SRI was Received, Failed", streams->length() == 0 );
  delete streams;


  //
  // Push SRI for IP2
  //
  filter_stream_id =  "stream-2-1";
  srate=22.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  RH_DEBUG(this->logger, "Multiout SRI Filter - sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri );

  streams = this->ip1->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-1-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = this->ip2->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-2-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = this->ip3->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  streams = this->ip4->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 4, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered  - Port 4, SRI was Received, Failed", streams->length() == 0 );
  delete streams;


  //
  // Push SRI for IP3
  //
  filter_stream_id =  "stream-3-1";
  srate=33.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  RH_DEBUG(this->logger, "Multiout SRI Filter - sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri );

  streams = this->ip1->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-1-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = this->ip2->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-2-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;


  streams = this->ip3->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-3-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = this->ip4->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 4, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered  - Port 4, SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  //
  // Push SRI for IP4
  //
  filter_stream_id =  "stream-4-1";
  srate=44.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  RH_DEBUG(this->logger, "Multiout SRI Filter - sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri );

  streams = this->ip1->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-1-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = this->ip2->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-2-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = this->ip3->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-3-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = this->ip4->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-4-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  //
  // Send EOS downstream and check activeSRIs
  //
  filter_stream_id = "stream-1-1";
  this->port->pushPacket( v, TS, true, filter_stream_id );

  typename IN_PORT::dataTransfer *pkt;
  pkt  = this->ip1->getPacket(bulkio::Const::NON_BLOCKING );;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 1) ;

  filter_stream_id = "stream-2-1";
  this->port->pushPacket( v, TS, true, filter_stream_id );
  pkt  = this->ip2->getPacket(bulkio::Const::NON_BLOCKING );;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 1) ;

  filter_stream_id = "stream-3-1";
  this->port->pushPacket( v, TS, true, filter_stream_id );
  pkt  = this->ip3->getPacket(bulkio::Const::NON_BLOCKING );;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 1) ;

  filter_stream_id = "stream-4-1";
  this->port->pushPacket( v, TS, true, filter_stream_id );
  pkt  = this->ip4->getPacket(bulkio::Const::NON_BLOCKING );;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 1) ;

  streams = this->ip1->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 SRI was Received, Failed", streams->length() == 0 );
  delete streams;
  streams = this->ip2->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 SRI was Received, Failed", streams->length() == 0 );
  delete streams;
  streams = this->ip3->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3 SRI was Received, Failed", streams->length() == 0 );
  delete streams;
  streams = this->ip3->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 4 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 4 SRI was Received, Failed", streams->length() == 0 );
  delete streams;

}


template < typename OUT_PORT, typename IN_PORT >
void  Bulkio_MultiOut_Data_Port< OUT_PORT, IN_PORT >::test_multiout_data_filtered( ) {

  RH_DEBUG(this->logger, "Multiout Data Filter - 1 stream id , 4 independent consumers" );

  RH_DEBUG(this->logger, "Multiout Data Filter - setup connections" );
  this->port->connectPort( this->ip1->_this(), "connection_1");
  this->port->connectPort( this->ip2->_this(), "connection_2");
  this->port->connectPort( this->ip3->_this(), "connection_3");
  this->port->connectPort( this->ip4->_this(), "connection_4");

  ExtendedCF::UsesConnectionSequence *clist = this->port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  RH_DEBUG(this->logger, "Multiout Data Filter - Check connections:" << clist->length() );
  CPPUNIT_ASSERT( clist->length() == 4 );
  delete clist;

  this->port->updateConnectionFilter( this->desc_list );

  //
  //  Test Filter for IP1
  //

  std::string  filter_stream_id( "stream-1-1" );
  double srate=11.0;
  double xdelta = 1.0/srate;
  BULKIO::StreamSRI sri;
  BULKIO::PrecisionUTCTime TS = bulkio::time::utils::now();
  RH_DEBUG(this->logger, "Multiout Data Filter - Pushing vector to consumers,  sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri );

  typename OUT_PORT::NativeSequenceType v(91);
  this->port->pushPacket( v, TS, false, filter_stream_id );

  // check all the consumers to see if they got the correct packet
  typename IN_PORT::dataTransfer *pkt ;
  pkt  = this->ip1->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - SRI Mismatch:", pkt->SRI.mode == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - Data Length:",  pkt->dataBuffer.size() == 91 ) ;
  if ( pkt ) delete pkt;

  //
  // make sure others did not get data ip2, ip3, ip4
  //
  pkt  = this->ip2->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  pkt  = this->ip3->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  pkt  = this->ip4->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  //  Test Filter for IP2
  //
  filter_stream_id =  "stream-2-1";
  srate=22.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  RH_DEBUG(this->logger, "Multiout Data Filter - Pushing vector to consumers,  sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri );
 
  this->port->pushPacket( v, TS, false, filter_stream_id );

  // check all the consumers to see if they got the correct packet
  pkt  = this->ip1->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  // make sure others did not get data ip1, ip3, ip4
  //
  pkt  = this->ip2->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - SRI Mismatch:", pkt->SRI.mode == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - Data Length:",  pkt->dataBuffer.size() == 91 ) ;
  if ( pkt ) delete pkt;

  pkt  = this->ip3->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  pkt  = this->ip4->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  //  Test Filter for IP3
  //
  filter_stream_id =  "stream-3-1";
  srate=33.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  RH_DEBUG(this->logger, "Multiout Data Filter - Pushing vector to consumers,  sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri );
 
  this->port->pushPacket( v, TS, false, filter_stream_id );

  // check all the consumers to see if they got the correct packet
  pkt  = this->ip1->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  // make sure others did not get data ip1, ip3, ip4
  //
  pkt  = this->ip2->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;  

  pkt  = this->ip3->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - SRI Mismatch:", pkt->SRI.mode == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - Data Length:",  pkt->dataBuffer.size() == 91 ) ;
  if ( pkt ) delete pkt;


  pkt  = this->ip4->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  //  Test Filter for IP4
  //
  filter_stream_id =  "stream-4-1";
  srate = 44.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  RH_DEBUG(this->logger, "Multiout Data Filter - Pushing vector to consumers,  sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri );
 
  this->port->pushPacket( v, TS, false, filter_stream_id );

  // check all the consumers to see if they got the correct packet
  pkt  = this->ip1->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  // make sure others did not get data ip1, ip3, ip4
  //
  pkt  = this->ip2->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;  

  pkt  = this->ip3->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;


  pkt  = this->ip4->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - SRI Mismatch:", pkt->SRI.mode == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - Data Length:",  pkt->dataBuffer.size() == 91 ) ;
  if ( pkt ) delete pkt;

}


//
// test_multiout_data_sri_filtered( )
//
// Test pushPacket data operations on each port do not affect the other port's state
//
template < typename OUT_PORT, typename IN_PORT >
void  Bulkio_MultiOut_Data_Port< OUT_PORT, IN_PORT >::test_multiout_data_sri_filtered( ) {

  RH_DEBUG(this->logger, "Multiout Data/SRI Filter - 1 stream id , 4 independent consumers" );

  RH_DEBUG(this->logger, "Multiout Data Filter - setup connections" );
  this->port->connectPort( this->ip1->_this(), "connection_1");
  this->port->connectPort( this->ip2->_this(), "connection_2");
  this->port->connectPort( this->ip3->_this(), "connection_3");
  this->port->connectPort( this->ip4->_this(), "connection_4");

  ExtendedCF::UsesConnectionSequence *clist = this->port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  RH_DEBUG(this->logger, "Multiout Data Filter - Check connections:" << clist->length() );
  CPPUNIT_ASSERT( clist->length() == 4 );
  delete clist;

  this->port->updateConnectionFilter( this->desc_list );

  //
  //  Test Filter for IP1
  //

  std::string  filter_stream_id( "stream-1-1" );
  double srate=11.0;
  double xdelta = 1.0/srate;
  BULKIO::StreamSRI sri;
  BULKIO::PrecisionUTCTime TS = bulkio::time::utils::now();
  RH_DEBUG(this->logger, "Multiout Data Filter - Pushing vector to consumers,  sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri );

  typename OUT_PORT::NativeSequenceType v(91);
  this->port->pushPacket( v, TS, false, filter_stream_id );

  // check all the consumers to see if they got the correct packet
  typename IN_PORT::dataTransfer *pkt ;
  pkt  = this->ip1->getPacket(bulkio::Const::NON_BLOCKING );
  RH_DEBUG(this->logger, "Multiout Data Filter - " << pkt->SRI.streamID << " exp:" << filter_stream_id );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - SRI Mismatch:", pkt->SRI.mode == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - Data Length:",  pkt->dataBuffer.size() == 91 ) ;
  if ( pkt ) delete pkt;

  //
  // make sure others did not get data ip2, ip3, ip4
  //
  pkt  = this->ip2->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  pkt  = this->ip3->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  pkt  = this->ip4->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  //  Test Filter for IP2
  //
  filter_stream_id =  "stream-2-1";
  srate=22.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  RH_DEBUG(this->logger, "Multiout Data Filter - Pushing vector to consumers,  sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri );
 
  this->port->pushPacket( v, TS, false, filter_stream_id );

  // check all the consumers to see if they got the correct packet
  pkt  = this->ip1->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  // make sure others did not get data ip1, ip3, ip4
  //
  pkt  = this->ip2->getPacket(bulkio::Const::NON_BLOCKING );
  RH_DEBUG(this->logger, "Multiout Data Filter - " << pkt->SRI.streamID << " exp:" << filter_stream_id );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - SRI Mismatch:", pkt->SRI.mode == 0 ) ;
  CPPUNIT_ASSERT_DOUBLES_EQUAL( xdelta,  pkt->SRI.xdelta, 0.01 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - Data Length:",  pkt->dataBuffer.size() == 91 ) ;
  if ( pkt ) delete pkt;

  pkt  = this->ip3->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  pkt  = this->ip4->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  //  Test Filter for IP3
  //
  filter_stream_id =  "stream-3-1";
  srate=33.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  RH_DEBUG(this->logger, "Multiout Data Filter - Pushing vector to consumers,  sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri );
 
  this->port->pushPacket( v, TS, false, filter_stream_id );

  // check all the consumers to see if they got the correct packet
  pkt  = this->ip1->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  // make sure others did not get data ip1, ip3, ip4
  //
  pkt  = this->ip2->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;  

  pkt  = this->ip3->getPacket(bulkio::Const::NON_BLOCKING );
  RH_DEBUG(this->logger, "Multiout Data Filter - " << pkt->SRI.streamID << " exp:" << filter_stream_id );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - SRI Mismatch:", pkt->SRI.mode == 0 ) ;
  CPPUNIT_ASSERT_DOUBLES_EQUAL( xdelta,  pkt->SRI.xdelta, 0.01 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - Data Length:",  pkt->dataBuffer.size() == 91 ) ;
  if ( pkt ) delete pkt;

  pkt  = this->ip4->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  //  Test Filter for IP4
  //
  filter_stream_id =  "stream-4-1";
  srate=44.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  RH_DEBUG(this->logger, "Multiout Data Filter - Pushing vector to consumers,  sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri );
 
  this->port->pushPacket( v, TS, false, filter_stream_id );

  // check all the consumers to see if they got the correct packet
  pkt  = this->ip1->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  // make sure others did not get data ip1, ip3, ip4
  //
  pkt  = this->ip2->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;  

  pkt  = this->ip3->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  pkt  = this->ip4->getPacket(bulkio::Const::NON_BLOCKING );
  RH_DEBUG(this->logger, "Multiout Data Filter - " << pkt->SRI.streamID << " exp:" << filter_stream_id );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:", pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - SRI Mismatch:", pkt->SRI.mode == 0 ) ;
  CPPUNIT_ASSERT_DOUBLES_EQUAL( xdelta,  pkt->SRI.xdelta, 0.01 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - Data Length:",  pkt->dataBuffer.size() == 91 ) ;
  if ( pkt ) delete pkt;

}

//
// test_multiout_sri()
//
//   Test pushing out SRI to a single port and ensure other ports did not receive the SRI data
//
template < typename OUT_PORT, typename IN_PORT , typename STREAM_DEF >
void  Bulkio_MultiOut_Attachable_Port< OUT_PORT, IN_PORT, STREAM_DEF >::test_multiout_sri( ) {

  RH_DEBUG(this->logger, "Multiout SRI Filtered - BEGIN " );

  ExtendedCF::UsesConnectionSequence *clist = this->port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  RH_DEBUG(this->logger, "Multiout SRI Filtered - Create Connections and Filter list " );
  this->port->connectPort( this->ip1->_this(), "connection_1");
  this->port->connectPort( this->ip2->_this(), "connection_2");
  this->port->connectPort( this->ip3->_this(), "connection_3");
  this->port->connectPort( this->ip4->_this(), "connection_4");
  this->port->updateConnectionFilter( this->desc_list );

  //
  // Push SRI for IP1
  //

  std::string  filter_stream_id( "stream-1-1" );
  double srate=11.0;
  double xdelta = 1.0/srate;
  BULKIO::StreamSRI sri;
  BULKIO::PrecisionUTCTime TS = bulkio::time::utils::now();
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri, TS );

  BULKIO::StreamSRISequence  *streams = this->ip1->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  delete streams;

  streams = this->ip2->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  streams = this->ip3->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  streams = this->ip4->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 4, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered  - Port 4, SRI was Received, Failed", streams->length() == 0 );
  delete streams;
}

//
// test_multiout_sri_filtered()
//
// Test pushing out SRI to each port and ensure other ports did not receive the SRI data
//
template < typename OUT_PORT, typename IN_PORT , typename STREAM_DEF >
void  Bulkio_MultiOut_Attachable_Port< OUT_PORT, IN_PORT, STREAM_DEF >::test_multiout_sri_filtered( ) {

  RH_DEBUG(this->logger, "Multiout SRI Filtered - BEGIN " );
  std::ostringstream msg;

  ExtendedCF::UsesConnectionSequence *clist = this->port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  RH_DEBUG(this->logger, "Multiout SRI Filtered - Create Connections and Filter list " );
  this->port->connectPort( this->ip1->_this(), "connection_1");
  this->port->connectPort( this->ip2->_this(), "connection_2");
  this->port->connectPort( this->ip3->_this(), "connection_3");
  this->port->connectPort( this->ip4->_this(), "connection_4");
  this->port->updateConnectionFilter( this->desc_list );

  this->ip1->setNewSriListener(this, &Bulkio_MultiOut_Attachable_Port<OUT_PORT,IN_PORT,STREAM_DEF>::newSriCallback);
  this->ip2->setNewSriListener(this, &Bulkio_MultiOut_Attachable_Port<OUT_PORT,IN_PORT,STREAM_DEF>::newSriCallback);
  this->ip3->setNewSriListener(this, &Bulkio_MultiOut_Attachable_Port<OUT_PORT,IN_PORT,STREAM_DEF>::newSriCallback);
  this->ip4->setNewSriListener(this, &Bulkio_MultiOut_Attachable_Port<OUT_PORT,IN_PORT,STREAM_DEF>::newSriCallback);

  //
  // Push SRI for IP1
  //

  std::string  filter_stream_id( "stream-1-1" );
  double srate=11.0;
  double xdelta = 1.0/srate;
  BULKIO::StreamSRI sri;
  BULKIO::PrecisionUTCTime TS = bulkio::time::utils::now();
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri, TS );

  BULKIO::StreamSRISequence  *streams = this->ip1->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  BULKIO::StreamSRI asri;
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = this->ip2->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  streams = this->ip3->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  streams = this->ip4->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 4, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered  - Port 4, SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  // Check if SRI callbacks are firing off
  msg.str(""); msg.clear();
  msg << "Multiout SRI Filtered - SriCallbacksEvents=" << newSriEvents;
  CPPUNIT_ASSERT_MESSAGE( msg.str().c_str(), newSriEvents == 1 ) ;
  msg.str(""); msg.clear();
  msg << "Multiout SRI Filtered - UpdateSRIEvents=" << updateSriEvents;
  CPPUNIT_ASSERT_MESSAGE( msg.str(), updateSriEvents == 0 ) ;

  //
  // Push SRI for IP2
  //
  filter_stream_id =  "stream-2-1";
  srate=22.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  RH_DEBUG(this->logger, "Multiout SRI Filter - sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri, TS );

  streams = this->ip1->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-1-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = this->ip2->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-2-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = this->ip3->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  streams = this->ip4->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 4, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered  - Port 4, SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  // Check if SRI callbacks are firing off
  msg.str(""); msg.clear();
  msg << "Multiout SRI Filtered - SriCallbacksEvents=" << newSriEvents;
  CPPUNIT_ASSERT_MESSAGE( msg.str().c_str(), newSriEvents == 2 ) ;
  msg.str(""); msg.clear();
  msg << "Multiout SRI Filtered - UpdateSRIEvents=" << updateSriEvents;
  CPPUNIT_ASSERT_MESSAGE( msg.str(), updateSriEvents == 0 ) ;

  //
  // Push SRI for IP3
  //
  filter_stream_id =  "stream-3-1";
  srate=33.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  RH_DEBUG(this->logger, "Multiout SRI Filter - sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri, TS );

  streams = this->ip1->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-1-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = this->ip2->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-2-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;


  streams = this->ip3->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-3-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = this->ip4->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 4, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered  - Port 4, SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  // Check if SRI callbacks are firing off
  msg.str(""); msg.clear();
  msg << "Multiout SRI Filtered - SriCallbacksEvents=" << newSriEvents;
  CPPUNIT_ASSERT_MESSAGE( msg.str().c_str(), newSriEvents == 3 ) ;
  msg.str(""); msg.clear();
  msg << "Multiout SRI Filtered - UpdateSRIEvents=" << updateSriEvents;
  CPPUNIT_ASSERT_MESSAGE( msg.str(), updateSriEvents == 0 ) ;
  
  //
  // Push SRI for IP4
  //
  filter_stream_id =  "stream-4-1";
  srate=44.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  RH_DEBUG(this->logger, "Multiout SRI Filter - sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  this->port->pushSRI( sri, TS );

  streams = this->ip1->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-1-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = this->ip2->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-2-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = this->ip3->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-3-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = this->ip4->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-4-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;
  
  // Check if SRI callbacks are firing off
  msg.str(""); msg.clear();
  msg << "Multiout SRI Filtered - SriCallbacksEvents=" << newSriEvents;
  CPPUNIT_ASSERT_MESSAGE( msg.str().c_str(), newSriEvents == 4 ) ;
  msg.str(""); msg.clear();
  msg << "Multiout SRI Filtered - UpdateSRIEvents=" << updateSriEvents;
  CPPUNIT_ASSERT_MESSAGE( msg.str(), updateSriEvents == 0 ) ;
}

//
// test_multiout_attach()
//
template < typename OUT_PORT, typename IN_PORT , typename STREAM_DEF >
void  Bulkio_MultiOut_Attachable_Port< OUT_PORT, IN_PORT, STREAM_DEF >::test_multiout_attach( ) {

  RH_DEBUG(this->logger, "Multiout SRI Filtered - BEGIN " );

  ExtendedCF::UsesConnectionSequence *clist = this->port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  RH_DEBUG(this->logger, "Multiout SRI Filtered - Create Connections and Filter list " );
  this->port->connectPort( this->ip1->_this(), "connection_1");
  this->port->connectPort( this->ip2->_this(), "connection_2");
  this->port->connectPort( this->ip3->_this(), "connection_3");
  this->port->connectPort( this->ip4->_this(), "connection_4");
  this->port->updateConnectionFilter( this->desc_list );

  //
  // Push SRI for IP1
  //

  // Validate that no attachments exist before stream is defined
  BULKIO::StringSequence* attIds; 

  attIds= this->ip1->attachmentIds();
  CPPUNIT_ASSERT( attIds->length() == 0 );
  delete attIds;
  attIds= this->ip2->attachmentIds();
  CPPUNIT_ASSERT( attIds->length() == 0 );
  delete attIds;
  attIds= this->ip3->attachmentIds();
  CPPUNIT_ASSERT( attIds->length() == 0 );
  delete attIds;
  attIds= this->ip4->attachmentIds();
  CPPUNIT_ASSERT( attIds->length() == 0 );
  delete attIds;

  STREAM_DEF newStreamDef;
  DefinitionGenerator::generateDefinition("stream-1-1", 12345, newStreamDef);
  this->port->addStream(newStreamDef);
  
  // Validate that attachments exists after stream is defined
  attIds = this->ip1->attachmentIds();
  CPPUNIT_ASSERT( attIds->length() == 1 );
  delete attIds;
  attIds = this->ip2->attachmentIds();
  CPPUNIT_ASSERT( attIds->length() == 0 );
  delete attIds;
  attIds = this->ip3->attachmentIds();
  CPPUNIT_ASSERT( attIds->length() == 0 );
  delete attIds;
  attIds = this->ip4->attachmentIds();
  CPPUNIT_ASSERT( attIds->length() == 0 );
  delete attIds;
}


// Registers the fixture into the 'registry'
// this also worked sans type name in output CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutUInt8 );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutUInt8_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutInt16_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutUInt16_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutInt32_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutUInt32_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutInt64_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutUInt64_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutDouble_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutFloat_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutSDDS_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutVITA49_Port );

//template class Bulkio_MultiOut_Data_Port< bulkio::OutCharPort, bulkio::InInt8Port >;
template class Bulkio_MultiOut_Data_Port< bulkio::OutOctetPort, bulkio::InUInt8Port >;
template class Bulkio_MultiOut_Data_Port< bulkio::OutInt16Port, bulkio::InInt16Port >;
template class Bulkio_MultiOut_Data_Port< bulkio::OutUInt16Port, bulkio::InUInt16Port >;
template class Bulkio_MultiOut_Data_Port< bulkio::OutInt32Port, bulkio::InInt32Port >;
template class Bulkio_MultiOut_Data_Port< bulkio::OutUInt32Port, bulkio::InUInt32Port >;
template class Bulkio_MultiOut_Data_Port< bulkio::OutInt64Port, bulkio::InInt64Port >;
template class Bulkio_MultiOut_Data_Port< bulkio::OutUInt64Port, bulkio::InUInt64Port >;
template class Bulkio_MultiOut_Data_Port< bulkio::OutDoublePort, bulkio::InDoublePort >;
template class Bulkio_MultiOut_Data_Port< bulkio::OutFloatPort, bulkio::InFloatPort >;
template class Bulkio_MultiOut_Attachable_Port< bulkio::OutSDDSPort, bulkio::InSDDSPort, BULKIO::SDDSStreamDefinition>;
template class Bulkio_MultiOut_Attachable_Port< bulkio::OutVITA49Port, bulkio::InVITA49Port, BULKIO::VITA49StreamDefinition >;

