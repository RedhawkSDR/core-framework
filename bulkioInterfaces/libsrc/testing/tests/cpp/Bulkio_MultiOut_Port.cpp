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
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutSDDS_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutVITA49_Port );

template class Bulkio_MultiOut_Attachable_Port< bulkio::OutSDDSPort, bulkio::InSDDSPort, BULKIO::SDDSStreamDefinition>;
template class Bulkio_MultiOut_Attachable_Port< bulkio::OutVITA49Port, bulkio::InVITA49Port, BULKIO::VITA49StreamDefinition >;

