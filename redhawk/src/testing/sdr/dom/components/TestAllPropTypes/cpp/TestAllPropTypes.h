/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file 
 * distributed with this source distribution.
 * 
 * This file is part of REDHAWK core.
 * 
 * REDHAWK core is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version.
 * 
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */


#ifndef TESTALLPROPTYPES_IMPL_H
#define TESTALLPROPTYPES_IMPL_H

#include "TestAllPropTypes_base.h"

class TestAllPropTypes_i;

class TestAllPropTypes_i : public TestAllPropTypes_base
{
    ENABLE_LOGGING
    public: 
        TestAllPropTypes_i(const char *uuid, const char *label);
        ~TestAllPropTypes_i();
        int serviceFunction();
        // Call back functions
        void simpleStringChanged(const std::string&);
        void simpleBooleanChanged(const std::string&);
        void simpleULongChanged(const std::string&);
        void simpleObjRefChanged(const std::string&);
        void simpleShortChanged(const std::string&);
        void simpleFloatChanged(const std::string&);
        void simpleOctetChanged(const std::string&);
        void simpleCharChanged(const std::string&);
        void simpleUShortChanged(const std::string&);
        void simpleDoubleChanged(const std::string&);
        void simpleLongChanged(const std::string&);
        void simpleLongLongChanged(const std::string&);
        void simpleULongLongChanged(const std::string&);
        void seqStringChanged(const std::string&);
        void seqBooleanChanged(const std::string&);
        void seqULongChanged(const std::string&);
        void seqObjRefChanged(const std::string&);
        void seqShortChanged(const std::string&);
        void seqFloatChanged(const std::string&);
        void seqOctetChanged(const std::string&);
        void seqCharChanged(const std::string&);
        void seqUShortChanged(const std::string&);
        void seqDoubleChanged(const std::string&);
        void seqLongChanged(const std::string&);
        void seqLongLongChanged(const std::string&);
        void seqULongLongChanged(const std::string&);
        void structChanged(const std::string&);
        void structSeqChanged(const std::string&);
};

#endif
