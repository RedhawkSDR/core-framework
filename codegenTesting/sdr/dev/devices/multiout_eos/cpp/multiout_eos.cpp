/**************************************************************************

    This is the device code. This file contains the child class where
    custom functionality can be added to the device. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "multiout_eos.h"

PREPARE_LOGGING(multiout_eos_i)

multiout_eos_i::multiout_eos_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    multiout_eos_base(devMgr_ior, id, lbl, sftwrPrfl)
{
}

multiout_eos_i::multiout_eos_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    multiout_eos_base(devMgr_ior, id, lbl, sftwrPrfl, compDev)
{
}

multiout_eos_i::multiout_eos_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    multiout_eos_base(devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
}

multiout_eos_i::multiout_eos_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    multiout_eos_base(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev)
{
}

multiout_eos_i::~multiout_eos_i()
{
}

void multiout_eos_i::constructor()
{
    /***********************************************************************************
     This is the RH constructor. All properties are properly initialized before this function is called 

     For a tuner device, the structure frontend_tuner_status needs to match the number
     of tuners that this device controls and what kind of device it is.
     The options for devices are: TX, RX, RX_DIGITIZER, CHANNELIZER, DDC, RC_DIGITIZER_CHANNELIZER
     
     For example, if this device has 5 physical
     tuners, each an RX_DIGITIZER, then the code in the construct function should look like this:

     this->setNumChannels(5, "RX_DIGITIZER");
     
     The incoming request for tuning contains a string describing the requested tuner
     type. The string for the request must match the string in the tuner status.
    ***********************************************************************************/
    this->setNumChannels(1, "RX_DIGITIZER");
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

        To create a StreamSRI object based on tuner status structure index 'idx' and collector center frequency of 100:
                std::string stream_id = "my_stream_id";
                BULKIO::StreamSRI sri = this->create(stream_id, this->frontend_tuner_status[idx], 100);

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
            // This example assumes that the device has two ports:
            //  An input (provides) port of type bulkio::InShortPort called dataShort_in
            //  An output (uses) port of type bulkio::OutFloatPort called dataFloat_out
            // The mapping between the port and the class is found
            // in the device base class header file
            // The device class must have an output stream member; add to
            // multiout_eos.h:
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

        Interactions with non-BULKIO ports are left up to the device developer's discretion
        
    Messages:
    
        To receive a message, you need (1) an input port of type MessageEvent, (2) a message prototype described
        as a structure property of kind message, (3) a callback to service the message, and (4) to register the callback
        with the input port.
        
        Assuming a property of type message is declared called "my_msg", an input port called "msg_input" is declared of
        type MessageEvent, create the following code:
        
        void multiout_eos_i::my_message_callback(const std::string& id, const my_msg_struct &msg){
        }
        
        Register the message callback onto the input port with the following form:
        this->msg_input->registerMessage("my_msg", this, &multiout_eos_i::my_message_callback);
        
        To send a message, you need to (1) create a message structure, (2) a message prototype described
        as a structure property of kind message, and (3) send the message over the port.
        
        Assuming a property of type message is declared called "my_msg", an output port called "msg_output" is declared of
        type MessageEvent, create the following code:
        
        ::my_msg_struct msg_out;
        this->msg_output->sendMessage(msg_out);

    Accessing the Device Manager and Domain Manager:
    
        Both the Device Manager hosting this Device and the Domain Manager hosting
        the Device Manager are available to the Device.
        
        To access the Domain Manager:
            CF::DomainManager_ptr dommgr = this->getDomainManager()->getRef();
        To access the Device Manager:
            CF::DeviceManager_ptr devmgr = this->getDeviceManager()->getRef();
    
    Properties:
        
        Properties are accessed directly as member variables. For example, if the
        property name is "baudRate", it may be accessed within member functions as
        "baudRate". Unnamed properties are given the property id as its name.
        Property types are mapped to the nearest C++ type, (e.g. "string" becomes
        "std::string"). All generated properties are declared in the base class
        (multiout_eos_base).
    
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
        addPropertyListener(<property>, this, &multiout_eos_i::<callback method>)
        in the constructor.

        The callback method receives two arguments, the old and new values, and
        should return nothing (void). The arguments can be passed by value,
        receiving a copy (preferred for primitive types), or by const reference
        (preferred for strings, structs and vectors).

        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A struct property called status
            
        //Add to multiout_eos.cpp
        multiout_eos_i::multiout_eos_i(const char *uuid, const char *label) :
            multiout_eos_base(uuid, label)
        {
            addPropertyListener(scaleValue, this, &multiout_eos_i::scaleChanged);
            addPropertyListener(status, this, &multiout_eos_i::statusChanged);
        }

        void multiout_eos_i::scaleChanged(float oldValue, float newValue)
        {
            LOG_DEBUG(multiout_eos_i, "scaleValue changed from" << oldValue << " to " << newValue);
        }
            
        void multiout_eos_i::statusChanged(const status_struct& oldValue, const status_struct& newValue)
        {
            LOG_DEBUG(multiout_eos_i, "status changed");
        }
            
        //Add to multiout_eos.h
        void scaleChanged(float oldValue, float newValue);
        void statusChanged(const status_struct& oldValue, const status_struct& newValue);
        
    Allocation:
    
        Allocation callbacks are available to customize the Device's response to 
        allocation requests. For example, if the Device contains the allocation 
        property "my_alloc" of type string, the allocation and deallocation
        callbacks follow the pattern (with arbitrary function names
        my_alloc_fn and my_dealloc_fn):
        
        bool multiout_eos_i::my_alloc_fn(const std::string &value)
        {
            // perform logic
            return true; // successful allocation
        }
        void multiout_eos_i::my_dealloc_fn(const std::string &value)
        {
            // perform logic
        }
        
        The allocation and deallocation functions are then registered with the Device
        base class with the setAllocationImpl call. Note that the variable for the property is used rather
        than its id:
        
        this->setAllocationImpl(my_alloc, this, &multiout_eos_i::my_alloc_fn, &multiout_eos_i::my_dealloc_fn);
        
        

************************************************************************************************/
int multiout_eos_i::serviceFunction()
{
    LOG_DEBUG(multiout_eos_i, "serviceFunction() example log message");
    
	if (this->_usageState == CF::Device::IDLE) {
		return NOOP;
	}

	std::vector<short> data;
	data.resize(10);
	this->output_stream.write(data, bulkio::time::utils::now());

    return NOOP;
}

/*************************************************************
Functions supporting tuning allocation
*************************************************************/
void multiout_eos_i::deviceEnable(frontend_tuner_status_struct_struct &fts, size_t tuner_id){
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    Make sure to set the 'enabled' member of fts to indicate that tuner as enabled
    ************************************************************/
    fts.enabled = true;
    return;
}
void multiout_eos_i::deviceDisable(frontend_tuner_status_struct_struct &fts, size_t tuner_id){
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    Make sure to reset the 'enabled' member of fts to indicate that tuner as disabled
    ************************************************************/
    fts.enabled = false;
    return;
}
bool multiout_eos_i::deviceSetTuning(const frontend::frontend_tuner_allocation_struct &request, frontend_tuner_status_struct_struct &fts, size_t tuner_id){
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
      At a minimum, bandwidth, center frequency, and sample_rate have to be set
      If the device is tuned to exactly what the request was, the code should be:
        fts.bandwidth = request.bandwidth;
        fts.center_frequency = request.center_frequency;
        fts.sample_rate = request.sample_rate;

    return true if the tuning succeeded, and false if it failed
    ************************************************************/
	if (this->_usageState == CF::Device::ACTIVE) {
		return false;
	}
	std::string stream_id("hello");
	output_stream = this->dataShort_out->createStream(stream_id);
    bulkio::connection_descriptor_struct new_connection;
    new_connection.connection_id = request.allocation_id;
    new_connection.port_name = this->dataShort_out->getName();
    new_connection.stream_id = stream_id;

    this->connectionTable.push_back(new_connection);
    this->connectionTableChanged(&this->connectionTable, &this->connectionTable);

    fts.bandwidth = request.bandwidth;
    fts.center_frequency = request.center_frequency;
    fts.sample_rate = request.sample_rate;

	this->_usageState = CF::Device::ACTIVE;

	return true;
}
bool multiout_eos_i::deviceDeleteTuning(frontend_tuner_status_struct_struct &fts, size_t tuner_id) {
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    return true if the tune deletion succeeded, and false if it failed
    ************************************************************/
	if (this->_usageState == CF::Device::ACTIVE) {
		output_stream.close();
		this->_usageState = CF::Device::IDLE;
	}

    return true;
}

/*************************************************************
Functions servicing the tuner control port
*************************************************************/
std::string multiout_eos_i::getTunerType(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].tuner_type;
}

bool multiout_eos_i::getTunerDeviceControl(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if (getControlAllocationId(idx) == allocation_id)
        return true;
    return false;
}

std::string multiout_eos_i::getTunerGroupId(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].group_id;
}

std::string multiout_eos_i::getTunerRfFlowId(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].rf_flow_id;
}

void multiout_eos_i::setTunerCenterFrequency(const std::string& allocation_id, double freq) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (freq<0) throw FRONTEND::BadParameterException();
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[idx].center_frequency = freq;
}

double multiout_eos_i::getTunerCenterFrequency(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].center_frequency;
}

void multiout_eos_i::setTunerBandwidth(const std::string& allocation_id, double bw) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (bw<0) throw FRONTEND::BadParameterException();
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[idx].bandwidth = bw;
}

double multiout_eos_i::getTunerBandwidth(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].bandwidth;
}

void multiout_eos_i::setTunerAgcEnable(const std::string& allocation_id, bool enable)
{
    throw FRONTEND::NotSupportedException("setTunerAgcEnable not supported");
}

bool multiout_eos_i::getTunerAgcEnable(const std::string& allocation_id)
{
    throw FRONTEND::NotSupportedException("getTunerAgcEnable not supported");
}

void multiout_eos_i::setTunerGain(const std::string& allocation_id, float gain)
{
    throw FRONTEND::NotSupportedException("setTunerGain not supported");
}

float multiout_eos_i::getTunerGain(const std::string& allocation_id)
{
    throw FRONTEND::NotSupportedException("getTunerGain not supported");
}

void multiout_eos_i::setTunerReferenceSource(const std::string& allocation_id, long source)
{
    throw FRONTEND::NotSupportedException("setTunerReferenceSource not supported");
}

long multiout_eos_i::getTunerReferenceSource(const std::string& allocation_id)
{
    throw FRONTEND::NotSupportedException("getTunerReferenceSource not supported");
}

void multiout_eos_i::setTunerEnable(const std::string& allocation_id, bool enable) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[idx].enabled = enable;
}

bool multiout_eos_i::getTunerEnable(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].enabled;
}

void multiout_eos_i::setTunerOutputSampleRate(const std::string& allocation_id, double sr) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (sr<0) throw FRONTEND::BadParameterException();
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[idx].sample_rate = sr;
}

double multiout_eos_i::getTunerOutputSampleRate(const std::string& allocation_id){
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].sample_rate;
}

/*************************************************************
Functions servicing the RFInfo port(s)
- port_name is the port over which the call was received
*************************************************************/
std::string multiout_eos_i::get_rf_flow_id(const std::string& port_name)
{
    return std::string("none");
}

void multiout_eos_i::set_rf_flow_id(const std::string& port_name, const std::string& id)
{
}

frontend::RFInfoPkt multiout_eos_i::get_rfinfo_pkt(const std::string& port_name)
{
    frontend::RFInfoPkt pkt;
    return pkt;
}

void multiout_eos_i::set_rfinfo_pkt(const std::string& port_name, const frontend::RFInfoPkt &pkt)
{
}

