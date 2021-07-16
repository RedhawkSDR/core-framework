/*#
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
 #*/
//% extends "pull/resource.cpp"
/*{% block constructorBody %}*/
    /***********************************************************************************
     This is the RH constructor. All properties are properly initialized before this function is called 
/*{% if 'FrontendTuner' in component.implements %}*/

     For a tuner device, the structure frontend_tuner_status needs to match the number
     of tuners that this device controls and what kind of device it is.
     The options for devices are: ANTENNA, RX, RX_ARRAY, DBOT, ABOT, ARDC, RDC, SRDC, DRDC, TX, TX_ARRAY, TDC
     
     An example of setting up this device as an ABOT would look like this:

     this->addChannels(1, "ABOT");
     
     The incoming request for tuning contains a string describing the requested tuner
     type. The string for the request must match the string in the tuner status.
/*{% endif %}*/
    ***********************************************************************************/
/*{% if 'FrontendTuner' in component.implements %}*/
    this->addChannels(1, "ABOT");
/*{% endif %}*/
/*{% endblock %}*/

/*{% if 'FrontendTuner' in component.implements %}*/
/*{%   block allocateBlock %}*/
CF::Device::Allocations* ${className}::allocate (const CF::Properties& capacities) {
    CF::Device::Allocations_var result = new CF::Device::Allocations();
    result = ${baseClass}::allocate(capacities);
    /*
     * Add data and control ports to response if length is greater than 0
     */
    return result._retn();
}
/*{%   endblock %}*/
/*{% endif %}*/

/*{% block updateUsageState %}*/
/*{%   for sc in component.superclasses if sc.name == "Device_impl" %}*/
${super()}
/*{%-  endfor %}*/
/*{% endblock %}*/

/*{% block extensions %}*/
/*{% if 'FrontendTuner' in component.implements %}*/

/*************************************************************
Functions supporting tuning allocation
*************************************************************/
void ${className}::deviceEnable(frontend_tuner_status_struct_struct &fts, size_t tuner_id){
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    Make sure to set the 'enabled' member of fts to indicate that tuner as enabled
    ************************************************************/
    #warning deviceEnable(): Enable the given tuner  *********
    fts.enabled = true;
    return;
}
void ${className}::deviceDisable(frontend_tuner_status_struct_struct &fts, size_t tuner_id){
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    Make sure to reset the 'enabled' member of fts to indicate that tuner as disabled
    ************************************************************/
    #warning deviceDisable(): Disable the given tuner  *********
    fts.enabled = false;
    return;
}
/*{% if 'ScanningTuner' in component.implements %}*/
bool ${className}::deviceSetTuningScan(const frontend::frontend_tuner_allocation_struct &request, const frontend::frontend_scanner_allocation_struct &scan_request, frontend_tuner_status_struct_struct &fts, size_t tuner_id){
    /************************************************************

    This function is called when the allocation request contains a scanner allocation

    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
      At a minimum, bandwidth, center frequency, and sample_rate have to be set
      If the device is tuned to exactly what the request was, the code should be:
        fts.bandwidth = request.bandwidth;
        fts.center_frequency = request.center_frequency;
        fts.sample_rate = request.sample_rate;

    return true if the tuning succeeded, and false if it failed
    ************************************************************/
    #warning deviceSetTuning(): Evaluate whether or not a tuner is added  *********
    return true;
}
/*{% endif %}*/
bool ${className}::deviceSetTuning(const frontend::frontend_tuner_allocation_struct &request, frontend_tuner_status_struct_struct &fts, size_t tuner_id){
    /************************************************************
/*{% if 'ScanningTuner' in component.implements %}*/

    This function is called when the allocation request does not contain a scanner allocation

/*{% endif %}*/
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
      At a minimum, bandwidth, center frequency, and sample_rate have to be set
      If the device is tuned to exactly what the request was, the code should be:
        fts.bandwidth = request.bandwidth;
        fts.center_frequency = request.center_frequency;
        fts.sample_rate = request.sample_rate;

    return true if the tuning succeeded, and false if it failed
    ************************************************************/
    #warning deviceSetTuning(): Evaluate whether or not a tuner is added  *********
    return true;
}
bool ${className}::deviceDeleteTuning(frontend_tuner_status_struct_struct &fts, size_t tuner_id) {
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    return true if the tune deletion succeeded, and false if it failed
    ************************************************************/
    #warning deviceDeleteTuning(): Deallocate an allocated tuner  *********
    return true;
}
/*{% endif %}*/
/*{% endblock %}*/

/*{% block fei_port_delegations %}*/
/*{% if 'FrontendTuner' in component.implements %}*/
/*************************************************************
Functions servicing the tuner control port
*************************************************************/
std::string ${className}::getTunerType(const std::string& allocation_id) {
    return frontend_tuner_status[0].tuner_type;
}

bool ${className}::getTunerDeviceControl(const std::string& allocation_id) {
    return true;
}

std::string ${className}::getTunerGroupId(const std::string& allocation_id) {
    return frontend_tuner_status[0].group_id;
}

std::string ${className}::getTunerRfFlowId(const std::string& allocation_id) {
    return frontend_tuner_status[0].rf_flow_id;
}
/*{% endif %}*/
/*{% if 'AnalogTuner' in component.implements %}*/

void ${className}::setTunerCenterFrequency(const std::string& allocation_id, double freq) {
    if (freq<0) throw FRONTEND::BadParameterException("Center frequency cannot be less than 0");
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[0].center_frequency = freq;
}

double ${className}::getTunerCenterFrequency(const std::string& allocation_id) {
    return frontend_tuner_status[0].center_frequency;
}

void ${className}::setTunerBandwidth(const std::string& allocation_id, double bw) {
    if (bw<0) throw FRONTEND::BadParameterException("Bandwidth cannot be less than 0");
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[0].bandwidth = bw;
}

double ${className}::getTunerBandwidth(const std::string& allocation_id) {
    return frontend_tuner_status[0].bandwidth;
}

void ${className}::setTunerAgcEnable(const std::string& allocation_id, bool enable)
{
    throw FRONTEND::NotSupportedException("setTunerAgcEnable not supported");
}

bool ${className}::getTunerAgcEnable(const std::string& allocation_id)
{
    throw FRONTEND::NotSupportedException("getTunerAgcEnable not supported");
}

void ${className}::setTunerGain(const std::string& allocation_id, float gain)
{
    throw FRONTEND::NotSupportedException("setTunerGain not supported");
}

float ${className}::getTunerGain(const std::string& allocation_id)
{
    throw FRONTEND::NotSupportedException("getTunerGain not supported");
}

void ${className}::setTunerReferenceSource(const std::string& allocation_id, long source)
{
    throw FRONTEND::NotSupportedException("setTunerReferenceSource not supported");
}

long ${className}::getTunerReferenceSource(const std::string& allocation_id)
{
    throw FRONTEND::NotSupportedException("getTunerReferenceSource not supported");
}

void ${className}::setTunerEnable(const std::string& allocation_id, bool enable) {
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[0].enabled = enable;
}

bool ${className}::getTunerEnable(const std::string& allocation_id) {
    return frontend_tuner_status[0].enabled;
}
/*{% endif %}*/
/*{% if 'DigitalTuner' in component.implements %}*/

void ${className}::setTunerOutputSampleRate(const std::string& allocation_id, double sr) {
    if (sr<0) throw FRONTEND::BadParameterException("Sample rate cannot be less than 0");
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[0].sample_rate = sr;
}

double ${className}::getTunerOutputSampleRate(const std::string& allocation_id){
    return frontend_tuner_status[0].sample_rate;
}

void ${className}::configureTuner(const std::string& allocation_id, const CF::Properties& tunerSettings){
    // set the appropriate tuner settings
}

CF::Properties* ${className}::getTunerSettings(const std::string& allocation_id){
    // return the tuner settings
    redhawk::PropertyMap* tuner_settings = new redhawk::PropertyMap();
    return tuner_settings;
}
/*{% endif %}*/
/*{% if 'TransmitControl' in component.implements %}*/

void ${className}::reset(const std::string& allocation_id, const std::string& stream_id) {
}

bool ${className}::hold(const std::string& allocation_id, const std::string& stream_id) {
    return false;
}

std::vector<std::string> ${className}::held(const std::string& allocation_id, const std::string& stream_id) {
    std::vector<std::string> _held;
    return _held;
}

bool ${className}::allow(const std::string& allocation_id, const std::string& stream_id) {
    return false;
}

void ${className}::setTransmitParemeters(const std::string& allocation_id, const frontend::TransmitParameters& transmit_parameters) {
}

frontend::TransmitParameters ${className}::getTransmitParemeters(const std::string& allocation_id) {
    frontend::TransmitParameters transmit_parameters;
    return transmit_parameters;
}
/*{% endif %}*/
/*{% if 'ScanningTuner' in component.implements %}*/

frontend::ScanStatus ${className}::getScanStatus(const std::string& allocation_id) {
    frontend::ManualStrategy* tmp = new frontend::ManualStrategy(0);
    frontend::ScanStatus retval(tmp);
    return retval;
}

void ${className}::setScanStartTime(const std::string& allocation_id, const BULKIO::PrecisionUTCTime& start_time) {
}

void ${className}::setScanStrategy(const std::string& allocation_id, const frontend::ScanStrategy* scan_strategy) {
}
/*{% endif %}*/
/*{% if 'GPS' in component.implements %}*/    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());


frontend::GPSInfo ${className}::get_gps_info(const std::string& port_name)
{
    frontend::GPSInfo gps_info;
    return gps_info;
}

void ${className}::set_gps_info(const std::string& port_name, const frontend::GPSInfo &gps_info)
{
}

frontend::GpsTimePos ${className}::get_gps_time_pos(const std::string& port_name)
{
    frontend::GpsTimePos gps_time_pos;
    return gps_time_pos;
}

void ${className}::set_gps_time_pos(const std::string& port_name, const frontend::GpsTimePos &gps_time_pos)
{
}
/*{% endif %}*/
/*{% if 'NavData' in component.implements %}*/

frontend::NavigationPacket ${className}::get_nav_packet(const std::string& port_name)
{
    frontend::NavigationPacket nav_info;
    return nav_info;
}

void ${className}::set_nav_packet(const std::string& port_name, const frontend::NavigationPacket &nav_info)
{
}
/*{% endif %}*/
/*{% if 'RFInfo' in component.implements %}*/

/*************************************************************
Functions servicing the RFInfo port(s)
- port_name is the port over which the call was received
*************************************************************/
std::string ${className}::get_rf_flow_id(const std::string& port_name)
{
    return std::string("none");
}

void ${className}::set_rf_flow_id(const std::string& port_name, const std::string& id)
{
}

frontend::RFInfoPkt ${className}::get_rfinfo_pkt(const std::string& port_name)
{
    frontend::RFInfoPkt pkt;
    return pkt;
}

void ${className}::set_rfinfo_pkt(const std::string& port_name, const frontend::RFInfoPkt &pkt)
{
}
/*{% endif %}*/
/*{% if 'RFSource' in component.implements %}*/

std::vector<frontend::RFInfoPkt> ${className}::get_available_rf_inputs(const std::string& port_name)
{
    std::vector<frontend::RFInfoPkt> inputs;
    return inputs;
}

void ${className}::set_available_rf_inputs(const std::string& port_name, const std::vector<frontend::RFInfoPkt> &inputs)
{
}

frontend::RFInfoPkt ${className}::get_current_rf_input(const std::string& port_name)
{
    frontend::RFInfoPkt pkt;
    return pkt;
}

void ${className}::set_current_rf_input(const std::string& port_name, const frontend::RFInfoPkt &pkt)
{
}
/*{% endif %}*/
/*{% endblock %}*/
