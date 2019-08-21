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



template < typename PT, typename OP, typename IP >
  class Test_Factory_Base : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( Test_Factory_Base );
  CPPUNIT_TEST( test_transport_factory );
  CPPUNIT_TEST( test_transport_type );
  CPPUNIT_TEST( test_default_priority );
  CPPUNIT_TEST( test_create_input_manager );
  CPPUNIT_TEST( test_create_output_manager );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void test_transport_factory( );
  void test_transport_type( );
  void test_default_priority( );
  void test_create_input_manager( );
  void test_create_output_manager( );

  virtual std::string getPortName() const =0;

   custom_transport::CustomTransportFactory  *factory;
    OP* outPort;
    IP* inPort;
};

template< typename PT, typename OP, typename IP >
void Test_Factory_Base< PT, OP, IP >::setUp()
{
    ossie::corba::CorbaInit(0,0);
    setUp_bulkio_test();
    factory = new custom_transport::CustomTransportFactory();
}


template< typename PT, typename OP, typename IP >
void Test_Factory_Base< PT, OP, IP >::tearDown()
{
    tearDown_reset_env();
    if ( factory ) delete factory;
}


template< typename PT, typename OP, typename IP >
void Test_Factory_Base< PT, OP, IP >::test_transport_factory()
{
    CPPUNIT_ASSERT_MESSAGE( "factory create failed ", factory != 0 );

}

template< typename PT, typename OP, typename IP >
void Test_Factory_Base< PT, OP, IP >::test_transport_type()
{

    CPPUNIT_ASSERT_MESSAGE( "factory create failed ", factory != 0 );

    std::string type=factory->transportType();
    std::string e("custom");

    CPPUNIT_ASSERT_EQUAL( e, type );
}

template< typename PT, typename OP, typename IP >
void Test_Factory_Base< PT, OP, IP >::test_default_priority()
{
   CPPUNIT_ASSERT_MESSAGE( "factory create failed ", factory != 0 );

    int prior=factory->defaultPriority();
    int e=0;
    CPPUNIT_ASSERT_EQUAL( e, prior );

    setUp_transport_priority("custom", "3");
    prior=factory->defaultPriority();
    e=3;
    CPPUNIT_ASSERT_EQUAL( e, prior );
}

template< typename PT, typename OP, typename IP >
void Test_Factory_Base< PT, OP, IP >::test_create_input_manager()
{

    CPPUNIT_ASSERT_MESSAGE( "factory create failed ", factory != 0 );

    IP *port= new IP("data" + getPortName() + "_in");
    bulkio::InputManager< PT > *mgr=factory->createInputManager( port );
    CPPUNIT_ASSERT_MESSAGE( "input manager create failed ", mgr != 0 );
    delete mgr;
    delete port;

}

template< typename PT, typename OP, typename IP >
void Test_Factory_Base< PT, OP, IP >::test_create_output_manager()
{

  OP *port= new OP("data" + getPortName() + "_out");
  bulkio::OutputManager< PT > *mgr=factory->createOutputManager( port );
    CPPUNIT_ASSERT_MESSAGE( "output manager create failed ", mgr != 0 );
    delete mgr;
    delete port;

}

#define CREATE_TEST(x,y)                                                 \
    class Custom_Factory_##x##_Test : public Test_Factory_Base< BULKIO::data##x , bulkio::Out##y##Port , bulkio::In##y##Port  > \
    {                                                                   \
        typedef Test_Factory_Base< BULKIO::data##x , bulkio::Out##y##Port , bulkio::In##y##Port > TestBase; \
        CPPUNIT_TEST_SUB_SUITE(Custom_Factory_##x##_Test, TestBase);               \
        CPPUNIT_TEST_SUITE_END();                                       \
        virtual std::string getPortName() const { return #y; };         \
    };                                                                  \
    CPPUNIT_TEST_SUITE_REGISTRATION(Custom_Factory_##x##_Test);

CREATE_TEST(Float,Float);
