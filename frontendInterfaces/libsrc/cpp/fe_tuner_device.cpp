/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK frontendInterfaces.
 *
 * REDHAWK frontendInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK frontendInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include "fe_tuner_device.h"
#include <exception>

namespace frontend {
    template < typename TunerStatusStructType > 
      PREPARE_ALT_LOGGING(FrontendTunerDevice<TunerStatusStructType>, FrontendTunerDevice );


    /* validateRequestVsSRI is a helper function to check that the input data stream can support
     * the allocation request. The output mode (true if complex output) is used when determining
     * the necessary sample rate required to satisfy the request. The entire frequency band of the
     * request must be available for True to be returned, not just the center frequency.
     * True is returned upon success, otherwise FRONTEND::BadParameterException is thrown.
     * If the CHAN_RF and FRONTEND::BANDWIDTH keywords are not found in the sri,
     * FRONTEND::BadParameterException is thrown.
     */
    bool validateRequestVsSRI(const frontend_tuner_allocation_struct& request, const BULKIO::StreamSRI& upstream_sri, bool output_mode){

        // get center frequency and bandwidth from SRI keywords
        double upstream_cf, upstream_bw;
        bool found_cf(false), found_bw(false);
        unsigned long key_size = upstream_sri.keywords.length();
        for (unsigned int i = 0; i < key_size; i++) {
            if (!strcmp(upstream_sri.keywords[i].id, "CHAN_RF")) {
                if (upstream_sri.keywords[i].value >>= upstream_cf) found_cf = true;
            } else if (!strcmp(upstream_sri.keywords[i].id, "FRONTEND::BANDWIDTH")) {
                if (upstream_sri.keywords[i].value >>= upstream_bw) found_bw = true;
            }
        }
        if(!found_cf || !found_bw){
            throw FRONTEND::BadParameterException("CANNOT VERIFY REQUEST -- SRI missing required keywords");
        }

        // check bandwidth
        double min_upstream_freq = upstream_cf-(upstream_bw/2);
        double max_upstream_freq = upstream_cf+(upstream_bw/2);
        double min_requested_freq = request.center_frequency-(request.bandwidth/2);
        double max_requested_freq = request.center_frequency+(request.bandwidth/2);

        if( !validateRequest(min_upstream_freq,max_upstream_freq,min_requested_freq,max_requested_freq) ){
            throw FRONTEND::BadParameterException("INVALID REQUEST -- input data stream cannot support freq/bw request");
        }

        // check sample rate
        double upstream_sr = 1/upstream_sri.xdelta;
        size_t input_scaling_factor = (upstream_sri.mode) ? 2 : 4; // adjust for complex data
        min_upstream_freq = upstream_cf-(upstream_sr/input_scaling_factor);
        max_upstream_freq = upstream_cf+(upstream_sr/input_scaling_factor);
        size_t output_scaling_factor = (output_mode) ? 2 : 4; // adjust for complex data
        min_requested_freq = request.center_frequency-(request.sample_rate/output_scaling_factor);
        max_requested_freq = request.center_frequency+(request.sample_rate/output_scaling_factor);

        if ( !validateRequest(min_upstream_freq,max_upstream_freq,min_requested_freq,max_requested_freq) ){
            throw FRONTEND::BadParameterException("INVALID REQUEST -- input data stream cannot support freq/sr request");
        }
        return true;
    }

    /* validateRequestVsDevice is a helper function to check that the input data stream and the
     * device can support an allocation request. The output mode (true if complex output) is used
     * when determining the necessary sample rate required to satisfy the request. The entire
     * frequency band of the request must be available for True to be returned, not just the center
     * frequency.
     * True is returned upon success, otherwise FRONTEND::BadParameterException is thrown.
     * If the CHAN_RF and FRONTEND::BANDWIDTH keywords are not found in the sri,
     * FRONTEND::BadParameterException is thrown.
     */
    bool validateRequestVsDevice(const frontend_tuner_allocation_struct& request, const BULKIO::StreamSRI& upstream_sri,
            bool output_mode, double min_device_center_freq, double max_device_center_freq, double max_device_bandwidth, double max_device_sample_rate){

        // check if request can be satisfied using the available upstream data
        if( !validateRequestVsSRI(request,upstream_sri, output_mode) ){
            throw FRONTEND::BadParameterException("INVALID REQUEST -- falls outside of input data stream");
        }

        // check device constraints

        // check vs. device center frequency capability (ensure 0 <= request <= max device capability)
        if ( !validateRequest(min_device_center_freq,max_device_center_freq,request.center_frequency) ){
            throw FRONTEND::BadParameterException("INVALID REQUEST -- device capabilities cannot support freq request");
        }

        // check vs. device bandwidth capability (ensure 0 <= request <= max device capability)
        if ( !validateRequest(0,max_device_bandwidth,request.bandwidth) ){
            throw FRONTEND::BadParameterException("INVALID REQUEST -- device capabilities cannot support bw request");
        }

        // check vs. device sample rate capability (ensure 0 <= request <= max device capability)
        if ( !validateRequest(0,max_device_sample_rate,request.sample_rate) ){
            throw FRONTEND::BadParameterException("INVALID REQUEST -- device capabilities cannot support sr request");
        }

        // calculate overall frequency range of the device (not just CF range)
        const size_t output_scaling_factor = (output_mode) ? 2 : 4; // adjust for complex data
        const double min_device_freq = min_device_center_freq-(max_device_sample_rate/output_scaling_factor);
        const double max_device_freq = max_device_center_freq+(max_device_sample_rate/output_scaling_factor);

        // check based on bandwidth
        // this duplicates part of check above if device freq range = input freq range
        double min_requested_freq = request.center_frequency-(request.bandwidth/2);
        double max_requested_freq = request.center_frequency+(request.bandwidth/2);
        if ( !validateRequest(min_device_freq,max_device_freq,min_requested_freq,max_requested_freq) ){
            throw FRONTEND::BadParameterException("INVALID REQUEST -- device capabilities cannot support freq/bw request");
        }

        // check based on sample rate
        // this duplicates part of check above if device freq range = input freq range
        min_requested_freq = request.center_frequency-(request.sample_rate/output_scaling_factor);
        max_requested_freq = request.center_frequency+(request.sample_rate/output_scaling_factor);
        if ( !validateRequest(min_device_freq,max_device_freq,min_requested_freq,max_requested_freq) ){
            throw FRONTEND::BadParameterException("INVALID REQUEST -- device capabilities cannot support freq/sr request");
        }

        return true;
    }

    /* validateRequestVsRFInfo is a helper function to check that the analog capabilities can support
     * the allocation request. The mode (true if complex) is used when determining the necessary
     * sample rate required to satisfy the request. The entire frequency band of the request must be
     * available for True to be returned, not just the center frequency.
     * True is returned upon success, otherwise FRONTEND::BadParameterException is thrown.
     */
    bool validateRequestVsRFInfo(const frontend_tuner_allocation_struct& request, const frontend::RFInfoPkt& rfinfo, bool mode){

        double min_analog_freq = rfinfo.rf_center_freq-(rfinfo.rf_bandwidth/2);
        double max_analog_freq = rfinfo.rf_center_freq+(rfinfo.rf_bandwidth/2);

        // check bandwidth
        double min_requested_freq = request.center_frequency-(request.bandwidth/2);
        double max_requested_freq = request.center_frequency+(request.bandwidth/2);

        if ( !validateRequest(min_analog_freq,max_analog_freq,min_requested_freq,max_requested_freq) ){
            throw FRONTEND::BadParameterException("INVALID REQUEST -- analog freq range (RFinfo) cannot support freq/bw request");
        }

        // check sample rate
        size_t scaling_factor = (mode) ? 2 : 4; // adjust for complex data
        min_requested_freq = request.center_frequency-(request.sample_rate/scaling_factor);
        max_requested_freq = request.center_frequency+(request.sample_rate/scaling_factor);

        if ( !validateRequest(min_analog_freq,max_analog_freq,min_requested_freq,max_requested_freq) ){
            throw FRONTEND::BadParameterException("INVALID REQUEST -- analog freq range (RFinfo) cannot support freq/sr request");
        }
        return true;
    }

    /* validateRequestVsDevice is a helper function to check that the analog capabilities and the
     * device can support the allocation request. The mode (true if complex) is used when
     * determining the necessary sample rate required to satisfy the request. The entire frequency
     * band of the request must be available for True to be returned, not just the center frequency.
     * True is returned upon success, otherwise FRONTEND::BadParameterException is thrown.
     */
    bool validateRequestVsDevice(const frontend_tuner_allocation_struct& request, const frontend::RFInfoPkt& rfinfo,
            bool mode, double min_device_center_freq, double max_device_center_freq, double max_device_bandwidth, double max_device_sample_rate){

        // check if request can be satisfied using the available upstream data
        if( request.tuner_type != "TX" && !validateRequestVsRFInfo(request,rfinfo, mode) ){
            throw FRONTEND::BadParameterException("INVALID REQUEST -- analog freq range (RFinfo) cannot support request");
        }

        // check device constraints
        // see if IF center frequency is set in rfinfo packet
        double request_if_center_freq = request.center_frequency;
        if(request.tuner_type != "TX" && floatingPointCompare(rfinfo.if_center_freq,0) > 0 && floatingPointCompare(rfinfo.rf_center_freq,rfinfo.if_center_freq) > 0)
            request_if_center_freq = request.center_frequency - (rfinfo.rf_center_freq-rfinfo.if_center_freq);

        // check vs. device center freq capability (ensure 0 <= request <= max device capability)
        if ( !validateRequest(min_device_center_freq,max_device_center_freq,request_if_center_freq) ) {
            throw FRONTEND::BadParameterException("INVALID REQUEST -- device capabilities cannot support freq request");
        }

        // check vs. device bandwidth capability (ensure 0 <= request <= max device capability)
        if ( !validateRequest(0,max_device_bandwidth,request.bandwidth) ){
            throw FRONTEND::BadParameterException("INVALID REQUEST -- device capabilities cannot support bw request");
        }

        // check vs. device sample rate capability (ensure 0 <= request <= max device capability)
        if ( !validateRequest(0,max_device_sample_rate,request.sample_rate) ){
            throw FRONTEND::BadParameterException("INVALID REQUEST -- device capabilities cannot support sr request");
        }

        // calculate overall frequency range of the device (not just CF range)
        const size_t scaling_factor = (mode) ? 2 : 4; // adjust for complex data
        const double min_device_freq = min_device_center_freq-(max_device_sample_rate/scaling_factor);
        const double max_device_freq = max_device_center_freq+(max_device_sample_rate/scaling_factor);

        // check based on bandwidth
        double min_requested_freq = request_if_center_freq-(request.bandwidth/2);
        double max_requested_freq = request_if_center_freq+(request.bandwidth/2);

        if ( !validateRequest(min_device_freq,max_device_freq,min_requested_freq,max_requested_freq) ) {
            throw FRONTEND::BadParameterException("INVALID REQUEST -- device capabilities cannot support freq/bw request");
        }

        // check based on sample rate
        min_requested_freq = request_if_center_freq-(request.sample_rate/scaling_factor);
        max_requested_freq = request_if_center_freq+(request.sample_rate/scaling_factor);

        if ( !validateRequest(min_device_freq,max_device_freq,min_requested_freq,max_requested_freq) ){
            throw FRONTEND::BadParameterException("INVALID REQUEST -- device capabilities cannot support freq/sr request");
        }

        return true;
    }

    template < typename TunerStatusStructType >
    FrontendTunerDevice<TunerStatusStructType>::FrontendTunerDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
        Device_impl(devMgr_ior, id, lbl, sftwrPrfl)
    {
        construct();
    }

    template < typename TunerStatusStructType >
    FrontendTunerDevice<TunerStatusStructType>::FrontendTunerDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
        Device_impl(devMgr_ior, id, lbl, sftwrPrfl, compDev)
    {
        construct();
    }

    template < typename TunerStatusStructType >
    FrontendTunerDevice<TunerStatusStructType>::FrontendTunerDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
        Device_impl(devMgr_ior, id, lbl, sftwrPrfl, capacities)
    {
        construct();
    }

    template < typename TunerStatusStructType >
    FrontendTunerDevice<TunerStatusStructType>::FrontendTunerDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
        Device_impl(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev)
    {
        construct();
    }

    template < typename TunerStatusStructType >
    void FrontendTunerDevice<TunerStatusStructType>::construct()
    {
        Resource_impl::_started = false;
        loadProperties();
    }

    template < typename TunerStatusStructType >
    FrontendTunerDevice<TunerStatusStructType>::~FrontendTunerDevice()
    {
        tuner_allocation_ids.clear();
    }

    /*******************************************************************************************
        Framework-level functions
        These functions are generally called by the framework to perform housekeeping.
    *******************************************************************************************/

    template < typename TunerStatusStructType >
    void FrontendTunerDevice<TunerStatusStructType>::loadProperties()
    {
        addProperty(device_kind,
                    "FRONTEND::TUNER",
                    "DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d",
                    "device_kind",
                    "readonly",
                    "",
                    "eq",
                    "allocation,configure");

        addProperty(device_model,
                    "",
                    "DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb",
                    "device_model",
                    "readonly",
                    "",
                    "eq",
                    "allocation,configure");

        addProperty(frontend_tuner_allocation,
                    frontend::frontend_tuner_allocation_struct(),
                    "FRONTEND::tuner_allocation",
                    "frontend_tuner_allocation",
                    "readwrite",
                    "",
                    "external",
                    "allocation");

        addProperty(frontend_listener_allocation,
                    frontend::frontend_listener_allocation_struct(),
                    "FRONTEND::listener_allocation",
                    "frontend_listener_allocation",
                    "readwrite",
                    "",
                    "external",
                    "allocation");

        addProperty(frontend_tuner_status,
                    "FRONTEND::tuner_status",
                    "frontend_tuner_status",
                    "readonly",
                    "",
                    "external",
                    "configure");

    }

    template < typename TunerStatusStructType >
    std::string FrontendTunerDevice<TunerStatusStructType>::createAllocationIdCsv(size_t tuner_id){
        //LOG_TRACE(FrontendTunerDevice<TunerStatusStructType>,__PRETTY_FUNCTION__);
        std::string alloc_id_csv = "";
        // ensure control allocation_id is first in list
        if (!tuner_allocation_ids[tuner_id].control_allocation_id.empty())
            alloc_id_csv = tuner_allocation_ids[tuner_id].control_allocation_id + ",";
        std::vector<std::string>::iterator it = tuner_allocation_ids[tuner_id].listener_allocation_ids.begin();
        for(; it != tuner_allocation_ids[tuner_id].listener_allocation_ids.end(); it++)
            alloc_id_csv += *it + ",";
        if(!alloc_id_csv.empty())
            alloc_id_csv.erase(alloc_id_csv.size()-1);
        return alloc_id_csv;
    }

    template < typename TunerStatusStructType >
    std::string FrontendTunerDevice<TunerStatusStructType>::getControlAllocationId(size_t tuner_id){
        return tuner_allocation_ids[tuner_id].control_allocation_id;
    }
    template < typename TunerStatusStructType >
    std::vector<std::string> FrontendTunerDevice<TunerStatusStructType>::getListenerAllocationIds(size_t tuner_id){
        return tuner_allocation_ids[tuner_id].listener_allocation_ids;
    }

    /*****************************************************************/
    /* Allocation/Deallocation of Capacity                           */
    /*****************************************************************/
    template < typename TunerStatusStructType >
    CF::Device::UsageType FrontendTunerDevice<TunerStatusStructType>::updateUsageState() {
        //LOG_TRACE(FrontendTunerDevice<TunerStatusStructType>,__PRETTY_FUNCTION__);
        size_t tunerAllocated = 0;
        for (size_t tuner_id = 0; tuner_id < tuner_allocation_ids.size(); tuner_id++) {
            if (!tuner_allocation_ids[tuner_id].control_allocation_id.empty())
                tunerAllocated++;
        }
        // If no tuners are allocated, device is idle
        if (tunerAllocated == 0)
            return CF::Device::IDLE;
        // If all tuners are allocated, device is busy
        if (tunerAllocated == tuner_allocation_ids.size())
            return CF::Device::BUSY;
        // Else, device is active
        return CF::Device::ACTIVE;
    }

    template < typename TunerStatusStructType >
    CORBA::Boolean FrontendTunerDevice<TunerStatusStructType>::allocateCapacity(const CF::Properties & capacities)
    throw (CORBA::SystemException, CF::Device::InvalidCapacity, CF::Device::InvalidState) {
        if (this->tuner_allocation_ids.size() != this->frontend_tuner_status.size()) {
            this->tuner_allocation_ids.resize(this->frontend_tuner_status.size());
        }
        LOG_TRACE(FrontendTunerDevice<TunerStatusStructType>,__PRETTY_FUNCTION__);
        CORBA::ULong ii;
        try{
            for (ii = 0; ii < capacities.length(); ++ii) {
                const std::string id = (const char*) capacities[ii].id;
                if (id != "FRONTEND::tuner_allocation" && id != "FRONTEND::listener_allocation"){
                    LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>, "UNKNOWN ALLOCATION PROPERTY1");
                    throw CF::Device::InvalidCapacity("UNKNOWN ALLOCATION PROPERTY1", capacities);
                }
                PropertyInterface* property = getPropertyFromId(id);
                if(!property){
                    LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>, "UNKNOWN PROPERTY");
                    throw CF::Device::InvalidCapacity("UNKNOWN PROPERTY", capacities);
                }
                try{
                    property->setValue(capacities[ii].value);
                }
                catch(const std::logic_error &e){
                    LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>, "COULD NOT PARSE CAPACITY: " << e.what());
                    throw CF::Device::InvalidCapacity("COULD NOT PARSE CAPACITY", capacities);
                };
                if (id == "FRONTEND::tuner_allocation"){
                    // Check allocation_id
                    if (frontend_tuner_allocation.allocation_id.empty()) {
                        LOG_INFO(FrontendTunerDevice<TunerStatusStructType>,"allocateCapacity: MISSING ALLOCATION_ID");
                        throw CF::Device::InvalidCapacity("MISSING ALLOCATION_ID", capacities);
                    }
                    // Check if allocation ID has already been used
                    if(getTunerMapping(frontend_tuner_allocation.allocation_id) >= 0){
                        LOG_INFO(FrontendTunerDevice<TunerStatusStructType>,"allocateCapacity: ALLOCATION_ID ALREADY IN USE: [" << frontend_tuner_allocation.allocation_id << "]");
                        throw AllocationAlreadyExists("ALLOCATION_ID ALREADY IN USE", capacities);
                    }

                    // Check if available tuner
                    exclusive_lock lock(allocation_id_mapping_lock);

                    // Next, try to allocate a new tuner
                    for (size_t tuner_id = 0; tuner_id < tuner_allocation_ids.size(); tuner_id++) {
                        if(frontend_tuner_status[tuner_id].tuner_type != frontend_tuner_allocation.tuner_type) {
                            LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>,
                              "allocateCapacity: Requested tuner type '"<<frontend_tuner_allocation.tuner_type <<"' does not match tuner[" << tuner_id << "].tuner_type ("<<frontend_tuner_status[tuner_id].tuner_type<<")");
                            continue;
                        }

                        if(!frontend_tuner_allocation.group_id.empty() && frontend_tuner_allocation.group_id != frontend_tuner_status[tuner_id].group_id ){
                            LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>,
                              "allocateCapacity: Requested group_id '"<<frontend_tuner_allocation.group_id <<"' does not match tuner[" << tuner_id << "].group_id ("<<frontend_tuner_status[tuner_id].group_id<<")");
                            continue;
                        }

                        // special case because allocation is specifying the input stream, which determines the rf_flow_id, etc.
                        if(!frontend_tuner_allocation.rf_flow_id.empty()
                            && frontend_tuner_allocation.rf_flow_id != frontend_tuner_status[tuner_id].rf_flow_id
                            && frontend_tuner_allocation.tuner_type != "CHANNELIZER"){
                            LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>,
                              "allocateCapacity: Requested rf_flow_id '"<<frontend_tuner_allocation.rf_flow_id <<"' does not match tuner[" << tuner_id << "].rf_flow_id ("<<frontend_tuner_status[tuner_id].rf_flow_id<<")");
                            continue;
                        }

                        if(frontend_tuner_allocation.device_control){
                            double orig_bw = frontend_tuner_status[tuner_id].bandwidth;
                            double orig_cf = frontend_tuner_status[tuner_id].center_frequency;
                            double orig_sr = frontend_tuner_status[tuner_id].sample_rate;
                            // pre-load frontend_tuner_status values (just in case the request is filled but the values are not populated)
                            frontend_tuner_status[tuner_id].bandwidth = frontend_tuner_allocation.bandwidth;
                            frontend_tuner_status[tuner_id].center_frequency = frontend_tuner_allocation.center_frequency;
                            frontend_tuner_status[tuner_id].sample_rate = frontend_tuner_allocation.sample_rate;
                            // device control
                            if(!tuner_allocation_ids[tuner_id].control_allocation_id.empty() || !deviceSetTuning(frontend_tuner_allocation, frontend_tuner_status[tuner_id], tuner_id)){
                                if (frontend_tuner_status[tuner_id].bandwidth == frontend_tuner_allocation.bandwidth)
                                    frontend_tuner_status[tuner_id].bandwidth = orig_bw;
                                if (frontend_tuner_status[tuner_id].center_frequency == frontend_tuner_allocation.center_frequency)
                                    frontend_tuner_status[tuner_id].center_frequency = orig_cf;
                                if (frontend_tuner_status[tuner_id].sample_rate == frontend_tuner_allocation.sample_rate)
                                    frontend_tuner_status[tuner_id].sample_rate = orig_sr;
                                // either not available or didn't succeed setting tuning, try next tuner
                                LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>,
                                    "allocateCapacity: Tuner["<<tuner_id<<"] is either not available or didn't succeed while setting tuning ");
                                continue;
                            }
                            tuner_allocation_ids[tuner_id].control_allocation_id = frontend_tuner_allocation.allocation_id;
                            allocation_id_to_tuner_id.insert(std::pair<std::string, size_t > (frontend_tuner_allocation.allocation_id, tuner_id));
                            frontend_tuner_status[tuner_id].allocation_id_csv = createAllocationIdCsv(tuner_id);
                        } else {
                            // channelizer allocations must specify device control = true
                            if(frontend_tuner_allocation.tuner_type == "CHANNELIZER" || frontend_tuner_allocation.tuner_type == "TX"){
                                std::ostringstream eout;
                                eout<<frontend_tuner_allocation.tuner_type<<" allocation with device_control=false is invalid.";
                                LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>, eout.str());
                                throw CF::Device::InvalidCapacity(eout.str().c_str(), capacities);
                            }
                            // listener
                            if(tuner_allocation_ids[tuner_id].control_allocation_id.empty() || !listenerRequestValidation(frontend_tuner_allocation, tuner_id)){
                                // either not allocated or can't support listener request
                                LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>,
                                    "allocateCapacity: Tuner["<<tuner_id<<"] is either not available or can not support listener request ");
                                continue;
                            }
                            tuner_allocation_ids[tuner_id].listener_allocation_ids.push_back(frontend_tuner_allocation.allocation_id);
                            allocation_id_to_tuner_id.insert(std::pair<std::string, size_t > (frontend_tuner_allocation.allocation_id, tuner_id));
                            frontend_tuner_status[tuner_id].allocation_id_csv = createAllocationIdCsv(tuner_id);
                            this->assignListener(frontend_tuner_allocation.allocation_id,tuner_allocation_ids[tuner_id].control_allocation_id);
                        }
                        // if we've reached here, we found an eligible tuner with correct frequency

                        // check tolerances
                        // only check when sample_rate was not set to don't care)
                        LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>, std::fixed << " allocateCapacity - SR requested: " << frontend_tuner_allocation.sample_rate
                                                                                                           << "  SR got: " << frontend_tuner_status[tuner_id].sample_rate);
                        if( (floatingPointCompare(frontend_tuner_allocation.sample_rate,0)!=0) &&
                            (floatingPointCompare(frontend_tuner_status[tuner_id].sample_rate,frontend_tuner_allocation.sample_rate)<0 ||
                            floatingPointCompare(frontend_tuner_status[tuner_id].sample_rate,frontend_tuner_allocation.sample_rate+frontend_tuner_allocation.sample_rate * frontend_tuner_allocation.sample_rate_tolerance/100.0)>0 )){
                            std::ostringstream eout;
                            eout<<std::fixed<<"allocateCapacity("<<int(tuner_id)<<"): returned sr "<<frontend_tuner_status[tuner_id].sample_rate<<" does not meet tolerance criteria of "<<frontend_tuner_allocation.sample_rate_tolerance<<" percent";
                            LOG_INFO(FrontendTunerDevice<TunerStatusStructType>, eout.str());
                            throw std::logic_error(eout.str().c_str());
                        }
                        LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>, std::fixed << " allocateCapacity - BW requested: " << frontend_tuner_allocation.bandwidth
                                                                                                           << "  BW got: " << frontend_tuner_status[tuner_id].bandwidth);
                        // Only check when bandwidth was not set to don't care
                        if( (floatingPointCompare(frontend_tuner_allocation.bandwidth,0)!=0) &&
                            (floatingPointCompare(frontend_tuner_status[tuner_id].bandwidth,frontend_tuner_allocation.bandwidth)<0 ||
                            floatingPointCompare(frontend_tuner_status[tuner_id].bandwidth,frontend_tuner_allocation.bandwidth+frontend_tuner_allocation.bandwidth * frontend_tuner_allocation.bandwidth_tolerance/100.0)>0 )){
                            std::ostringstream eout;
                            eout<<std::fixed<<"allocateCapacity("<<int(tuner_id)<<"): returned bw "<<frontend_tuner_status[tuner_id].bandwidth<<" does not meet tolerance criteria of "<<frontend_tuner_allocation.bandwidth_tolerance<<" percent";
                            LOG_INFO(FrontendTunerDevice<TunerStatusStructType>, eout.str());
                            throw std::logic_error(eout.str().c_str());
                        }

                        if(frontend_tuner_allocation.device_control){
                            // enable tuner after successful allocation
                            try {
                                enableTuner(tuner_id,true);
                            } catch(...){
                                std::ostringstream eout;
                                eout<<"allocateCapacity: Failed to enable tuner after allocation";
                                LOG_INFO(FrontendTunerDevice<TunerStatusStructType>, eout.str());
                                throw std::logic_error(eout.str().c_str());
                            }
                        }
                        _usageState = updateUsageState();
                        return true;
                    }
                    // if we made it here, we failed to find an available tuner
                    std::ostringstream eout;
                    eout<<"allocateCapacity: NO AVAILABLE TUNER. Make sure that the device has an initialized frontend_tuner_status";
                    LOG_INFO(FrontendTunerDevice<TunerStatusStructType>, eout.str());
                    throw std::logic_error(eout.str().c_str());
                    
                } else if (id == "FRONTEND::listener_allocation") {
                    // Check validity of allocation_id's
                    if (frontend_listener_allocation.existing_allocation_id.empty()){
                        LOG_INFO(FrontendTunerDevice<TunerStatusStructType>,"allocateCapacity: MISSING EXISTING ALLOCATION ID");
                        throw CF::Device::InvalidCapacity("MISSING EXISTING ALLOCATION ID", capacities);
                    }
                    if (frontend_listener_allocation.listener_allocation_id.empty()){
                        LOG_INFO(FrontendTunerDevice<TunerStatusStructType>,"allocateCapacity: MISSING LISTENER ALLOCATION ID");
                        throw CF::Device::InvalidCapacity("MISSING LISTENER ALLOCATION ID", capacities);
                    }

                    exclusive_lock lock(allocation_id_mapping_lock);

                    // Check if listener allocation ID has already been used
                    if(getTunerMapping(frontend_listener_allocation.listener_allocation_id) >= 0){
                        LOG_INFO(FrontendTunerDevice<TunerStatusStructType>,"allocateCapacity: LISTENER ALLOCATION ID ALREADY IN USE: [" << frontend_listener_allocation.listener_allocation_id << "]");
                        throw AllocationAlreadyExists("LISTENER ALLOCATION ID ALREADY IN USE", capacities);
                    }
                    // Do not allocate if existing allocation ID does not exist
                    long tuner_id = getTunerMapping(frontend_listener_allocation.existing_allocation_id);
                    if (tuner_id < 0){
                        LOG_INFO(FrontendTunerDevice<TunerStatusStructType>,"allocateCapacity: UNKNOWN CONTROL ALLOCATION ID: ["<< frontend_listener_allocation.existing_allocation_id <<"]");
                        throw FRONTEND::BadParameterException("UNKNOWN CONTROL ALLOCATION ID");
                    }

                    // listener allocations are not permitted for channelizers or TX
                    if(frontend_tuner_status[tuner_id].tuner_type == "CHANNELIZER" || frontend_tuner_status[tuner_id].tuner_type == "TX"){
                        std::ostringstream eout;
                        eout<<"allocateCapacity: listener allocations are not permitted for " << std::string(frontend_tuner_status[tuner_id].tuner_type) << " tuner type";
                        LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>, eout.str());
                        throw CF::Device::InvalidCapacity(eout.str().c_str(), capacities);
                    }

                    tuner_allocation_ids[tuner_id].listener_allocation_ids.push_back(frontend_listener_allocation.listener_allocation_id);
                    allocation_id_to_tuner_id.insert(std::pair<std::string, size_t > (frontend_listener_allocation.listener_allocation_id, tuner_id));
                    frontend_tuner_status[tuner_id].allocation_id_csv = createAllocationIdCsv(tuner_id);
                    this->assignListener(frontend_listener_allocation.listener_allocation_id,frontend_listener_allocation.existing_allocation_id);
                    return true;
                }
                else {
                    LOG_INFO(FrontendTunerDevice<TunerStatusStructType>,"allocateCapacity: UNKNOWN ALLOCATION PROPERTY2");
                    throw CF::Device::InvalidCapacity("UNKNOWN ALLOCATION PROPERTY2", capacities);
                }
            }
        }
        catch(const std::logic_error &e) {
            deallocateCapacity(capacities);
            return false;
        }
        catch(AllocationAlreadyExists &e) {
            // Don't call deallocateCapacity if the allocationId already exists
            //   - Would end up deallocating a valid tuner/listener
            throw static_cast<CF::Device::InvalidCapacity>(e); 
        }
        catch(CF::Device::InvalidCapacity &e) {
            deallocateCapacity(capacities);
            throw e;
        }
        catch(FRONTEND::BadParameterException &e) {
            deallocateCapacity(capacities);
            return false;
        }
        catch(...){
            deallocateCapacity(capacities);
            throw;
        };
        
        return true;
    }

    template < typename TunerStatusStructType >
    void FrontendTunerDevice<TunerStatusStructType>::deallocateCapacity(const CF::Properties & capacities)
    throw (CORBA::SystemException, CF::Device::InvalidCapacity, CF::Device::InvalidState) {
        LOG_TRACE(FrontendTunerDevice<TunerStatusStructType>,__PRETTY_FUNCTION__);
        for (CORBA::ULong ii = 0; ii < capacities.length(); ++ii) {
            try{
                const std::string id = (const char*) capacities[ii].id;
                if (id != "FRONTEND::tuner_allocation" && id != "FRONTEND::listener_allocation"){
                    LOG_INFO(FrontendTunerDevice<TunerStatusStructType>,"deallocateCapacity: UNKNOWN ALLOCATION PROPERTY");
                    throw CF::Device::InvalidCapacity("UNKNOWN ALLOCATION PROPERTY", capacities);
                }
                PropertyInterface* property = getPropertyFromId(id);
                if(!property){
                    LOG_INFO(FrontendTunerDevice<TunerStatusStructType>,"deallocateCapacity: UNKNOWN PROPERTY");
                    throw CF::Device::InvalidCapacity("UNKNOWN PROPERTY", capacities);
                }
                try{
                    property->setValue(capacities[ii].value);
                }
                catch(const std::logic_error &e){
                    LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>, "COULD NOT PARSE CAPACITY: " << e.what());
                    throw CF::Device::InvalidCapacity("COULD NOT PARSE CAPACITY", capacities);
                };
                if (id == "FRONTEND::tuner_allocation"){
                    //LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>,std::string(__PRETTY_FUNCTION__)+" tuner_allocation");
                    // Try to remove control of the device
                    long tuner_id = getTunerMapping(frontend_tuner_allocation.allocation_id);
                    if (tuner_id < 0){
                        LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>, "ALLOCATION_ID NOT FOUND: [" << frontend_tuner_allocation.allocation_id <<"]");
                        throw CF::Device::InvalidCapacity("ALLOCATION_ID NOT FOUND", capacities);
                    }
                    //LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>,std::string(__PRETTY_FUNCTION__)+" tuner_id = " << tuner_id);
                    if(tuner_allocation_ids[tuner_id].control_allocation_id == frontend_tuner_allocation.allocation_id){
                        //LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>,std::string(__PRETTY_FUNCTION__)+" deallocating control for tuner_id = " << tuner_id);
                        enableTuner(tuner_id, false);
                        removeTunerMapping(tuner_id);
                        frontend_tuner_status[tuner_id].allocation_id_csv = createAllocationIdCsv(tuner_id);
                    }
                    else {
                        //LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>,std::string(__PRETTY_FUNCTION__)+" deallocating listener for tuner_id = " << tuner_id);
                        // send EOS to listener connection only
                        removeTunerMapping(tuner_id,frontend_tuner_allocation.allocation_id);
                        frontend_tuner_status[tuner_id].allocation_id_csv = createAllocationIdCsv(tuner_id);
                    }
                }
                else if (id == "FRONTEND::listener_allocation") {
                    //LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>,std::string(__PRETTY_FUNCTION__)+" listener_allocation");
                    long tuner_id = getTunerMapping(frontend_listener_allocation.listener_allocation_id);
                    if (tuner_id < 0){
                        LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>, "ALLOCATION_ID NOT FOUND: [" << frontend_listener_allocation.listener_allocation_id <<"]");
                        throw CF::Device::InvalidCapacity("ALLOCATION_ID NOT FOUND", capacities);
                    }
                    //LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>,std::string(__PRETTY_FUNCTION__)+" tuner_id = " << tuner_id);
                    // send EOS to listener connection only
                    removeTunerMapping(tuner_id,frontend_listener_allocation.listener_allocation_id);
                    frontend_tuner_status[tuner_id].allocation_id_csv = createAllocationIdCsv(tuner_id);
                }
                else {
                    LOG_TRACE(FrontendTunerDevice<TunerStatusStructType>,"WARNING: UNKNOWN ALLOCATION PROPERTY \""+ std::string(property->name) + "\". IGNORING!");
                }
            }
            catch(...){
                LOG_DEBUG(FrontendTunerDevice<TunerStatusStructType>,"ERROR WHEN DEALLOCATING. SKIPPING...");
            }
        }
        _usageState = updateUsageState();
    }

    /*****************************************************************/
    /* Tuner Configurations                                          */
    /*****************************************************************/

    template < typename TunerStatusStructType >
    bool FrontendTunerDevice<TunerStatusStructType>::enableTuner(size_t tuner_id, bool enable) {
        //LOG_TRACE(FrontendTunerDevice<TunerStatusStructType>,__PRETTY_FUNCTION__);
        
        bool prev_enabled = frontend_tuner_status[tuner_id].enabled;
        
        // If going from disabled to enabled
        if (!prev_enabled && enable) {
            deviceEnable(frontend_tuner_status[tuner_id], tuner_id);
        }
        
        // If going from enabled to disabled
        if (prev_enabled && !enable) {

            deviceDisable(frontend_tuner_status[tuner_id], tuner_id);
        }

        return true;
    }

    template < typename TunerStatusStructType >
    bool FrontendTunerDevice<TunerStatusStructType>::listenerRequestValidation(frontend_tuner_allocation_struct &request, size_t tuner_id){
        LOG_TRACE(FrontendTunerDevice<TunerStatusStructType>,__PRETTY_FUNCTION__);

        // ensure requested values are non-negative
        if(floatingPointCompare(request.center_frequency,0)<0 || floatingPointCompare(request.bandwidth,0)<0 || floatingPointCompare(request.sample_rate,0)<0 || floatingPointCompare(request.bandwidth_tolerance,0)<0 || floatingPointCompare(request.sample_rate_tolerance,0)<0)
            return false;

        // ensure lower end of requested band fits
        //if((request.center_frequency - (request.bandwidth*0.5)) < (frontend_tuner_status[tuner_id].center_frequency - (frontend_tuner_status[tuner_id].bandwidth*0.5))){
        if( floatingPointCompare((request.center_frequency-(request.bandwidth*0.5)),(frontend_tuner_status[tuner_id].center_frequency-(frontend_tuner_status[tuner_id].bandwidth*0.5))) < 0 ){
            LOG_TRACE(FrontendTunerDevice<TunerStatusStructType>,__PRETTY_FUNCTION__ << " FAILED LOWER END TEST");
            return false;
        }

        // ensure upper end of requested band fits
        //if((request.center_frequency + (request.bandwidth*0.5)) > (frontend_tuner_status[tuner_id].center_frequency + (frontend_tuner_status[tuner_id].bandwidth*0.5))){
        if( floatingPointCompare((request.center_frequency + (request.bandwidth*0.5)),(frontend_tuner_status[tuner_id].center_frequency + (frontend_tuner_status[tuner_id].bandwidth*0.5))) > 0 ){
            LOG_TRACE(FrontendTunerDevice<TunerStatusStructType>,__PRETTY_FUNCTION__ << " FAILED UPPER END TEST");
            return false;
        }

        // ensure tuner bandwidth meets requested tolerance
        //if(request.bandwidth > frontend_tuner_status[tuner_id].bandwidth)
        if( floatingPointCompare(request.bandwidth,frontend_tuner_status[tuner_id].bandwidth) > 0 )
            return false;

        //if(request.bandwidth != 0 && (request.bandwidth+(request.bandwidth*request.bandwidth_tolerance/100)) < frontend_tuner_status[tuner_id].bandwidth)
        if( floatingPointCompare(request.bandwidth,0)!=0 && floatingPointCompare((request.bandwidth+(request.bandwidth*request.bandwidth_tolerance/100)),frontend_tuner_status[tuner_id].bandwidth) < 0 )
            return false;

        // ensure tuner sample rate meets requested tolerance
        //if(request.sample_rate > frontend_tuner_status[tuner_id].sample_rate)
        if( floatingPointCompare(request.sample_rate,frontend_tuner_status[tuner_id].sample_rate) > 0 )
            return false;

        //if(request.sample_rate != 0 && (request.sample_rate+(request.sample_rate*request.sample_rate_tolerance/100)) < frontend_tuner_status[tuner_id].sample_rate)
        if(floatingPointCompare(request.sample_rate,0)!=0 && floatingPointCompare((request.sample_rate+(request.sample_rate*request.sample_rate_tolerance/100)),frontend_tuner_status[tuner_id].sample_rate) < 0 )
            return false;

        return true;
    };

    ////////////////////////////
    //        MAPPING         //
    ////////////////////////////

    template < typename TunerStatusStructType >
    long FrontendTunerDevice<TunerStatusStructType>::getTunerMapping(std::string allocation_id) {
        //LOG_TRACE(FrontendTunerDevice<TunerStatusStructType>,__PRETTY_FUNCTION__);
        long NO_VALID_TUNER = -1;

        string_number_mapping::iterator iter = allocation_id_to_tuner_id.find(allocation_id);
        if (iter != allocation_id_to_tuner_id.end())
            return iter->second;

        return NO_VALID_TUNER;
    }
    

    template < typename TunerStatusStructType >
    void FrontendTunerDevice<TunerStatusStructType>::sendEOS(std::string allocation_id) {
        CF::PortSet::PortInfoSequence_var ports = this->getPortSet();
        for (unsigned int port_idx=0; port_idx<ports->length(); port_idx++) {
            std::string repid = std::string(ports[port_idx].repid);
            if (repid.find("BULKIO") != std::string::npos) {
                ExtendedCF::QueryablePort_ptr prt = ExtendedCF::QueryablePort::_narrow(ports[port_idx].obj_ptr);
                try {
                    prt->disconnectPort(allocation_id.c_str());
                } catch ( ... ) {
                    continue;
                }
                ExtendedCF::UsesConnectionSequence_var _connections = prt->connections();
                for (unsigned int connection_idx=0; connection_idx<_connections->length(); connection_idx++) {
                    if (std::string(_connections[connection_idx].connectionId) == allocation_id) {
                        prt->connectPort(_connections[connection_idx].port, allocation_id.c_str());
                        break;
                    }
                }
            }
        }
    }
    
    template < typename TunerStatusStructType >
    bool FrontendTunerDevice<TunerStatusStructType>::removeTunerMapping(size_t tuner_id, std::string allocation_id) {
        LOG_TRACE(FrontendTunerDevice<TunerStatusStructType>,__PRETTY_FUNCTION__);
        removeListener(allocation_id);
        sendEOS(allocation_id);
        std::vector<std::string>::iterator it = tuner_allocation_ids[tuner_id].listener_allocation_ids.begin();
        while(it != tuner_allocation_ids[tuner_id].listener_allocation_ids.end()){
            if(*it == allocation_id){
                tuner_allocation_ids[tuner_id].listener_allocation_ids.erase(it);
            } else {
                ++it;
            }
        }
        exclusive_lock lock(allocation_id_mapping_lock);
        if(allocation_id_to_tuner_id.erase(allocation_id) > 0)
            return true;
        return false;
    }
    
    template < typename TunerStatusStructType >
    bool FrontendTunerDevice<TunerStatusStructType>::removeTunerMapping(size_t tuner_id) {
        LOG_TRACE(FrontendTunerDevice<TunerStatusStructType>,__PRETTY_FUNCTION__);
        deviceDeleteTuning(frontend_tuner_status[tuner_id], tuner_id);
        removeAllocationIdRouting(tuner_id);

        long cnt = 0;
        exclusive_lock lock(allocation_id_mapping_lock);
        string_number_mapping::iterator it = allocation_id_to_tuner_id.begin();
        while(it != allocation_id_to_tuner_id.end()){
            if(it->second == tuner_id){
                std::string allocation_id = it->first;
                removeListener(allocation_id);
                sendEOS(allocation_id);
                allocation_id_to_tuner_id.erase(it++);
                cnt++;
            } else {
                ++it;
            }
        }
        tuner_allocation_ids[tuner_id].reset();
        return cnt > 0;
    }

    template < typename TunerStatusStructType >
    void FrontendTunerDevice<TunerStatusStructType>::assignListener(const std::string& listen_alloc_id, const std::string& alloc_id) {
    };

    template < typename TunerStatusStructType >
    void FrontendTunerDevice<TunerStatusStructType>::removeListener(const std::string& listen_alloc_id) {
    };

}; // end frontend namespace
