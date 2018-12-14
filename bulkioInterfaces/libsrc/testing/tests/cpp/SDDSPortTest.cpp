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

#include "InPortTestFixture.h"
#include <bulkio/bulkio.h>

class InSDDSPortTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(InSDDSPortTest);
  CPPUNIT_TEST(testBasicAPI);
  CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        port = new bulkio::InSDDSPort("dataSDDS_in");
    }

    void tearDown()
    {
        delete port;
    }

    void testBasicAPI()
    {
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
        CORBA::String_var aid = port->attach( sdef, "test_sdds_port_api" );
        CPPUNIT_ASSERT( aid != NULL );

        BULKIO::SDDSStreamSequence  *sss = port->attachedStreams();
        CPPUNIT_ASSERT( sss != NULL );
        CPPUNIT_ASSERT( sss->length() == 1 );
        std::string paddr;
        paddr = (*sss)[0].multicastAddress;
        //std::cout << "port address " << paddr << std::endl;
  
        CPPUNIT_ASSERT( strcmp( paddr.c_str(), "1.1.1.1") == 0  );
        delete sss;

        CORBA::String_var uid = port->getUser(aid);
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

private:
    bulkio::InSDDSPort* port;
};

CPPUNIT_TEST_SUITE_REGISTRATION(InSDDSPortTest);

class NewSriCallback  {

public:

    std::vector<std::string>  sids;

    ~NewSriCallback() {};

    void newSriCB( const BULKIO::StreamSRI& sri) {
        std::string sid(sri.streamID);
        sids.push_back( sid );
    }
};

// Global connection/disconnection callbacks
static void port_connected( const char* connectionId ) {

}

static void port_disconnected( const char* connectionId ) {

}

class OutSDDSPortTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OutSDDSPortTest);
  CPPUNIT_TEST(testBasicAPI);
  CPPUNIT_TEST(testSRI);
  CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        logger = rh_logger::Logger::getLogger("BulkioOutPort");
        logger->setLevel( rh_logger::Level::getInfo());
        port = new bulkio::OutSDDSPort("dataSDDS_out", logger);
    }

    void tearDown()
    {
        delete port;
    }

    void testBasicAPI()
    {
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

    void testSRI()
    {
        ExtendedCF::UsesConnectionSequence *clist = port->connections();
        CPPUNIT_ASSERT( clist != NULL );
        delete clist;

        NewSriCallback sri_cb;
        bulkio::InSDDSPort *p  = new bulkio::InSDDSPort("sink_1", logger );
        PortableServer::ObjectId_var p_oid = ossie::corba::RootPOA()->activate_object(p);
        p->setNewSriListener(&sri_cb, &NewSriCallback::newSriCB );

        BULKIO::StreamSRI sri;
        BULKIO::SDDSStreamDefinition sdds;
        sri.streamID = "stream1";
        sri.xdelta = 1/1000.0;
        sdds.id = "stream1";
        sdds.dataFormat = BULKIO::SDDS_SB;
        sdds.multicastAddress = "bad.ip.address";
        sdds.port = 9999;
        sdds.vlan = 0;
        port->addStream(sdds);
        port->pushSRI(sri, bulkio::time::utils::now());

        sri.streamID = "stream2";
        sdds.id = "stream2";
        port->addStream(sdds);
        port->pushSRI(sri, bulkio::time::utils::now());

        port->connectPort( p->_this(), "connection_1");

        int slen = sri_cb.sids.size();
        CPPUNIT_ASSERT( slen == 2 ) ;
  
        port->disconnectPort( "connection_1");
        ossie::corba::RootPOA()->deactivate_object(p_oid);
    }

private:
    bulkio::OutSDDSPort* port;
    rh_logger::LoggerPtr logger;
};

CPPUNIT_TEST_SUITE_REGISTRATION(OutSDDSPortTest);
