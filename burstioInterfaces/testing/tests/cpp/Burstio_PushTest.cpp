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
#include "Burstio_PushTest.h"
#include "bulkio.h"
#include "burstio.h"


template < typename OUT_PORT, typename IN_PORT >
void Burstio_PushBursts< OUT_PORT, IN_PORT >::setUp()
{
  logger = rh_logger::Logger::getLogger("Burstio_PushBursts -" + lname );
  logger->setLevel( rh_logger::Level::getError());
  orb = ossie::corba::CorbaInit(0,NULL);

  RH_DEBUG(logger, "Setup - Burstio Create Ports Table " );

  ip1 = new IN_PORT("sink_1" );
  ip1_oid = ossie::corba::RootPOA()->activate_object(ip1);
  ip2 = new IN_PORT("sink_2" );
  ip2_oid = ossie::corba::RootPOA()->activate_object(ip2);
  ip3 = new IN_PORT("sink_3" );
  ip3_oid = ossie::corba::RootPOA()->activate_object(ip3);
  ip4 = new IN_PORT("sink_4" );
  ip4_oid = ossie::corba::RootPOA()->activate_object(ip4);
  op1 = new OUT_PORT("source_1" );
  op1_oid = ossie::corba::RootPOA()->activate_object(op1);
  op2 = new OUT_PORT("source_2" );
  op2_oid = ossie::corba::RootPOA()->activate_object(op2);
  op3 = new OUT_PORT("source_3" );
  op3_oid = ossie::corba::RootPOA()->activate_object(op3);
  op4 = new OUT_PORT("source_4" );
  op4_oid = ossie::corba::RootPOA()->activate_object(op4);


  desc_list.clear();
  RH_DEBUG(logger, "Setup - Burstio Connection Table " );
  bulkio::connection_descriptor_struct desc;
  desc.connection_id = "connection_1";
  desc.stream_id = "stream-1-1";
  desc.port_name = "source_1";
  desc_list.push_back(desc);
  desc.connection_id = "connection_1";
  desc.stream_id = "stream-1-2";
  desc.port_name = "source_1";
  desc_list.push_back(desc);
  desc.connection_id = "connection_1";
  desc.stream_id = "stream-1-3";
  desc.port_name = "source_1";
  desc_list.push_back(desc);
  desc.connection_id = "connection_2";
  desc.stream_id = "stream-2-1";
  desc.port_name = "source_1";
  desc_list.push_back(desc);
  desc.connection_id = "connection_2";
  desc.stream_id = "stream-2-2";
  desc.port_name = "source_1";
  desc_list.push_back(desc);
  desc.connection_id = "connection_2";
  desc.stream_id = "stream-2-3";
  desc.port_name = "source_1";
  desc_list.push_back(desc);
  desc.connection_id = "connection_3";
  desc.stream_id = "stream-3-1";
  desc.port_name = "source_1";
  desc_list.push_back(desc);
  desc.connection_id = "connection_3";
  desc.stream_id = "stream-3-2";
  desc.port_name = "source_1";
  desc_list.push_back(desc);
  desc.connection_id = "connection_3";
  desc.stream_id = "stream-3-3";
  desc.port_name = "source_1";
  desc_list.push_back(desc);
  desc.connection_id = "connection_4";
  desc.stream_id = "stream-4-1";
  desc.port_name = "source_1";
  desc_list.push_back(desc);

}

template < typename OUT_PORT, typename IN_PORT >
void Burstio_PushBursts< OUT_PORT, IN_PORT >::tearDown()
{
  RH_DEBUG(logger, "TearDown - Deactivate Servants " );
  ossie::corba::RootPOA()->deactivate_object(ip1_oid);
  ossie::corba::RootPOA()->deactivate_object(ip2_oid);
  ossie::corba::RootPOA()->deactivate_object(ip3_oid);
  ossie::corba::RootPOA()->deactivate_object(ip4_oid);
  ossie::corba::RootPOA()->deactivate_object(op1_oid);
  ossie::corba::RootPOA()->deactivate_object(op2_oid);
  ossie::corba::RootPOA()->deactivate_object(op3_oid);
  ossie::corba::RootPOA()->deactivate_object(op4_oid);
  RH_DEBUG(logger, "TearDown - Shutdown the ORB " );
}

template < typename OUT_PORT, typename IN_PORT >
BURSTIO::BurstSRI Burstio_PushBursts< OUT_PORT, IN_PORT >::make_sri_test(const  std::string &sid, const std::string & id) {
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

//
// test_multiout_sri_filtered()
//
template < typename OUT_PORT, typename IN_PORT >
void  Burstio_PushBursts< OUT_PORT, IN_PORT >::test_push_simple( ) {

  RH_DEBUG(logger, "Burstio Flow - SIMPLE FLOW - BEGIN " );

  ExtendedCF::UsesConnectionSequence *clist = op1->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  std::string cid("connection_1");

  RH_DEBUG(logger, "Burstio Flow - Create Connections and Filter list " );
  op1->connectPort( ip1->_this(), cid.c_str());

  // need to allow for flow of data
  op1->start();
  ip1->start();

  BULKIO::PrecisionUTCTime TS = burstio::utils::now();
  std::string sid = "stream-1-1";
  std::string id = "id-1";
  BURSTIO::BurstSRI sri = make_sri_test( sid, id );

  // push Bursts
  typename OUT_PORT::BurstSequenceType bursts;
  bursts.length(1);
  typename OUT_PORT::BurstType burst;
  burst.SRI = sri;
  burst.EOS = false;
  burst.T = burstio::utils::now();
  burst.data.length(50);
  bursts[0] = burst;

  op1->pushBursts( bursts);

  typename IN_PORT::PacketType *pkt;
  RH_DEBUG(logger, "Burstio Flow - Simple Flow -- GetPacket " );
  pkt  = ip1->getBurst(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_SIMPLE_FLOW getBursts PKT == NULL ", NULL != pkt );
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_SIMPLE_FLOW getBursts EOS mismatch ", false == pkt->getEOS() );
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_SIMPLE_FLOW getBursts Data Length mismatch ", 50 == pkt->getSize() );
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_SIMPLE_FLOW getBursts Mode ", 0 == pkt->isComplex() );
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_SIMPLE_FLOW getBursts SRI mismatch ", sid == pkt->getStreamID().c_str() );
  RH_DEBUG(logger, "Burstio Flow - Simple Flow -- Passed basic PKT examination " );
  BURSTIO::BurstSRI asri = pkt->getSRI();
  CORBA::String_var t = asri.modulation;
  std::string  expected("mod");
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_SIMPLE_FLOW getBursts SRI.modulation", expected == t.in() );
  t = asri.fec;  
  expected = "vit";
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_SIMPLE_FLOW getBursts SRI.fec", expected == t.in() );
  t = asri.fecrate;  
  expected = "7/8";
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_SIMPLE_FLOW getBursts SRI.fec", expected == t.in() );
  t = asri.randomizer;  
  expected = "R20";
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_SIMPLE_FLOW getBursts SRI.fec", expected == t.in() );
  t = asri.overhead;  
  expected = "unknown";
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_SIMPLE_FLOW getBursts SRI.overhead", expected == t.in() );
  RH_DEBUG(logger, "Burstio Flow - Simple Flow -- Passed basic SRI examination " );

  RH_DEBUG(logger, "Burstio Flow - Simple Flow -- GetPacket " );
  pkt  = ip1->getBurst(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_SIMPLE_FLOW getBursts PKT != NULL ", NULL == pkt );

  op1->disconnectPort(cid.c_str());
  RH_DEBUG(logger, "Burstio Flow - Simple Flow -- Disconnected" );

  RH_DEBUG(logger, "Burstio Flow - SIMPLE FLOW - END " );

}

template < typename OUT_PORT, typename IN_PORT >
void  Burstio_PushBursts< OUT_PORT, IN_PORT >::test_push_bursts_sequence( ) {

  RH_DEBUG(logger, "Burstio PUSH SEQUENCE BEGIN " );

  ExtendedCF::UsesConnectionSequence *clist = op1->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  std::string cid("connection_1");

  RH_DEBUG(logger, "Burstio Flow - Create Connections and Filter list " );
  op1->connectPort( ip1->_this(), cid.c_str());

  // need to allow for flow of data
  op1->start();
  ip1->start();

  BULKIO::PrecisionUTCTime TS = burstio::utils::now();
  std::string sid = "stream-1-1";
  std::string id = "id-1";
  BURSTIO::BurstSRI sri = make_sri_test( sid, id );

  // push Bursts
  typename OUT_PORT::BurstSequenceType bursts;
  int i,j;
  int nseq=10;
  int burst_count[nseq];
  int totalBursts=0;
  for ( i=0; i<nseq;i++ ) {
    int nbursts=i+1;
    burst_count[i] = nbursts;
    bursts.length( nbursts );
    for ( j=0; j< nbursts;j++ ) {
      typename OUT_PORT::BurstType burst;
      burst.SRI = sri;
      burst.EOS = false;
      burst.T = burstio::utils::now();
      burst.data.length(50);
      totalBursts++;
      bursts[j] = burst;
    }
    //RH_DEBUG(logger, "Burstio Flow - OP1 PushBurts " << bursts.length()  << " == " << burst_count[i] );
    op1->pushBursts( bursts );
  }

  RH_DEBUG(logger, "Burstio Flow - PUSH SEQUENCE -- SENT NSEQ:" << nseq << " TOTAL BURSTS:" << totalBursts );

  typename IN_PORT::BurstSequenceVar in_bursts;
  for ( i=0; i<nseq;i++ ) {  
    RH_DEBUG(logger, "Burstio Flow - PUSH SEQUENCE -- GET BURST SEQ:" << i );
    in_bursts  = ip1->getBursts(bulkio::Const::NON_BLOCKING );
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts Sequence is empty ",  in_bursts->length() != 0  );
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts Sequence Len mismatch ", in_bursts->length() == burst_count[i]);
  
    RH_DEBUG(logger, "Burstio Flow - PUSH SEQUENCE -- IN BURSTS:" << in_bursts->length() );
    for ( j=0; j < in_bursts->length(); j++ ) {
      RH_DEBUG(logger, "Burstio Flow - PUSH SEQUENCE -- GET BURST:" << j);
      typename IN_PORT::BurstType burst=in_bursts[j];
        
      CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts EOS mismatch ", false == burst.EOS );
      CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts Data Length mismatch ", 50 == burst.data.length() );
      RH_DEBUG(logger, "Burstio Flow - Push Sequence -- Passed basic BURST examination " );
      BURSTIO::BurstSRI asri = burst.SRI;
      CORBA::String_var t;

      t = burst.SRI.streamID;
      std::string  expected(sid);
      CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts StreamID mismatch ", expected == t.in() );

      t = asri.modulation;
      expected = "mod";
      CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.modulation", expected == t.in() );
      t = asri.fec;  
      expected = "vit";
      CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.fec", expected == t.in() );
      t = asri.fecrate;  
      expected = "7/8";
      CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.fec", expected == t.in() );
      t = asri.randomizer;  
      expected = "R20";
      CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.fec", expected == t.in() );
      t = asri.overhead;  
      expected = "unknown";
      CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.overhead", expected == t.in() );
      RH_DEBUG(logger, "Burstio Flow - Push Sequence -- Passed basic SRI examination " );
    }

  }

  RH_DEBUG(logger, "Burstio Flow - Push Seqeunce-- GetPacket " );
  typename IN_PORT::PacketType *pkt  = ip1->getBurst(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts PKT == NULL ", NULL == pkt );

  op1->disconnectPort(cid.c_str());
  RH_DEBUG(logger, "Burstio Flow - Push Sequence -- Disconnected" );

  RH_DEBUG(logger, "BURSTIO PUSH-BURSTS SEQUENCE END " );

}


template < typename OUT_PORT, typename IN_PORT >
void  Burstio_PushBursts< OUT_PORT, IN_PORT >::test_push_burst_sequence( ) {

  RH_DEBUG(logger, "BURSTIO PUSH-BURST SEQUENCE BEGIN " );

  ExtendedCF::UsesConnectionSequence *clist = op1->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  std::string cid("connection_1");

  RH_DEBUG(logger, "Burstio Flow - Create Connections and Filter list " );
  op1->connectPort( ip1->_this(), cid.c_str());

  // need to allow for flow of data
  op1->start();
  ip1->start();

  BULKIO::PrecisionUTCTime TS = burstio::utils::now();
  std::string sid = "stream-1-1";
  std::string id = "id-1";
  BURSTIO::BurstSRI sri = make_sri_test( sid, id );

  // push Bursts
  typename OUT_PORT::BurstSequenceType bursts;
  int i,j;
  int nseq=10;
  int burst_count[nseq];
  int totalBursts=0;
  for ( i=0; i<nseq;i++ ) {
    int nbursts=i+1;
    burst_count[i] = nbursts;
    bursts.length( nbursts );
    for ( j=0; j< nbursts;j++ ) {
      typename OUT_PORT::BurstType burst;
      burst.SRI = sri;
      burst.EOS = false;
      burst.T = burstio::utils::now();
      burst.data.length(50);
      totalBursts++;
      bursts[j] = burst;
    }
    //RH_DEBUG(logger, "Burstio Flow - OP1 PushBurts " << bursts.length()  << " == " << burst_count[i] );
    op1->pushBursts( bursts );
  }

  RH_DEBUG(logger, "Burstio Flow - PUSH SEQUENCE -- SENT NSEQ:" << nseq << " TOTAL BURSTS:" << totalBursts );
  int tmp = ip1->getQueueDepth();
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE queue size mismatch ", tmp == totalBursts );

  typename IN_PORT::BurstSequenceVar in_bursts;
  for ( i=0; i<totalBursts;i++ ) {  

    typename IN_PORT::PacketType *pkt  = ip1->getBurst(bulkio::Const::NON_BLOCKING );
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts PKT == NULL ", NULL != pkt );
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts EOS mismatch ", false == pkt->getEOS() );
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts Data Length mismatch ", 50 == pkt->getSize() );
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts Mode ", 0 == pkt->isComplex() );
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI mismatch ", sid == pkt->getStreamID().c_str() );
    BURSTIO::BurstSRI asri = pkt->getSRI();
    CORBA::String_var t = asri.streamID;
    std::string  expected(sid);
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts StreamID mismatch ", expected == t.in() );

    t = asri.modulation;
    expected = "mod";
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.modulation", expected == t.in() );
    t = asri.fec;  
    expected = "vit";
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.fec", expected == t.in() );
    t = asri.fecrate;  
    expected = "7/8";
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.fec", expected == t.in() );
    t = asri.randomizer;  
    expected = "R20";
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.fec", expected == t.in() );
    t = asri.overhead;  
    expected = "unknown";
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.overhead", expected == t.in() );
    RH_DEBUG(logger, "Burstio Flow - Push Sequence -- Passed basic SRI examination " );
  }

  RH_DEBUG(logger, "Burstio Flow - Push Seqeunce-- GetPacket " );
  typename IN_PORT::PacketType *pkt  = ip1->getBurst(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts PKT == NULL ", NULL == pkt );

  op1->disconnectPort(cid.c_str());
  RH_DEBUG(logger, "Burstio Flow - Push Sequence -- Disconnected" );

  RH_DEBUG(logger, "BURSTIO PUSH-BURSTS SEQUENCE END " );

}



template < typename OUT_PORT, typename IN_PORT >
int  Burstio_PushBursts< OUT_PORT, IN_PORT >::test_fan_in_push( OUT_PORT *op, int oid , int nbursts ) {

  BULKIO::PrecisionUTCTime TS = burstio::utils::now();
  std::ostringstream os;
  os << "stream-" << oid << "-1";
  std::string sid = os.str();
  os.clear();
  os << "id-" << oid;
  std::string id = os.str();
  BURSTIO::BurstSRI sri = make_sri_test( sid, id );
  
  // push Bursts
  typename OUT_PORT::BurstSequenceType bursts;
  int i,j;
  int totalBursts=0;
  bursts.length( nbursts );
  for ( j=0; j< nbursts;j++ ) {
      typename OUT_PORT::BurstType burst;
      burst.SRI = sri;
      burst.EOS = false;
      burst.T = burstio::utils::now();
      burst.data.length(50);
      totalBursts++;
      bursts[j] = burst;
  }

  RH_DEBUG(logger, "Burstio Flow - Push Sequence --ID" << oid << " nbursts//TOTAL " << nbursts << totalBursts );
  op->pushBursts( bursts );

  return totalBursts;
}



template < typename OUT_PORT, typename IN_PORT >
int  Burstio_PushBursts< OUT_PORT, IN_PORT >::test_fan_in_push_results( IN_PORT *ip, int oid, int nbursts ) {

  std::ostringstream os;
  os << "stream-" << oid << "-1";
  std::string sid = os.str();
  os.clear();
  os << "id-" << oid;
  std::string id = os.str();

  for (int i=0; i<nbursts;i++ ) {  

    typename IN_PORT::PacketType *pkt  = ip->getBurst(bulkio::Const::NON_BLOCKING );
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts PKT == NULL ", NULL != pkt );
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts EOS mismatch ", false == pkt->getEOS() );
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts Data Length mismatch ", 50 == pkt->getSize() );
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts Mode ", 0 == pkt->isComplex() );
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI mismatch ", sid == pkt->getStreamID().c_str() );
    BURSTIO::BurstSRI asri = pkt->getSRI();
    CORBA::String_var t = asri.streamID;
    std::string  expected(sid);
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts StreamID mismatch ", expected == t.in() );

    t = asri.modulation;
    expected = "mod";
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.modulation", expected == t.in() );
    t = asri.fec;  
    expected = "vit";
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.fec", expected == t.in() );
    t = asri.fecrate;  
    expected = "7/8";
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.fec", expected == t.in() );
    t = asri.randomizer;  
    expected = "R20";
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.fec", expected == t.in() );
    t = asri.overhead;  
    expected = "unknown";
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.overhead", expected == t.in() );
    RH_DEBUG(logger, "Burstio Flow - Push Sequence -- Passed basic SRI examination " );
    delete pkt;
  }

  return nbursts;
}


template < typename OUT_PORT, typename IN_PORT >
void  Burstio_PushBursts< OUT_PORT, IN_PORT >::test_fan_in( ) {

  RH_DEBUG(logger, "BURSTIO - FAN IN - BEGIN " );

  RH_DEBUG(logger, "Burstio Flow - Create Connections and Filter list " );
  op1->connectPort( ip1->_this(), "connection_1" );
  op2->connectPort( ip1->_this(), "connection_2" );
  op3->connectPort( ip1->_this(), "connection_3" );
  op4->connectPort( ip1->_this(), "connection_4" );

  // need to allow for flow of data
  op1->start();
  op2->start();
  op3->start();
  op4->start();
  ip1->start();
  int nseq=10;
  int nbursts=10;
  // setup to allow for all bursts to be pushed without blocking
  ip1->setQueueThreshold( nbursts * nseq * 4 );
  int totalBursts = 0;
  int pbursts[nseq][4];
  int n;
  for ( int i=0; i < nseq; i++ ) {
    n = test_fan_in_push( op1, 1, nbursts );
    pbursts[i][0] = n;
    totalBursts += n;
    n = test_fan_in_push( op2, 2, nbursts );
    pbursts[i][1] = n;
    totalBursts += n;
    n = test_fan_in_push( op3,3, nbursts );
    pbursts[i][2] = n;
    totalBursts += n;
    n = test_fan_in_push( op4,4, nbursts );
    pbursts[i][3] = n;
    totalBursts += n;
  }

  // check the results
  for ( int i=0; i < nseq; i++ ) {
    test_fan_in_push_results( ip1, 1, pbursts[i][0] );
    test_fan_in_push_results( ip1, 2, pbursts[i][1] );
    test_fan_in_push_results( ip1, 3, pbursts[i][2] );
    test_fan_in_push_results( ip1, 4, pbursts[i][3] );
  }

  RH_DEBUG(logger, "BURSTIO - FAN IN - END " );

}

template < typename OUT_PORT, typename IN_PORT >
void  Burstio_PushBursts< OUT_PORT, IN_PORT >::test_fan_out( ) {


  RH_DEBUG(logger, "BURSTIO - FAN OUT - BEGIN " );
  /**
   -- Can not perform Fan Out since data gets "moved" from output port 
   -- to first input port, 2, 3, 4 get nothing... Issue is when
   -- both ports are in the same process space
  op1->setLogger( logger );

  RH_DEBUG(logger, "Burstio Flow - Create Connections and Filter list " );
  op1->connectPort( ip1->_this(), "connection_1" );
  op1->connectPort( ip2->_this(), "connection_2" );
  op1->connectPort( ip3->_this(), "connection_3" );
  op1->connectPort( ip4->_this(), "connection_4" );

  // need to allow for flow of data
  op1->start();
  ip1->start();
  ip2->start();
  ip3->start();
  ip4->start();

  int nseq=4;
  int nbursts=10;
  int n;

  // setup to allow for all bursts to be pushed without blocking
  ip1->setQueueThreshold( nbursts * nseq );
  ip2->setQueueThreshold( nbursts * nseq );
  ip3->setQueueThreshold( nbursts * nseq );
  ip4->setQueueThreshold( nbursts * nseq );

  // push out data
  n = test_fan_in_push( op1, 1, nbursts );
  n = test_fan_in_push( op1, 2, nbursts );
  n = test_fan_in_push( op1, 3, nbursts );
  n = test_fan_in_push( op1, 4, nbursts );

  return;

  // check the results
  for ( int i=0; i < 4; i++ ) {
    test_fan_in_push_results( ip1, i+1, nbursts );
    test_fan_in_push_results( ip2, i+1, nbursts );
    test_fan_in_push_results( ip3, i+1, nbursts );
    test_fan_in_push_results( ip4, i+1, nbursts );
  }
  **/
  RH_DEBUG(logger, "BURSTIO - FAN OUT - END " );

}


template < typename OUT_PORT, typename IN_PORT >
int  Burstio_PushBursts< OUT_PORT, IN_PORT >::multiout_push( OUT_PORT *op,  std::string &streamId , int oid, int nbursts ) {

  BULKIO::PrecisionUTCTime TS = burstio::utils::now();
  std::ostringstream os;
  std::string sid = streamId;
  os.clear();
  os << "id-" << oid;
  std::string id = os.str();
  BURSTIO::BurstSRI sri = make_sri_test( sid, id );
  
  // push Bursts
  typename OUT_PORT::BurstSequenceType bursts;
  int i,j;
  int totalBursts=0;
  bursts.length( nbursts );
  for ( j=0; j< nbursts;j++ ) {
      typename OUT_PORT::BurstType burst;
      burst.SRI = sri;
      burst.EOS = false;
      burst.T = burstio::utils::now();
      burst.data.length(50);
      totalBursts++;
      bursts[j] = burst;
  }

  RH_DEBUG(logger, "Burstio Flow - Push Sequence --ID" << oid << " nbursts//TOTAL " << nbursts << totalBursts );
  op->pushBursts( bursts );

  return totalBursts;
}


template < typename OUT_PORT, typename IN_PORT >
int  Burstio_PushBursts< OUT_PORT, IN_PORT >::multiout_push2( OUT_PORT *op,  std::string &streamId , int oid, int nbursts ) {

  BULKIO::PrecisionUTCTime TS = burstio::utils::now();
  std::ostringstream os;
  std::string sid = streamId;
  os.clear();
  os << "id-" << oid;
  std::string id = os.str();
  BURSTIO::BurstSRI sri = make_sri_test( sid, id );
  
  // push Bursts
  int i,j;
  int totalBursts=0;
  for ( j=0; j< nbursts;j++ ) {
      typename OUT_PORT::BurstType burst;
      TS = burstio::utils::now();
      burst.data.length(50);
      totalBursts++;
      op->pushBurst( burst.data, sri, TS );
  }

  RH_DEBUG(logger, "Burstio Flow - Push Sequence --ID" << oid << " nbursts//TOTAL " << nbursts << totalBursts );


  return totalBursts;
}



template < typename OUT_PORT, typename IN_PORT >
int  Burstio_PushBursts< OUT_PORT, IN_PORT >::multiout_results( IN_PORT *ip, std::string &streamId, int oid, int nbursts ) {


  std::string sid = streamId;
  std::ostringstream os;
  os << "id-" << oid;
  std::string id = os.str();
  int ncnt=0;

  for (int i=0; i<nbursts;i++ ) {  

    typename IN_PORT::PacketType *pkt  = ip->getBurst(bulkio::Const::NON_BLOCKING );
    if ( pkt == NULL )  break;
    ncnt++;
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts PKT == NULL ", NULL != pkt );
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts EOS mismatch ", false == pkt->getEOS() );
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts Data Length mismatch ", 50 == pkt->getSize() );
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts Mode ", 0 == pkt->isComplex() );
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI mismatch ", sid == pkt->getStreamID().c_str() );
    BURSTIO::BurstSRI asri = pkt->getSRI();
    CORBA::String_var t = asri.streamID;
    std::string  expected(sid);
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts StreamID mismatch ", expected == t.in() );

    t = asri.modulation;
    expected = "mod";
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.modulation", expected == t.in() );
    t = asri.fec;  
    expected = "vit";
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.fec", expected == t.in() );
    t = asri.fecrate;  
    expected = "7/8";
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.fec", expected == t.in() );
    t = asri.randomizer;  
    expected = "R20";
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.fec", expected == t.in() );
    t = asri.overhead;  
    expected = "unknown";
    CPPUNIT_ASSERT_MESSAGE( "BURSTIO_PUSH_SEQUENCE getBursts SRI.overhead", expected == t.in() );
    RH_DEBUG(logger, "Burstio Flow - Push Sequence -- Passed basic SRI examination " );
    delete pkt;
    
  }

  return ncnt;

}




//
// 
//
// Test pushPacket data operations on each port do not affect the other port's state
//
template < typename OUT_PORT, typename IN_PORT >
void  Burstio_PushBursts< OUT_PORT, IN_PORT >::test_multiout_push( ) {

  RH_DEBUG(this->logger, "Multiout Data Filter - 1 stream id , 4 independent consumers" );

  RH_DEBUG(this->logger, "Multiout Data Filter - setup connections" );
  this->op1->connectPort( this->ip1->_this(), "connection_1");
  this->op1->connectPort( this->ip2->_this(), "connection_2");
  this->op1->connectPort( this->ip3->_this(), "connection_3");
  this->op1->connectPort( this->ip4->_this(), "connection_4");

  ExtendedCF::UsesConnectionSequence *clist = this->op1->connections();
  CPPUNIT_ASSERT( clist != NULL );
  RH_DEBUG(this->logger, "Multiout Data Filter - Check connections:" << clist->length() );
  CPPUNIT_ASSERT( clist->length() == 4 );
  delete clist;

  this->op1->updateConnectionFilter( this->desc_list );

  // need to allow for flow of data
  this->op1->start();
  this->ip1->start();
  this->ip2->start();
  this->ip3->start();
  this->ip4->start();

  //
  //  Test Filter for IP1
  //
  int nbursts=10;
  int n=0;
  std::string  filter_stream_id( "stream-1-1" );
  multiout_push( this->op1, filter_stream_id, 1, nbursts );
  n= multiout_results( this->ip1, filter_stream_id, 1, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 1- Input Port 1 Pkts received mismatch", nbursts == n );  

  // make sure the other ports did not get the data
  n= multiout_results( this->ip2, filter_stream_id, 1, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 1- Input Port 2 Pkts received mismatch", 0 == n );  
  n= multiout_results( this->ip3, filter_stream_id, 1, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 1- Input Port 3 Pkts received mismatch", 0 == n );  
  n= multiout_results( this->ip4, filter_stream_id, 1, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 1- Input Port 4 Pkts received mismatch", 0 == n );  


  //
  //  Test Filter for IP2
  //
  filter_stream_id="stream-2-1";
  multiout_push( this->op1, filter_stream_id, 2, nbursts );
  n= multiout_results( this->ip2, filter_stream_id, 2, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 2- Input Port 2 Pkts received mismatch", nbursts == n );  

  // make sure the other ports did not get the data
  n= multiout_results( this->ip1, filter_stream_id, 2, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 2- Input Port 1 Pkts received mismatch", 0 == n );  
  n= multiout_results( this->ip3, filter_stream_id, 2, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 2- Input Port 3 Pkts received mismatch", 0 == n );  
  n= multiout_results( this->ip4, filter_stream_id, 2, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 2- Input Port 4 Pkts received mismatch", 0 == n );  


  //
  //  Test Filter for IP3
  //
  filter_stream_id="stream-3-1";
  multiout_push( this->op1, filter_stream_id, 3, nbursts );
  n= multiout_results( this->ip3, filter_stream_id, 3, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 3- Input Port 3 Pkts received mismatch", nbursts == n );  

  // make sure the other ports did not get the data
  n= multiout_results( this->ip1, filter_stream_id, 3, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 3- Input Port 1 Pkts received mismatch", 0 == n );  
  n= multiout_results( this->ip2, filter_stream_id, 3, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 3- Input Port 2 Pkts received mismatch", 0 == n );  
  n= multiout_results( this->ip4, filter_stream_id, 3, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 3- Input Port 4 Pkts received mismatch", 0 == n );  


  //
  //  Test Filter for IP4
  //
  filter_stream_id="stream-4-1";
  multiout_push( this->op1, filter_stream_id, 4, nbursts );
  n= multiout_results( this->ip4, filter_stream_id, 4, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 4- Input Port 4 Pkts received mismatch", nbursts == n );  

  // make sure the other ports did not get the data
  n= multiout_results( this->ip1, filter_stream_id, 4, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 4- Input Port 1 Pkts received mismatch", 0 == n );  
  n= multiout_results( this->ip2, filter_stream_id, 4, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 4- Input Port 2 Pkts received mismatch", 0 == n );  
  n= multiout_results( this->ip3, filter_stream_id, 4, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 4- Input Port 3 Pkts received mismatch", 0 == n );  

}




//
// 
//
// Test pushPacket data operations on each port do not affect the other port's state
//
template < typename OUT_PORT, typename IN_PORT >
void  Burstio_PushBursts< OUT_PORT, IN_PORT >::test_multiout_push2( ) {

  RH_DEBUG(this->logger, "Multiout Data Filter - 1 stream id , 4 independent consumers" );

  RH_DEBUG(this->logger, "Multiout Data Filter - setup connections" );
  this->op1->connectPort( this->ip1->_this(), "connection_1");
  this->op1->connectPort( this->ip2->_this(), "connection_2");
  this->op1->connectPort( this->ip3->_this(), "connection_3");
  this->op1->connectPort( this->ip4->_this(), "connection_4");

  ExtendedCF::UsesConnectionSequence *clist = this->op1->connections();
  CPPUNIT_ASSERT( clist != NULL );
  RH_DEBUG(this->logger, "Multiout Data Filter - Check connections:" << clist->length() );
  CPPUNIT_ASSERT( clist->length() == 4 );
  delete clist;

  this->op1->updateConnectionFilter( this->desc_list );
  this->op1->setRoutingMode( burstio::ROUTE_CONNECTION_STREAMS );

  // need to allow for flow of data
  this->op1->start();
  this->ip1->start();
  this->ip2->start();
  this->ip3->start();
  this->ip4->start();

  //
  //  Test Filter for IP1
  //
  int nbursts=10;
  int n=0;
  std::string  filter_stream_id( "stream-1-1" );
  multiout_push2( this->op1, filter_stream_id, 1, nbursts );
  n= multiout_results( this->ip1, filter_stream_id, 1, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 1- Input Port 1 Pkts received mismatch", nbursts == n );  

  // make sure the other ports did not get the data
  n= multiout_results( this->ip2, filter_stream_id, 1, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 1- Input Port 2 Pkts received mismatch", 0 == n );  
  n= multiout_results( this->ip3, filter_stream_id, 1, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 1- Input Port 3 Pkts received mismatch", 0 == n );  
  n= multiout_results( this->ip4, filter_stream_id, 1, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 1- Input Port 4 Pkts received mismatch", 0 == n );  


  //
  //  Test Filter for IP2
  //
  filter_stream_id="stream-2-1";
  multiout_push2( this->op1, filter_stream_id, 2, nbursts );
  n= multiout_results( this->ip2, filter_stream_id, 2, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 2- Input Port 2 Pkts received mismatch", nbursts == n );  

  // make sure the other ports did not get the data
  n= multiout_results( this->ip1, filter_stream_id, 2, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 2- Input Port 1 Pkts received mismatch", 0 == n );  
  n= multiout_results( this->ip3, filter_stream_id, 2, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 2- Input Port 3 Pkts received mismatch", 0 == n );  
  n= multiout_results( this->ip4, filter_stream_id, 2, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 2- Input Port 4 Pkts received mismatch", 0 == n );  


  //
  //  Test Filter for IP3
  //
  filter_stream_id="stream-3-1";
  multiout_push2( this->op1, filter_stream_id, 3, nbursts );
  n= multiout_results( this->ip3, filter_stream_id, 3, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 3- Input Port 3 Pkts received mismatch", nbursts == n );  

  // make sure the other ports did not get the data
  n= multiout_results( this->ip1, filter_stream_id, 3, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 3- Input Port 1 Pkts received mismatch", 0 == n );  
  n= multiout_results( this->ip2, filter_stream_id, 3, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 3- Input Port 2 Pkts received mismatch", 0 == n );  
  n= multiout_results( this->ip4, filter_stream_id, 3, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 3- Input Port 4 Pkts received mismatch", 0 == n );  


  //
  //  Test Filter for IP4
  //
  filter_stream_id="stream-4-1";
  multiout_push2( this->op1, filter_stream_id, 4, nbursts );
  n= multiout_results( this->ip4, filter_stream_id, 4, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 4- Input Port 4 Pkts received mismatch", nbursts == n );  

  // make sure the other ports did not get the data
  n= multiout_results( this->ip1, filter_stream_id, 4, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 4- Input Port 1 Pkts received mismatch", 0 == n );  
  n= multiout_results( this->ip2, filter_stream_id, 4, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 4- Input Port 2 Pkts received mismatch", 0 == n );  
  n= multiout_results( this->ip3, filter_stream_id, 4, nbursts );
  CPPUNIT_ASSERT_MESSAGE( "MultiOut Filter Port 4- Input Port 3 Pkts received mismatch", 0 == n );  

}



// Registers the fixture into the 'registry'
// this also worked sans type name in output CPPUNIT_TEST_SUITE_REGISTRATION( PushBurstsUInt8 );
CPPUNIT_TEST_SUITE_REGISTRATION( PushBurstsUInt8_Fixture );
CPPUNIT_TEST_SUITE_REGISTRATION( PushBurstsInt16_Fixture );
CPPUNIT_TEST_SUITE_REGISTRATION( PushBurstsUInt16_Fixture );
CPPUNIT_TEST_SUITE_REGISTRATION( PushBurstsInt32_Fixture );
CPPUNIT_TEST_SUITE_REGISTRATION( PushBurstsUInt32_Fixture );
CPPUNIT_TEST_SUITE_REGISTRATION( PushBurstsInt64_Fixture );
CPPUNIT_TEST_SUITE_REGISTRATION( PushBurstsUInt64_Fixture );
CPPUNIT_TEST_SUITE_REGISTRATION( PushBurstsDouble_Fixture );
CPPUNIT_TEST_SUITE_REGISTRATION( PushBurstsFloat_Fixture );

//template class PushBursts< burstio::BurstCharPort, burstio::BurstInt8Port >;
template class Burstio_PushBursts< burstio::BurstByteOut, burstio::BurstByteIn >;
template class Burstio_PushBursts< burstio::BurstShortOut, burstio::BurstShortIn >;
template class Burstio_PushBursts< burstio::BurstUshortOut, burstio::BurstUshortIn >;
template class Burstio_PushBursts< burstio::BurstLongOut, burstio::BurstLongIn >;
template class Burstio_PushBursts< burstio::BurstUlongOut, burstio::BurstUlongIn >;
template class Burstio_PushBursts< burstio::BurstLongLongOut, burstio::BurstLongLongIn >;
template class Burstio_PushBursts< burstio::BurstUlongLongOut, burstio::BurstUlongLongIn >;
template class Burstio_PushBursts< burstio::BurstDoubleOut, burstio::BurstDoubleIn >;
template class Burstio_PushBursts< burstio::BurstFloatOut, burstio::BurstFloatIn >;
