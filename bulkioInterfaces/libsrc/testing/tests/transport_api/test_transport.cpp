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
#include <boost/regex.hpp>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/XmlOutputter.h>
#include "log4cxx/logger.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"
using namespace std;


int main(int argc, char* argv[])
{

  int c;
  int verbose_flag=0;
  std::string   prefix;
  std::string   subtest;
  std::string cfgname("log4j.properties");

  static struct option long_options[] =
    {
      /* These options set a flag. */
      { "with-xunit", required_argument, 0, 'x' },
      {"verbose", no_argument,       &verbose_flag, 1 },
      {"brief",   no_argument,       &verbose_flag, 0 },
      {"suite",  required_argument, 0, 's'},
      {"tests",    required_argument, 0, 't'},
      {"logcfg",  required_argument, 0, 'l'},
      {0, 0, 0, 0}
    };
  /* getopt_long stores the option index here. */
  int option_index = 0;
  const char *xunit_file=0;

  while (( c = getopt_long (argc, argv, "s:l:t:vbhx:", long_options, &option_index)) != -1 ) {
    switch (c) {
    case 0:
      /* If this option set a flag, do nothing else now. */
      if (long_options[option_index].flag != 0)
	break;
      printf ("option %s", long_options[option_index].name);
      if (optarg)
	printf (" with arg %s", optarg);
      printf ("\n");
      break;

    case 's':
      printf ("option -s with value `%s'\n", optarg);
      prefix=optarg;
      break;

    case 't':
      printf ("option -t with value `%s'\n", optarg);
      subtest=optarg;
      break;
    case 'l':
      printf ("option -l with value `%s'\n", optarg);
      cfgname=optarg;
      break;
      
    case 'x':
        xunit_file = optarg;
        break;
      

    case '?':
    default:
      std::cout << "usage: test_transport " << std::endl;
      std::cout << "        option   --suite  <test suite selection>   - search for test suites with name (regx)" << std::endl;
      std::cout << "        option   --tests   <test method selection>  - search for test methods within a suite (regx)" << std::endl;
      std::cout << "        option   --logcfg <logging properties>     - logging config file" << std::endl;
      std::cout << "        option   --verbose  - turns on debug message" << std::endl;
      std::cout << "        option   --brief  - turns off debug message" << std::endl;
      std::cout << "        option   --with-xunit - save xunit xml result file" << std::endl;
      exit(-1);
    }
  }


  // Set up a simple configuration that logs on the console.
  log4cxx::PropertyConfigurator::configure(cfgname.c_str());

  // Get the top level suite from the registry
  CppUnit::Test *all_suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
  CppUnit::Test *suite = all_suite;
  if ( prefix != "" ) {
    CppUnit::TestSuite *filter_suite=new CppUnit::TestSuite();
    boost::regex expr(prefix);
    boost::regex subt_expr(subtest);
    int tcnt=all_suite->getChildTestCount();
    if ( verbose_flag )   std::cout << "child cnt = " << tcnt << std::endl;
    for (int i=0; i< tcnt; i++ ) {
      CppUnit::Test *t = all_suite->getChildTestAt(i);
      std::string tname = t->getName();
      if (verbose_flag ) std::cout << "reg = " << prefix << " test ... " << tname << std::endl;
      if ( boost::regex_search( tname,  expr )  ) {
        if ( subtest != "" ) {
            int tcnt=t->getChildTestCount();
            for (int i=0; i< tcnt; i++ ) {
                CppUnit::Test *subt = t->getChildTestAt(i);
                std::string sub_tname = subt->getName();
                if (verbose_flag ) std::cout << "(sub) reg = " << subtest << " test ... " << sub_tname << std::endl;
                if ( boost::regex_search( sub_tname,  subt_expr )  ) {
                    if (verbose_flag ) std::cout << "Adding via method : test ... " << sub_tname << std::endl;
                    filter_suite->addTest(subt);
                }
            }

        }
        else {
            if (verbose_flag ) std::cout << "Adding test ... " << tname << std::endl;
            filter_suite->addTest(t);
        }
      }
    }

    suite=filter_suite;
  }

  // Create the event manager and test controller
  CppUnit::TestResult controller;
  // Add a listener that collects test result
  CppUnit::TestResultCollector result;
  controller.addListener ( &result );
  CppUnit::TextUi::TestRunner *runner = new CppUnit::TextUi::TestRunner;

  std::string ofile="../transportapi-results.xml";
  if (xunit_file) {
      // Write XML output file to specified file
      ofile=xunit_file;
  }
  
  ofstream xmlout ( ofile.c_str() );
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
