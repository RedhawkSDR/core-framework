#ifndef TESTCOMPLEXPROPS_BASE_IMPL_BASE_H
#define TESTCOMPLEXPROPS_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include "struct_props.h"

class TestComplexProps_base : public Component, protected ThreadedComponent
{
    public:
        TestComplexProps_base(const char *uuid, const char *label);
        ~TestComplexProps_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Property: complexBooleanProp
        std::complex<bool> complexBooleanProp;
        /// Property: complexULongProp
        std::complex<CORBA::ULong> complexULongProp;
        /// Property: complexShortProp
        std::complex<short> complexShortProp;
        /// Property: complexFloatProp
        std::complex<float> complexFloatProp;
        /// Property: complexOctetProp
        std::complex<unsigned char> complexOctetProp;
        /// Property: complexUShort
        std::complex<unsigned short> complexUShort;
        /// Property: complexDouble
        std::complex<double> complexDouble;
        /// Property: complexLong
        std::complex<CORBA::Long> complexLong;
        /// Property: complexLongLong
        std::complex<CORBA::LongLong> complexLongLong;
        /// Property: complexULongLong
        std::complex<CORBA::ULongLong> complexULongLong;
        /// Property: complexFloatSequence
        std::vector<std::complex<float> > complexFloatSequence;
        /// Property: FloatStruct
        FloatStruct_struct FloatStruct;
        /// Property: complexFloatStruct
        complexFloatStruct_struct complexFloatStruct;
        /// Property: FloatStructSequence
        std::vector<FloatStructSequenceMember_struct> FloatStructSequence;
        /// Property: complexFloatStructSequence
        std::vector<complexFloatStructSequenceMember_struct> complexFloatStructSequence;

    private:
};
#endif // TESTCOMPLEXPROPS_BASE_IMPL_BASE_H
