#ifndef VALUETEST_H
#define VALUETEST_H

#include <cppunit/extensions/HelperMacros.h>

class ValueTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(ValueTest);
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testType);
    CPPUNIT_TEST(testNumericConversion);
    CPPUNIT_TEST(testStringConversion);
    CPPUNIT_TEST(testCast);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testConstructor();

    void testType();

    void testNumericConversion();
    void testStringConversion();

    void testCast();
};

#endif // VALUETEST_H
