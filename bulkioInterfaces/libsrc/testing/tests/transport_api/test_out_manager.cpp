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
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <bulkio/bulkio.h>
#include "transport_api_test.h"
#include "custom_transport.h"


template < typename PT, typename OP >
  class Test_Manager_Base : public CppUnit::TestFixture
{
    typedef custom_transport::CustomTransportFactory FACTORY;
    typedef bulkio::OutputManager< PT >              OM;
    typedef bulkio::OutputTransport< PT >            TRANSPORT_BASE;
    typedef custom_transport::CustomOutputTransport  TRANSPORT;

    CPPUNIT_TEST_SUITE( Test_Manager_Base );
    CPPUNIT_TEST( test_transport_type );
    CPPUNIT_TEST( test_transport_properties );
    CPPUNIT_TEST( test_transport_properties_disable );
    CPPUNIT_TEST( test_create_transport_normal );
    CPPUNIT_TEST( test_create_transport_disable_env );
    CPPUNIT_TEST( test_get_negotiation_props_bad_transport );
    CPPUNIT_TEST( test_get_negotiation_props );
    CPPUNIT_TEST( test_set_negotiation_results );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void setUp_post();
    void tearDown();

    void test_transport_type( );
    void test_transport_properties( );
    void test_transport_properties_disable( );
    void test_create_transport_normal( );
    void test_create_transport_disable_env( );
    void test_get_negotiation_props_bad_transport( );
    void test_get_negotiation_props( );
    void test_set_negotiation_results( );

    virtual std::string getProtocol() const = 0;

    FACTORY*        factory;
    OM*             manager;
    OP*             port;
    TRANSPORT_BASE* transport;
    TRANSPORT*      custom_transport;

    rh_logger::LoggerPtr logger;
};

#define CHECK_PROP_EXISTS(pid)   \
    { \
      std::string prop_id(pid); \
      bool val=props.contains(prop_id); \
      bool exp=true; \
      CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE( "transport properties missing "#pid , CPPUNIT_ASSERT_EQUAL( exp,val )); \
   }

#define CHECK_PROP_NOT_EXISTS(pid)   \
    { \
      std::string prop_id(pid); \
      bool val=props.contains(prop_id); \
      bool exp=true; \
      CPPUNIT_ASSERT_ASSERTION_FAIL_MESSAGE( "transport properties should not contain "#pid , CPPUNIT_ASSERT_EQUAL( exp,  val ) ); \
   }



template< typename PT, typename OP >
void Test_Manager_Base< PT, OP >::setUp()
{
    ossie::corba::CorbaInit(0,0);
    factory = 0;
    port= 0;
    manager= 0;
    transport=0;
    custom_transport=0;
    logger=rh_logger::Logger::getLogger("OutManagerTest");
    setUp_bulkio_test();
}

template< typename PT, typename OP >
void Test_Manager_Base< PT, OP >::setUp_post()
{
    factory = new FACTORY();
    port= new OP("outport");
    manager=factory->createOutputManager( port );
    transport=0;
    custom_transport=0;
}



template< typename PT, typename OP >
void Test_Manager_Base< PT, OP >::tearDown()
{
    if (transport) delete transport;
    if (manager)    delete manager;
    if (port)    delete port;
    if (factory) delete factory;

    tearDown_reset_env();
}


template< typename PT, typename OP >
void Test_Manager_Base< PT, OP >::test_transport_type()
{
    setUp_post();

    CPPUNIT_ASSERT_MESSAGE( "output manager create failed ", manager != 0 );
    std::string type=manager->transportType();
    std::string exp("custom");

    CPPUNIT_ASSERT_EQUAL( exp, type );
}

template< typename PT, typename OP >
void Test_Manager_Base< PT, OP >::test_transport_properties()
{
    setUp_post();
   CPPUNIT_ASSERT_MESSAGE( "output manager create failed ", manager != 0 );

   redhawk::PropertyMap props=redhawk::PropertyMap::cast(manager->transportProperties());
   CPPUNIT_ASSERT_MESSAGE( "no transport properties available ", props.length() != 0 );

   // reset for required properties: disable and config
   bool val=props.contains("custom::disable");
   bool exp=true;
   CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE( "Transport properties missing custom::disable", CPPUNIT_ASSERT_EQUAL( exp,val ));
   CPPUNIT_ASSERT_EQUAL( exp,val );

}



template< typename PT, typename OP >
void Test_Manager_Base< PT, OP >::test_transport_properties_disable()
{
    setUp_disable_custom();
    setUp_post();
    CPPUNIT_ASSERT_MESSAGE( "output manager create failed ", manager != 0 );

    redhawk::PropertyMap props=redhawk::PropertyMap::cast(manager->transportProperties());
    CPPUNIT_ASSERT_MESSAGE( "no transport properties available ", props.length() != 0 );

    // reset for required properties: disable and config
    bool val=props.contains("custom::disable");
    bool exp=true;
    CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE( "Transport properties missing custom::disable", CPPUNIT_ASSERT_EQUAL( exp,val ));

    int disable=props["custom::disable"].toLong();
    int exp_i=1;
    CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE( "Transport properties, custom::disable reporting incorrect value", CPPUNIT_ASSERT_EQUAL( exp_i, disable ));

}


template< typename PT, typename OP >
void Test_Manager_Base< PT, OP >::test_create_transport_disable_env()
{
    setUp_disable_custom();
    setUp_post();
    CPPUNIT_ASSERT_MESSAGE( "output manager create failed ", manager != 0 );

    // test bad parameter passing
    typename OM::PtrType obj=NULL;
    std::string connectionId("connection-1");
    redhawk::PropertyMap props=redhawk::PropertyMap::cast(manager->transportProperties());

    transport=manager->createOutputTransport( obj, connectionId, props );
    TRANSPORT_BASE *exp_t=0;
    CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE( "create transport did not fail.", CPPUNIT_ASSERT( exp_t == transport ) );
}



template< typename PT, typename OP >
void Test_Manager_Base< PT, OP >::test_create_transport_normal()
{
    setUp_bulkio_test();
    setUp_post();
    CPPUNIT_ASSERT_MESSAGE( "output manager create failed ", manager != 0 );

    // test bad parameter passing
    typename OM::PtrType obj=NULL;
    std::string connectionId("connection-1");
    redhawk::PropertyMap props;

    transport=manager->createOutputTransport( obj, connectionId, props );
    TRANSPORT_BASE *exp_t=0;
    CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE( "create transport failed", CPPUNIT_ASSERT( exp_t != transport ) );

    // down cast for specialize api
    custom_transport=dynamic_cast< TRANSPORT * >(transport);
    TRANSPORT   *exp_p=0;
    CPPUNIT_ASSERT_ASSERTION_FAIL_MESSAGE( "custom out transport failed ", CPPUNIT_ASSERT_EQUAL( exp_p, custom_transport ) );

    std::string exp_s("custom");
    std::string v_s = transport->transportType();
    CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE( "transport reports incorrect type", CPPUNIT_ASSERT_EQUAL( exp_s, v_s ));

    props=redhawk::PropertyMap::cast(transport->transportInfo());

    RH_INFO(logger, " transport Info" << props );

    // check vita properties
    CHECK_PROP_EXISTS("bulkio::custom::transport_side_information");
    CHECK_PROP_EXISTS("bulkio::custom::another_number");

}


template< typename PT, typename OP >
void Test_Manager_Base< PT, OP >::test_get_negotiation_props_bad_transport()
{
    // test bad parameter passing
    typename OM::PtrType obj=NULL;
    std::string connectionId;
    redhawk::PropertyMap props;
    redhawk::PropertyMap neg_props;

    setUp_bulkio_test();
    setUp_post();
    CPPUNIT_ASSERT_MESSAGE( "output manager create failed ", manager != 0 );

    // invalid transport should throw.. if not then props should be empt
    CPPUNIT_ASSERT_THROW_MESSAGE("Invalid transport did not fail", props=manager->getNegotiationProperties(transport), redhawk::FatalTransportError );
}


template< typename PT, typename OP >
void Test_Manager_Base< PT, OP >::test_get_negotiation_props()
{
    setUp_bulkio_test();
    setUp_post();
    CPPUNIT_ASSERT_MESSAGE( "output manager create failed ", manager != 0 );

    // test bad parameter passing
    typename OM::PtrType obj=NULL;
    std::string connectionId;
    redhawk::PropertyMap props;
    redhawk::PropertyMap neg_props;

    transport=manager->createOutputTransport( obj, connectionId, props );
    TRANSPORT_BASE *exp_t=0;
    CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE( "create transport failed", CPPUNIT_ASSERT( exp_t != transport ) );

    custom_transport=dynamic_cast< TRANSPORT * >(transport);
    TRANSPORT   *exp_p=0;
    CPPUNIT_ASSERT_ASSERTION_FAIL_MESSAGE( "Custom Out Transport Failed ", CPPUNIT_ASSERT_EQUAL( exp_p, custom_transport ) );

    // after initial creation out transport should have control connection, but no data connection endpoint properties
    neg_props=manager->getNegotiationProperties( transport );

    std::string prop_id="bulkio::custom::data::protocol";
    props[prop_id]=getProtocol();

    CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE( "getNegotiationProperties  == ",   CPPUNIT_ASSERT( props == neg_props ) );
}


template< typename PT, typename OP >
void Test_Manager_Base< PT, OP >::test_set_negotiation_results()
{
    setUp_bulkio_test();
    setUp_post();
    CPPUNIT_ASSERT_MESSAGE( "output manager create failed ", manager != 0 );

    // test bad parameter passing
    typename OM::PtrType obj=NULL;
    std::string connectionId;
    redhawk::PropertyMap props;

    // invalid transport should throw.. if not then props should be empt
    CPPUNIT_ASSERT_THROW_MESSAGE("Invalid transport did not fail", manager->setNegotiationResult(transport,props), redhawk::FatalTransportError);

    transport=manager->createOutputTransport( obj, connectionId, props );
    TRANSPORT_BASE *exp_t=0;
    CPPUNIT_ASSERT_ASSERTION_PASS_MESSAGE( "create transport failed", CPPUNIT_ASSERT( exp_t != transport ) );

    custom_transport=dynamic_cast< TRANSPORT * >(transport);
    TRANSPORT   *exp_p=0;
    CPPUNIT_ASSERT_ASSERTION_FAIL_MESSAGE( "Custom Out Transport Failed ", CPPUNIT_ASSERT_EQUAL( exp_p, custom_transport ) );
}


#define CREATE_TEST(x,y,pp)						\
  class Custom_OutManager_##x##_##pp##_Test : public Test_Manager_Base< BULKIO::data##x , bulkio::Out##y##Port  > \
    {                                                                   \
      typedef Test_Manager_Base< BULKIO::data##x , bulkio::Out##y##Port > TestBase;	\
        CPPUNIT_TEST_SUB_SUITE(Custom_OutManager_##x##_##pp##_Test, TestBase);               \
        CPPUNIT_TEST_SUITE_END();                                       \
        virtual std::string getProtocol() const {                       \
	    std::string proto(#pp);                                     \
	    std::transform(proto.begin(),proto.end(),proto.begin(),::tolower); \
	    return proto; }; 						\
    };                                                                  \
  CPPUNIT_TEST_SUITE_REGISTRATION(Custom_OutManager_##x##_##pp##_Test);

CREATE_TEST(Float,Float,udp);
