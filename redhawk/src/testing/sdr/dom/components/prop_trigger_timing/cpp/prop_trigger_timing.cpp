/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "prop_trigger_timing.h"

PREPARE_LOGGING(prop_trigger_timing_i)

prop_trigger_timing_i::prop_trigger_timing_i(const char *uuid, const char *label) :
    prop_trigger_timing_base(uuid, label)
{
    // Avoid placing constructor code here. Instead, use the "constructor" function.
    this->prop_1_trigger = false;
    this->prop_2_trigger = false;
    this->prop_3_trigger = false;
    this->prop_4_trigger = false;

    this->addPropertyChangeListener("prop_1", this, &prop_trigger_timing_i::prop_1_changed);
    this->addPropertyChangeListener("prop_2", this, &prop_trigger_timing_i::prop_2_changed);
    this->addPropertyChangeListener("prop_3", this, &prop_trigger_timing_i::prop_3_changed);
    this->addPropertyChangeListener("prop_4", this, &prop_trigger_timing_i::prop_4_changed);

}

void prop_trigger_timing_i::prop_1_changed(const std::string* oldValue, const std::string* newValue)
{
    this->prop_1_trigger = true;
}

void prop_trigger_timing_i::prop_2_changed(const std::vector<std::string>* oldValue, const std::vector<std::string>* newValue)
{
    this->prop_2_trigger = true;
}

void prop_trigger_timing_i::prop_3_changed(const prop_3_struct* oldValue, const prop_3_struct* newValue)
{
    this->prop_3_trigger = true;
}

void prop_trigger_timing_i::prop_4_changed(const std::vector<prop_4_a_struct>* oldValue, const std::vector<prop_4_a_struct>* newValue)
{
    this->prop_4_trigger = true;
}

prop_trigger_timing_i::~prop_trigger_timing_i()
{
}

void prop_trigger_timing_i::constructor()
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
        UDP multicast (dataSDDS and dataVITA49) and string-based (dataString, dataXML and
        dataFile) do not support streams.

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
            // The component class must have an output stream member; add to
            // prop_trigger_timing.h:
            //   bulkio::OutFloatStream outputStream;

            bulkio::InShortStream inputStream = dataShort_in->getCurrentStream();
            if (!inputStream) { // No streams are available
                return NOOP;
            }

            bulkio::ShortDataBlock block = inputStream.read();
            if (!block) { // No data available
                // Propagate end-of-stream
                if (inputStream.eos()) {
                   outputStream.close();
                }
                return NOOP;
            }

            short* inputData = block.data();
            std::vector<float> outputData;
            outputData.resize(block.size());
            for (size_t index = 0; index < block.size(); ++index) {
                outputData[index] = (float) inputData[index];
            }

            // If there is no output stream open, create one
            if (!outputStream) {
                outputStream = dataFloat_out->createStream(block.sri());
            } else if (block.sriChanged()) {
                // Update output SRI
                outputStream.sri(block.sri());
            }

            // Write to the output stream
            outputStream.write(outputData, block.getTimestamps());

            // Propagate end-of-stream
            if (inputStream.eos()) {
              outputStream.close();
            }

            return NORMAL;

        If working with complex data (i.e., the "mode" on the SRI is set to
        true), the data block's complex() method will return true. Data blocks
        provide functions that return the correct interpretation of the data
        buffer and number of complex elements:

            if (block.complex()) {
                std::complex<short>* data = block.cxdata();
                for (size_t index = 0; index < block.cxsize(); ++index) {
                    data[index] = std::abs(data[index]);
                }
                outputStream.write(data, block.cxsize(), bulkio::time::utils::now());
            }

        Interactions with non-BULKIO ports are left up to the component developer's discretion
        
    Messages:
    
        To receive a message, you need (1) an input port of type MessageEvent, (2) a message prototype described
        as a structure property of kind message, (3) a callback to service the message, and (4) to register the callback
        with the input port.
        
        Assuming a property of type message is declared called "my_msg", an input port called "msg_input" is declared of
        type MessageEvent, create the following code:
        
        void prop_trigger_timing_i::my_message_callback(const std::string& id, const my_msg_struct &msg){
        }
        
        Register the message callback onto the input port with the following form:
        this->msg_input->registerMessage("my_msg", this, &prop_trigger_timing_i::my_message_callback);
        
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
        (prop_trigger_timing_base).
    
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
        addPropertyListener(<property>, this, &prop_trigger_timing_i::<callback method>)
        in the constructor.

        The callback method receives two arguments, the old and new values, and
        should return nothing (void). The arguments can be passed by value,
        receiving a copy (preferred for primitive types), or by const reference
        (preferred for strings, structs and vectors).

        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A struct property called status
            
        //Add to prop_trigger_timing.cpp
        prop_trigger_timing_i::prop_trigger_timing_i(const char *uuid, const char *label) :
            prop_trigger_timing_base(uuid, label)
        {
            addPropertyListener(scaleValue, this, &prop_trigger_timing_i::scaleChanged);
            addPropertyListener(status, this, &prop_trigger_timing_i::statusChanged);
        }

        void prop_trigger_timing_i::scaleChanged(float oldValue, float newValue)
        {
            LOG_DEBUG(prop_trigger_timing_i, "scaleValue changed from" << oldValue << " to " << newValue);
        }
            
        void prop_trigger_timing_i::statusChanged(const status_struct& oldValue, const status_struct& newValue)
        {
            LOG_DEBUG(prop_trigger_timing_i, "status changed");
        }
            
        //Add to prop_trigger_timing.h
        void scaleChanged(float oldValue, float newValue);
        void statusChanged(const status_struct& oldValue, const status_struct& newValue);
        

************************************************************************************************/
int prop_trigger_timing_i::serviceFunction()
{
    LOG_DEBUG(prop_trigger_timing_i, "serviceFunction() example log message");
    
    return NOOP;
}

