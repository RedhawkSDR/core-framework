#ifndef PROPS_IMPL_H
#define PROPS_IMPL_H

#include "props_base.h"

class props_i;

class props_i : public props_base
{
    ENABLE_LOGGING
    public:
        props_i(const char *uuid, const char *label);
        ~props_i();
        int serviceFunction();

        void ushortSimpleChanged (const std::string&);
        void charSimpleChanged (const std::string&);
        void octetSimpleChanged (const std::string&);
        void floatSimpleChanged (const std::string&);
        void shortSimpleChanged (const std::string&);
        void ulongSimpleChanged (const std::string&);
        void boolSimpleChanged (const std::string&);
        void stringSimpleChanged (const std::string&);
        void doubleSimpleChanged (const std::string&);
        void longSimpleChanged (const std::string&);
        void longlongSimpleChanged (const std::string&);
        void ulonglongSimpleChanged (const std::string&);
        void boolSeqChanged (const std::string&);
        void stringSeqChanged (const std::string&);
        void ulongSeqChanged (const std::string&);
        void shortSeqChanged (const std::string&);
        void floatSeqChanged (const std::string&);
        void octetSeqChanged (const std::string&);
        void charSeqChanged (const std::string&);
        void ushortSeqChanged (const std::string&);
        void simpleRangeChanged (const std::string&);
        void structSeqPropChanged (const std::string&);
        void structPropChanged (const std::string&);
        void ulonglongSeqChanged (const std::string&);
        void longlongSeqChanged (const std::string&);
        void longSeqChanged (const std::string&);
        void doubleSeqChanged (const std::string&);
};

#endif
