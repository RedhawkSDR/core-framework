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
using namespace std;


int main(int argc, char* argv[])
{

  // Set up a simple configuration that logs on the console.
  log4cxx::PropertyConfigurator::configure("log4j.props");

  // Get the top level suite from the registry
  CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  // Create the event manager and test controller
  CppUnit::TestResult controller;
  // Add a listener that collects test result
  CppUnit::TestResultCollector result;
  controller.addListener ( &result );
  CppUnit::TextUi::TestRunner *runner = new CppUnit::TextUi::TestRunner;

  ofstream xmlout ( "../cppunit-results.xml" );
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
