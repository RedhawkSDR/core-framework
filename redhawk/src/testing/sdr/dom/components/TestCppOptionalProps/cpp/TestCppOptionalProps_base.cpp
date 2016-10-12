#include "TestCppOptionalProps_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

TestCppOptionalProps_base::TestCppOptionalProps_base(const char *uuid, const char *label) :
    Component(uuid, label),
    ThreadedComponent()
{
    loadProperties();
}

TestCppOptionalProps_base::~TestCppOptionalProps_base()
{
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void TestCppOptionalProps_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Component::start();
    ThreadedComponent::startThread();
}

void TestCppOptionalProps_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Component::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void TestCppOptionalProps_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    Component::releaseObject();
}

void TestCppOptionalProps_base::loadProperties()
{
    addProperty(my_struct_name,
                my_struct_name_struct(),
                "my_struct",
                "my_struct_name",
                "readwrite",
                "",
                "external",
                "configure");

}

void TestCppOptionalProps_base::runTest (CORBA::ULong TestID, CF::Properties& testValues)
    throw (CF::UnknownProperties, CF::TestableObject::UnknownTest, CORBA::SystemException)
{
	if (TestID == 0) {
		this->testOptional(testValues);
	}
}

void TestCppOptionalProps_base::testOptional(CF::Properties& testValues)
{
	redhawk::PropertyMap props(testValues);
	printStatus("struct_octet_name", my_struct_name.struct_octet_name.isSet());
	printStatus("struct_short_name", my_struct_name.struct_short_name.isSet());
	printStatus("struct_ushort_name", my_struct_name.struct_ushort_name.isSet());
	printStatus("struct_long_name", my_struct_name.struct_long_name.isSet());
	printStatus("struct_ulong_name", my_struct_name.struct_ulong_name.isSet());
	printStatus("struct_longlong_name", my_struct_name.struct_longlong_name.isSet());
	printStatus("struct_ulonglong_name", my_struct_name.struct_ulonglong_name.isSet());
	printStatus("struct_string_name", my_struct_name.struct_string_name.isSet());
	printStatus("struct_seq_octet_name", my_struct_name.struct_seq_octet_name.isSet());
	printStatus("struct_seq_short_name", my_struct_name.struct_seq_short_name.isSet());
	printStatus("struct_seq_ushort_name", my_struct_name.struct_seq_ushort_name.isSet());
	printStatus("struct_seq_long_name", my_struct_name.struct_seq_long_name.isSet());
	printStatus("struct_seq_ulong_name", my_struct_name.struct_seq_ulong_name.isSet());
	printStatus("struct_seq_longlong_name", my_struct_name.struct_seq_longlong_name.isSet());
	printStatus("struct_seq_ulonglong_name", my_struct_name.struct_seq_ulonglong_name.isSet());
}


