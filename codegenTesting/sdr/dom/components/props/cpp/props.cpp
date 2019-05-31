/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK codegenTesting.
 *
 * REDHAWK codegenTesting is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK codegenTesting is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "props.h"

PREPARE_LOGGING(props_i)

props_i::props_i(const char *uuid, const char *label) :
    props_base(uuid, label)
{
    old_bool = false;
    setPropertyChangeListener("stringSimple", this, &props_i::stringSimpleChanged);
    setPropertyChangeListener("boolSimple", this, &props_i::boolSimpleChanged);
    setPropertyChangeListener("ulongSimple", this, &props_i::ulongSimpleChanged);
    setPropertyChangeListener("shortSimple", this, &props_i::shortSimpleChanged);
    setPropertyChangeListener("floatSimple", this, &props_i::floatSimpleChanged);
    setPropertyChangeListener("octetSimple", this, &props_i::octetSimpleChanged);
    setPropertyChangeListener("charSimple", this, &props_i::charSimpleChanged);
    setPropertyChangeListener("ushortSimple", this, &props_i::ushortSimpleChanged);
    setPropertyChangeListener("doubleSimple", this, &props_i::doubleSimpleChanged);
    setPropertyChangeListener("longSimple", this, &props_i::longSimpleChanged);
    setPropertyChangeListener("longlongSimple", this, &props_i::longlongSimpleChanged);
    setPropertyChangeListener("ulonglongSimple", this, &props_i::ulonglongSimpleChanged);
    setPropertyChangeListener("stringSeq", this, &props_i::stringSeqChanged);
    setPropertyChangeListener("boolSeq", this, &props_i::boolSeqChanged);
    setPropertyChangeListener("ulongSeq", this, &props_i::ulongSeqChanged);
    setPropertyChangeListener("shortSeq", this, &props_i::shortSeqChanged);
    setPropertyChangeListener("floatSeq", this, &props_i::floatSeqChanged);
    setPropertyChangeListener("octetSeq", this, &props_i::octetSeqChanged);
    setPropertyChangeListener("charSeq", this, &props_i::charSeqChanged);
    setPropertyChangeListener("ushortSeq", this, &props_i::ushortSeqChanged);
    setPropertyChangeListener("doubleSeq", this, &props_i::doubleSeqChanged);
    setPropertyChangeListener("longSeq", this, &props_i::longSeqChanged);
    setPropertyChangeListener("longlongSeq", this, &props_i::longlongSeqChanged);
    setPropertyChangeListener("ulonglongSeq", this, &props_i::ulonglongSeqChanged);
    setPropertyChangeListener("structProp", this, &props_i::structPropChanged);
    setPropertyChangeListener("structSeqProp", this, &props_i::structSeqPropChanged);
}

props_i::~props_i()
{
}

void props_i::stringSimpleChanged (const std::string& id)
{
    stringSimple = stringSimple + stringSimple;
}

void props_i::boolSimpleChanged (const std::string& id)
{
    if (!old_bool) {
        boolSimple = !boolSimple;
        old_bool = true;
    } else {
        old_bool = false;
    }
}

void props_i::ulongSimpleChanged (const std::string& id)
{
    ulongSimple = ulongSimple * 2;
}

void props_i::shortSimpleChanged (const std::string& id)
{
    shortSimple = shortSimple * 2;
}

void props_i::floatSimpleChanged (const std::string& id)
{
    floatSimple = floatSimple * 2;
}

void props_i::octetSimpleChanged (const std::string& id)
{
    octetSimple = octetSimple * 2;
}

void props_i::charSimpleChanged (const std::string& id)
{
    charSimple = std::toupper(charSimple);
}

void props_i::ushortSimpleChanged (const std::string& id)
{
    ushortSimple = ushortSimple * 2;
}

void props_i::doubleSimpleChanged (const std::string& id)
{
    doubleSimple = doubleSimple * 2;
}

void props_i::longSimpleChanged (const std::string& id)
{
    longSimple = longSimple * 2;
}

void props_i::longlongSimpleChanged (const std::string& id)
{
    longlongSimple = longlongSimple * 2;
}

void props_i::ulonglongSimpleChanged (const std::string& id)
{
    ulonglongSimple = ulonglongSimple * 2;
}

void props_i::stringSeqChanged (const std::string& id)
{
    std::vector<std::string> _stringSeq;

    for(int i = stringSeq.size()-1; i >= 0; i--){
        _stringSeq.push_back(stringSeq.at(i));
    }

    stringSeq.clear();

    for(unsigned int i = 0; i < _stringSeq.size(); i++){
        stringSeq.push_back(_stringSeq.at(i));
    }
}

void props_i::boolSeqChanged (const std::string& id)
{
    std::vector<bool> _boolSeq;

    for(int i = boolSeq.size()-1; i >= 0; i--){
        _boolSeq.push_back(boolSeq.at(i));
    }

    boolSeq.clear();

    for(unsigned int i = 0; i < _boolSeq.size(); i++){
        boolSeq.push_back(_boolSeq.at(i));
    }
}

void props_i::ulongSeqChanged (const std::string& id)
{
    std::vector<CORBA::ULong> _ulongSeq;

    for(int i = ulongSeq.size()-1; i >= 0; i--){
        _ulongSeq.push_back(ulongSeq.at(i));
    }

    ulongSeq.clear();

    for(unsigned int i = 0; i < _ulongSeq.size(); i++){
        ulongSeq.push_back(_ulongSeq.at(i));
    }
}

void props_i::shortSeqChanged (const std::string& id)
{
    std::vector<CORBA::Short> _shortSeq;

    for(int i = shortSeq.size()-1; i >= 0; i--){
        _shortSeq.push_back(shortSeq.at(i));
    }

    shortSeq.clear();

    for(unsigned int i = 0; i < _shortSeq.size(); i++){
        shortSeq.push_back(_shortSeq.at(i));
    }
}

void props_i::floatSeqChanged (const std::string& id)
{
    std::vector<CORBA::Float> _floatSeq;

    for(int i = floatSeq.size()-1; i >= 0; i--){
        _floatSeq.push_back(floatSeq.at(i));
    }

    floatSeq.clear();

    for(unsigned int i = 0; i < _floatSeq.size(); i++){
        floatSeq.push_back(_floatSeq.at(i));
    }
}

void props_i::octetSeqChanged (const std::string& id)
{
    std::vector<CORBA::Octet> _octetSeq;

    for(int i = octetSeq.size()-1; i >= 0; i--){
        _octetSeq.push_back(octetSeq.at(i));
    }

    octetSeq.clear();

    for(unsigned int i = 0; i < _octetSeq.size(); i++){
        octetSeq.push_back(_octetSeq.at(i));
    }
}

void props_i::charSeqChanged (const std::string& id)
{
    std::vector<char> _charSeq;

    for(int i = charSeq.size()-1; i >= 0; i--){
        _charSeq.push_back(charSeq.at(i));
    }

    charSeq.clear();

    for(unsigned int i = 0; i < _charSeq.size(); i++){
        charSeq.push_back(_charSeq.at(i));
    }
}

void props_i::ushortSeqChanged (const std::string& id)
{
    std::vector<CORBA::UShort> _ushortSeq;

    for(int i = ushortSeq.size()-1; i >= 0; i--){
        _ushortSeq.push_back(ushortSeq.at(i));
    }

    ushortSeq.clear();

    for(unsigned int i = 0; i < _ushortSeq.size(); i++){
        ushortSeq.push_back(_ushortSeq.at(i));
    }
}

void props_i::doubleSeqChanged (const std::string& id)
{
    std::vector<CORBA::Double> _doubleSeq;

    for(int i = doubleSeq.size()-1; i >= 0; i--){
        _doubleSeq.push_back(doubleSeq.at(i));
    }

    doubleSeq.clear();

    for(unsigned int i = 0; i < _doubleSeq.size(); i++){
        doubleSeq.push_back(_doubleSeq.at(i));
    }
}

void props_i::longSeqChanged (const std::string& id)
{
    std::vector<CORBA::Long> _longSeq;

    for(int i = longSeq.size()-1; i >= 0; i--){
        _longSeq.push_back(longSeq.at(i));
    }

    longSeq.clear();

    for(unsigned int i = 0; i < _longSeq.size(); i++){
        longSeq.push_back(_longSeq.at(i));
    }
}

void props_i::longlongSeqChanged (const std::string& id)
{
    std::vector<CORBA::LongLong> _longlongSeq;

    for(int i = longlongSeq.size()-1; i >= 0; i--){
        _longlongSeq.push_back(longlongSeq.at(i));
    }

    longlongSeq.clear();

    for(unsigned int i = 0; i < _longlongSeq.size(); i++){
        longlongSeq.push_back(_longlongSeq.at(i));
    }
}

void props_i::ulonglongSeqChanged (const std::string& id)
{
    std::vector<CORBA::ULongLong> _ulonglongSeq;

    for(int i = ulonglongSeq.size()-1; i >= 0; i--){
        _ulonglongSeq.push_back(ulonglongSeq.at(i));
    }

    ulonglongSeq.clear();

    for(unsigned int i = 0; i < _ulonglongSeq.size(); i++){
        ulonglongSeq.push_back(_ulonglongSeq.at(i));
    }
}

void props_i::structPropChanged (const std::string& id)
{
    try{
        structProp.structBoolSimple = !structProp.structBoolSimple;
        structProp.structDoubleSimple = structProp.structDoubleSimple * 2;
        structProp.structFloatSimple = structProp.structFloatSimple * 2;
        structProp.structLongSimple = structProp.structLongSimple * 2;
        structProp.structLonglongSimple = structProp.structLonglongSimple * 2;
        structProp.structOctetSimple = structProp.structOctetSimple * 2;
        structProp.structShortSimple = structProp.structShortSimple * 2;
        structProp.structStringSimple = structProp.structStringSimple + structProp.structStringSimple;
        structProp.structUlongSimple = structProp.structUlongSimple * 2;
        structProp.structUlonglongSimple = structProp.structUlonglongSimple * 2;
        structProp.structUshortSimple = structProp.structUshortSimple * 2;
        structProp.structCharSimple = std::toupper(structProp.structCharSimple);
    } catch (std::exception e) {
        //pass
    }
}

void props_i::structSeqPropChanged (const std::string& id)
{
    for(unsigned int i = 0; i < structSeqProp.size(); i++){
        structSeqProp.at(i).structSeqBoolSimple = !structSeqProp.at(i).structSeqBoolSimple;
        structSeqProp.at(i).structSeqFloatSimple = structSeqProp.at(i).structSeqFloatSimple * 2;
        structSeqProp.at(i).structSeqShortSimple = structSeqProp.at(i).structSeqShortSimple * 2;
        structSeqProp.at(i).structSeqStringSimple = structSeqProp.at(i).structSeqStringSimple + structSeqProp.at(i).structSeqStringSimple;
    }
}

/***********************************************************************************************

    Basic functionality:

        The service function is called by the serviceThread object (of type ProcessThread).
        This call happens immediately after the previous call if the return value for
        the previous call was NORMAL.
        If the return value for the previous call was NOOP, then the serviceThread waits
        an amount of time defined in the serviceThread's constructor.
        
    SRI:
        To create a StreamSRI object, use the following code:
        	stream_id = "";
	    	sri = BULKIO::StreamSRI();
	    	sri.hversion = 1;
	    	sri.xstart = 0.0;
	    	sri.xdelta = 0.0;
	    	sri.xunits = BULKIO::UNITS_TIME;
	    	sri.subsize = 0;
	    	sri.ystart = 0.0;
	    	sri.ydelta = 0.0;
	    	sri.yunits = BULKIO::UNITS_NONE;
	    	sri.mode = 0;
	    	sri.streamID = this->stream_id.c_str();

	Time:
	    To create a PrecisionUTCTime object, use the following code:
	        struct timeval tmp_time;
	        struct timezone tmp_tz;
	        gettimeofday(&tmp_time, &tmp_tz);
	        double wsec = tmp_time.tv_sec;
	        double fsec = tmp_time.tv_usec / 1e6;;
	        BULKIO::PrecisionUTCTime tstamp = BULKIO::PrecisionUTCTime();
	        tstamp.tcmode = BULKIO::TCM_CPU;
	        tstamp.tcstatus = (short)1;
	        tstamp.toff = 0.0;
	        tstamp.twsec = wsec;
	        tstamp.tfsec = fsec;
        
    Ports:

        Data is passed to the serviceFunction through the getPacket call (BULKIO only).
        The dataTransfer class is a port-specific class, so each port implementing the
        BULKIO interface will have its own type-specific dataTransfer.

        The argument to the getPacket function is a floating point number that specifies
        the time to wait in seconds. A zero value is non-blocking. A negative value
        is blocking.

        Each received dataTransfer is owned by serviceFunction and *MUST* be
        explicitly deallocated.

        To send data using a BULKIO interface, a convenience interface has been added 
        that takes a std::vector as the data input

        NOTE: If you have a BULKIO dataSDDS port, you must manually call 
              "port->updateStats()" to update the port statistics when appropriate.

        Example:
            // this example assumes that the component has two ports:
            //  A provides (input) port of type BULKIO::dataShort called short_in
            //  A uses (output) port of type BULKIO::dataFloat called float_out
            // The mapping between the port and the class is found
            // in the component base class header file

            BULKIO_dataShort_In_i::dataTransfer *tmp = short_in->getPacket(-1);
            if (not tmp) { // No data is available
                return NOOP;
            }

            std::vector<float> outputData;
            outputData.resize(tmp->dataBuffer.size());
            for (unsigned int i=0; i<tmp->dataBuffer.size(); i++) {
                outputData[i] = (float)tmp->dataBuffer[i];
            }

            // NOTE: You must make at least one valid pushSRI call
            if (tmp->sriChanged) {
                float_out->pushSRI(tmp->SRI);
            }
            float_out->pushPacket(outputData, tmp->T, tmp->EOS, tmp->streamID);

            delete tmp; // IMPORTANT: MUST RELEASE THE RECEIVED DATA BLOCK
            return NORMAL;

        Interactions with non-BULKIO ports are left up to the component developer's discretion

    Properties:
        
        Properties are accessed directly as member variables. For example, if the
        property name is "baudRate", it may be accessed within member functions as
        "baudRate". Unnamed properties are given a generated name of the form
        "prop_n", where "n" is the ordinal number of the property in the PRF file.
        Property types are mapped to the nearest C++ type, (e.g. "string" becomes
        "std::string"). All generated properties are declared in the base class
        (props_base).
    
        Simple sequence properties are mapped to "std::vector" of the simple type.
        Struct properties, if used, are mapped to C++ structs defined in the
        generated file "struct_props.h". Field names are taken from the name in
        the properties file; if no name is given, a generated name of the form
        "field_n" is used, where "n" is the ordinal number of the field.
        
        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A boolean called scaleInput
              
            if (scaleInput) {
                dataOut[i] = dataIn[i] * scaleValue;
            } else {
                dataOut[i] = dataIn[i];
            }
            
        A callback method can be associated with a property so that the method is
        called each time the property value changes.  This is done by calling 
        setPropertyChangeListener(<property name>, this, &props_i::<callback method>)
        in the constructor.
            
        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            
        //Add to props.cpp
        props_i::props_i(const char *uuid, const char *label) :
            props_base(uuid, label)
        {
            setPropertyChangeListener("scaleValue", this, &props_i::scaleChanged);
        }

        void props_i::scaleChanged(const std::string& id){
            std::cout << "scaleChanged scaleValue " << scaleValue << std::endl;
        }
            
        //Add to props.h
        void scaleChanged(const std::string&);
        
        
************************************************************************************************/
int props_i::serviceFunction()
{
    LOG_DEBUG(props_i, "serviceFunction() example log message");
    
    return NOOP;
}
