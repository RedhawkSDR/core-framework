/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef MESSAGINGTEST_H
#define MESSAGINGTEST_H

#include "CFTest.h"

#include <ossie/MessageInterface.h>

#include "PortManager.h"

class MessagingTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(MessagingTest);
    CPPUNIT_TEST(testConnections);
    CPPUNIT_TEST(testSendMessage);
    CPPUNIT_TEST(testSendMessageDirect);
    CPPUNIT_TEST(testSendMessageConnectionId);
    CPPUNIT_TEST(testSendMessages);
    CPPUNIT_TEST(testSendMessagesDirect);
    CPPUNIT_TEST(testSendMessagesConnectionId);
    CPPUNIT_TEST(testGenericCallback);
    CPPUNIT_TEST(testPush);
    CPPUNIT_TEST(testPushConnectionId);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testConnections();

    void testSendMessage();
    void testSendMessageDirect();
    void testSendMessageConnectionId();

    void testSendMessages();
    void testSendMessagesDirect();
    void testSendMessagesConnectionId();

    void testGenericCallback();

    void testPush();
    void testPushConnectionId();

private:
    PortManager _portManager;

    MessageSupplierPort* _supplier;
    MessageConsumerPort* _consumer;
};

#endif // MESSAGINGTEST_H
