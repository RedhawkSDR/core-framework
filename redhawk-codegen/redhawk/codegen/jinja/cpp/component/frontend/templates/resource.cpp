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
     The options for devices are: TX, RX, RX_DIGITIZER, CHANNELIZER, DDC, RX_DIGITIZER_CHANNELIZER
     
     For example, if this device has 5 physical
     tuners, 3 RX_DIGITIZER and 2 CHANNELIZER, then the code in the construct function 
     should look like this:

     this->addChannels(3, "RX_DIGITIZER");
     this->addChannels(2, "CHANNELIZER");
     
     The incoming request for tuning contains a string describing the requested tuner
     type. The string for the request must match the string in the tuner status.
/*{% endif %}*/
    ***********************************************************************************/
/*{% if 'FrontendTuner' in component.implements %}*/
    this->addChannels(1, "RX_DIGITIZER");
/*{% endif %}*/
/*{% endblock %}*/

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
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].tuner_type;
}

bool ${className}::getTunerDeviceControl(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if (getControlAllocationId(idx) == allocation_id)
        return true;
    return false;
}

std::string ${className}::getTunerGroupId(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].group_id;
}

std::string ${className}::getTunerRfFlowId(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].rf_flow_id;
}
/*{% endif %}*/
/*{% if 'AnalogTuner' in component.implements %}*/

void ${className}::setTunerCenterFrequency(const std::string& allocation_id, double freq) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (freq<0) throw FRONTEND::BadParameterException("Center frequency cannot be less than 0");
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[idx].center_frequency = freq;
}

double ${className}::getTunerCenterFrequency(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].center_frequency;
}

void ${className}::setTunerBandwidth(const std::string& allocation_id, double bw) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (bw<0) throw FRONTEND::BadParameterException("Bandwidth cannot be less than 0");
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[idx].bandwidth = bw;
}

double ${className}::getTunerBandwidth(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].bandwidth;
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
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[idx].enabled = enable;
}

bool ${className}::getTunerEnable(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].enabled;
}
/*{% endif %}*/
/*{% if 'DigitalTuner' in component.implements %}*/

void ${className}::setTunerOutputSampleRate(const std::string& allocation_id, double sr) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (sr<0) throw FRONTEND::BadParameterException("Sample rate cannot be less than 0");
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[idx].sample_rate = sr;
}

double ${className}::getTunerOutputSampleRate(const std::string& allocation_id){
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    return frontend_tuner_status[idx].sample_rate;
}
/*{% endif %}*/
/*{% if 'ScanningTuner' in component.implements %}*/
frontend::ScanStatus ${className}::getScanStatus(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    frontend::ManualStrategy* tmp = new frontend::ManualStrategy(0);
    frontend::ScanStatus retval(tmp);
    return retval;
}

void ${className}::setScanStartTime(const std::string& allocation_id, const BULKIO::PrecisionUTCTime& start_time) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
}

void ${className}::setScanStrategy(const std::string& allocation_id, const frontend::ScanStrategy* scan_strategy) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::FrontendException("Invalid allocation id");
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
}
/*{% endif %}*/
/*{% if 'GPS' in component.implements %}*/

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
