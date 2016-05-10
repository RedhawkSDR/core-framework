#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "log4cxx/logger.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"
#include "logtestdebug.h"


LOGGER_CFG("LOGGER-CFG-TEST");

int main(int argc, char* argv[])
{

  std::string cfgname("log4j.stdout");
  std::string testname("");
  if ( argc > 1 ) {
    testname = argv[1];
  }

  // Set up a simple configuration that logs on the console.
  log4cxx::PropertyConfigurator::configure(cfgname.c_str());

  // Get the top level suite from the registry
  CppUnit::Test *suite;
  if ( testname != "" ) {
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry(testname);
    suite = registry.makeTest();
  }
  else {
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    registry.registerFactory( &CppUnit::TestFactoryRegistry::getRegistry("test_three") );
    suite = registry.makeTest();
  }

  // Adds the test to the list of test to run
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( suite );

  // Change the default outputter to a compiler error format outputter
  runner.setOutputter( new CppUnit::CompilerOutputter( &runner.result(),
                                                       std::cerr ) );
  // Run the tests.
  bool wasSucessful = runner.run();

  LOGGER_END("LOGGER-CFG-TEST");

  // Return error code 1 if the one of test failed.
  return wasSucessful ? 0 : 1;
}
