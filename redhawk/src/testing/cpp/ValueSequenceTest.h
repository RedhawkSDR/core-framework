#ifndef VALUESEQUENCETEST_H
#define VALUESEQUENCETEST_H

#include <cppunit/extensions/HelperMacros.h>

class ValueSequenceTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(ValueSequenceTest);
    CPPUNIT_TEST(testDefaultConstructor);
    CPPUNIT_TEST(testAnySeqConstructor);
    CPPUNIT_TEST(testConstCast);
    CPPUNIT_TEST(testCast);
    CPPUNIT_TEST(testPushBack);
    CPPUNIT_TEST(testConstIndexing);
    CPPUNIT_TEST(testIndexing);
    CPPUNIT_TEST(testConstIteration);
    CPPUNIT_TEST(testMutableIteration);
    CPPUNIT_TEST(testFromConstValue);
    CPPUNIT_TEST(testFromMutableValue);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testDefaultConstructor();
    void testAnySeqConstructor();

    void testConstCast();
    void testCast();

    void testPushBack();

    void testConstIndexing();
    void testIndexing();

    void testConstIteration();
    void testMutableIteration();

    void testFromConstValue();
    void testFromMutableValue();
};

#endif // VALUESEQUENCETEST_H
