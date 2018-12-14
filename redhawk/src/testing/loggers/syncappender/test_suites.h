#ifndef TEST_SUITES_H
#define TEST_SUITES_H
#include <cppunit/extensions/HelperMacros.h>
#include <boost/interprocess/shared_memory_object.hpp>
#include<log4cxx/logger.h>

class test_suite_one : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( test_suite_one );
  CPPUNIT_TEST( test_one );
  CPPUNIT_TEST( test_two );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void test_one();
  void test_two();

  log4cxx::LoggerPtr logger;
};


class test_suite_two : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( test_suite_two );
  CPPUNIT_TEST( test_loop );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void test_loop();

  log4cxx::LoggerPtr logger;

};


class test_suite_three : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( test_suite_three );
  CPPUNIT_TEST( test_cleanmem );
  CPPUNIT_TEST_EXCEPTION( test_cleanmem_missing, boost::interprocess::interprocess_exception );
  CPPUNIT_TEST( test_cleanmem_path );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void test_cleanmem();
  void test_cleanmem_path();
  void test_cleanmem_missing();

  log4cxx::LoggerPtr logger;

};


#endif  // TEST_ONE
