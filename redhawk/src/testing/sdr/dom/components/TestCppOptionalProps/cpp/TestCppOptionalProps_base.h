#ifndef TESTCPPOPTIONALPROPS_IMPL_BASE_H
#define TESTCPPOPTIONALPROPS_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include "struct_props.h"

class TestCppOptionalProps_base : public Component, protected ThreadedComponent
{
    public:
        TestCppOptionalProps_base(const char *uuid, const char *label);
        ~TestCppOptionalProps_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

        void runTest (CORBA::ULong TestID, CF::Properties& testValues)
        	throw (CF::UnknownProperties, CF::TestableObject::UnknownTest, CORBA::SystemException);

    protected:
        // Member variables exposed as properties
        my_struct_name_struct my_struct_name;

    private:
        void testOptional(CF::Properties& testValues);

        void printStatus(const char* name, bool status) {
        	if (status)
        		std::cout << name << " is set." << std::endl;
        	else
        		std::cout << name << " is not set." << std::endl;
        }

};
#endif // TESTCPPOPTIONALPROPS_IMPL_BASE_H
