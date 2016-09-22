#ifndef PROPERTYMAPTEST_H
#define PROPERTYMAPTEST_H

#include <cppunit/extensions/HelperMacros.h>

class PropertyMapTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(PropertyMapTest);
    CPPUNIT_TEST(testDefaultConstructor);
    CPPUNIT_TEST(testPropertiesConstructor);
    CPPUNIT_TEST(testConstCast);
    CPPUNIT_TEST(testCast);
    CPPUNIT_TEST(testPushBack);
    CPPUNIT_TEST(testConstIndexing);
    CPPUNIT_TEST(testMutableIndexing);
    CPPUNIT_TEST(testConstMapping);
    CPPUNIT_TEST(testMutableMapping);
    CPPUNIT_TEST(testConstFind);
    CPPUNIT_TEST(testMutableFind);
    CPPUNIT_TEST(testConstIteration);
    CPPUNIT_TEST(testMutableIteration);
    CPPUNIT_TEST(testUpdate);
    CPPUNIT_TEST(testErase);
    CPPUNIT_TEST(testGet);
    CPPUNIT_TEST(testToString);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testDefaultConstructor();
    void testPropertiesConstructor();

    void testConstCast();
    void testCast();

    void testPushBack();

    void testContains();

    void testConstIndexing();
    void testMutableIndexing();

    void testConstMapping();
    void testMutableMapping();

    void testConstFind();
    void testMutableFind();

    void testConstIteration();
    void testMutableIteration();

    void testUpdate();
    void testErase();

    void testGet();

    void testToString();
};

#endif // PROPERTYMAPTEST_H
