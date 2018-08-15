#include "TestComplexProps_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

TestComplexProps_base::TestComplexProps_base(const char *uuid, const char *label) :
    Component(uuid, label),
    ThreadedComponent()
{
    loadProperties();
}

TestComplexProps_base::~TestComplexProps_base()
{
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void TestComplexProps_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Component::start();
    ThreadedComponent::startThread();
}

void TestComplexProps_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Component::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void TestComplexProps_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    Component::releaseObject();
}

void TestComplexProps_base::loadProperties()
{
    addProperty(complexBooleanProp,
                std::complex<bool>(false,true),
                "complexBooleanProp",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(complexULongProp,
                std::complex<CORBA::ULong>(4,5),
                "complexULongProp",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(complexShortProp,
                std::complex<short>(4,5),
                "complexShortProp",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(complexFloatProp,
                std::complex<float>(4.0,5.0),
                "complexFloatProp",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(complexOctetProp,
                std::complex<unsigned char>(4,5),
                "complexOctetProp",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(complexUShort,
                std::complex<unsigned short>(4,5),
                "complexUShort",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(complexDouble,
                std::complex<double>(4.0,5.0),
                "complexDouble",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(complexLong,
                std::complex<CORBA::Long>(4,5),
                "complexLong",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(complexLongLong,
                std::complex<CORBA::LongLong>(4LL,5LL),
                "complexLongLong",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(complexULongLong,
                std::complex<CORBA::ULongLong>(4LL,5LL),
                "complexULongLong",
                "",
                "readwrite",
                "",
                "external",
                "property");

    // Set the sequence with its initial values
    complexFloatSequence.push_back(std::complex<float>(6.0,7.0));
    complexFloatSequence.push_back(std::complex<float>(4.0,5.0));
    complexFloatSequence.push_back(std::complex<float>(4.0,5.0));
    addProperty(complexFloatSequence,
                complexFloatSequence,
                "complexFloatSequence",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(FloatStruct,
                FloatStruct_struct(),
                "FloatStruct",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(complexFloatStruct,
                complexFloatStruct_struct(),
                "complexFloatStruct",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(FloatStructSequence,
                "FloatStructSequence",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(complexFloatStructSequence,
                complexFloatStructSequence,
                "complexFloatStructSequence",
                "",
                "readwrite",
                "",
                "external",
                "property");

    {
        complexFloatStructSequenceMember_struct __tmp;
        __tmp.complex_float_seq.push_back(std::complex<float>(9.0,4.0));
        __tmp.complexFloatStructSequenceMemberMemember = std::complex<float>(6.0,5.0);
        complexFloatStructSequence.push_back(__tmp);
    }

}


