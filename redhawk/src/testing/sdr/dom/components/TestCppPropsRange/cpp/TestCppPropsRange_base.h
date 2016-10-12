#ifndef TESTCPPPROPSRANGE_IMPL_BASE_H
#define TESTCPPPROPSRANGE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include "struct_props.h"

class TestCppPropsRange_base : public Component, protected ThreadedComponent
{
    public:
        TestCppPropsRange_base(const char *uuid, const char *label);
        ~TestCppPropsRange_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        unsigned char my_octet_name;
        short my_short_name;
        unsigned short my_ushort_name;
        CORBA::Long my_long_name;
        CORBA::ULong my_ulong_name;
        CORBA::LongLong my_longlong_name;
        CORBA::ULongLong my_ulonglong_name;
        std::vector<unsigned char> seq_octet_name;
        std::vector<short> seq_short_name;
        std::vector<unsigned short> seq_ushort_name;
        std::vector<CORBA::Long> seq_long_name;
        std::vector<CORBA::ULong> seq_ulong_name;
        std::vector<CORBA::LongLong> seq_longlong_name;
        std::vector<CORBA::ULongLong> seq_ulonglong_name;
        std::vector<char> seq_char_name;
        my_struct_name_struct my_struct_name;
        std::vector<ss_struct_name_struct> my_structseq_name;

    private:
};
#endif // TESTCPPPROPSRANGE_IMPL_BASE_H
