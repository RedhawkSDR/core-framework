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

#include <boost/foreach.hpp>

#include "EnumTest.h"

PREPARE_LOGGING(EnumTest_i)

EnumTest_i::EnumTest_i(const char *uuid, const char *label) :
    EnumTest_base(uuid, label)
{
    // Avoid placing constructor code here. Instead, use the "constructor" function.

}

EnumTest_i::~EnumTest_i()
{
}

void EnumTest_i::constructor()
{
    /***********************************************************************************
     This is the RH constructor. All properties are properly initialized before this function is called 
    ***********************************************************************************/
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

        Data is passed to the serviceFunction through by reading from input streams
        (BulkIO only). The input stream class is a port-specific class, so each port
        implementing the BulkIO interface will have its own type-specific input stream.
        UDP multicast (dataSDDS and dataVITA49) ports do not support streams.

        The input stream from which to read can be requested with the getCurrentStream()
        method. The optional argument to getCurrentStream() is a floating point number that
        specifies the time to wait in seconds. A zero value is non-blocking. A negative value
        is blocking.  Constants have been defined for these values, bulkio::Const::BLOCKING and
        bulkio::Const::NON_BLOCKING.

        More advanced uses of input streams are possible; refer to the REDHAWK documentation
        for more details.

        Input streams return data blocks that automatically manage the memory for the data
        and include the SRI that was in effect at the time the data was received. It is not
        necessary to delete the block; it will be cleaned up when it goes out of scope.

        To send data using a BulkIO interface, create an output stream and write the
        data to it. When done with the output stream, the close() method sends and end-of-
        stream flag and cleans up.

        NOTE: If you have a BULKIO dataSDDS or dataVITA49  port, you must manually call 
              "port->updateStats()" to update the port statistics when appropriate.

        Example:
            // This example assumes that the component has two ports:
            //  An input (provides) port of type bulkio::InShortPort called dataShort_in
            //  An output (uses) port of type bulkio::OutFloatPort called dataFloat_out
            // The mapping between the port and the class is found
            // in the component base class header file

            bulkio::InShortStream inputStream = dataShort_in->getCurrentStream();
            if (!inputStream) { // No streams are available
                return NOOP;
            }

            // Get the output stream, creating it if it doesn't exist yet
            bulkio::OutFloatStream outputStream = dataFloat_out->getStream(inputStream.streamID());
            if (!outputStream) {
                outputStream = dataFloat_out->createStream(inputStream.sri());
            }

            bulkio::ShortDataBlock block = inputStream.read();
            if (!block) { // No data available
                // Propagate end-of-stream
                if (inputStream.eos()) {
                   outputStream.close();
                }
                return NOOP;
            }

            if (block.sriChanged()) {
                // Update output SRI
                outputStream.sri(block.sri());
            }

            // Get read-only access to the input data
            redhawk::shared_buffer<short> inputData = block.buffer();

            // Acquire a new buffer to hold the output data
            redhawk::buffer<float> outputData(inputData.size());

            // Transform input data into output data
            for (size_t index = 0; index < inputData.size(); ++index) {
                outputData[index] = (float) inputData[index];
            }

            // Write to the output stream; outputData must not be modified after
            // this method call
            outputStream.write(outputData, block.getStartTime());

            return NORMAL;

        If working with complex data (i.e., the "mode" on the SRI is set to
        true), the data block's complex() method will return true. Data blocks
        provide a cxbuffer() method that returns a complex interpretation of the
        buffer without making a copy:

            if (block.complex()) {
                redhawk::shared_buffer<std::complex<short> > inData = block.cxbuffer();
                redhawk::buffer<std::complex<float> > outData(inData.size());
                for (size_t index = 0; index < inData.size(); ++index) {
                    outData[index] = inData[index];
                }
                outputStream.write(outData, block.getStartTime());
            }

        Interactions with non-BULKIO ports are left up to the component developer's discretion
        
    Messages:
    
        To receive a message, you need (1) an input port of type MessageEvent, (2) a message prototype described
        as a structure property of kind message, (3) a callback to service the message, and (4) to register the callback
        with the input port.
        
        Assuming a property of type message is declared called "my_msg", an input port called "msg_input" is declared of
        type MessageEvent, create the following code:
        
        void EnumTest_i::my_message_callback(const std::string& id, const my_msg_struct &msg){
        }
        
        Register the message callback onto the input port with the following form:
        this->msg_input->registerMessage("my_msg", this, &EnumTest_i::my_message_callback);
        
        To send a message, you need to (1) create a message structure, (2) a message prototype described
        as a structure property of kind message, and (3) send the message over the port.
        
        Assuming a property of type message is declared called "my_msg", an output port called "msg_output" is declared of
        type MessageEvent, create the following code:
        
        ::my_msg_struct msg_out;
        this->msg_output->sendMessage(msg_out);

    Accessing the Application and Domain Manager:
    
        Both the Application hosting this Component and the Domain Manager hosting
        the Application are available to the Component.
        
        To access the Domain Manager:
            CF::DomainManager_ptr dommgr = this->getDomainManager()->getRef();
        To access the Application:
            CF::Application_ptr app = this->getApplication()->getRef();
    
    Properties:
        
        Properties are accessed directly as member variables. For example, if the
        property name is "baudRate", it may be accessed within member functions as
        "baudRate". Unnamed properties are given the property id as its name.
        Property types are mapped to the nearest C++ type, (e.g. "string" becomes
        "std::string"). All generated properties are declared in the base class
        (EnumTest_base).
    
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
        addPropertyListener(<property>, this, &EnumTest_i::<callback method>)
        in the constructor.

        The callback method receives two arguments, the old and new values, and
        should return nothing (void). The arguments can be passed by value,
        receiving a copy (preferred for primitive types), or by const reference
        (preferred for strings, structs and vectors).

        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A struct property called status
            
        //Add to EnumTest.cpp
        EnumTest_i::EnumTest_i(const char *uuid, const char *label) :
            EnumTest_base(uuid, label)
        {
            addPropertyListener(scaleValue, this, &EnumTest_i::scaleChanged);
            addPropertyListener(status, this, &EnumTest_i::statusChanged);
        }

        void EnumTest_i::scaleChanged(float oldValue, float newValue)
        {
            LOG_DEBUG(EnumTest_i, "scaleValue changed from" << oldValue << " to " << newValue);
        }
            
        void EnumTest_i::statusChanged(const status_struct& oldValue, const status_struct& newValue)
        {
            LOG_DEBUG(EnumTest_i, "status changed");
        }
            
        //Add to EnumTest.h
        void scaleChanged(float oldValue, float newValue);
        void statusChanged(const status_struct& oldValue, const status_struct& newValue);
        

************************************************************************************************/
int EnumTest_i::serviceFunction()
{
    LOG_DEBUG(EnumTest_i, "serviceFunction() example log message");
    
    return NOOP;
}

void EnumTest_i::runTest(CORBA::ULong testid, CF::Properties& testValues) throw (CF::UnknownProperties, CF::TestableObject::UnknownTest, CORBA::SystemException)
{
    redhawk::PropertyMap& testProps = redhawk::PropertyMap::cast(testValues);
    switch (testid) {
    case 0:
        runEnumTest(testProps);
        break;
    default:
        throw CF::TestableObject::UnknownTest();
    }
}

void EnumTest_i::runEnumTest(redhawk::PropertyMap& testValues)
{
    redhawk::PropertyMap unknown;

    BOOST_FOREACH(redhawk::PropertyType& prop, testValues) {
        const std::string prop_id = prop.getId();
        redhawk::PropertyMap value;
        if (prop_id == "floatenum") {
            value["DEFAULT"] = enums::floatenum::DEFAULT;
            value["OTHER"] = enums::floatenum::OTHER;
        } else if (prop_id == "stringenum") {
            value["START"] = enums::stringenum::START;
            value["STOPPED"] = enums::stringenum::STOPPED;
        } else if (prop_id == "structprop") {
            redhawk::PropertyMap number_enums;
            number_enums["ZERO"] = enums::structprop::number::ZERO;
            number_enums["ONE"] = enums::structprop::number::ONE;
            number_enums["TWO"] = enums::structprop::number::TWO;
            value["structprop::number"] = number_enums;

            redhawk::PropertyMap alpha_enums;
            alpha_enums["ABC"] = enums::structprop::alpha::ABC;
            alpha_enums["DEF"] = enums::structprop::alpha::DEF;
            value["structprop::alpha"] = alpha_enums;
        } else if (prop_id == "structseq") {
            redhawk::PropertyMap number_enums;
            number_enums["POSITIVE"] = enums::structseq_struct::number::POSITIVE;
            number_enums["ZERO"] = enums::structseq_struct::number::ZERO;
            number_enums["NEGATIVE"] = enums::structseq_struct::number::NEGATIVE;
            value["structseq::number"] = number_enums;

            redhawk::PropertyMap text_enums;
            text_enums["HEADER"] = enums::structseq_struct::text::HEADER;
            text_enums["BODY"] = enums::structseq_struct::text::BODY;
            text_enums["FOOTER"] = enums::structseq_struct::text::FOOTER;
            value["structseq::text"] = text_enums;
        } else {
            LOG_ERROR(EnumTest_i, "Unknown property " << prop_id);
            unknown.push_back(prop);
            continue;
        }
        prop.setValue(value);
    }

    if (!unknown.empty()) {
        throw CF::UnknownProperties(unknown);
    }
}
