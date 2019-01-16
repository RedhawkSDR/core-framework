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
#include <iostream>

#include <getopt.h>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TextTestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestPath.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/XmlOutputter.h>

#include <ossie/CorbaUtils.h>

// log4cxx includes need to follow CorbaUtils, otherwise "ossie/debug.h" will
// issue warnings about the logging macros
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>

int main(int argc, char* argv[])
{
    const char* short_options = "v";
    struct option long_options[] = {
        { "with-xunit", required_argument, 0, 'x' },
        { "log-level",  required_argument, 0, 'l' },
        { "log-config", required_argument, 0, 'c' },
        { "verbose",    no_argument,       0, 'v' },
        { 0, 0, 0, 0 }
    };

    bool verbose = false;
    const char* xunit_file = 0;
    const char* log_config = 0;
    std::string log_level;
    int status;
    while ((status = getopt_long(argc, argv, short_options, long_options, NULL)) >= 0) {
        switch (status) {
        case '?': // Invalid option
            return -1;
        case 'x':
            xunit_file = optarg;
            break;
        case 'l':
            log_level = optarg;
            break;
        case 'c':
            log_config = optarg;
            break;
        case 'v':
            verbose = true;
            break;
        }
    }

    // Many tests require CORBA, and possibly the REDHAWK ORB singleton, so
    // initialize up front.
    ossie::corba::CorbaInit(0,0);

    // If a log4j configuration file was given, read it.
    if (log_config) {
        log4cxx::PropertyConfigurator::configure(log_config);
    } else {
        // Set up a simple configuration that logs on the console.
        log4cxx::BasicConfigurator::configure();
    }

    // Apply the log level (can override config file).
    log4cxx::LevelPtr level = log4cxx::Level::toLevel(log_level, log4cxx::Level::getInfo());
    log4cxx::Logger::getRootLogger()->setLevel(level);

    // Create the test runner.
    CppUnit::TextTestRunner runner;

    // Enable verbose output, displaying the name of each test as it runs.
    if (verbose) {
        runner.eventManager().addListener(new CppUnit::BriefTestProgressListener());
    }

    // Use a compiler outputter instead of the default text one.
    runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr));

    // Get the top level suite from the registry.
    CppUnit::Test* suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
    runner.addTest(suite);

    // If an argument was given, assume it was the name of a test or suite.
    std::string test_path;
    if (optind < argc) {
        test_path = argv[optind];
    }

    // Run the tests: don't pause, write output, don't print progress in
    // verbose mode (which seems ironic, but the test progress listener will
    // print each test name)
    bool success = runner.run(test_path, false, true, !verbose);

    // Write XML file, if requested.
    if (xunit_file) {
        std::ofstream file(xunit_file);
        CppUnit::XmlOutputter xml_outputter(&runner.result(), file);
        xml_outputter.write();
    }

    // Return error code 1 if the one of test failed.
    return success ? 0 : 1;
}
