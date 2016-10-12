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
import CF.AggregateDevice;
import CF.DataType;
import CF.DeviceManager;
import CF.DevicePackage.InvalidCapacity;
import CF.DevicePackage.UsageType;
import CF.InvalidObjectReference;
import CF.DevicePackage.InvalidState;
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
import org.omg.CORBA.ORB;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
import org.ossie.component.Device;
import org.omg.CORBA.Any;
import org.ossie.properties.Action;
import org.ossie.properties.Allocator;
import org.ossie.properties.AnyUtils;
import org.ossie.properties.Kind;
import org.ossie.properties.Mode;
import org.ossie.properties.StringProperty;
import org.ossie.properties.StructProperty;
import org.ossie.properties.StructSequenceProperty;

public abstract class FrontendTunerDevice<TunerStatusStructType extends frontend.FrontendTunerStructProps.default_frontend_tuner_status_struct_struct> extends Device {

    public enum timeTypes{
        J1970,
        JCY
    }

    public class indivTuner<T extends frontend.FrontendTunerStructProps.default_frontend_tuner_status_struct_struct> {

        public indivTuner(){
            complex = true;
            frontend_status = null;
            lock = null;
        }

        public void reset(){
            sri = new BULKIO.StreamSRI(1,0.0,1.0,(short)1,1,0.0,1.0,(short)1,(short)0,"",false,new CF.DataType[0]);        
            control_allocation_id = "";
            if (frontend_status != null){
                frontend_status.allocation_id_csv.setValue("");
                frontend_status.center_frequency.setValue(0.0);
                frontend_status.bandwidth.setValue(0.0);
                frontend_status.sample_rate.setValue(0.0);
                frontend_status.enabled.setValue(false);
            }
        }

        public BULKIO.StreamSRI sri;
        public boolean complex;
        public Object lock;
        public String control_allocation_id;
        public T frontend_status;
    }


    public final StringProperty device_kind =
        new StringProperty(
            "DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d", //id
            "device_kind", //name
            "FRONTEND::TUNER", //default value
            Mode.READONLY, //mode
            Action.EQ, //action
            new Kind[] {Kind.ALLOCATION,Kind.CONFIGURE} //kind
            );

    public final StringProperty device_model =
        new StringProperty(
            "DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb", //id
            "device_model", //name
            "USRP", //default value
            Mode.READONLY, //mode
            Action.EQ, //action
            new Kind[] {Kind.ALLOCATION,Kind.CONFIGURE} //kind
            );

    public final StructProperty<frontend.FrontendTunerStructProps.frontend_tuner_allocation_struct> frontend_tuner_allocation =
        new StructProperty<frontend.FrontendTunerStructProps.frontend_tuner_allocation_struct>(
            "FRONTEND::tuner_allocation", //id
            "frontend_tuner_allocation", //name
            frontend.FrontendTunerStructProps.frontend_tuner_allocation_struct.class, //type
            new frontend.FrontendTunerStructProps.frontend_tuner_allocation_struct(), //default value
            Mode.READWRITE, //mode
            new Kind[] {Kind.ALLOCATION} //kind
            );

    public final StructProperty<frontend.FrontendTunerStructProps.frontend_listener_allocation_struct> frontend_listener_allocation =
        new StructProperty<frontend.FrontendTunerStructProps.frontend_listener_allocation_struct>(
            "FRONTEND::listener_allocation", //id
            "frontend_listener_allocation", //name
            frontend.FrontendTunerStructProps.frontend_listener_allocation_struct.class, //type
            new frontend.FrontendTunerStructProps.frontend_listener_allocation_struct(), //default value
            Mode.READWRITE, //mode
            new Kind[] {Kind.ALLOCATION} //kind
            );

    private Class<TunerStatusStructType> frontend_tuner_status_class_type;

    public final StructSequenceProperty<TunerStatusStructType> frontend_tuner_status =
        new StructSequenceProperty<TunerStatusStructType> (
            "FRONTEND::tuner_status", //id
            "frontend_tuner_status", //name
            frontend_tuner_status_class_type, //type
            (List)new ArrayList<TunerStatusStructType>(),
            Mode.READONLY, //mode
            new Kind[] { Kind.CONFIGURE } //kind
        );

    protected Map<String, Integer> allocationID_to_tunerID;

    protected Map<String, Integer> streamID_to_tunerID;

    protected Object allocationID_MappingLock;

    protected List<indivTuner<TunerStatusStructType>> tunerChannels; 

    public FrontendTunerDevice(DeviceManager devMgr, String compId, String label, String softwareProfile, ORB orb, POA poa) throws InvalidObjectReference, ServantNotActive, WrongPolicy, CF.DevicePackage.InvalidCapacity {
        super(devMgr,compId,label,softwareProfile,orb,poa);
        construct();
    }
 
    public FrontendTunerDevice(DeviceManager devMgr, AggregateDevice compositeDevice, String compId, String label, String softwareProfile, ORB orb, POA poa) throws InvalidObjectReference, ServantNotActive, WrongPolicy, CF.DevicePackage.InvalidCapacity {
        super(devMgr,compositeDevice,compId,label,softwareProfile,orb,poa);
        construct();
    }

    // this is implemented in the generated base class once all properties are known
    public void loadProperties(){
        addProperty(device_kind);
        addProperty(device_model);
        addProperty(frontend_tuner_allocation);
        addProperty(frontend_listener_allocation);
        //addProperty(frontend_tuner_status);
    }

    String create_allocation_id_csv(int tuner_id){
        StringBuilder alloc_id_csv = new StringBuilder();
        // ensure control allocation_id is first in list
        if (tunerChannels.get(tuner_id).control_allocation_id != null && !tunerChannels.get(tuner_id).control_allocation_id.isEmpty())
                alloc_id_csv.append(tunerChannels.get(tuner_id).control_allocation_id + ",");
        // now add the rest
        Iterator<Map.Entry<String,Integer>> iter = allocationID_to_tunerID.entrySet().iterator();
        while (iter.hasNext()) {
            Map.Entry<String,Integer> entry = iter.next();
            if((tuner_id == entry.getValue()) && (entry.getKey() != tunerChannels.get(tuner_id).control_allocation_id)){
                alloc_id_csv.append(entry.getKey() + ",");
            }
        }
        // and get rid of the trailing comma
        if(alloc_id_csv.length() > 0){
            alloc_id_csv.setLength(alloc_id_csv.length()-1);
        }
        return alloc_id_csv.toString();
    }

    public boolean allocateTuner(frontend.FrontendTunerStructProps.frontend_tuner_allocation_struct capacity) throws CF.DevicePackage.InvalidCapacity, Exception {
        try{
            if(!_valid_tuner_type(frontend_tuner_allocation.getValue().tuner_type.getValue())){
                System.out.println("allocateTuner: UNKNOWN FRONTEND TUNER TYPE");
                throw new CF.DevicePackage.InvalidCapacity("UNKNOWN FRONTEND TUNER TYPE", new CF.DataType[]{new DataType("frontend_tuner_allocation", capacity.toAny())});
            }

            // Check allocation_id
            if (frontend_tuner_allocation.getValue().allocation_id != null &&
                frontend_tuner_allocation.getValue().allocation_id.getValue().isEmpty()) {
                System.out.println("allocateTuner: MISSING ALLOCATION_ID");
                throw new CF.DevicePackage.InvalidCapacity("MISSING ALLOCATION ID", new CF.DataType[]{new DataType("frontend_tuner_allocation", capacity.toAny())});
            }
            // Check if allocation ID has already been used
            if(getTunerMapping(frontend_tuner_allocation.getValue().allocation_id.getValue()) >= 0){
                System.out.println("allocateTuner: ALLOCATION_ID ALREADY IN USE");
                throw new CF.DevicePackage.InvalidCapacity("ALLOCATION_ID ALREADY IN USE", new CF.DataType[]{new DataType("frontend_tuner_allocation", capacity.toAny())});
            }
            // Check if available tuner (if not requesting device control, this is all that's needed to add listener)
            int tuner_id = addTunerMapping(frontend_tuner_allocation.getValue());
            if (tuner_id < 0) {
                String msg;
                msg = "NO AVAILABLE TUNER";
                System.out.println("allocateTuner: NO AVAILABLE TUNER");
                throw new RuntimeException(msg);
            }

            // Initialize the tuner (only if requesting device control)
            if (frontend_tuner_allocation.getValue().device_control.getValue()){
                {
                    synchronized(tunerChannels.get(tuner_id).lock){
                        if(frontend_tuner_allocation.getValue().group_id != null &&
                           !frontend_tuner_allocation.getValue().group_id.getValue().isEmpty() && 
                           frontend_tuner_allocation.getValue().group_id.getValue() != tunerChannels.get(tuner_id).frontend_status.group_id.getValue() ){
                            System.out.println("allocateTuner: CANNOT ALLOCATE A TUNER WITH THAT GROUP ID");
                            throw new FRONTEND.BadParameterException("CAN NOT ALLOCATE A TUNER WITH THAT GROUP ID!");
                        }

                        if(frontend_tuner_allocation.getValue().rf_flow_id != null &&
                           !frontend_tuner_allocation.getValue().rf_flow_id.getValue().isEmpty() && 
                           frontend_tuner_allocation.getValue().rf_flow_id.getValue() != tunerChannels.get(tuner_id).frontend_status.rf_flow_id.getValue() ){
                            System.out.println("allocateTuner: CANNOT ALLOCATE A TUNER WITH THAT RF FLOW ID");
                            throw new FRONTEND.BadParameterException("CAN NOT ALLOCATE A TUNER WITH RF FLOW ID = " + frontend_tuner_allocation.getValue().rf_flow_id.getValue() + " !");
                        }
                        //Check Validity
                        if (!_valid_center_frequency(frontend_tuner_allocation.getValue().center_frequency.getValue(),tuner_id)){
                            System.out.println("allocateTuner: INVALID FREQUENCY");
                            throw new FRONTEND.BadParameterException("allocateTuner(): INVALID FREQUENCY");
                        }
                        if (!_valid_bandwidth(frontend_tuner_allocation.getValue().bandwidth.getValue(),tuner_id)){
                            System.out.println("allocateTuner: INVALID BANDWIDTH");
                            throw new FRONTEND.BadParameterException("allocateTuner(): INVALID BANDWIDTH");
                        }
                        if (!_valid_sample_rate(frontend_tuner_allocation.getValue().sample_rate.getValue(),tuner_id)){
                            System.out.println("allocateTuner: INVALID RATE");
                            throw new FRONTEND.BadParameterException("allocateTuner(): INVALID RATE");
                        }

                        try {
                            _dev_set_all(frontend_tuner_allocation.getValue().center_frequency.getValue(),
                                         frontend_tuner_allocation.getValue().bandwidth.getValue(),
                                         frontend_tuner_allocation.getValue().sample_rate.getValue(),
                                         tuner_id);
                        } catch(Exception e){
                            String msg;
                            msg="allocateTuner(" + tuner_id + ") failed when configuring device hardware";
                            System.out.println("allocateTuner: failed when querying device hardware");
                            throw new RuntimeException(msg);
                        };

                        // Only check non-TX when bandwidth was not set to don't care
                        if( (tunerChannels.get(tuner_id).frontend_status.tuner_type.getValue() != "TX" && 
                             frontend_tuner_allocation.getValue().bandwidth.getValue() != 0.0) &&
                            (tunerChannels.get(tuner_id).frontend_status.bandwidth.getValue() < frontend_tuner_allocation.getValue().bandwidth.getValue() ||
                             tunerChannels.get(tuner_id).frontend_status.bandwidth.getValue() > (frontend_tuner_allocation.getValue().bandwidth.getValue()+frontend_tuner_allocation.getValue().bandwidth.getValue() * frontend_tuner_allocation.getValue().bandwidth_tolerance.getValue()/100.0 ))){
                            String msg;
                            msg = "allocateTuner(" + tuner_id + "): returned bw \"" + tunerChannels.get(tuner_id).frontend_status.bandwidth.getValue() + "\" does not meet tolerance criteria of \"" + frontend_tuner_allocation.getValue().bandwidth.getValue() + "+" + frontend_tuner_allocation.getValue().bandwidth_tolerance.getValue() + "  percent\". ";
                            System.out.println("allocateTuner: did not meet BW tolerance");
                            throw new RuntimeException(msg);
                        }
                        // always check TX, but only check non-TX when sample_rate was not set to don't care)
                        if( (tunerChannels.get(tuner_id).frontend_status.tuner_type.getValue() == "TX" || frontend_tuner_allocation.getValue().sample_rate.getValue() != 0.0) &&
                            (tunerChannels.get(tuner_id).frontend_status.sample_rate.getValue() < frontend_tuner_allocation.getValue().sample_rate.getValue() ||
                              tunerChannels.get(tuner_id).frontend_status.sample_rate.getValue() > frontend_tuner_allocation.getValue().sample_rate.getValue() +frontend_tuner_allocation.getValue().sample_rate.getValue() * frontend_tuner_allocation.getValue().sample_rate_tolerance.getValue()/100.0 )){
                            String msg;
                            msg = "allocateTuner(" + tuner_id + "): returned sample rate \"" + tunerChannels.get(tuner_id).frontend_status.sample_rate.getValue() + "\" does not meet tolerance criteria of " + frontend_tuner_allocation.getValue().sample_rate.getValue() + "+" + frontend_tuner_allocation.getValue().sample_rate_tolerance.getValue() + " percent\". ";
                            System.out.println("allocateTuner: did not meet sample rate tolerance");
                            throw new RuntimeException(msg);
                        }
                    } // synchronized
                } // release tuner lock

                // enable tuner after successful allocation
                try {
                    enableTuner(tuner_id,true);
                } catch(Exception e){
                    String msg;
                    msg = "FAILED TO ENABLE TUNER AFTER ALLOCATION";
                    System.out.println("allocateTuner: FAILED TO ENABLE TUNER AFTER ALLOCATION");
                    throw new RuntimeException(msg);
                }
            }
        } catch(RuntimeException e) {
            deallocateTuner(capacity);
            return false;
        } catch(CF.DevicePackage.InvalidCapacity e) {
            // without the following check, a valid allocation could be deallocated due to an attempt to alloc w/ an existing alloc id
            String exceptionMessage = e.getMessage();
            if (exceptionMessage != null && 
               exceptionMessage.indexOf("ALLOCATION_ID ALREADY IN USE") == -1){
                deallocateTuner(capacity);
            }
            throw e;
        } catch(FRONTEND.BadParameterException e) {
            deallocateTuner(capacity);
            return false;
        } catch(Exception e){
            deallocateTuner(capacity);
            throw e;
        };
        return true;
    }

    public void deallocateTuner(frontend.FrontendTunerStructProps.frontend_tuner_allocation_struct capacity){
        try{
            // Try to remove control of the device
            int tuner_id = getTunerMapping(frontend_tuner_allocation.getValue().allocation_id.getValue());
            if (tuner_id < 0){
                throw new CF.DevicePackage.InvalidState();
            }
            if(tunerChannels.get(tuner_id).control_allocation_id == frontend_tuner_allocation.getValue().allocation_id.getValue()){
                removeTuner(tuner_id);
                removeTunerMapping(tuner_id);
            } else {
                // send EOS to listener connection only
                push_EOS_on_listener(frontend_tuner_allocation.getValue().allocation_id.getValue());
                removeTunerMapping(frontend_tuner_allocation.getValue().allocation_id.getValue());
            }
            tunerChannels.get(tuner_id).frontend_status.allocation_id_csv.setValue(create_allocation_id_csv(tuner_id));

        } catch (Exception e){
            System.out.println("deallocateTuner: ERROR WHEN DEALLOCATING");
        }
    }

    public boolean allocateListener(frontend.FrontendTunerStructProps.frontend_listener_allocation_struct capacity) throws CF.DevicePackage.InvalidCapacity, Exception {
        try{
            // Check validity of allocation_id's
            if (frontend_listener_allocation.getValue().existing_allocation_id == null || 
                frontend_listener_allocation.getValue().existing_allocation_id.getValue().isEmpty()){
                System.out.println("allocateListener: MISSING EXISTING ALLOCATION ID");
                throw new CF.DevicePackage.InvalidCapacity("MISSING EXISTING ALLOCATION ID", new CF.DataType[]{new DataType("frontend_listener_allocation", capacity.toAny())});
            }
            if (frontend_listener_allocation.getValue().listener_allocation_id.getValue().isEmpty()){
                System.out.println("allocateListener: MISSING LISTENER ALLOCATION ID");
                throw new CF.DevicePackage.InvalidCapacity("MISSING LISTENER ALLOCATION ID", new CF.DataType[]{new DataType("frontend_listener_allocation", capacity.toAny())});
            }
            // Check if listener allocation ID has already been used
            if(getTunerMapping(frontend_listener_allocation.getValue().listener_allocation_id.getValue()) >= 0){
                System.out.println("allocateListener: LISTENER ALLOCATION ID ALREADY IN USE");
                throw new CF.DevicePackage.InvalidCapacity("LISTENER ALLOCATION ID ALREADY IN USE", new CF.DataType[]{new DataType("frontend_listener_allocation", capacity.toAny())});
            }

            if(addListenerMapping(capacity) < 0){
                System.out.println("allocateListener: UNKNOWN CONTROL ALLOCATION ID");
                throw new FRONTEND.BadParameterException("UNKNOWN CONTROL ALLOCATION ID");
            }
        }catch(CF.DevicePackage.InvalidCapacity e){
            String exceptionMessage = e.getMessage();
            if (exceptionMessage != null && 
               exceptionMessage.indexOf("ALLOCATION_ID ALREADY IN USE") == -1){
                deallocateListener(capacity);
            }
            throw e;
        } catch (FRONTEND.BadParameterException e){
            deallocateListener(capacity);
            return false;
        } catch (Exception e){
            deallocateListener(capacity);
            throw e;
        }
        return true;
    }


    public void deallocateListener(frontend.FrontendTunerStructProps.frontend_listener_allocation_struct capacity){
        try{
            int tuner_id = getTunerMapping(frontend_listener_allocation.getValue().listener_allocation_id.getValue());
            if (tuner_id < 0){
                throw new CF.DevicePackage.InvalidState();
            }
            // send EOS to listener connection only
            push_EOS_on_listener(frontend_listener_allocation.getValue().listener_allocation_id.getValue());
            removeTunerMapping(frontend_listener_allocation.getValue().listener_allocation_id.getValue());
            tunerChannels.get(tuner_id).frontend_status.allocation_id_csv.setValue(create_allocation_id_csv(tuner_id));
        } catch (Exception e){
            System.out.println("deallocateTuner: ERROR WHEN DEALLOCATING");
        }
    }

    /*****************************************************************/
    /* Tuner Configurations                                          */
    /*****************************************************************/
    // Configure tuner - gets called during allocation
    public boolean removeTuner(int tuner_id){
        enableTuner(tuner_id, false);
        tunerChannels.get(tuner_id).reset();
        return true;
    }

    // assumes collector RF and channel RF are the same if not true, override function 
    public boolean enableTuner(int tuner_id, boolean enable){
        // Lock the tuner
        synchronized(tunerChannels.get(tuner_id).lock){

            boolean prev_enabled = tunerChannels.get(tuner_id).frontend_status.enabled.getValue();
            tunerChannels.get(tuner_id).frontend_status.enabled.setValue(enable);
            // If going from disabled to enabled
            if (!prev_enabled && enable) {
                int mode = tunerChannels.get(tuner_id).complex?1:0;
                configureTunerSRI(  tunerChannels.get(tuner_id).sri,
                                    tunerChannels.get(tuner_id).frontend_status.center_frequency.getValue(),
                                    tunerChannels.get(tuner_id).frontend_status.bandwidth.getValue(),
                                    tunerChannels.get(tuner_id).frontend_status.sample_rate.getValue(),
                                    mode, 
                                    tunerChannels.get(tuner_id).frontend_status.rf_flow_id.getValue());
                streamID_to_tunerID.put(tunerChannels.get(tuner_id).sri.streamID, tuner_id);
                _dev_enable(tuner_id);
            }

            // If going from enabled to disabled
            if (prev_enabled && !enable && tunerChannels.get(tuner_id).sri.streamID != null && !tunerChannels.get(tuner_id).sri.streamID.isEmpty()) {
                _dev_disable(tuner_id);
                String streamID = new String(tunerChannels.get(tuner_id).sri.streamID);
                streamID_to_tunerID.remove(streamID);
                tunerChannels.get(tuner_id).sri = new BULKIO.StreamSRI(1,0.0,1.0,(short)1,1,0.0,1.0,(short)1,(short)0,"",false,new CF.DataType[0]);        
            }
            return true;
        }
    }

    ////////////////////////////
    //        MAPPING         //
    ////////////////////////////
    // Mapping and translation helpers. External string identifiers to internal numerical identifiers
    public int addTunerMapping(frontend.FrontendTunerStructProps.frontend_tuner_allocation_struct frontend_alloc){
        int NO_VALID_TUNER = -1;
        // Do not allocate if allocation ID has already been used
        if(getTunerMapping(frontend_alloc.allocation_id.getValue()) >= 0)
            return NO_VALID_TUNER;
                
        synchronized(allocationID_MappingLock){
            // Next, try to allocate a new tuner
            int numChannels = tunerChannels.size();
            for (int tuner_id = 0; tuner_id < numChannels; tuner_id++) {
                if(tunerChannels.get(tuner_id).frontend_status.tuner_type.getValue() != frontend_alloc.tuner_type.getValue())
                    continue;
                //listen
                if (!frontend_alloc.device_control.getValue() && 
                     tunerChannels.get(tuner_id).control_allocation_id != null && 
                     !tunerChannels.get(tuner_id).control_allocation_id.isEmpty()){
                    boolean freq_valid = is_freq_valid(
                                                        frontend_alloc.center_frequency.getValue(),
                                                        frontend_alloc.bandwidth.getValue(),
                                                        frontend_alloc.sample_rate.getValue(),
                                                        tunerChannels.get(tuner_id).frontend_status.center_frequency.getValue(),
                                                        tunerChannels.get(tuner_id).frontend_status.bandwidth.getValue(),
                                                        tunerChannels.get(tuner_id).frontend_status.sample_rate.getValue());
                    if (freq_valid){
                        allocationID_to_tunerID.put(frontend_alloc.allocation_id.getValue(), tuner_id);
                        tunerChannels.get(tuner_id).frontend_status.allocation_id_csv.setValue(create_allocation_id_csv(tuner_id));
                        return tuner_id;
                    }
                //control
                } else if (frontend_alloc.device_control.getValue() && 
                           tunerChannels.get(tuner_id).control_allocation_id != null &&
                           !tunerChannels.get(tuner_id).control_allocation_id.isEmpty()){
                    tunerChannels.get(tuner_id).control_allocation_id = frontend_alloc.allocation_id.getValue();
                    allocationID_to_tunerID.put(frontend_alloc.allocation_id.getValue(), tuner_id);
                    tunerChannels.get(tuner_id).frontend_status.allocation_id_csv.setValue(create_allocation_id_csv(tuner_id));
                    return tuner_id;
                }
            }
            return NO_VALID_TUNER;
        }
    }

    public int addListenerMapping(frontend.FrontendTunerStructProps.frontend_listener_allocation_struct frontend_listener_alloc){
        int NO_VALID_TUNER = -1;
        // Do not allocate if allocation ID has already been used
        if (getTunerMapping(frontend_listener_alloc.listener_allocation_id.getValue()) >= 0)
            return NO_VALID_TUNER;

        synchronized(allocationID_MappingLock){
            int tuner_id = NO_VALID_TUNER;
            // Do not allocate if existing allocation ID does not exist
            if ((tuner_id = getTunerMapping(frontend_listener_alloc.existing_allocation_id.getValue())) < 0)
                return NO_VALID_TUNER;

            allocationID_to_tunerID.put(frontend_listener_alloc.listener_allocation_id.getValue(), tuner_id);
            tunerChannels.get(tuner_id).frontend_status.allocation_id_csv.setValue(create_allocation_id_csv(tuner_id));
            return tuner_id;
        }
    }

    public int getTunerMapping(String allocation_id){
        int NO_VALID_TUNER = -1;

        synchronized(allocationID_MappingLock){
            if (allocationID_to_tunerID.containsKey(allocation_id)){
                return allocationID_to_tunerID.get(allocation_id);
            } 
            return NO_VALID_TUNER;
        }

    }

    public boolean removeTunerMapping(String allocation_id){
        synchronized(allocationID_MappingLock){
            if (allocationID_to_tunerID.containsKey(allocation_id)){
                allocationID_to_tunerID.remove(allocation_id);
                return true;
            }
            return false;

        }
    }

    public boolean removeTunerMapping(int tuner_id){
        synchronized(allocationID_MappingLock){
            int cnt = 0;
            if (allocationID_to_tunerID.containsValue(tuner_id)){
                Iterator<Map.Entry<String,Integer>> iter = allocationID_to_tunerID.entrySet().iterator();
                while (iter.hasNext()) {
                    Map.Entry<String,Integer> entry = iter.next();
                    if(tuner_id == entry.getValue()){
                        iter.remove();
                        cnt++;
                    }
                }
            }
            return cnt > 0;
        }
    }
    public boolean is_connectionID_valid_for_tunerID(final int tuner_id, final String connectionID){
        if (!allocationID_to_tunerID.containsKey(connectionID)){
            return false;
        }else{
            int tunerID = allocationID_to_tunerID.get(connectionID);
            if(tunerID != tuner_id){
                return false;
            }
            return true;
        }
    }
    public boolean is_connectionID_valid_for_streamID(final String streamID, final String connectionID){
        if (!streamID_to_tunerID.containsKey(streamID)){
            return false;
        }else{
            int tunerID = streamID_to_tunerID.get(streamID);
            return is_connectionID_valid_for_tunerID(tunerID, connectionID);
        }
    }
    public boolean is_connectionID_controller_for_streamID(final String streamID, final String connectionID){
        if (!streamID_to_tunerID.containsKey(streamID)){
            return false;
        }else{
            int tunerID = streamID_to_tunerID.get(streamID);
            if (!is_connectionID_valid_for_tunerID(tunerID, connectionID)){
                return false;
            }
            if (tunerChannels.get(tunerID).control_allocation_id == connectionID)
                return false;
        }
        return true;
    }
    public boolean is_connectionID_listener_for_streamID(final String streamID, final String connectionID){
        if (!streamID_to_tunerID.containsKey(streamID)){
            return false;
        }else{
            int tunerID = streamID_to_tunerID.get(streamID);
            if (!is_connectionID_valid_for_tunerID(tunerID, connectionID)){
                return false;
            }
            if (tunerChannels.get(tunerID).control_allocation_id == connectionID)
                return false;
        }
        return true;
    }
    public boolean is_freq_valid(double req_cf, double req_bw, double req_sr, double cf, double bw, double sr){
        double req_min_bw_sr = Math.min(req_bw,req_sr);
        double min_bw_sr = Math.min(bw,sr);
        if( (req_cf + req_min_bw_sr/2 <= cf + min_bw_sr/2) && (req_cf - req_min_bw_sr/2 >= cf - min_bw_sr/2) ){
            return true;
        }
        return false;
    }


    protected abstract boolean push_EOS_on_listener(String listener_allocation_id);
    protected abstract boolean _valid_tuner_type(String tuner_type);
    protected abstract boolean _valid_center_frequency(double req_freq, int tuner_id);
    protected abstract boolean _valid_bandwidth(double req_bw, int tuner_id);
    protected abstract boolean _valid_sample_rate(double req_sr, int tuner_id);
    protected abstract boolean _dev_enable(int tuner_id);
    protected abstract boolean _dev_disable(int tuner_id);
    protected abstract boolean _dev_set_all(double req_freq, double req_bw, double req_sr, int tuner_id);
    protected abstract boolean _dev_set_center_frequency(double req_freq, int tuner_id);
    protected abstract boolean _dev_set_bandwidth(double req_bw, int tuner_id);
    protected abstract boolean _dev_set_sample_rate(double req_sr, int tuner_id);
    protected abstract boolean _dev_get_all(double freq, double bw, double sr, int tuner_id);
    protected abstract double _dev_get_center_frequency(int tuner_id);
    protected abstract double _dev_get_bandwidth(int tuner_id);
    protected abstract double _dev_get_sample_rate(int tuner_id);
   
    ////////////////////////////
    // Other helper functions //
    ////////////////////////////
    protected double optimize_rate(final double req_rate, final double max_rate, final double min_rate){
        if (req_rate < min_rate)
            return min_rate;
        return req_rate;
    }

    boolean addModifyKeyword(BULKIO.StreamSRI sri, String id, Any value) {
        this.addModifyKeyword(sri, id, value, false);
        return true;
    }

    boolean addModifyKeyword(BULKIO.StreamSRI sri, String id, Any value, boolean addOnly) {
        if (!addOnly) {
            // find and replace
            for(int ii=0; ii<sri.keywords.length; ii++){
                if (sri.keywords[ii].id == id){
                    sri.keywords[ii].value = value;
                    return true;
                }
            }
        }
        CF.DataType[] updatedKeywords = new CF.DataType[sri.keywords.length+1];
        for (int ii=0; ii<sri.keywords.length; ii++){
            updatedKeywords[ii] = sri.keywords[ii];
        }
        updatedKeywords[sri.keywords.length].id = id;
        updatedKeywords[sri.keywords.length].value = value;
        sri.keywords = updatedKeywords;
        return true;
    }
    public void configureTunerSRI(BULKIO.StreamSRI sri, Double channel_frequency, Double bandwidth, Double sample_rate, int mode, String rf_flow_id) {
        configureTunerSRI(sri, channel_frequency, bandwidth, sample_rate, mode, rf_flow_id, -1.0);
    }

    public void configureTunerSRI(BULKIO.StreamSRI sri, Double channel_frequency, Double bandwidth, Double sample_rate, int mode, String rf_flow_id, Double collector_frequency) {
        if (sri == null)
            return;

        //Convert CenterFreq into string
        long chanFreq = channel_frequency.longValue();

        //Create new streamID
        String streamID = "tuner_freq_" + Long.toString(chanFreq).substring(0,15) + "_Hz_" + java.util.UUID.randomUUID().toString();

        sri.mode = (short)mode;
        sri.hversion = 0;
        sri.xstart = 0;
        sri.xdelta = (1.0 / (sample_rate));
        sri.subsize = 0;// 1-dimensional data
        sri.xunits = 1;
        sri.ystart = 0;
        sri.ydelta = 0.001;
        sri.yunits = 1;
        sri.streamID = streamID;
        sri.blocking = false;

        // for some devices, colFreq is the same as chanFreq
        long colFreq;
        if (collector_frequency < 0)
            colFreq = chanFreq;
        else
            colFreq = collector_frequency.longValue();

        Any colFreqAny = AnyUtils.toAny(colFreq,"double"); 
        addModifyKeyword(sri, "COL_RF", colFreqAny);
        Any chanFreqAny = AnyUtils.toAny(chanFreq,"double"); 
        addModifyKeyword(sri, "CHAN_RF", chanFreqAny);
        Any rfFlowIdAny = AnyUtils.toAny(rf_flow_id,"string"); 
        addModifyKeyword(sri,"FRONTEND::RF_FLOW_ID",rfFlowIdAny);
        Any bandwidthAny = AnyUtils.toAny(bandwidth,"double"); 
        addModifyKeyword(sri,"FRONTEND::BANDWIDTH", bandwidthAny);
        Any identifierAny = AnyUtils.toAny(this.identifier(), "string"); 
        addModifyKeyword(sri,"FRONTEND::DEVICE_ID",identifierAny);
    }

    public void printSRI(BULKIO.StreamSRI sri){
        this.printSRI(sri, "DEBUG SRI");
    }

    public void printSRI(BULKIO.StreamSRI sri, String strHeader){
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

    private void construct() {
        loadProperties();
        frontend_tuner_allocation.setAllocator(new Allocator<frontend.FrontendTunerStructProps.frontend_tuner_allocation_struct>() {
            public boolean allocate(frontend.FrontendTunerStructProps.frontend_tuner_allocation_struct capacity){
                boolean status = false;
                try{
                    status = allocateTuner(capacity);
                }catch(CF.DevicePackage.InvalidCapacity e){

                }catch(Exception e){

                }
                return status;
            }
            public void deallocate(frontend.FrontendTunerStructProps.frontend_tuner_allocation_struct capacity){
                deallocateTuner(capacity);
            }
        });
        frontend_listener_allocation.setAllocator(new Allocator<frontend.FrontendTunerStructProps.frontend_listener_allocation_struct>() {
            public boolean allocate(frontend.FrontendTunerStructProps.frontend_listener_allocation_struct capacity) {
                boolean status = false;
                try{
                    status = allocateListener(capacity);
                }catch(CF.DevicePackage.InvalidCapacity e){

                }catch(Exception e){

                }
                return status;
            }
            public void deallocate(frontend.FrontendTunerStructProps.frontend_listener_allocation_struct capacity){
                deallocateListener(capacity);
            }
        });
    }
}
