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
/**************************************************************************

    This is the device code. This file contains the child class where
    custom functionality can be added to the device. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "CppTestDevice.h"

PREPARE_LOGGING(CppTestDevice_i)

CppTestDevice_i::CppTestDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    CppTestDevice_base(devMgr_ior, id, lbl, sftwrPrfl)
{
}

CppTestDevice_i::CppTestDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    CppTestDevice_base(devMgr_ior, id, lbl, sftwrPrfl, compDev)
{
}

CppTestDevice_i::CppTestDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    CppTestDevice_base(devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
}

CppTestDevice_i::CppTestDevice_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    CppTestDevice_base(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev)
{
}

CppTestDevice_i::~CppTestDevice_i()
{
}

void CppTestDevice_i::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
	CppTestDevice_base::initialize();
	setAllocationImpl("memory_allocation", this, &CppTestDevice_i::allocate_memory, &CppTestDevice_i::deallocate_memory);
	setAllocationImpl("load_average", this, &CppTestDevice_i::allocate_load, &CppTestDevice_i::deallocate_load);
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
                std::string stream_id = "testStream";
                BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);

	Time:
	    To create a PrecisionUTCTime object, use the following code:
                BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();

        
    Ports:

        Data is passed to the serviceFunction through the getPacket call (BULKIO only).
        The dataTransfer class is a port-specific class, so each port implementing the
        BULKIO interface will have its own type-specific dataTransfer.

        The argument to the getPacket function is a floating point number that specifies
        the time to wait in seconds. A zero value is non-blocking. A negative value
        is blocking.  Constants have been defined for these values, bulkio::Const::BLOCKING and
        bulkio::Const::NON_BLOCKING.

        Each received dataTransfer is owned by serviceFunction and *MUST* be
        explicitly deallocated.

        To send data using a BULKIO interface, a convenience interface has been added 
        that takes a std::vector as the data input

        NOTE: If you have a BULKIO dataSDDS port, you must manually call 
              "port->updateStats()" to update the port statistics when appropriate.

        Example:
            // this example assumes that the device has two ports:
            //  A provides (input) port of type bulkio::InShortPort called short_in
            //  A uses (output) port of type bulkio::OutFloatPort called float_out
            // The mapping between the port and the class is found
            // in the device base class header file

            bulkio::InShortPort::dataTransfer *tmp = short_in->getPacket(bulkio::Const::BLOCKING);
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

        If working with complex data (i.e., the "mode" on the SRI is set to
        true), the std::vector passed from/to BulkIO can be typecast to/from
        std::vector< std::complex<dataType> >.  For example, for short data:

            bulkio::InShortPort::dataTransfer *tmp = myInput->getPacket(bulkio::Const::BLOCKING);
            std::vector<std::complex<short> >* intermediate = (std::vector<std::complex<short> >*) &(tmp->dataBuffer);
            // do work here
            std::vector<short>* output = (std::vector<short>*) intermediate;
            myOutput->pushPacket(*output, tmp->T, tmp->EOS, tmp->streamID);

        Interactions with non-BULKIO ports are left up to the device developer's discretion

    Properties:
        
        Properties are accessed directly as member variables. For example, if the
        property name is "baudRate", it may be accessed within member functions as
        "baudRate". Unnamed properties are given a generated name of the form
        "prop_n", where "n" is the ordinal number of the property in the PRF file.
        Property types are mapped to the nearest C++ type, (e.g. "string" becomes
        "std::string"). All generated properties are declared in the base class
        (CppTestDevice_base).
    
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
            
        Callback methods can be associated with a property so that the methods are
        called each time the property value changes.  This is done by calling 
        addPropertyChangeListener(<property name>, this, &CppTestDevice_i::<callback method>)
        in the constructor.

        Callback methods should take two arguments, both const pointers to the value
        type (e.g., "const float *"), and return void.

        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            
        //Add to CppTestDevice.cpp
        CppTestDevice_i::CppTestDevice_i(const char *uuid, const char *label) :
            CppTestDevice_base(uuid, label)
        {
            addPropertyChangeListener("scaleValue", this, &CppTestDevice_i::scaleChanged);
        }

        void CppTestDevice_i::scaleChanged(const float *oldValue, const float *newValue)
        {
            std::cout << "scaleValue changed from" << *oldValue << " to " << *newValue
                      << std::endl;
        }
            
        //Add to CppTestDevice.h
        void scaleChanged(const float* oldValue, const float* newValue);
        
        
************************************************************************************************/
int CppTestDevice_i::serviceFunction()
{
    LOG_DEBUG(CppTestDevice_i, "serviceFunction() example log message");
    
    return NOOP;
}

bool CppTestDevice_i::allocate_memory(const memory_allocation_struct& capacity) {
	if (capacity.contiguous) {
		// Pretend that there is never enough contiguous memory.
		return false;
	}

	int requested_mem = capacity.capacity;

	// Check that there's enough total memory.
	if (requested_mem > this->memory_capacity) {
		return false;
	}

	// If request is for shared memory, check that as well.
	if (capacity.memory_type == 1) {
    	if (requested_mem > this->shared_memory) {
    		return false;
    	}
    	// Update shared memory capacity.
    	this->shared_memory -= requested_mem;
	}

	// Update total memory capacity.
	this->memory_capacity -= requested_mem;

	return true;
}

void CppTestDevice_i::deallocate_memory(const memory_allocation_struct& capacity) {
	int released_capacity = capacity.capacity;
	this->memory_capacity += released_capacity;

	if (capacity.memory_type == 1) {
		this->shared_memory += released_capacity;
	}
}

bool CppTestDevice_i::allocate_load(const float& capacity) {
	if (this->load_average + capacity > MAX_LOAD) {
		return false;
	}
	this->load_average += capacity;
	return true;
}

void CppTestDevice_i::deallocate_load(const float& capacity) {
	if (capacity > this->load_average) {
		throw std::range_error("Deallocated capacity exceeds maximum");
	}
	this->load_average -= capacity;
}
