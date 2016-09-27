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

#include "ExecutorServiceTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ExecutorServiceTest);

namespace {

    class CommandTracker
    {
    public:
        CommandTracker()
        {
        }

        void run()
        {
            runCommand(std::string());
        }

        void runCommand(const std::string& command)
        {
            boost::mutex::scoped_lock lock(_mutex);
            _commands.push_back(command);
            _cond.notify_all();
        }

        int count() const
        {
            return _commands.size();
        }

        bool wait(size_t count, boost::system_time when)
        {
            boost::mutex::scoped_lock lock(_mutex);
            while (_commands.size() < count) {
                if (!_cond.timed_wait(lock, when)) {
                    break;
                }
            }
            return _commands.size() >= count;
        }

        void reset()
        {
            _commands.clear();
        }

        static CommandTracker& Global()
        {
            static CommandTracker tracker;
            return tracker;
        }

        static void runGlobal()
        {
            Global().run();
        }

    private:
        boost::mutex _mutex;
        boost::condition_variable _cond;

        std::vector<std::string> _commands;
    };
}

void ExecutorServiceTest::setUp()
{
    CommandTracker::Global().reset();
}

void ExecutorServiceTest::tearDown()
{
    _service.stop();
}

void ExecutorServiceTest::testExecute()
{
    _service.start();

    // No-argument (global) function
    CPPUNIT_ASSERT_EQUAL(0, CommandTracker::Global().count());
    _service.execute(&CommandTracker::runGlobal);

    CommandTracker tracker;
    CPPUNIT_ASSERT_EQUAL(0, tracker.count());

    // One-argument (member) function
    _service.execute(&CommandTracker::run, &tracker);

    // Two-argument (member) function
    _service.execute(&CommandTracker::runCommand, &tracker, "testExecute");

    // Wait up to 1000us for the commands to be executed
    boost::system_time timeout = boost::get_system_time() + boost::posix_time::microseconds(1000);
    CPPUNIT_ASSERT(CommandTracker::Global().wait(1, timeout));
    CPPUNIT_ASSERT_EQUAL(1, CommandTracker::Global().count());

    CPPUNIT_ASSERT(tracker.wait(2, timeout));
    CPPUNIT_ASSERT_EQUAL(2, tracker.count());
}

void ExecutorServiceTest::testSchedule()
{
    _service.start();

    // No-argument (global) function
    CPPUNIT_ASSERT_EQUAL(0, CommandTracker::Global().count());
    boost::system_time first = boost::get_system_time() + boost::posix_time::microseconds(1000);
    _service.schedule(first, &CommandTracker::runGlobal);

    CommandTracker tracker;
    CPPUNIT_ASSERT_EQUAL(0, tracker.count());

    // One-argument (member) function
    boost::system_time second = first + boost::posix_time::microseconds(1000);
    _service.schedule(second, &CommandTracker::run, &tracker);

    // Two-argument (member) function
    boost::system_time third = second + boost::posix_time::microseconds(1000);
    _service.schedule(third, &CommandTracker::runCommand, &tracker, "testSchedule");

    // Use maximum wait of 10000us (from now) for the commands to be executed
    // (it should take approximately 3000us, but allow some slack in the event
    // of scheduler delays)
    boost::system_time timeout = boost::get_system_time() + boost::posix_time::microseconds(10000);

    // Wait for the first command, and check enough time has passed
    CPPUNIT_ASSERT(CommandTracker::Global().wait(1, timeout));
    CPPUNIT_ASSERT_EQUAL(1, CommandTracker::Global().count());
    CPPUNIT_ASSERT(boost::get_system_time() >= first);

    // Wait for the second and third commands, again checking that enough time
    // has passed
    CPPUNIT_ASSERT(tracker.wait(1, timeout));
    CPPUNIT_ASSERT(boost::get_system_time() >= second);
    CPPUNIT_ASSERT(tracker.wait(2, timeout));
    CPPUNIT_ASSERT(boost::get_system_time() >= third);
    CPPUNIT_ASSERT_EQUAL(2, tracker.count());
}

void ExecutorServiceTest::testClear()
{
    CommandTracker tracker;

    _service.execute(&CommandTracker::run, &tracker);

    boost::system_time when = boost::get_system_time() + boost::posix_time::seconds(1);
    _service.schedule(when, &CommandTracker::run, &tracker);

    _service.clear();
    _service.start();

    // TODO: Better determination of pending
    usleep(1000);

    CPPUNIT_ASSERT_EQUAL(0, tracker.count());
}
