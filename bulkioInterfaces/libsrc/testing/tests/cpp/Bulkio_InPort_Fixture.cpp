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
#include <boost/scoped_ptr.hpp>

#include "Bulkio_InPort_Fixture.h"
#include "bulkio.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Bulkio_InPort_Fixture );

class SriListener {
public:
  SriListener() :
    sri_(),
    sriChanged_(false)
  {
  }

  void updateSRI(BULKIO::StreamSRI& sri)
  {
    sri_ = sri;
    sriChanged_ = true;
  }

  void reset()
  {
    sriChanged_ = false;
  }

  bool sriChanged()
  {
    return sriChanged_;
  }

private:
  BULKIO::StreamSRI sri_;
  bool sriChanged_;
};


class MyFloatPort : public bulkio::InFloatPort {

public:

  MyFloatPort( std::string pname, bulkio::LOGGER_PTR logger ) : 
    bulkio::InFloatPort( pname, logger ) {};

  //
  // over ride default behavior for pushPacket and pushSRI
  //
  void pushPacket(const bulkio::InFloatPort::PortSequenceType & data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID) {
      stats->update(10, (float)workQueue.size()/(float)queueSem->getMaxValue(), EOS, streamID, false);
      queueSem->setCurrValue(workQueue.size());
      bulkio::InFloatPort::pushPacket( data, T, EOS, streamID );
  }

  void pushSRI(const BULKIO::StreamSRI& H) {
      queueSem->setCurrValue(workQueue.size());
      bulkio::InFloatPort::pushSRI(H);
  }
};


void 
Bulkio_InPort_Fixture::setUp()
{
   logger =rh_logger::Logger::getLogger("BulkioInPort");
   logger->setLevel( rh_logger::Level::getInfo());
}


void 
Bulkio_InPort_Fixture::tearDown()
{
}

template< typename T>
void  Bulkio_InPort_Fixture::test_port_api( T *port  ) {

 RH_DEBUG(logger, "Running tests port:" << port->getName() );

  BULKIO::PortStatistics *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT( rt == BULKIO::IDLE );

  BULKIO::StreamSRISequence  *streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  delete streams;

  int tmp = port->getMaxQueueDepth();
  CPPUNIT_ASSERT( tmp == 100 );

  tmp = port->getCurrentQueueDepth();
  CPPUNIT_ASSERT( tmp == 0 );

  port->setMaxQueueDepth(22);
  tmp = port->getMaxQueueDepth();
  CPPUNIT_ASSERT( tmp == 22 );

  // check that port queue is empty
  typename T::dataTransfer *pkt  = port->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT( pkt == NULL );

  BULKIO::StreamSRI sri;
  sri = bulkio::sri::create();
  sri.streamID = "test_port_api";
  port->pushSRI( sri );

  streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  delete streams;

  typename T::PortSequenceType v;
  BULKIO::PrecisionUTCTime TS;
  port->pushPacket( v, TS, false, "test_port_api" );

  // grab off packet
  pkt  = port->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT( pkt != NULL );
  CPPUNIT_ASSERT( pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT( pkt->SRI.mode == 0 ) ;
  delete pkt;

  sri.mode = 1;
  port->pushSRI(sri);

  streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  delete streams;

  port->pushPacket( v, TS, false, "test_port_api" );

  // grab off packet
  pkt  = port->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT( pkt != NULL );
  CPPUNIT_ASSERT( pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT( pkt->SRI.mode == 1 ) ;
  delete pkt;

  // test for EOS..
  port->pushPacket( v, TS, true, "test_port_api" );

  // grab off packet
  pkt  = port->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT( pkt != NULL );
  CPPUNIT_ASSERT( pkt->EOS == 1 ) ;
  CPPUNIT_ASSERT( pkt->SRI.mode == 1 ) ;
  delete pkt;

  port->enableStats( false );

  port->block();

  port->unblock();

  test_sri_change(port);
}

template<>
void  Bulkio_InPort_Fixture::test_port_api( bulkio::InFilePort *port  ) {

  BULKIO::PortStatistics *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT( rt == BULKIO::IDLE );

  BULKIO::StreamSRISequence  *streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  delete streams;

  int tmp = port->getMaxQueueDepth();
  CPPUNIT_ASSERT( tmp == 100 );

  tmp = port->getCurrentQueueDepth();
  CPPUNIT_ASSERT( tmp == 0 );

  port->setMaxQueueDepth(22);
  tmp = port->getMaxQueueDepth();
  CPPUNIT_ASSERT( tmp == 22 );

  // check that port queue is empty
  bulkio::InFilePort::dataTransfer *pkt  = port->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT( pkt == NULL );

  BULKIO::StreamSRI sri;
  port->pushSRI( sri );

  streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  delete streams;

  bulkio::InFilePort::PortSequenceType v = new bulkio::InFilePort::TransportType[1];
  BULKIO::PrecisionUTCTime TS;
  port->pushPacket( v, TS, false, "test_port_api" );

  // grab off packet
  pkt  = port->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT( pkt != NULL );
  delete pkt;

  port->enableStats( false );

  port->block();

  port->unblock();

  test_sri_change(port);
}


template<>
void  Bulkio_InPort_Fixture::test_port_api( bulkio::InXMLPort *port  ) {

  BULKIO::PortStatistics *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT( rt == BULKIO::IDLE );

  BULKIO::StreamSRISequence  *streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  delete streams;

  int tmp = port->getMaxQueueDepth();
  CPPUNIT_ASSERT( tmp == 100 );

  tmp = port->getCurrentQueueDepth();
  CPPUNIT_ASSERT( tmp == 0 );

  port->setMaxQueueDepth(22);
  tmp = port->getMaxQueueDepth();
  CPPUNIT_ASSERT( tmp == 22 );

  // check that port queue is empty
  bulkio::InXMLPort::dataTransfer *pkt  = port->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT( pkt == NULL );

  BULKIO::StreamSRI sri;
  port->pushSRI( sri );

  streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  delete streams;

  bulkio::InXMLPort::PortSequenceType v = new bulkio::InXMLPort::TransportType[1];
  BULKIO::PrecisionUTCTime TS;
  port->pushPacket( v, TS, false, "test_port_api" );

  // grab off packet
  pkt  = port->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT( pkt != NULL );
  delete pkt;

  port->enableStats( false );

  port->block();

  port->unblock();

  test_sri_change(port);
}

template< typename T>
void  Bulkio_InPort_Fixture::test_sri_change( T *port  ) {
  typename T::PortSequenceType v;
  BULKIO::PrecisionUTCTime TS;

  // Push data without an SRI to check that the sriChanged flag is still set
  // and the SRI callback gets called
  boost::scoped_ptr<typename T::dataTransfer> packet;
  SriListener listener;
  port->setNewStreamListener(&listener, &SriListener::updateSRI);
  port->pushPacket(v, TS, false, "invalid_stream");
  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT(packet->sriChanged == true);
  CPPUNIT_ASSERT(listener.sriChanged() == true);

  // Push again to the same stream ID; sriChanged should now be false and the
  // SRI callback should not get called
  listener.reset();
  port->pushPacket(v, TS, false, "invalid_stream");
  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT(packet->sriChanged == false);
  CPPUNIT_ASSERT(listener.sriChanged() == false);
}

template <typename T>
void Bulkio_InPort_Fixture::test_stream_disable(T* port)
{
  typedef typename T::PortSequenceType PortSequenceType;
  typedef typename T::StreamType StreamType;
  typedef typename StreamType::DataBlockType DataBlockType;

  // Remove any existing stream listener
  port->setNewStreamListener((bulkio::SriListener*) 0);

  // Create a new stream and push some data to it
  BULKIO::StreamSRI sri = bulkio::sri::create("test_stream_disable");
  port->pushSRI(sri);
  PortSequenceType data;
  data.length(1024);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri.streamID);

  // Get the input stream and read the first packet
  StreamType stream = port->getStream("test_stream_disable");
  CPPUNIT_ASSERT_EQUAL(!stream, false);

  DataBlockType block = stream.read();
  CPPUNIT_ASSERT_EQUAL(!block, false);

  // Push a couple more packets, but only read part of the first
  int current_depth = port->getCurrentQueueDepth();
  data.length(1024);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri.streamID);
  data.length(1024);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri.streamID);

  // Read half of the first packet
  block = stream.read(512);

  // Disable the stream
  stream.disable();
  CPPUNIT_ASSERT(!stream.enabled());
  CPPUNIT_ASSERT(!stream.ready());
  CPPUNIT_ASSERT_EQUAL((size_t) 0, stream.samplesAvailable());
  CPPUNIT_ASSERT_EQUAL(current_depth, port->getCurrentQueueDepth());

  // Push a couple more packets; they should get dropped
  data.length(1024);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri.streamID);
  data.length(1024);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri.streamID);
  CPPUNIT_ASSERT_EQUAL(current_depth, port->getCurrentQueueDepth());

  // Push an end-of-stream packet
  port->pushPacket(data, bulkio::time::utils::notSet(), true, sri.streamID);

  // Re-enable the stream
  stream.enable();
  block = stream.read();
  CPPUNIT_ASSERT(!block);
  CPPUNIT_ASSERT(stream.eos());
}


template <typename T>
void Bulkio_InPort_Fixture::test_stream_sri_changed(T* port)
{
  typedef typename T::PortSequenceType PortSequenceType;
  typedef typename T::StreamType StreamType;
  typedef typename StreamType::DataBlockType DataBlockType;

  // Create a new stream and push some data to it
  BULKIO::StreamSRI sri = bulkio::sri::create("test_stream_sri_changed");
  // push sri , data seqeunce
  port->pushSRI(sri);
  PortSequenceType data;
  data.length(1024);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri.streamID);

  // Get the input stream and read the first packet
  StreamType stream = port->getStream("test_stream_sri_changed");
  CPPUNIT_ASSERT_EQUAL(!stream, false);

  DataBlockType block = stream.read();
  CPPUNIT_ASSERT_EQUAL(!block, false);

  CPPUNIT_ASSERT_EQUAL(true, block.sriChanged());

  data.length(1024);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri.streamID);
  block = stream.read();
  CPPUNIT_ASSERT_EQUAL(!block, false);
  CPPUNIT_ASSERT_EQUAL(false, block.sriChanged());

  // push sri , data seqeunce
  sri.mode = 1;
  port->pushSRI(sri);
  data.length(1024);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri.streamID);

  block = stream.read();
  CPPUNIT_ASSERT_EQUAL(true, block.sriChanged());
  int srichangedflags = block.sriChangeFlags();
  bool modeset = srichangedflags == bulkio::sri::MODE;
  CPPUNIT_ASSERT_EQUAL(true, modeset);

  data.length(1024);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri.streamID);
  block = stream.read();
  CPPUNIT_ASSERT_EQUAL(!block, false);
  CPPUNIT_ASSERT_EQUAL(false, block.sriChanged());

  data.length(1024);
  port->pushPacket(data, bulkio::time::utils::now(), true, sri.streamID);
  block = stream.read();
  CPPUNIT_ASSERT_EQUAL(!block, false);
  CPPUNIT_ASSERT_EQUAL(false, block.sriChanged());
  CPPUNIT_ASSERT_EQUAL(true, stream.eos());

}

template <typename T>
void Bulkio_InPort_Fixture::test_get_packet_stream_removed(T* port)
{
  typedef typename T::PortSequenceType PortSequenceType;
  typedef typename T::StreamList StreamList;

  const size_t stream_count = port->getStreams().size();

  // Create a new stream and push some data to it
  const char* stream_id = "test_get_packet_stream_removed";
  BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
  // push sri , data seqeunce
  port->pushSRI(sri);
  PortSequenceType data;
  data.length(1024);
  port->pushPacket(data, bulkio::time::utils::now(), true, sri.streamID);

  StreamList streams = port->getStreams();
  CPPUNIT_ASSERT_EQUAL(stream_count + 1, streams.size());

  boost::scoped_ptr<typename T::dataTransfer> packet;
  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(true, packet->EOS);

  // The set of streams should have returned to the original
  streams = port->getStreams();
  CPPUNIT_ASSERT_EQUAL(stream_count, streams.size());
  for (typename StreamList::iterator iter = streams.begin(); iter != streams.end(); ++iter) {
      if (iter->streamID() == stream_id) {
          CPPUNIT_FAIL("stream not removed on getPacket() with EOS");
      }
  }
}


template <typename T>
void Bulkio_InPort_Fixture::test_queue_flush_flags(T* port)
{
  typedef typename T::PortSequenceType PortSequenceType;

  // Push 1 packet for the normal data stream
  BULKIO::StreamSRI sri_data = bulkio::sri::create("stream_data");
  sri_data.blocking = false;
  port->pushSRI(sri_data);

  PortSequenceType data;
  data.length(1);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri_data.streamID);

  // Push 1 packet for the EOS test stream
  BULKIO::StreamSRI sri_eos = bulkio::sri::create("stream_eos");
  sri_eos.blocking = false;
  port->pushSRI(sri_eos);

  data.length(1);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri_eos.streamID);

  // Push 1 packet for the SRI change stream
  BULKIO::StreamSRI sri_change = bulkio::sri::create("stream_sri");
  sri_change.blocking = false;
  sri_change.mode = 0;
  port->pushSRI(sri_change);

  data.length(1);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri_change.streamID);

  // Grab the packets to ensure the initial conditions are correct
  boost::scoped_ptr<typename T::dataTransfer> packet;
  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_data.streamID), packet->streamID);

  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_eos.streamID), packet->streamID);

  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_change.streamID), packet->streamID);

  // Push an EOS packet for the EOS stream
  port->pushPacket(PortSequenceType(), bulkio::time::utils::notSet(), true, sri_eos.streamID);

  // Modify the SRI for the SRI change stream and push another packet
  sri_change.mode = 1;
  port->pushSRI(sri_change);
  data.length(2);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri_change.streamID);

  // Cause a queue flush by lowering the ceiling and pushing packets
  port->setMaxQueueDepth(3);
  data.length(1);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri_data.streamID);
  data.length(1);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri_data.streamID);

  // Push another packet for the SRI change stream
  data.length(2);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri_change.streamID);

  // 1st packet should be for EOS stream, with no data or SRI change flag
  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_eos.streamID), packet->streamID);
  CPPUNIT_ASSERT(packet->inputQueueFlushed);
  CPPUNIT_ASSERT(packet->EOS);
  CPPUNIT_ASSERT(!packet->sriChanged);
  CPPUNIT_ASSERT(packet->dataBuffer.empty());

  // 2nd packet should be for data stream, with no EOS or SRI change flag
  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_data.streamID), packet->streamID);
  CPPUNIT_ASSERT(!packet->inputQueueFlushed);
  CPPUNIT_ASSERT(!packet->EOS);
  CPPUNIT_ASSERT(!packet->sriChanged);

  // 3rd packet should contain the "lost" SRI change flag
  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_change.streamID), packet->streamID);
  CPPUNIT_ASSERT(!packet->inputQueueFlushed);
  CPPUNIT_ASSERT(!packet->EOS);
  CPPUNIT_ASSERT(packet->sriChanged);
}

template <>
void Bulkio_InPort_Fixture::test_queue_flush_flags(bulkio::InFilePort*)
{
  // Create a new port instance to avoid interference from other parts of the
  // test, due to the way testing is structured in 2.0
  boost::scoped_ptr<bulkio::InFilePort> port(new bulkio::InFilePort("dataFile_in", logger));

  // Push 1 packet for the normal data stream
  BULKIO::StreamSRI sri_data = bulkio::sri::create("stream_data");
  sri_data.blocking = false;
  port->pushSRI(sri_data);

  const char* data = "file:///var/tmp/test";
  port->pushPacket(data, bulkio::time::utils::now(), false, sri_data.streamID);

  // Push 1 packet for the EOS test stream
  BULKIO::StreamSRI sri_eos = bulkio::sri::create("stream_eos");
  sri_eos.blocking = false;
  port->pushSRI(sri_eos);

  port->pushPacket(data, bulkio::time::utils::now(), false, sri_eos.streamID);

  // Push 1 packet for the SRI change stream
  BULKIO::StreamSRI sri_change = bulkio::sri::create("stream_sri");
  sri_change.blocking = false;
  sri_change.mode = 0;
  port->pushSRI(sri_change);

  port->pushPacket(data, bulkio::time::utils::now(), false, sri_change.streamID);

  // Grab the packets to ensure the initial conditions are correct
  boost::scoped_ptr<bulkio::InFilePort::dataTransfer> packet;
  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_data.streamID), packet->streamID);

  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_eos.streamID), packet->streamID);

  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_change.streamID), packet->streamID);

  // Push an EOS packet for the EOS stream
  port->pushPacket("", bulkio::time::utils::notSet(), true, sri_eos.streamID);

  // Modify the SRI for the SRI change stream and push another packet
  sri_change.mode = 1;
  port->pushSRI(sri_change);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri_change.streamID);

  // Cause a queue flush by lowering the ceiling and pushing packets
  port->setMaxQueueDepth(3);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri_data.streamID);
  port->pushPacket(data, bulkio::time::utils::now(), false, sri_data.streamID);

  // Push another packet for the SRI change stream
  port->pushPacket(data, bulkio::time::utils::now(), false, sri_change.streamID);

  // 1st packet should be for EOS stream, with no data or SRI change flag
  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_eos.streamID), packet->streamID);
  CPPUNIT_ASSERT(packet->inputQueueFlushed);
  CPPUNIT_ASSERT(packet->EOS);
  CPPUNIT_ASSERT(!packet->sriChanged);
  CPPUNIT_ASSERT(packet->dataBuffer.empty());

  // 2nd packet should be for data stream, with no EOS or SRI change flag
  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_data.streamID), packet->streamID);
  CPPUNIT_ASSERT(!packet->inputQueueFlushed);
  CPPUNIT_ASSERT(!packet->EOS);
  CPPUNIT_ASSERT(!packet->sriChanged);

  // 3rd packet should contain the "lost" SRI change flag
  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_change.streamID), packet->streamID);
  CPPUNIT_ASSERT(!packet->inputQueueFlushed);
  CPPUNIT_ASSERT(!packet->EOS);
  CPPUNIT_ASSERT(packet->sriChanged);
}

template <>
void Bulkio_InPort_Fixture::test_queue_flush_flags(bulkio::InXMLPort*)
{
  // Create a new port instance to avoid interference from other parts of the
  // test, due to the way testing is structured in 2.0
  boost::scoped_ptr<bulkio::InXMLPort> port(new bulkio::InXMLPort("dataXML_in", logger));

  // Push 1 packet for the normal data stream
  BULKIO::StreamSRI sri_data = bulkio::sri::create("stream_data");
  sri_data.blocking = false;
  port->pushSRI(sri_data);

  const char* data = "<document/>";
  port->pushPacket(data, false, sri_data.streamID);

  // Push 1 packet for the EOS test stream
  BULKIO::StreamSRI sri_eos = bulkio::sri::create("stream_eos");
  sri_eos.blocking = false;
  port->pushSRI(sri_eos);

  port->pushPacket(data, false, sri_eos.streamID);

  // Push 1 packet for the SRI change stream
  BULKIO::StreamSRI sri_change = bulkio::sri::create("stream_sri");
  sri_change.blocking = false;
  sri_change.mode = 0;
  port->pushSRI(sri_change);

  port->pushPacket(data, false, sri_change.streamID);

  // Grab the packets to ensure the initial conditions are correct
  boost::scoped_ptr<bulkio::InXMLPort::dataTransfer> packet;
  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_data.streamID), packet->streamID);

  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_eos.streamID), packet->streamID);

  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_change.streamID), packet->streamID);

  // Push an EOS packet for the EOS stream
  port->pushPacket("", true, sri_eos.streamID);

  // Modify the SRI for the SRI change stream and push another packet
  sri_change.mode = 1;
  port->pushSRI(sri_change);
  port->pushPacket(data, false, sri_change.streamID);

  // Cause a queue flush by lowering the ceiling and pushing packets
  port->setMaxQueueDepth(3);
  port->pushPacket(data, false, sri_data.streamID);
  port->pushPacket(data, false, sri_data.streamID);

  // Push another packet for the SRI change stream
  port->pushPacket(data, false, sri_change.streamID);

  // 1st packet should be for EOS stream, with no data or SRI change flag
  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_eos.streamID), packet->streamID);
  CPPUNIT_ASSERT(packet->inputQueueFlushed);
  CPPUNIT_ASSERT(packet->EOS);
  CPPUNIT_ASSERT(!packet->sriChanged);
  CPPUNIT_ASSERT(packet->dataBuffer.empty());

  // 2nd packet should be for data stream, with no EOS or SRI change flag
  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_data.streamID), packet->streamID);
  CPPUNIT_ASSERT(!packet->inputQueueFlushed);
  CPPUNIT_ASSERT(!packet->EOS);
  CPPUNIT_ASSERT(!packet->sriChanged);

  // 3rd packet should contain the "lost" SRI change flag
  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT_EQUAL(std::string(sri_change.streamID), packet->streamID);
  CPPUNIT_ASSERT(!packet->inputQueueFlushed);
  CPPUNIT_ASSERT(!packet->EOS);
  CPPUNIT_ASSERT(packet->sriChanged);
}

template <typename T>
void Bulkio_InPort_Fixture::test_queue_size(T* inport)
{
  // Create a new port instance to avoid interference from other parts of the
  // test, due to the way testing is structured in 2.0
  T instance(inport->getName() + "2", logger);
  T* port = &instance;

  BULKIO::StreamSRI sri = bulkio::sri::create("queue_size");
  port->pushSRI(sri);

  // Start with a reasonably small queue depth and check that a flush occurs at
  // the expected time
  port->setMaxQueueDepth(10);
  for (int ii = 0; ii < 10; ++ii) {
    _pushTestPacket(port, 1, bulkio::time::utils::now(), false, sri.streamID);
  }
  CPPUNIT_ASSERT_EQUAL(10, port->getCurrentQueueDepth());
  _pushTestPacket(port, 1, bulkio::time::utils::now(), false, sri.streamID);
  CPPUNIT_ASSERT_EQUAL(1, port->getCurrentQueueDepth());

  boost::scoped_ptr<typename T::dataTransfer> packet;
  packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
  CPPUNIT_ASSERT(packet);
  CPPUNIT_ASSERT(packet->inputQueueFlushed);

  // Set queue depth to unlimited and push a lot of packets
  port->setMaxQueueDepth(-1);
  const int QUEUE_SIZE = 250;
  for (int ii = 0; ii < QUEUE_SIZE; ++ii) {
      _pushTestPacket(port, 1, bulkio::time::utils::now(), false, sri.streamID);
  }
  CPPUNIT_ASSERT_EQUAL(QUEUE_SIZE, port->getCurrentQueueDepth());
  for (int ii = 0; ii < QUEUE_SIZE; ++ii) {
      packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
      CPPUNIT_ASSERT(packet);
      CPPUNIT_ASSERT(!packet->inputQueueFlushed);
  }
}

template<>
void  Bulkio_InPort_Fixture::test_port_api( bulkio::InSDDSPort *port  ) {

  BULKIO::PortStatistics *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT( rt == BULKIO::IDLE );

  BULKIO::StreamSRISequence  *streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  delete streams;

  BULKIO::StreamSRI sri;
  BULKIO::PrecisionUTCTime TS;
  port->pushSRI( sri, TS );

  streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  delete streams;


  BULKIO::SDDSStreamDefinition sdef;
  sdef.id = "test_sdds_id";
  sdef.dataFormat = BULKIO::SDDS_SB;
  sdef.multicastAddress = "1.1.1.1";
  sdef.vlan = 1234;
  sdef.port = 5678;
  char *aid = port->attach( sdef, "test_sdds_port_api" );
  CPPUNIT_ASSERT( aid != NULL );

  BULKIO::SDDSStreamSequence  *sss = port->attachedStreams();
  CPPUNIT_ASSERT( sss != NULL );
  CPPUNIT_ASSERT( sss->length() == 1 );
  std::string paddr;
  paddr = (*sss)[0].multicastAddress;
  //std::cout << "port address " << paddr << std::endl;
  
  CPPUNIT_ASSERT( strcmp( paddr.c_str(), "1.1.1.1") == 0  );
  delete sss;

  char *uid = port->getUser(aid);
  CPPUNIT_ASSERT( uid != NULL );
  //std::cout << "user id " << uid << std::endl;
  CPPUNIT_ASSERT( strcmp( uid, "test_sdds_port_api" ) == 0 );


  port->detach( aid );

  sss = port->attachedStreams();
  CPPUNIT_ASSERT( sss != NULL );
  CPPUNIT_ASSERT( sss->length() == 0 );
  delete sss;

  port->enableStats( false );


}




void 
Bulkio_InPort_Fixture::test_create_int8()
{
  bulkio::InInt8Port *port = new bulkio::InInt8Port("test_ctor_int8", logger );
  CPPUNIT_ASSERT( port != NULL );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void 
Bulkio_InPort_Fixture::test_int8()
{
  bulkio::InInt8Port *port = new bulkio::InInt8Port("test_api_int8", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );
  test_stream_disable( port );
  test_stream_sri_changed( port );
  test_get_packet_stream_removed( port );
  test_queue_flush_flags( port );
  test_queue_size( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}

void 
Bulkio_InPort_Fixture::test_create_int16()
{
  bulkio::InInt16Port *port = new bulkio::InInt16Port("test_ctor_int16");
  CPPUNIT_ASSERT( port != NULL );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void 
Bulkio_InPort_Fixture::test_int16()
{
  bulkio::InInt16Port *port = new bulkio::InInt16Port("test_api_int16");
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );
  test_stream_disable( port );
  test_stream_sri_changed( port );
  test_get_packet_stream_removed( port );
  test_queue_flush_flags( port );
  test_queue_size( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}

void 
Bulkio_InPort_Fixture::test_create_int32()
{
  bulkio::InInt32Port *port = new bulkio::InInt32Port("test_ctor_int32");
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_InPort_Fixture::test_int32()
{
  bulkio::InInt32Port *port = new bulkio::InInt32Port("test_api_int32");
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );
  test_stream_disable( port );
  test_stream_sri_changed( port );
  test_get_packet_stream_removed( port );
  test_queue_flush_flags( port );
  test_queue_size( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Bulkio_InPort_Fixture::test_create_int64()
{
  bulkio::InInt64Port *port = new bulkio::InInt64Port("test_ctor_int64");
  CPPUNIT_ASSERT( port != NULL );
}


void 
Bulkio_InPort_Fixture::test_int64()
{
  bulkio::InInt64Port *port = new bulkio::InInt64Port("test_api_int64");
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );
  test_stream_disable( port );
  test_stream_sri_changed( port );
  test_get_packet_stream_removed( port );
  test_queue_flush_flags( port );
  test_queue_size( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Bulkio_InPort_Fixture::test_create_uint8()
{
  bulkio::InUInt8Port *port = new bulkio::InUInt8Port("test_ctor_uint8");
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_InPort_Fixture::test_uint8()
{
  bulkio::InUInt8Port *port = new bulkio::InUInt8Port("test_api_uint8");
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );
  test_stream_disable( port );
  test_stream_sri_changed( port );
  test_get_packet_stream_removed( port );
  test_queue_flush_flags( port );
  test_queue_size( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}

void 
Bulkio_InPort_Fixture::test_create_uint16()
{
  bulkio::InUInt16Port *port = new bulkio::InUInt16Port("test_ctor_uint16");
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_InPort_Fixture::test_uint16()
{
  bulkio::InUInt16Port *port = new bulkio::InUInt16Port("test_api_uint16");
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );
  test_stream_disable( port );
  test_stream_sri_changed( port );
  test_get_packet_stream_removed( port );
  test_queue_flush_flags( port );
  test_queue_size( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Bulkio_InPort_Fixture::test_create_uint32()
{
  bulkio::InUInt32Port *port = new bulkio::InUInt32Port("test_ctor_uint32");
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_InPort_Fixture::test_uint32()
{
  bulkio::InUInt32Port *port = new bulkio::InUInt32Port("test_api_uint32");
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );
  test_stream_disable( port );
  test_stream_sri_changed( port );
  test_get_packet_stream_removed( port );
  test_queue_flush_flags( port );
  test_queue_size( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Bulkio_InPort_Fixture::test_create_uint64()
{
  bulkio::InUInt64Port *port = new bulkio::InUInt64Port("test_ctor_uint64");
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_InPort_Fixture::test_uint64()
{
  bulkio::InUInt64Port *port = new bulkio::InUInt64Port("test_api_uint64");
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );
  test_stream_disable( port );
  test_stream_sri_changed( port );
  test_get_packet_stream_removed( port );
  test_queue_flush_flags( port );
  test_queue_size( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}



void 
Bulkio_InPort_Fixture::test_create_float()
{
  bulkio::InFloatPort *port = new bulkio::InFloatPort("test_ctor_float");
  CPPUNIT_ASSERT( port != NULL );
}

void Bulkio_InPort_Fixture::test_float()
{
  bulkio::InFloatPort *port = new bulkio::InFloatPort("test_api_float");
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );
  test_stream_disable( port );
  test_stream_sri_changed( port );
  test_get_packet_stream_removed( port );
  test_queue_flush_flags( port );
  test_queue_size( port );
}

void 
Bulkio_InPort_Fixture::test_create_double()
{
  bulkio::InDoublePort *port = new bulkio::InDoublePort("test_ctor_float");
  CPPUNIT_ASSERT( port != NULL );
}

void Bulkio_InPort_Fixture::test_double()
{
  bulkio::InDoublePort *port = new bulkio::InDoublePort("test_api_double");
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );
  test_stream_disable( port );
  test_stream_sri_changed( port );
  test_get_packet_stream_removed( port );
  test_queue_flush_flags( port );
  test_queue_size( port );
}


void 
Bulkio_InPort_Fixture::test_create_file()
{
  bulkio::InFilePort *port = new bulkio::InFilePort("test_ctor_file", logger );
  CPPUNIT_ASSERT( port != NULL );
}


void 
Bulkio_InPort_Fixture::test_file()
{
  bulkio::InFilePort *port = new bulkio::InFilePort("test_api_file", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );
  test_queue_flush_flags( port );
  test_queue_size( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Bulkio_InPort_Fixture::test_create_xml()
{
  bulkio::InXMLPort *port = new bulkio::InXMLPort("test_ctor_xml", logger );
  CPPUNIT_ASSERT( port != NULL );
}



void 
Bulkio_InPort_Fixture::test_xml()
{
  bulkio::InXMLPort *port = new bulkio::InXMLPort("test_api_xml", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );
  test_queue_flush_flags( port );
  test_queue_size( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Bulkio_InPort_Fixture::test_create_sdds()
{
  bulkio::InSDDSPort *port = new bulkio::InSDDSPort("test_ctor_sdds", logger );
  CPPUNIT_ASSERT( port != NULL );
}



void 
Bulkio_InPort_Fixture::test_sdds()
{
  bulkio::InSDDSPort *port = new bulkio::InSDDSPort("test_api_sdds", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Bulkio_InPort_Fixture::test_subclass()
{
  bulkio::InFloatPort *port = new MyFloatPort("test_api_subclass", logger );
  
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}

// Backported from 2.2 unit tests
template <typename T>
void Bulkio_InPort_Fixture::_pushTestPacket(T* port, size_t length,
                                            const BULKIO::PrecisionUTCTime& time,
                                            bool eos, const char* streamID)
{
    typename T::PortSequenceType data;
    data.length(length);
    port->pushPacket(data, time, eos, streamID);
}

template <>
void Bulkio_InPort_Fixture::_pushTestPacket(bulkio::InXMLPort* port, size_t length,
                                            const BULKIO::PrecisionUTCTime&,
                                            bool eos, const char* streamID)
{
    std::string data(length, ' ');
    port->pushPacket(data.c_str(), eos, streamID);
}

template <>
void Bulkio_InPort_Fixture::_pushTestPacket(bulkio::InFilePort* port, size_t length,
                                            const BULKIO::PrecisionUTCTime& time,
                                            bool eos, const char* streamID)
{
    std::string data(length, ' ');
    port->pushPacket(data.c_str(), time, eos, streamID);
}
