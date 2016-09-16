#ifndef SHAREDBUFFERTEST_H
#define SHAREDBUFFERTEST_H

#include <cppunit/extensions/HelperMacros.h>

class SharedBufferTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(SharedBufferTest);
    CPPUNIT_TEST(testDefaultConstructor);
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testEquals);
    CPPUNIT_TEST(testSharing);
    CPPUNIT_TEST(testSlicing);
    CPPUNIT_TEST(testTrim);
    CPPUNIT_TEST(testRecast);
    CPPUNIT_TEST(testAllocator);
    CPPUNIT_TEST(testCustomDeleter);
    CPPUNIT_TEST(testTransient);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testDefaultConstructor();
    void testConstructor();

    void testEquals();

    void testSharing();
    void testSlicing();
    void testTrim();
    void testRecast();

    void testAllocator();
    void testCustomDeleter();
    void testTransient();
};

#endif // SHARED_BUFFER_TEST_H
