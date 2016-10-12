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
package frontend;

import BULKIO.StreamSRI;
import BULKIO.UNITS_TIME;
import BULKIO.UNITS_NONE;
import CF.AggregateDevice;
import CF.DataType;
import CF.DeviceManager;
import CF.DevicePackage.UsageType.*;
import CF.DevicePackage.InvalidCapacity;
import CF.DevicePackage.InvalidCapacityHelper;
import CF.DevicePackage.InvalidState;
import CF.DevicePackage.UsageType;
import CF.PortSetPackage.PortInfoType;
import CF.InvalidObjectReference;
import ExtendedCF.UsesConnection;
import FRONTEND.RFInfoPkt;
import FRONTEND.BadParameterException;
import java.lang.Math.*;
import java.lang.reflect.*;
import java.text.*;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.concurrent.*;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.TimeZone;
import java.util.UUID.*;
import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
import org.ossie.component.ThreadedDevice;
import org.ossie.properties.Action;
import org.ossie.properties.Allocator;
import org.ossie.properties.AnyUtils;
import org.ossie.properties.Kind;
import org.ossie.properties.Mode;
import org.ossie.properties.StringProperty;
import org.ossie.properties.StructProperty;
import org.ossie.properties.StructSequenceProperty;

public abstract class FrontendTunerDevice<TunerStatusStructType extends frontend.FETypes.default_frontend_tuner_status_struct_struct> extends ThreadedDevice {

    /* validateRequestVsSRI is a helper function to check that the input data stream can support
     * the allocation request. The output mode (true if complex output) is used when determining
     * the necessary sample rate required to satisfy the request. The entire frequency band of the
     * request must be available for True to be returned, not just the center frequency.
     * True is returned upon success, otherwise FRONTEND.BadParameterException is thrown.
     * If the CHAN_RF and FRONTEND.BANDWIDTH keywords are not found in the sri,
     * FRONTEND.BadParameterException is thrown.
     */
    public boolean validateRequestVsSRI(final frontend.FETypes.frontend_tuner_allocation_struct request, final BULKIO.StreamSRI upstream_sri, boolean output_mode) throws FRONTEND.BadParameterException {
        // get center frequency and bandwidth from SRI keywords
        double upstream_cf = 0.0;
        double upstream_bw = 0.0;
        boolean found_cf = false;
        boolean found_bw = false;
        int key_size = upstream_sri.keywords.length;
        for (int i = 0; i < key_size; i++) {
            if (!upstream_sri.keywords[i].id.equals("CHAN_RF")) {
                Double val = (Double) AnyUtils.convertAny(upstream_sri.keywords[i].value);
                if (val != null){ 
                    found_cf = true;
                    upstream_cf = val.doubleValue();
                }
            } else if (!upstream_sri.keywords[i].id.equals("FRONTEND::BANDWIDTH")) {
                Double val = (Double) AnyUtils.convertAny(upstream_sri.keywords[i].value);
                if (val != null){ 
                    found_bw = true;
                    upstream_bw = val.doubleValue();
                }
            }
        }
        if(!found_cf || !found_bw){
            throw new FRONTEND.BadParameterException("CANNOT VERIFY REQUEST -- SRI missing required keywords");
        }

        // check bandwidth
        double min_upstream_freq = upstream_cf-(upstream_bw/2);
        double max_upstream_freq = upstream_cf+(upstream_bw/2);
        double min_requested_freq = request.center_frequency.getValue()-(request.bandwidth.getValue()/2);
        double max_requested_freq = request.center_frequency.getValue()+(request.bandwidth.getValue()/2);

        if( !validateRequest(min_upstream_freq,max_upstream_freq,min_requested_freq,max_requested_freq) ){
            throw new FRONTEND.BadParameterException("INVALID REQUEST -- input data stream cannot support freq/bw request");
        }

        // check sample rate
        double upstream_sr = 1/upstream_sri.xdelta;
        int input_scaling_factor = (upstream_sri.mode == 1) ? 2 : 4; // adjust for complex data
        min_upstream_freq = upstream_cf-(upstream_sr/input_scaling_factor);
        max_upstream_freq = upstream_cf+(upstream_sr/input_scaling_factor);
        int output_scaling_factor = (output_mode) ? 2 : 4; // adjust for complex data
        min_requested_freq = request.center_frequency.getValue()-(request.sample_rate.getValue()/output_scaling_factor);
        max_requested_freq = request.center_frequency.getValue()+(request.sample_rate.getValue()/output_scaling_factor);

        if ( !validateRequest(min_upstream_freq,max_upstream_freq,min_requested_freq,max_requested_freq) ){
            throw new FRONTEND.BadParameterException("INVALID REQUEST -- input data stream cannot support freq/sr request");
        }
        return true;
    }

    /* validateRequestVsDevice is a helper function to check that the input data stream and the
     * device can support an allocation request. The output mode (true if complex output) is used
     * when determining the necessary sample rate required to satisfy the request. The entire
     * frequency band of the request must be available for True to be returned, not just the center
     * frequency.
     * True is returned upon success, otherwise FRONTEND.BadParameterException is thrown.
     * If the CHAN_RF and FRONTEND::BANDWIDTH keywords are not found in the sri,
     * FRONTEND.BadParameterException is thrown.
     */
    public boolean validateRequestVsDevice(final frontend.FETypes.frontend_tuner_allocation_struct request, final BULKIO.StreamSRI upstream_sri,
            boolean output_mode, double min_device_center_freq, double max_device_center_freq, double max_device_bandwidth, double max_device_sample_rate) throws FRONTEND.BadParameterException {
        // check if request can be satisfied using the available upstream data
        if( !validateRequestVsSRI(request,upstream_sri, output_mode) ){
            throw new FRONTEND.BadParameterException("INVALID REQUEST -- falls outside of input data stream");
        }

        // check device constraints

        // check vs. device center frequency capability (ensure 0 <= request <= max device capability)
        if ( !validateRequest(min_device_center_freq,max_device_center_freq,request.center_frequency.getValue()) ){
            throw new FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support freq request");
        }

        // check vs. device bandwidth capability (ensure 0 <= request <= max device capability)
        if ( !validateRequest(0,max_device_bandwidth,request.bandwidth.getValue()) ){
            throw new FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support bw request");
        }

        // check vs. device sample rate capability (ensure 0 <= request <= max device capability)
        if ( !validateRequest(0,max_device_sample_rate,request.sample_rate.getValue()) ){
            throw new FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support sr request");
        }

        // calculate overall frequency range of the device (not just CF range)
        final int output_scaling_factor = (output_mode) ? 2 : 4; // adjust for complex data
        final double min_device_freq = min_device_center_freq-(max_device_sample_rate/output_scaling_factor);
        final double max_device_freq = max_device_center_freq+(max_device_sample_rate/output_scaling_factor);

        // check based on bandwidth
        // this duplicates part of check above if device freq range = input freq range
        double min_requested_freq = request.center_frequency.getValue()-(request.bandwidth.getValue()/2);
        double max_requested_freq = request.center_frequency.getValue()+(request.bandwidth.getValue()/2);
        if ( !validateRequest(min_device_freq,max_device_freq,min_requested_freq,max_requested_freq) ){
            throw new FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support freq/bw request");
        }

        // check based on sample rate
        // this duplicates part of check above if device freq range = input freq range
        min_requested_freq = request.center_frequency.getValue()-(request.sample_rate.getValue()/output_scaling_factor);
        max_requested_freq = request.center_frequency.getValue()+(request.sample_rate.getValue()/output_scaling_factor);
        if ( !validateRequest(min_device_freq,max_device_freq,min_requested_freq,max_requested_freq) ){
            throw new FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support freq/sr request");
        }

        return true;
    }

    /* validateRequestVsRFInfo is a helper function to check that the analog capabilities can support
     * the allocation request. The mode (true if complex) is used when determining the necessary
     * sample rate required to satisfy the request. The entire frequency band of the request must be
     * available for True to be returned, not just the center frequency.
     * True is returned upon success, otherwise FRONTEND.BadParameterException is thrown.
     */
    public boolean validateRequestVsRFInfo(final frontend.FETypes.frontend_tuner_allocation_struct request, final FRONTEND.RFInfoPkt rfinfo, boolean mode) throws FRONTEND.BadParameterException {

        double min_analog_freq = rfinfo.rf_center_freq-(rfinfo.rf_bandwidth/2);
        double max_analog_freq = rfinfo.rf_center_freq+(rfinfo.rf_bandwidth/2);

        // check bandwidth
        double min_requested_freq = request.center_frequency.getValue()-(request.bandwidth.getValue()/2);
        double max_requested_freq = request.center_frequency.getValue()+(request.bandwidth.getValue()/2);

        if ( !validateRequest(min_analog_freq,max_analog_freq,min_requested_freq,max_requested_freq) ){
            throw new FRONTEND.BadParameterException("INVALID REQUEST -- analog freq range (RFinfo) cannot support freq/bw request");
        }

        // check sample rate
        int scaling_factor = (mode) ? 2 : 4; // adjust for complex data
        min_requested_freq = request.center_frequency.getValue()-(request.sample_rate.getValue()/scaling_factor);
        max_requested_freq = request.center_frequency.getValue()+(request.sample_rate.getValue()/scaling_factor);

        if ( !validateRequest(min_analog_freq,max_analog_freq,min_requested_freq,max_requested_freq) ){
            throw new FRONTEND.BadParameterException("INVALID REQUEST -- analog freq range (RFinfo) cannot support freq/sr request");
        }
        return true;
    }

    /* validateRequestVsDevice is a helper function to check that the analog capabilities and the
     * device can support the allocation request. The mode (true if complex) is used when
     * determining the necessary sample rate required to satisfy the request. The entire frequency
     * band of the request must be available for True to be returned, not just the center frequency.
     * True is returned upon success, otherwise FRONTEND.BadParameterException is thrown.
     */
    public boolean validateRequestVsDevice(final frontend.FETypes.frontend_tuner_allocation_struct request, final FRONTEND.RFInfoPkt rfinfo,
            boolean mode, double min_device_center_freq, double max_device_center_freq, double max_device_bandwidth, double max_device_sample_rate) throws FRONTEND.BadParameterException {

        // check if request can be satisfied using the available upstream data
        if(!request.tuner_type.getValue().equals("TX") && !validateRequestVsRFInfo(request,rfinfo, mode) ){
            throw new FRONTEND.BadParameterException("INVALID REQUEST -- analog freq range (RFinfo) cannot support request");
        }

        // check device constraints
        // see if IF center frequency is set in rfinfo packet
        double request_if_center_freq = request.center_frequency.getValue();
        if(!request.tuner_type.getValue().equals("TX") && floatingPointCompare(rfinfo.if_center_freq,0) > 0 && floatingPointCompare(rfinfo.rf_center_freq,rfinfo.if_center_freq) > 0)
            request_if_center_freq = request.center_frequency.getValue() - (rfinfo.rf_center_freq-rfinfo.if_center_freq);

        // check vs. device center freq capability (ensure 0 <= request <= max device capability)
        if ( !validateRequest(min_device_center_freq,max_device_center_freq,request_if_center_freq) ) {
            throw new FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support freq request");
        }

        // check vs. device bandwidth capability (ensure 0 <= request <= max device capability)
        if ( !validateRequest(0,max_device_bandwidth,request.bandwidth.getValue()) ){
            throw new FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support bw request");
        }

        // check vs. device sample rate capability (ensure 0 <= request <= max device capability)
        if ( !validateRequest(0,max_device_sample_rate,request.sample_rate.getValue()) ){
            throw new FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support sr request");
        }

        // calculate overall frequency range of the device (not just CF range)
        final int scaling_factor = (mode) ? 2 : 4; // adjust for complex data
        final double min_device_freq = min_device_center_freq-(max_device_sample_rate/scaling_factor);
        final double max_device_freq = max_device_center_freq+(max_device_sample_rate/scaling_factor);

        // check based on bandwidth
        double min_requested_freq = request_if_center_freq-(request.bandwidth.getValue()/2);
        double max_requested_freq = request_if_center_freq+(request.bandwidth.getValue()/2);

        if ( !validateRequest(min_device_freq,max_device_freq,min_requested_freq,max_requested_freq) ) {
            throw new FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support freq/bw request");
        }

        // check based on sample rate
        min_requested_freq = request_if_center_freq-(request.sample_rate.getValue()/scaling_factor);
        max_requested_freq = request_if_center_freq+(request.sample_rate.getValue()/scaling_factor);

        if ( !validateRequest(min_device_freq,max_device_freq,min_requested_freq,max_requested_freq) ){
            throw new FRONTEND.BadParameterException("INVALID REQUEST -- device capabilities cannot support freq/sr request");
        }

        return true;
    }

    public FrontendTunerDevice() {
        super();
        construct();
    }
 
    private void construct() {
        loadProperties();
        allocation_id_to_tuner_id = new HashMap<String,Integer>();
        frontend_tuner_allocation.setAllocator(new Allocator<frontend.FETypes.frontend_tuner_allocation_struct>() {
            public boolean allocate(frontend.FETypes.frontend_tuner_allocation_struct capacity){
                boolean status = false;
                try{
                    status = allocateTuner(capacity);
                }catch(CF.DevicePackage.InvalidCapacity e){

                }catch(Exception e){

                }
                return status;
            }
            public void deallocate(frontend.FETypes.frontend_tuner_allocation_struct capacity){
                deallocateTuner(capacity);
            }
        });
        frontend_listener_allocation.setAllocator(new Allocator<frontend.FETypes.frontend_listener_allocation_struct>() {
            public boolean allocate(frontend.FETypes.frontend_listener_allocation_struct capacity) {
                boolean status = false;
                try{
                    status = allocateListener(capacity);
                }catch(CF.DevicePackage.InvalidCapacity e){

                }catch(Exception e){

                }
                return status;
            }
            public void deallocate(frontend.FETypes.frontend_listener_allocation_struct capacity){
                deallocateListener(capacity);
            }
        });
    }
    
    // this is implemented in the generated base class once all properties are known
    public void loadProperties(){
        addProperty(device_kind);
        addProperty(device_model);
        addProperty(frontend_tuner_allocation);
        addProperty(frontend_listener_allocation);
        addProperty(frontend_tuner_status);
    }

    protected String createAllocationIdCsv(int tuner_id){
        StringBuilder alloc_id_csv = new StringBuilder();
        // ensure control allocation_id is first in list
        if (tuner_allocation_ids.get(tuner_id).control_allocation_id != null && !tuner_allocation_ids.get(tuner_id).control_allocation_id.isEmpty())
            alloc_id_csv.append(tuner_allocation_ids.get(tuner_id).control_allocation_id + ",");
        // now add the rest
        Iterator<String> iter = tuner_allocation_ids.get(tuner_id).listener_allocation_ids.iterator();
        while (iter.hasNext()) {
            alloc_id_csv.append(iter.next() + ",");
        }
        // and get rid of the trailing comma
        if(alloc_id_csv.length() > 0){
            alloc_id_csv.setLength(alloc_id_csv.length()-1);
        }
        return alloc_id_csv.toString();
    }

    protected String getControlAllocationId(int tuner_id){
        return tuner_allocation_ids.get(tuner_id).control_allocation_id;
    }

    protected List<String> getListenerAllocationIds(int tuner_id){
        return tuner_allocation_ids.get(tuner_id).listener_allocation_ids;
    }

    /*****************************************************************/
    /* Allocation/Deallocation of Capacity                           */
    /*****************************************************************/
    // updateUsageState is not defined to return UsageType in Device base class
    public void updateUsageState() {
        getUsageState();
    }

    public CF.DevicePackage.UsageType getUsageState() {
        int tunerAllocated = 0;
        for (int tuner_id = 0; tuner_id < tuner_allocation_ids.size(); tuner_id++) {
            if (!tuner_allocation_ids.get(tuner_id).control_allocation_id.isEmpty())
                tunerAllocated++;
        }
        // If no tuners are allocated, device is idle
        if (tunerAllocated == 0)
            return CF.DevicePackage.UsageType.IDLE;
        // If all tuners are allocated, device is busy
        if (tunerAllocated == tuner_allocation_ids.size())
            return CF.DevicePackage.UsageType.BUSY;
        // Else, device is active
        return CF.DevicePackage.UsageType.ACTIVE;
    }

    public boolean allocateTuner(frontend.FETypes.frontend_tuner_allocation_struct frontend_tuner_allocation) throws CF.DevicePackage.InvalidCapacity, Exception {
        try{
            // Check allocation_id
            if (frontend_tuner_allocation.allocation_id != null &&
                frontend_tuner_allocation.allocation_id.getValue().isEmpty()) {
                logger.info("allocateTuner: MISSING ALLOCATION_ID");
                throw new CF.DevicePackage.InvalidCapacity("MISSING ALLOCATION ID", new CF.DataType[]{new DataType("frontend_tuner_allocation", frontend_tuner_allocation.toAny())});
            }
            // Check if allocation ID has already been used
            if(this.getTunerMapping(frontend_tuner_allocation.allocation_id.getValue()) >= 0){
                logger.info("allocateTuner: ALLOCATION_ID ALREADY IN USE: [" + frontend_tuner_allocation.allocation_id.getValue() + "]");
                throw new InvalidCapacity("ALLOCATION_ID ALREADY IN USE", new CF.DataType[]{new DataType("frontend_tuner_allocation", frontend_tuner_allocation.toAny())});
            }
            // Check if available tuner
            //synchronized(allocation_id_mapping_lock){
                // Next, try to allocate a new tuner
                for (int tuner_id = 0; tuner_id < this.tuner_allocation_ids.size(); tuner_id++) {
                    if(!frontend_tuner_status.getValue().get(tuner_id).tuner_type.getValue().equals(frontend_tuner_allocation.tuner_type.getValue())) {
                        logger.debug("allocateTuner: Requested tuner type '"+frontend_tuner_allocation.tuner_type.getValue() +"' does not match tuner[" + tuner_id + "].tuner_type (" + frontend_tuner_status.getValue().get(tuner_id).tuner_type.getValue()+")");
                        continue;
                    }

                    if(frontend_tuner_allocation.group_id != null &&
                       !frontend_tuner_allocation.group_id.getValue().isEmpty() && 
                       !frontend_tuner_allocation.group_id.getValue().equals(frontend_tuner_status.getValue().get(tuner_id).group_id.getValue()) ){
                        logger.debug("allocateTuner: Requested group_id '" + frontend_tuner_allocation.group_id.getValue() + "' does not match tuner[" + tuner_id + "].group_id (" + this.frontend_tuner_status.getValue().get(tuner_id).group_id.getValue() +")");
                        continue;
                    }

                    // special case because allocation is specifying the input stream, which determines the rf_flow_id, etc.
                   if( frontend_tuner_allocation.rf_flow_id != null &&
                        !frontend_tuner_allocation.rf_flow_id.getValue().isEmpty() &&
                        !frontend_tuner_allocation.rf_flow_id.getValue().equals(frontend_tuner_status.getValue().get(tuner_id).rf_flow_id.getValue()) &&
                        !frontend_tuner_allocation.tuner_type.equals("CHANNELIZER")){
                        logger.debug("allocateTuner: Requested rf_flow_id '" + frontend_tuner_allocation.rf_flow_id.getValue() +"' does not match tuner[" + tuner_id + "].rf_flow_id (" + this.frontend_tuner_status.getValue().get(tuner_id).rf_flow_id.getValue() + ")");
                        continue;
                    }
 
                    if(frontend_tuner_allocation.device_control.getValue()){
                        // device control
                        double orig_bw = frontend_tuner_status.getValue().get(tuner_id).bandwidth.getValue();
                        double orig_cf = frontend_tuner_status.getValue().get(tuner_id).center_frequency.getValue();
                        double orig_sr = frontend_tuner_status.getValue().get(tuner_id).sample_rate.getValue();
                        frontend_tuner_status.getValue().get(tuner_id).bandwidth.setValue(frontend_tuner_allocation.bandwidth.getValue());
                        frontend_tuner_status.getValue().get(tuner_id).center_frequency.setValue(frontend_tuner_allocation.center_frequency.getValue());
                        frontend_tuner_status.getValue().get(tuner_id).sample_rate.setValue(frontend_tuner_allocation.sample_rate.getValue());
                        if(tuner_allocation_ids.get(tuner_id).control_allocation_id != null &&
                           (!tuner_allocation_ids.get(tuner_id).control_allocation_id.isEmpty() || 
                            !deviceSetTuning(frontend_tuner_allocation, frontend_tuner_status.getValue().get(tuner_id), tuner_id))){
                            // either not available or didn't succeed setting tuning, try next tuner
                            if (frontend_tuner_status.getValue().get(tuner_id).bandwidth.getValue().equals(frontend_tuner_allocation.bandwidth.getValue()))
                                frontend_tuner_status.getValue().get(tuner_id).bandwidth.setValue(orig_bw);
                            if (frontend_tuner_status.getValue().get(tuner_id).center_frequency.getValue().equals(frontend_tuner_allocation.center_frequency.getValue()))
                                frontend_tuner_status.getValue().get(tuner_id).center_frequency.setValue(orig_cf);
                            if (frontend_tuner_status.getValue().get(tuner_id).sample_rate.getValue().equals(frontend_tuner_allocation.sample_rate.getValue()))
                                frontend_tuner_status.getValue().get(tuner_id).sample_rate.setValue(orig_sr);
                            logger.debug("allocateTuner: Tuner["+tuner_id+"] is either not available or didn't succeed while setting tuning ");
                            continue;
                        }
                        tuner_allocation_ids.get(tuner_id).control_allocation_id = frontend_tuner_allocation.allocation_id.getValue();
                        allocation_id_to_tuner_id.put(frontend_tuner_allocation.allocation_id.getValue(), tuner_id);
                        frontend_tuner_status.getValue().get(tuner_id).allocation_id_csv.setValue(createAllocationIdCsv(tuner_id));
                    } else {
                        // channelizer allocations must specify device control = true
                        if(frontend_tuner_allocation.tuner_type.getValue().equals("CHANNELIZER") || frontend_tuner_allocation.tuner_type.getValue().equals("TX")){
                            String eout;
                            eout = frontend_tuner_allocation.tuner_type.getValue() + " allocation with device_control=false is invalid.";
                            logger.debug(eout);
                            throw new CF.DevicePackage.InvalidCapacity(eout, new CF.DataType[]{new DataType("frontend_tuner_allocation", frontend_tuner_allocation.toAny())});
                        }
                        // listener
                        if(tuner_allocation_ids.get(tuner_id).control_allocation_id.isEmpty() || !listenerRequestValidation(frontend_tuner_allocation, tuner_id)){
                            // either not allocated or can't support listener request
                            logger.debug("allocateTuner: Tuner["+tuner_id+"] is either not available or can not support listener request ");
                            continue;
                        }
                        tuner_allocation_ids.get(tuner_id).listener_allocation_ids.add(frontend_tuner_allocation.allocation_id.getValue());
                        allocation_id_to_tuner_id.put(frontend_tuner_allocation.allocation_id.getValue(), tuner_id);
                        frontend_tuner_status.getValue().get(tuner_id).allocation_id_csv.setValue(createAllocationIdCsv(tuner_id));
                        this.assignListener(frontend_tuner_allocation.allocation_id.getValue(),tuner_allocation_ids.get(tuner_id).control_allocation_id);
                    }
                    // if we've reached here, we found an eligible tuner with correct frequency

                    // check tolerances
                    // only check when sample_rate was not set to don't care)
                    logger.debug(" allocateTuner - SR requested: " + frontend_tuner_allocation.sample_rate.getValue() + "  SR got: " + frontend_tuner_status.getValue().get(tuner_id).sample_rate.getValue());
                    if( (floatingPointCompare(frontend_tuner_allocation.sample_rate.getValue(),0)!=0) &&
                        (floatingPointCompare(frontend_tuner_status.getValue().get(tuner_id).sample_rate.getValue(),frontend_tuner_allocation.sample_rate.getValue())<0 ||
                        floatingPointCompare(frontend_tuner_status.getValue().get(tuner_id).sample_rate.getValue(),frontend_tuner_allocation.sample_rate.getValue()+frontend_tuner_allocation.sample_rate.getValue() * frontend_tuner_allocation.sample_rate_tolerance.getValue()/100.0)>0 )){
                        String eout = "allocateTuner(" + tuner_id + "): returned sr " + frontend_tuner_status.getValue().get(tuner_id).sample_rate.getValue()+" does not meet tolerance criteria of " + frontend_tuner_allocation.sample_rate_tolerance.getValue()+" percent";
                        logger.info(eout);
                        throw new RuntimeException(eout);
                    }
                    logger.debug(" allocateTuner - BW requested: " + frontend_tuner_allocation.bandwidth.getValue() + "  BW got: " + frontend_tuner_status.getValue().get(tuner_id).bandwidth.getValue());
                    // Only check when bandwidth was not set to don't care
                    if( (floatingPointCompare(frontend_tuner_allocation.bandwidth.getValue(),0)!=0) &&
                       (floatingPointCompare(frontend_tuner_status.getValue().get(tuner_id).bandwidth.getValue(),frontend_tuner_allocation.bandwidth.getValue())<0 ||
                        floatingPointCompare(frontend_tuner_status.getValue().get(tuner_id).bandwidth.getValue(),frontend_tuner_allocation.bandwidth.getValue()+frontend_tuner_allocation.bandwidth.getValue() * frontend_tuner_allocation.bandwidth_tolerance.getValue()/100.0)>0 )){
                        String eout = "allocateTuner(" + tuner_id + "): returned bw " + frontend_tuner_status.getValue().get(tuner_id).bandwidth.getValue()  + " does not meet tolerance criteria of " + frontend_tuner_allocation.bandwidth_tolerance.getValue() + " percent";
                        logger.info(eout);
                        throw new RuntimeException(eout);
                    }

                    if(frontend_tuner_allocation.device_control.getValue()){
                        // enable tuner after successful allocation
                        try {
                            enableTuner(tuner_id,true);
                        } catch(Exception e){
                            String eout = "allocateTuner: Failed to enable tuner after allocation";
                            logger.info(eout);
                            throw new RuntimeException(eout);
                        }
                    }
                    usageState = getUsageState();
                    return true;
                }
                // if we made it here, we failed to find an available tuner
                String eout = "allocateTuner: NO AVAILABLE TUNER. Make sure that the device has an initialized frontend_tuner_status";
                logger.info(eout);
                throw new RuntimeException(eout);
            //}
        } catch(RuntimeException e) {
            //deallocateTuner(frontend_tuner_allocation);
            return false;
        } catch(CF.DevicePackage.InvalidCapacity e) {
            // without the following check, a valid allocation could be deallocated due to an attempt to alloc w/ an existing alloc id
            String exceptionMessage = e.getMessage();
            if (exceptionMessage != null && 
               exceptionMessage.indexOf("ALLOCATION_ID ALREADY IN USE") == -1){
               //deallocateTuner(frontend_tuner_allocation);
            }
            throw e;
        } catch(Exception e){
            //deallocateTuner(frontend_tuner_allocation);
            throw e; 
        }
    }

    public void deallocateTuner(frontend.FETypes.frontend_tuner_allocation_struct frontend_tuner_deallocation){
        try{
            //logger.debug("deallocateTuner()");
            // Try to remove control of the device
            int tuner_id = this.getTunerMapping(frontend_tuner_deallocation.allocation_id.getValue());
            if (tuner_id < 0){
                logger.debug("ALLOCATION_ID NOT FOUND: [" + frontend_tuner_deallocation.allocation_id.getValue() +"]");
                throw new CF.DevicePackage.InvalidCapacity("ALLOCATION_ID NOT FOUND", new CF.DataType[]{new DataType("frontend_tuner_deallocation", frontend_tuner_deallocation.toAny())});
            }
            //logger.debug("deallocateTuner() tuner_id = " + tuner_id);
            if(tuner_allocation_ids.get(tuner_id).control_allocation_id.equals(frontend_tuner_deallocation.allocation_id.getValue())){
                //logger.debug("deallocateTuner() deallocating control for tuner_id = " + tuner_id);
                enableTuner(tuner_id, false);
                frontend_tuner_status.getValue().get(tuner_id).allocation_id_csv.setValue("");
                removeTunerMapping(tuner_id);
                tuner_allocation_ids.get(tuner_id).control_allocation_id = "";
            }else{
                // send EOS to listener connection only
                removeTunerMapping(tuner_id,frontend_tuner_deallocation.allocation_id.getValue());
                frontend_tuner_status.getValue().get(tuner_id).allocation_id_csv.setValue(createAllocationIdCsv(tuner_id));
            }
        } catch (Exception e){
            System.out.println("deallocateTuner: ERROR WHEN DEALLOCATING.  SKIPPING...");
        }
        usageState = getUsageState();
    }

    public boolean allocateListener(frontend.FETypes.frontend_listener_allocation_struct frontend_listener_allocation) throws CF.DevicePackage.InvalidCapacity, Exception {
        try{
            // Check validity of allocation_id's
            if (frontend_listener_allocation.existing_allocation_id == null || 
                frontend_listener_allocation.existing_allocation_id.getValue().isEmpty()){
                logger.info("allocateListener: MISSING EXISTING ALLOCATION ID");
                throw new CF.DevicePackage.InvalidCapacity("MISSING EXISTING ALLOCATION ID", new CF.DataType[]{new DataType("frontend_listener_allocation", frontend_listener_allocation.toAny())});
            }
            if (frontend_listener_allocation.listener_allocation_id == null ||
                frontend_listener_allocation.listener_allocation_id.getValue().isEmpty()){
                logger.info("allocateListener: MISSING LISTENER ALLOCATION ID");
                throw new CF.DevicePackage.InvalidCapacity("MISSING LISTENER ALLOCATION ID", new CF.DataType[]{new DataType("frontend_listener_allocation", frontend_listener_allocation.toAny())});
            }

            //synchronized(allocation_id_mapping_lock){
                // Check if listener allocation ID has already been used
                if(getTunerMapping(frontend_listener_allocation.listener_allocation_id.getValue()) >= 0){
                    logger.error("allocateListener: LISTENER ALLOCATION_ID ALREADY IN USE");
                    throw new InvalidCapacity("LISTENER ALLOCATION_ID ALREADY IN USE", new CF.DataType[]{new DataType("frontend_listener_allocation", frontend_listener_allocation.toAny())});
                }

                // Do not allocate if existing allocation ID does not exist
                int tuner_id = getTunerMapping(frontend_listener_allocation.existing_allocation_id.getValue());
                if (tuner_id < 0){
                    logger.info("allocateListener: UNKNOWN CONTROL ALLOCATION ID: ["+ frontend_listener_allocation.existing_allocation_id.getValue() +"]");
                    throw new FRONTEND.BadParameterException("UNKNOWN CONTROL ALLOCATION ID");
                }

                // listener allocations are not permitted for channelizers or TX
                if(frontend_tuner_status.getValue().get(tuner_id).tuner_type.getValue().equals("CHANNELIZER") || frontend_tuner_status.getValue().get(tuner_id).tuner_type.getValue().equals("TX")){
                    String eout = "allocateListener: listener allocations are not permitted for " + frontend_tuner_status.getValue().get(tuner_id).tuner_type.getValue() + " tuner type";
                    logger.debug(eout);
                    throw new CF.DevicePackage.InvalidCapacity(eout, new CF.DataType[]{new DataType("frontend_listener_allocation", frontend_listener_allocation.toAny())});
                }

                tuner_allocation_ids.get(tuner_id).listener_allocation_ids.add(frontend_listener_allocation.listener_allocation_id.getValue());
                allocation_id_to_tuner_id.put(frontend_listener_allocation.listener_allocation_id.getValue(), tuner_id);
                frontend_tuner_status.getValue().get(tuner_id).allocation_id_csv.setValue(createAllocationIdCsv(tuner_id));
                this.assignListener(frontend_listener_allocation.listener_allocation_id.getValue(),frontend_listener_allocation.existing_allocation_id.getValue());
                return true;
            //}
        }catch(CF.DevicePackage.InvalidCapacity e){
            String exceptionMessage = e.getMessage();
            // without the following check, a valid allocation could be deallocated due to an attempt to alloc w/ an existing alloc id
            if (exceptionMessage != null && 
               exceptionMessage.indexOf("ALLOCATION_ID ALREADY IN USE") == -1){
            }
            throw e;
        } catch (FRONTEND.BadParameterException e){
            return false;
        } catch (Exception e){
            throw e;
        }
    }

    public void deallocateListener(frontend.FETypes.frontend_listener_allocation_struct frontend_listener_allocation){
        try{
            int tuner_id = getTunerMapping(frontend_listener_allocation.listener_allocation_id.getValue());
            if (tuner_id < 0){
                logger.debug("ALLOCATION_ID NOT FOUND: [" + frontend_listener_allocation.listener_allocation_id.getValue() + "]");
                throw new CF.DevicePackage.InvalidCapacity("ALLOCATION_ID NOT FOUND", new CF.DataType[]{new DataType("frontend_listener_allocation", frontend_listener_allocation.toAny())});
            }
            // send EOS to listener connection only
            removeTunerMapping(tuner_id, frontend_listener_allocation.listener_allocation_id.getValue());
            frontend_tuner_status.getValue().get(tuner_id).allocation_id_csv.setValue(removeListenerId(tuner_id, frontend_listener_allocation.listener_allocation_id.getValue()));
        } catch (Exception e){
            logger.debug("deallocateListener: ERROR WHEN DEALLOCATING.  SKIPPING...");
        }
    }
    
    public String removeListenerId(final int tuner_id, final String allocation_id) {
        String[] split_id = frontend_tuner_status.getValue().get(tuner_id).allocation_id_csv.getValue().split(",");
        int idx;
        for (idx=0; idx<split_id.length; idx++) {
            if (split_id[idx].equals(allocation_id)) {
                break;
            }
        }
        String cleanedString = "";
        for (int idx_i=0; idx_i<split_id.length; idx_i++) {
            if (idx == idx_i) {
                continue;
            }
            cleanedString += split_id[idx_i] + ",";
        }
        cleanedString = cleanedString.replaceAll(",$","");
        return cleanedString;
    }

    /*****************************************************************/
    /* Tuner Configurations                                          */
    /*****************************************************************/
    // assumes collector RF and channel RF are the same if not true, override function 
    protected boolean enableTuner(int tuner_id, boolean enable){
        boolean prev_enabled = frontend_tuner_status.getValue().get(tuner_id).enabled.getValue();
        // If going from disabled to enabled
        if (!prev_enabled && enable) {
            deviceEnable(frontend_tuner_status.getValue().get(tuner_id), tuner_id);
        }

        // If going from enabled to disabled
        if (prev_enabled && !enable) {

            deviceDisable(frontend_tuner_status.getValue().get(tuner_id), tuner_id);
        }

        return true;
    }

    protected boolean listenerRequestValidation(frontend.FETypes.frontend_tuner_allocation_struct request, int tuner_id){
        logger.trace("listenerRequestValidation() tuner_id " + tuner_id);
        // ensure requested values are non-negative
        if(floatingPointCompare(request.center_frequency.getValue(),0)<0 || floatingPointCompare(request.bandwidth.getValue(),0)<0 || floatingPointCompare(request.sample_rate.getValue(),0)<0 || floatingPointCompare(request.bandwidth_tolerance.getValue(),0)<0 || floatingPointCompare(request.sample_rate_tolerance.getValue(),0)<0)
            return false;

        // ensure lower end of requested band fits
        if( floatingPointCompare((request.center_frequency.getValue()-(request.bandwidth.getValue()*0.5)),(frontend_tuner_status.getValue().get(tuner_id).center_frequency.getValue()-(frontend_tuner_status.getValue().get(tuner_id).bandwidth.getValue()*0.5))) < 0 ){
            logger.trace("listenerRequestValidation() FAILED LOWER END TEST");
            return false;
        }

        // ensure upper end of requested band fits
        if( floatingPointCompare((request.center_frequency.getValue() + (request.bandwidth.getValue()*0.5)),(frontend_tuner_status.getValue().get(tuner_id).center_frequency.getValue() + (frontend_tuner_status.getValue().get(tuner_id).bandwidth.getValue()*0.5))) > 0 ){
            logger.trace("listenerRequestValidation() FAILED UPPER END TEST");
            return false;
        }

        // ensure tuner bandwidth meets requested tolerance
        if( floatingPointCompare(request.bandwidth.getValue(),frontend_tuner_status.getValue().get(tuner_id).bandwidth.getValue()) > 0 )
            return false;

        if( floatingPointCompare(request.bandwidth.getValue(),0)!=0 && floatingPointCompare((request.bandwidth.getValue()+(request.bandwidth.getValue()*request.bandwidth_tolerance.getValue()/100)),frontend_tuner_status.getValue().get(tuner_id).bandwidth.getValue()) < 0 )
            return false;

        // ensure tuner sample rate meets requested tolerance
        if( floatingPointCompare(request.sample_rate.getValue(),frontend_tuner_status.getValue().get(tuner_id).sample_rate.getValue()) > 0 )
            return false;

        if(floatingPointCompare(request.sample_rate.getValue(),0)!=0 && floatingPointCompare((request.sample_rate.getValue()+(request.sample_rate.getValue()*request.sample_rate_tolerance.getValue()/100)),frontend_tuner_status.getValue().get(tuner_id).sample_rate.getValue()) < 0 )
            return false;

        return true;
    }

    ////////////////////////////
    //        MAPPING         //
    ////////////////////////////

    protected int getTunerMapping(String allocation_id){
        //logger.trace("getTunerMapping() allocation_id " + allocation_id);
        int NO_VALID_TUNER = -1;

        if (allocation_id_to_tuner_id.containsKey(allocation_id)){
            return allocation_id_to_tuner_id.get(allocation_id);
        } 
        return NO_VALID_TUNER;
    }

    protected void sendEOS(String allocation_id) {
        CF.PortSetPackage.PortInfoType ports[] = this.getPortSet();
        for (int port_idx=0; port_idx<ports.length; port_idx++) {
            String repid = ports[port_idx].repid;
            if (repid.indexOf("BULKIO") != -1) {
                ExtendedCF.QueryablePort prt = ExtendedCF.QueryablePortHelper.narrow(ports[port_idx].obj_ptr);
                try {
                    prt.disconnectPort(allocation_id);
                } catch (final Throwable t) {
                    continue;
                }
                ExtendedCF.UsesConnection _connections[] = prt.connections();
                for (int connection_idx=0; connection_idx<_connections.length; connection_idx++) {
                    if (_connections[connection_idx].connectionId == allocation_id) {
                        try {
                            prt.connectPort(_connections[connection_idx].port, allocation_id);
                        } catch (final Throwable t) {
                            break;
                        }
                    }
                }
            }
        }
    }

    protected boolean removeTunerMapping(int tuner_id, String allocation_id){
        logger.trace("removeTunerMapping() tuner_id " + tuner_id + ", allocation_id " + allocation_id);
        removeListener(allocation_id);
        this.sendEOS(allocation_id);
        Iterator<String> iter = tuner_allocation_ids.get(tuner_id).listener_allocation_ids.iterator();
        while(iter.hasNext()){
            String nextString = iter.next();
            if (nextString.equals(allocation_id)){
                iter.remove();
            }
        }
        //synchronized(allocation_id_mapping_lock){
            if (allocation_id_to_tuner_id.containsKey(allocation_id)){
                allocation_id_to_tuner_id.remove(allocation_id);
                return true;
            }
            return false;
        //}
    }

    protected boolean removeTunerMapping(int tuner_id){
        logger.trace("removeTunerMapping() tuner_id " + tuner_id);
        deviceDeleteTuning(frontend_tuner_status.getValue().get(tuner_id),tuner_id);
        removeAllocationIdRouting(tuner_id);

        int cnt = 0;
        //synchronized(allocation_id_mapping_lock){
            if (allocation_id_to_tuner_id.containsValue(tuner_id)){
                Iterator<Map.Entry<String,Integer>> iter = allocation_id_to_tuner_id.entrySet().iterator();
                while (iter.hasNext()) {
                    Map.Entry<String,Integer> entry = iter.next();
                    if(tuner_id == entry.getValue()){
                        this.removeListener(entry.getKey());
                        this.sendEOS(entry.getKey());
                        iter.remove();
                        cnt++;
                    }
                }
            }
            tuner_allocation_ids.get(tuner_id).reset();
            return cnt > 0;
        //}
    }

    protected void assignListener(final String listen_alloc_id, final String alloc_id){
    }

    protected void removeListener(final String listen_alloc_id){
    }
   

    /* floatingPointCompare is a helper function to handle floating point comparison
     * Return values:
     *   if lhs == rhs: 0.0
     *   if lhs >  rhs: 1.0 or greater
     *   if lhs <  rhs: -1.0 or less
     * Recommended usage is to convert a comparison such as: (lhs OP rhs)
     * to (floatingPointCompare(lhs,rhs) OP 0), where OP is a comparison operator
     * (==, <, >, <=, >=, !=).
     * "places" is used to specify precision. The default is 1, which
     * uses a single decimal place of precision.
     */
    public double floatingPointCompare(double lhs, double rhs){
        return floatingPointCompare(lhs, rhs, 1);
    }

    public double floatingPointCompare(double lhs, double rhs, int places){
        return java.lang.Math.rint((lhs-rhs)*java.lang.Math.pow(10.0,(double)places));
    }

    /* validateRequest is a helper function to verify a value is within a range, returning
     * true if the value requested_val falls within the range [available_min:available_max]
     * False is returned if min > max
     */
    public boolean validateRequest(double available_min, double available_max, double requested_val){
        if(floatingPointCompare(requested_val,available_min) < 0) return false;
        if(floatingPointCompare(requested_val,available_max) > 0) return false;
        if(floatingPointCompare(available_min,available_max) > 0) return false;
        return true;
    }

    /* validateRequest is a helper function to compare two ranges, returning true if the range
     * [requested_min:requested_max] falls within the range [available_min:available_max]
     * False is returned if min > max for either available for requested values
     */
    public boolean validateRequest(double available_min, double available_max, double requested_min, double requested_max){
        if(floatingPointCompare(requested_min,available_min) < 0) return false;
        if(floatingPointCompare(requested_max,available_max) > 0) return false;
        if(floatingPointCompare(available_min,available_max) > 0) return false;
        if(floatingPointCompare(requested_min,requested_max) > 0) return false;
        return true;
    }

    /* Tuner Allocation IDs struct. This structure contains allocation tracking data.
     *
     */

    public class tunerAllocationIdsStruct{
        public String control_allocation_id;
        public List<String> listener_allocation_ids;
        public tunerAllocationIdsStruct(){
            control_allocation_id = new String();
            listener_allocation_ids = new ArrayList<String>();
            reset();
        }
        public void reset(){
            control_allocation_id = "";
            listener_allocation_ids.clear();
        }
    }

    // tuner_allocation_ids is exclusively paired with property frontend_tuner_status.
    // tuner_allocation_ids tracks allocation ids while frontend_tuner_status provides tuner information.
    protected List<tunerAllocationIdsStruct> tuner_allocation_ids;

    protected StringProperty device_kind =
        new StringProperty(
            "DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d", //id
            "device_kind", //name
            "FRONTEND::TUNER", //default value
            Mode.READONLY, //mode
            Action.EQ, //action
            new Kind[] {Kind.ALLOCATION,Kind.CONFIGURE} //kind
            );

    protected StringProperty device_model =
        new StringProperty(
            "DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb", //id
            "device_model", //name
            "", //default value
            Mode.READONLY, //mode
            Action.EQ, //action
            new Kind[] {Kind.ALLOCATION,Kind.CONFIGURE} //kind
            );

    protected StructProperty<frontend.FETypes.frontend_tuner_allocation_struct> frontend_tuner_allocation =
        new StructProperty<frontend.FETypes.frontend_tuner_allocation_struct>(
            "FRONTEND::tuner_allocation", //id
            "frontend_tuner_allocation", //name
            frontend.FETypes.frontend_tuner_allocation_struct.class, //type
            new frontend.FETypes.frontend_tuner_allocation_struct(), //default value
            Mode.READWRITE, //mode
            new Kind[] {Kind.ALLOCATION} //kind
            );

    protected StructProperty<frontend.FETypes.frontend_listener_allocation_struct> frontend_listener_allocation =
        new StructProperty<frontend.FETypes.frontend_listener_allocation_struct>(
            "FRONTEND::listener_allocation", //id
            "frontend_listener_allocation", //name
            frontend.FETypes.frontend_listener_allocation_struct.class, //type
            new frontend.FETypes.frontend_listener_allocation_struct(), //default value
            Mode.READWRITE, //mode
            new Kind[] {Kind.ALLOCATION} //kind
            );

    private Class<TunerStatusStructType> frontend_tuner_status_class_type;

    protected StructSequenceProperty<TunerStatusStructType> frontend_tuner_status =
        new StructSequenceProperty<TunerStatusStructType> (
            "FRONTEND::tuner_status", //id
            "frontend_tuner_status", //name
            frontend_tuner_status_class_type, //type
            new ArrayList<TunerStatusStructType>(),
            Mode.READONLY, //mode
            new Kind[] { Kind.CONFIGURE } //kind
        );

    protected Map<String, Integer> allocation_id_to_tuner_id;

    protected Object allocation_id_mapping_lock;

    ///////////////////////////////
    // Device specific functions // -- to be implemented by device developer
    ///////////////////////////////
    protected abstract void deviceEnable(TunerStatusStructType fts, int tuner_id);
    protected abstract void deviceDisable(TunerStatusStructType fts, int tuner_id);
    protected abstract boolean deviceSetTuning(final frontend.FETypes.frontend_tuner_allocation_struct request, TunerStatusStructType fts, int tuner_id);
    protected abstract boolean deviceDeleteTuning(TunerStatusStructType fts, int tuner_id);

    ///////////////////////////////
    // Mapping and translation helpers. External string identifiers to internal numerical identifiers
    ///////////////////////////////
    protected abstract void removeAllocationIdRouting(final int tuner_id);

    ////////////////////////////
    // Other helper functions //
    ////////////////////////////
    protected BULKIO.StreamSRI create(String stream_id, TunerStatusStructType frontend_status) {
        return create(stream_id, frontend_status, -1.0);
    }

    protected BULKIO.StreamSRI create(String stream_id, TunerStatusStructType frontend_status, double collector_frequency) {
        BULKIO.StreamSRI sri = new BULKIO.StreamSRI();
        sri.hversion = 1;
        sri.xstart = 0.0;
        if ( frontend_status.sample_rate.getValue() <= 0.0 )
            sri.xdelta =  1.0;
        else
            sri.xdelta = 1/frontend_status.sample_rate.getValue();
        sri.xunits = BULKIO.UNITS_TIME.value;
        sri.subsize = 0;
        sri.ystart = 0.0;
        sri.ydelta = 0.0;
        sri.yunits = BULKIO.UNITS_NONE.value;
        sri.mode = 0;
        sri.blocking=false;
        sri.streamID = stream_id;
        double colFreq;
        if (collector_frequency < 0)
            colFreq = frontend_status.center_frequency.getValue();
        else
            colFreq = collector_frequency;
        Any colFreqAny = AnyUtils.toAny(colFreq,"double");
        addModifyKeyword(sri, "COL_RF", colFreqAny);
        Any chanFreqAny = AnyUtils.toAny(frontend_status.center_frequency.getValue(),"double");
        addModifyKeyword(sri, "CHAN_RF", chanFreqAny);
        Any rfFlowIdAny = AnyUtils.toAny(frontend_status.rf_flow_id.getValue(),"string");
        addModifyKeyword(sri,"FRONTEND::RF_FLOW_ID",rfFlowIdAny);
        Any bandwidthAny = AnyUtils.toAny(frontend_status.bandwidth.getValue(),"double");
        addModifyKeyword(sri,"FRONTEND::BANDWIDTH", bandwidthAny);
        Any identifierAny = AnyUtils.toAny(this.identifier(), "string");
        addModifyKeyword(sri,"FRONTEND::DEVICE_ID",identifierAny);
        return sri;
    }

    protected boolean addModifyKeyword(BULKIO.StreamSRI sri, String id, Any value) {
        return addModifyKeyword(sri, id, value, false);
    }

    protected boolean addModifyKeyword(BULKIO.StreamSRI sri, String id, Any value, boolean addOnly) {
        if (!addOnly && sri.keywords != null) {
            // find and replace
            for(int ii=0; ii<sri.keywords.length; ii++){
                if (sri.keywords[ii].id == id){
                    sri.keywords[ii].value = value;
                    return true;
                }
            }
        }
        CF.DataType[] updatedKeywords;
        if (sri.keywords != null){
            updatedKeywords = new CF.DataType[sri.keywords.length+1];
            for (int ii=0; ii<sri.keywords.length; ii++){
                updatedKeywords[ii] = sri.keywords[ii];
            }
            updatedKeywords[sri.keywords.length] = new DataType(id,value);
        }else{
            updatedKeywords = new CF.DataType[]{new DataType(id,value)};
        }
        sri.keywords = updatedKeywords;
        return true;
    }

    protected void printSRI(BULKIO.StreamSRI sri){
        this.printSRI(sri, "DEBUG SRI");
    }

    protected void printSRI(BULKIO.StreamSRI sri, String strHeader){
        System.out.println(strHeader);
        System.out.println("\thversion: " + sri.hversion);
        System.out.println("\txstart: " + sri.xstart);
        System.out.println("\txdelta: " + sri.xdelta);
        System.out.println("\txunits: " + sri.xunits);
        System.out.println("\tsubsize: " + sri.subsize);
        System.out.println("\tystart: " + sri.ystart);
        System.out.println("\tydelta: " + sri.ydelta);
        System.out.println("\tyunits: " + sri.yunits);
        System.out.println("\tmode: " + sri.mode);
        System.out.println("\tstreamID: " + sri.streamID);
        for (int ii=0; ii<sri.keywords.length; ii++){
            System.out.println("\tKEYWORD KEY:VAL : " + sri.keywords[ii].id + ":" + String.valueOf(AnyUtils.convertAny(sri.keywords[ii].value)));
        }
    }
}
