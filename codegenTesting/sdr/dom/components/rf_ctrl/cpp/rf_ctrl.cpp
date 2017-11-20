/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "rf_ctrl.h"

PREPARE_LOGGING(rf_ctrl_i)

rf_ctrl_i::rf_ctrl_i(const char *uuid, const char *label) :
    rf_ctrl_base(uuid, label)
{
    // Avoid placing constructor code here. Instead, use the "constructor" function.

}

rf_ctrl_i::~rf_ctrl_i()
{
}

void rf_ctrl_i::constructor()
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
        
        void rf_ctrl_i::my_message_callback(const std::string& id, const my_msg_struct &msg){
        }
        
        Register the message callback onto the input port with the following form:
        this->msg_input->registerMessage("my_msg", this, &rf_ctrl_i::my_message_callback);
        
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
        (rf_ctrl_base).
    
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
        addPropertyListener(<property>, this, &rf_ctrl_i::<callback method>)
        in the constructor.

        The callback method receives two arguments, the old and new values, and
        should return nothing (void). The arguments can be passed by value,
        receiving a copy (preferred for primitive types), or by const reference
        (preferred for strings, structs and vectors).

        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A struct property called status
            
        //Add to rf_ctrl.cpp
        rf_ctrl_i::rf_ctrl_i(const char *uuid, const char *label) :
            rf_ctrl_base(uuid, label)
        {
            addPropertyListener(scaleValue, this, &rf_ctrl_i::scaleChanged);
            addPropertyListener(status, this, &rf_ctrl_i::statusChanged);
        }

        void rf_ctrl_i::scaleChanged(float oldValue, float newValue)
        {
            LOG_DEBUG(rf_ctrl_i, "scaleValue changed from" << oldValue << " to " << newValue);
        }
            
        void rf_ctrl_i::statusChanged(const status_struct& oldValue, const status_struct& newValue)
        {
            LOG_DEBUG(rf_ctrl_i, "status changed");
        }
            
        //Add to rf_ctrl.h
        void scaleChanged(float oldValue, float newValue);
        void statusChanged(const status_struct& oldValue, const status_struct& newValue);
        

************************************************************************************************/
int rf_ctrl_i::serviceFunction()
{
    LOG_DEBUG(rf_ctrl_i, "serviceFunction() example log message");
    
    this->get_rfinfo = "ok";
    try {
    	this->rfinfo_out->rf_flow_id();
    } catch (redhawk::PortCallError &e) {
    	this->get_rfinfo = e.what();
    }
    this->set_rfinfo = "ok";
    try {
		std::string rf_flow_id("hello");
		this->rfinfo_out->rf_flow_id(rf_flow_id);
    } catch (redhawk::PortCallError &e) {
    	this->set_rfinfo = e.what();
    }
    this->get_current_rf = "ok";
    try {
    	this->rfsource_out->current_rf_input();
    } catch (redhawk::PortCallError &e) {
    	this->get_current_rf = e.what();
    }
    this->set_current_rf = "ok";
    try {
		frontend::RFInfoPkt foo;
		this->rfsource_out->current_rf_input(foo);
    } catch (redhawk::PortCallError &e) {
    	this->set_current_rf = e.what();
    }
    this->get_available_rf = "ok";
    try {
    	this->rfsource_out->available_rf_inputs();
    } catch (redhawk::PortCallError &e) {
    	this->get_available_rf = e.what();
    }
    this->set_available_rf = "ok";
    try {
		std::vector<frontend::RFInfoPkt> foo;
		this->rfsource_out->available_rf_inputs(foo);
    } catch (redhawk::PortCallError &e) {
    	this->set_available_rf = e.what();
    }
    this->bad_connection = "ok";
    try {
    	this->rfsource_out->_get_available_rf_inputs("invalid_connectionid");
    } catch (redhawk::PortCallError &e) {
    	this->bad_connection = e.what();
    }

    std::string tmp;
    this->get_tunertype = "ok";
    try {
    	this->digitaltuner_out->getTunerType(tmp);
    } catch (redhawk::PortCallError &e) {
    	this->get_tunertype = e.what();
    }
    this->get_tunerdevicecontrol = "ok";
    try {
		std::vector<frontend::RFInfoPkt> foo;
		this->digitaltuner_out->getTunerDeviceControl(tmp);
    } catch (redhawk::PortCallError &e) {
    	this->get_tunerdevicecontrol = e.what();
    }
    this->get_tunergroupid = "ok";
    try {
    	this->digitaltuner_out->getTunerGroupId(tmp);
    } catch (redhawk::PortCallError &e) {
    	this->get_tunergroupid = e.what();
    }
    this->get_tunerrfflowid = "ok";
    try {
		std::vector<frontend::RFInfoPkt> foo;
		this->digitaltuner_out->getTunerRfFlowId(tmp);
    } catch (redhawk::PortCallError &e) {
    	this->get_tunerrfflowid = e.what();
    }
    this->get_tunerstatus = "ok";
    try {
    	this->digitaltuner_out->getTunerStatus(tmp);
    } catch (redhawk::PortCallError &e) {
    	this->get_tunerstatus = e.what();
    }
    this->get_tunercenterfrequency = "ok";
    try {
		std::vector<frontend::RFInfoPkt> foo;
		this->digitaltuner_out->getTunerCenterFrequency(tmp);
    } catch (redhawk::PortCallError &e) {
    	this->get_tunercenterfrequency = e.what();
    }
    this->set_tunercenterfrequency = "ok";
    try {
    	this->digitaltuner_out->setTunerCenterFrequency(tmp, 1.0);
    } catch (redhawk::PortCallError &e) {
    	this->set_tunercenterfrequency = e.what();
    }
    this->get_tunerbandwidth = "ok";
    try {
		std::vector<frontend::RFInfoPkt> foo;
		this->digitaltuner_out->getTunerBandwidth(tmp);
    } catch (redhawk::PortCallError &e) {
    	this->get_tunerbandwidth = e.what();
    }
    this->set_tunerbandwidth = "ok";
    try {
    	this->digitaltuner_out->setTunerBandwidth(tmp, 1.0);
    } catch (redhawk::PortCallError &e) {
    	this->set_tunerbandwidth = e.what();
    }
    this->get_tuneragcenable = "ok";
    try {
		std::vector<frontend::RFInfoPkt> foo;
		this->digitaltuner_out->getTunerAgcEnable(tmp);
    } catch (redhawk::PortCallError &e) {
    	this->get_tuneragcenable = e.what();
    }
    this->set_tuneragcenable = "ok";
    try {
    	this->digitaltuner_out->setTunerAgcEnable(tmp, false);
    } catch (redhawk::PortCallError &e) {
    	this->set_tuneragcenable = e.what();
    }
    this->get_tunergain = "ok";
    try {
		std::vector<frontend::RFInfoPkt> foo;
		this->digitaltuner_out->getTunerGain(tmp);
    } catch (redhawk::PortCallError &e) {
    	this->get_tunergain = e.what();
    }
    this->set_tunergain = "ok";
    try {
    	this->digitaltuner_out->setTunerGain(tmp, 1.0);
    } catch (redhawk::PortCallError &e) {
    	this->set_tunergain = e.what();
    }
    this->get_tunerreferencesource = "ok";
    try {
		std::vector<frontend::RFInfoPkt> foo;
		this->digitaltuner_out->getTunerReferenceSource(tmp);
    } catch (redhawk::PortCallError &e) {
    	this->get_tunerreferencesource = e.what();
    }
    this->set_tunerreferencesource = "ok";
    try {
    	this->digitaltuner_out->setTunerReferenceSource(tmp, 2);
    } catch (redhawk::PortCallError &e) {
    	this->set_tunerreferencesource = e.what();
    }
    this->get_tunerenable = "ok";
    try {
		std::vector<frontend::RFInfoPkt> foo;
		this->digitaltuner_out->getTunerEnable(tmp);
    } catch (redhawk::PortCallError &e) {
    	this->get_tunerenable = e.what();
    }
    this->set_tunerenable = "ok";
    try {
    	this->digitaltuner_out->setTunerEnable(tmp, false);
    } catch (redhawk::PortCallError &e) {
    	this->set_tunerenable = e.what();
    }
    this->get_tuneroutputsamplerate = "ok";
    try {
		std::vector<frontend::RFInfoPkt> foo;
		this->digitaltuner_out->getTunerOutputSampleRate(tmp);
    } catch (redhawk::PortCallError &e) {
    	this->get_tuneroutputsamplerate = e.what();
    }
    this->set_tuneroutputsamplerate = "ok";
    try {
    	this->digitaltuner_out->setTunerOutputSampleRate(tmp, 1.0);
    } catch (redhawk::PortCallError &e) {
    	this->set_tuneroutputsamplerate = e.what();
    }

    this->get_gpsinfo = "ok";
    try {
		this->gps_out->gps_info();
    } catch (redhawk::PortCallError &e) {
    	this->get_gpsinfo = e.what();
    }
    this->set_gpsinfo = "ok";
    try {
    	frontend::GPSInfo _gps;
    	this->gps_out->gps_info(_gps);
    } catch (redhawk::PortCallError &e) {
    	this->set_gpsinfo = e.what();
    }
    this->get_gps_timepos = "ok";
    try {
		std::vector<frontend::RFInfoPkt> foo;
		this->gps_out->gps_time_pos();
    } catch (redhawk::PortCallError &e) {
    	this->get_gps_timepos = e.what();
    }
    this->set_gps_timepos = "ok";
    try {
    	frontend::GpsTimePos _gps;
    	this->gps_out->gps_time_pos(_gps);
    } catch (redhawk::PortCallError &e) {
    	this->set_gps_timepos = e.what();
    }

    this->get_nav_packet = "ok";
    try {
		this->navdata_out->nav_packet();
    } catch (redhawk::PortCallError &e) {
    	this->get_nav_packet = e.what();
    }
    this->set_nav_packet = "ok";
    try {
    	frontend::NavigationPacket _nav;
    	this->navdata_out->nav_packet(_nav);
    } catch (redhawk::PortCallError &e) {
    	this->set_nav_packet = e.what();
    }

    return NOOP;
}

