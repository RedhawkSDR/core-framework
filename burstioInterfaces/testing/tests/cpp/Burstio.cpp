/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK burstioInterfaces.
 *
 * REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include <iostream>
#include <getopt.h>
#include <stdlib.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/XmlOutputter.h>
#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"

int main(int argc, char* argv[])
{
    // Locate the logging configuration file relative to the source directory
    std::string log_config = "log4j.props";
    char* srcdir = getenv("srcdir");
    if (srcdir) {
        log_config = std::string(srcdir) + "/" + log_config;
    }

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

    
    log4cxx::PropertyConfigurator::configure(log_config);

    // Apply the log level (can override config file).
    log4cxx::LevelPtr level = log4cxx::Level::toLevel(log_level, log4cxx::Level::getInfo());
    log4cxx::Logger::getRootLogger()->setLevel(level);

    // Get the top level suite from the registry
    CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

    // Create the event manager and test controller
    CppUnit::TestResult controller;
    // Add a listener that collects test result
    CppUnit::TestResultCollector result;
    controller.addListener ( &result );
    CppUnit::TextUi::TestRunner *runner = new CppUnit::TextUi::TestRunner;

    std::string ofile= "../cppunit-results.xml";
    if (xunit_file) {
        ofile=xunit_file;
    }

    std::ofstream xmlout (ofile.c_str());
    CppUnit::XmlOutputter xmlOutputter ( &result, xmlout );
    CppUnit::CompilerOutputter compilerOutputter ( &result, std::cerr );

    // Run the tests.
    runner->addTest( suite );
    runner->run( controller );
    xmlOutputter.write();
    compilerOutputter.write();

    // Return error code 1 if the one of test failed.
    return result.wasSuccessful() ? 0 : 1;
}
