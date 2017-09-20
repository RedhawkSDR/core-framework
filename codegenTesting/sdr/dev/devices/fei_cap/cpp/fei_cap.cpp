/**************************************************************************

    This is the device code. This file contains the child class where
    custom functionality can be added to the device. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "fei_cap.h"

PREPARE_LOGGING(fei_cap_i)

fei_cap_i::fei_cap_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    fei_cap_base(devMgr_ior, id, lbl, sftwrPrfl)
{
}

fei_cap_i::fei_cap_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    fei_cap_base(devMgr_ior, id, lbl, sftwrPrfl, compDev)
{
}

fei_cap_i::fei_cap_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    fei_cap_base(devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
}

fei_cap_i::fei_cap_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    fei_cap_base(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev)
{
}

fei_cap_i::~fei_cap_i()
{
}

void fei_cap_i::constructor()
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
    this->setNumChannels(2, "RX_DIGITIZER");
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
            // fei_cap.h:
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

        void fei_cap_i::my_message_callback(const std::string& id, const my_msg_struct &msg){
        }

        Register the message callback onto the input port with the following form:
        this->msg_input->registerMessage("my_msg", this, &fei_cap_i::my_message_callback);

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
        (fei_cap_base).

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
        addPropertyListener(<property>, this, &fei_cap_i::<callback method>)
        in the constructor.

        The callback method receives two arguments, the old and new values, and
        should return nothing (void). The arguments can be passed by value,
        receiving a copy (preferred for primitive types), or by const reference
        (preferred for strings, structs and vectors).

        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A struct property called status

        //Add to fei_cap.cpp
        fei_cap_i::fei_cap_i(const char *uuid, const char *label) :
            fei_cap_base(uuid, label)
        {
            addPropertyListener(scaleValue, this, &fei_cap_i::scaleChanged);
            addPropertyListener(status, this, &fei_cap_i::statusChanged);
        }

        void fei_cap_i::scaleChanged(float oldValue, float newValue)
        {
            LOG_DEBUG(fei_cap_i, "scaleValue changed from" << oldValue << " to " << newValue);
        }

        void fei_cap_i::statusChanged(const status_struct& oldValue, const status_struct& newValue)
        {
            LOG_DEBUG(fei_cap_i, "status changed");
        }

        //Add to fei_cap.h
        void scaleChanged(float oldValue, float newValue);
        void statusChanged(const status_struct& oldValue, const status_struct& newValue);

    Allocation:

        Allocation callbacks are available to customize the Device's response to
        allocation requests. For example, if the Device contains the allocation
        property "my_alloc" of type string, the allocation and deallocation
        callbacks follow the pattern (with arbitrary function names
        my_alloc_fn and my_dealloc_fn):

        bool fei_cap_i::my_alloc_fn(const std::string &value)
        {
            // perform logic
            return true; // successful allocation
        }
        void fei_cap_i::my_dealloc_fn(const std::string &value)
        {
            // perform logic
        }

        The allocation and deallocation functions are then registered with the Device
        base class with the setAllocationImpl call. Note that the variable for the property is used rather
        than its id:

        this->setAllocationImpl(my_alloc, this, &fei_cap_i::my_alloc_fn, &fei_cap_i::my_dealloc_fn);



************************************************************************************************/
int fei_cap_i::serviceFunction()
{
    LOG_DEBUG(fei_cap_i, "serviceFunction() example log message");

    return NOOP;
}

/*************************************************************
Functions supporting tuning allocation
*************************************************************/
void fei_cap_i::deviceEnable(frontend_tuner_status_struct_struct &fts, size_t tuner_id){
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    Make sure to set the 'enabled' member of fts to indicate that tuner as enabled
    ************************************************************/
    fts.enabled = true;
    return;
}
void fei_cap_i::deviceDisable(frontend_tuner_status_struct_struct &fts, size_t tuner_id){
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    Make sure to reset the 'enabled' member of fts to indicate that tuner as disabled
    ************************************************************/
    fts.enabled = false;
    return;
}
bool fei_cap_i::deviceSetTuning(const frontend::frontend_tuner_allocation_struct &request, frontend_tuner_status_struct_struct &fts, size_t tuner_id){
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
      At a minimum, bandwidth, center frequency, and sample_rate have to be set
      If the device is tuned to exactly what the request was, the code should be:
        fts.bandwidth = request.bandwidth;
        fts.center_frequency = request.center_frequency;
        fts.sample_rate = request.sample_rate;

    return true if the tuning succeeded, and false if it failed
    ************************************************************/
    return true;
}
bool fei_cap_i::deviceDeleteTuning(frontend_tuner_status_struct_struct &fts, size_t tuner_id) {
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    return true if the tune deletion succeeded, and false if it failed
    ************************************************************/
    return true;
}

/*************************************************************
Functions servicing the tuner control port
*************************************************************/
std::string fei_cap_i::getTunerType(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].tuner_type;
}

bool fei_cap_i::getTunerDeviceControl(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if (getControlAllocationId(idx) == allocation_id)
        return true;
    return false;
}

std::string fei_cap_i::getTunerGroupId(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].group_id;
}

std::string fei_cap_i::getTunerRfFlowId(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].rf_flow_id;
}

void fei_cap_i::setTunerCenterFrequency(const std::string& allocation_id, double freq) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (freq<0) throw FRONTEND::BadParameterException();
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[idx].center_frequency = freq;
}

double fei_cap_i::getTunerCenterFrequency(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].center_frequency;
}

void fei_cap_i::setTunerBandwidth(const std::string& allocation_id, double bw) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (bw<0) throw FRONTEND::BadParameterException();
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[idx].bandwidth = bw;
}

double fei_cap_i::getTunerBandwidth(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].bandwidth;
}

void fei_cap_i::setTunerAgcEnable(const std::string& allocation_id, bool enable)
{
    throw FRONTEND::NotSupportedException("setTunerAgcEnable not supported");
}

bool fei_cap_i::getTunerAgcEnable(const std::string& allocation_id)
{
    throw FRONTEND::NotSupportedException("getTunerAgcEnable not supported");
}

void fei_cap_i::setTunerGain(const std::string& allocation_id, float gain)
{
    throw FRONTEND::NotSupportedException("setTunerGain not supported");
}

float fei_cap_i::getTunerGain(const std::string& allocation_id)
{
    throw FRONTEND::NotSupportedException("getTunerGain not supported");
}

void fei_cap_i::setTunerReferenceSource(const std::string& allocation_id, long source)
{
    throw FRONTEND::NotSupportedException("setTunerReferenceSource not supported");
}

long fei_cap_i::getTunerReferenceSource(const std::string& allocation_id)
{
    throw FRONTEND::NotSupportedException("getTunerReferenceSource not supported");
}

void fei_cap_i::setTunerEnable(const std::string& allocation_id, bool enable) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[idx].enabled = enable;
}

bool fei_cap_i::getTunerEnable(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].enabled;
}

void fei_cap_i::setTunerOutputSampleRate(const std::string& allocation_id, double sr) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (sr<0) throw FRONTEND::BadParameterException();
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[idx].sample_rate = sr;
}

double fei_cap_i::getTunerOutputSampleRate(const std::string& allocation_id){
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].sample_rate;
}

/*************************************************************
Functions servicing the RFInfo port(s)
- port_name is the port over which the call was received
*************************************************************/
std::string fei_cap_i::get_rf_flow_id(const std::string& port_name)
{
    return std::string("none");
}

void fei_cap_i::set_rf_flow_id(const std::string& port_name, const std::string& id)
{
}

frontend::RFInfoPkt fei_cap_i::get_rfinfo_pkt(const std::string& port_name)
{
    frontend::RFInfoPkt pkt;
    return pkt;
}

void fei_cap_i::set_rfinfo_pkt(const std::string& port_name, const frontend::RFInfoPkt &pkt)
{
    FRONTEND::RFInfoPkt_var tmp_pkt = returnRFInfoPkt(pkt);
    if (tmp_pkt->ext_path_delays.length() != 2) {
        throw FRONTEND::NotSupportedException("ext_path_delays needs to be length 2");
    }
    if (tmp_pkt->capabilities.freq_range.min_val!=1) {
        throw FRONTEND::NotSupportedException("capabilities.freq_range.min_val needs to be 1");
    }
    if (tmp_pkt->capabilities.bw_range.min_val!=3) {
        throw FRONTEND::NotSupportedException("capabilities.bw_range.min_val needs to be 3");
    }
    if (tmp_pkt->additional_info.length()!=3) {
        throw FRONTEND::NotSupportedException("additional_info.length() needs to be 3");
    }
    //Make sure the sensor is populated correctly
    FRONTEND::SensorInfo_var tempSensor = new FRONTEND::SensorInfo();
    tempSensor->feed.name = "testFeed";
    tempSensor->feed.polarization = "testPol";
    tempSensor->feed.freq_range.min_val = 0;
    tempSensor->feed.freq_range.max_val = 100;
    tempSensor->feed.freq_range.values.length(1);
    tempSensor->feed.freq_range.values[0] = 1;
    tempSensor->antenna.name = "testAnt";
    tempSensor->antenna.type = "testType";
    tempSensor->antenna.size = "testSize";
    tempSensor->antenna.description = "testDescription";
    tempSensor->mission = "testMission";
    tempSensor->collector = "testCollector";
    tempSensor->rx = "testRX";
    if (std::string(tmp_pkt->sensor.feed.name) != std::string(tempSensor->feed.name)){

        throw FRONTEND::NotSupportedException("SensorInfo feed name not passed");
    }
    if (std::string(tmp_pkt->sensor.feed.polarization) != std::string(tempSensor->feed.polarization)){

        throw FRONTEND::NotSupportedException("SensorInfo feed polarization not passed");
    }
    if (tmp_pkt->sensor.feed.freq_range.min_val != tempSensor->feed.freq_range.min_val){

        throw FRONTEND::NotSupportedException("SensorInfo feed freq_range min not passed");
    }
    if (tmp_pkt->sensor.feed.freq_range.max_val != tempSensor->feed.freq_range.max_val){

        throw FRONTEND::NotSupportedException("SensorInfo feed freq_range max not passed");
    }
    if (tmp_pkt->sensor.feed.freq_range.values.length() != 1){

        throw FRONTEND::NotSupportedException("SensorInfo feed freq_range values not passed");
    }
    if (std::string(tmp_pkt->sensor.antenna.name) != std::string(tempSensor->antenna.name)){

        throw FRONTEND::NotSupportedException("SensorInfo antenna name not passed");
    }
    if (std::string(tmp_pkt->sensor.antenna.type) != std::string(tempSensor->antenna.type)){

        throw FRONTEND::NotSupportedException("SensorInfo antenna type not passed");
    }
    if (std::string(tmp_pkt->sensor.antenna.size) != std::string(tempSensor->antenna.size)){

        throw FRONTEND::NotSupportedException("SensorInfo antenna size not passed");
    }
    if (std::string(tmp_pkt->sensor.antenna.description) != std::string(tempSensor->antenna.description)){

        throw FRONTEND::NotSupportedException("SensorInfo antenna description not passed");
    }
    if (std::string(tmp_pkt->sensor.mission) != std::string(tempSensor->mission)){

        throw FRONTEND::NotSupportedException("SensorInfo mission not passed");
    }
    if (std::string(tmp_pkt->sensor.collector) != std::string(tempSensor->collector)){

        throw FRONTEND::NotSupportedException("SensorInfo collector not passed");
    }
    if (std::string(tmp_pkt->sensor.rx) != std::string(tempSensor->rx)){

        throw FRONTEND::NotSupportedException("SensorInfo rx not passed");
    }


    if (tmp_pkt->spectrum_inverted){

        throw FRONTEND::NotSupportedException("spectrum_inverted not passed");
    }
    if (tmp_pkt->if_center_freq != 412.0){

        throw FRONTEND::NotSupportedException("if_center_freq not passed");
    }
    if (tmp_pkt->rf_bandwidth != 101.0){

        throw FRONTEND::NotSupportedException("rf_bandwidth not passed");
    }
    if (tmp_pkt->rf_center_freq != 256.0){

        throw FRONTEND::NotSupportedException("rf_center_freq not passed");
    }
    std::string tempString = "TestID";
    if ( tempString.compare(tmp_pkt->rf_flow_id._retn()) != 0){

        throw FRONTEND::NotSupportedException("rf_flow_id not passed");
    }
}
