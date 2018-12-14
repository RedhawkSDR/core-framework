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

import java.util.ArrayList;
import java.util.List;
import org.omg.CORBA.Any;
import org.ossie.properties.Action;
import org.ossie.properties.AnyUtils;
import org.ossie.properties.Kind;
import org.ossie.properties.Mode;
import org.ossie.properties.BooleanProperty;
import org.ossie.properties.DoubleProperty;
import org.ossie.properties.StringProperty;
import org.ossie.properties.StructDef;
import org.ossie.properties.StructProperty;

public class FETypes {

    public static enum timeTypes {
        J1950(1),
        J1970(2),
        JCY(3);
        public int value;
        private timeTypes(int value){
            this.value = value;
        }
    }

    public static class FreqRange {
        public double min_val;
        public double max_val;
        public List<Double> values;
        public FreqRange(){
            min_val = 0.0;
            max_val = 0.0;
            values = new ArrayList<Double>();
        }
    }

    public static class AntennaInfo {
        public String name;
        public String type;
        public String size;
        public String description;
        public AntennaInfo(){
            name = null;
            type = null;
            size = null;
            description = null;
        }
    }

    public static class FeedInfo {
        public String name;
        public String polarization;
        public FreqRange freq_range;
        public FeedInfo(){
            name = null;
            polarization = null;
            freq_range = new FreqRange();
        }
    }

    public static class SensorInfo {
        public String mission;
        public String collector;
        public String rx;
        public AntennaInfo antenna;
        public FeedInfo feed;
        public SensorInfo(){
            mission = null;
            collector = null;
            rx = null;
            antenna = new AntennaInfo();
            feed = new FeedInfo();
        }
    }

    public static class PathDelay {
        public double freq;
        public double delay_ns;
        public PathDelay(){
            freq = 0.0;
            delay_ns = 0.0;
        }
    }

    public static class RFCapabilities {
        public FreqRange freq_range;
        public FreqRange bw_range;
        public RFCapabilities(){
            freq_range = new FreqRange();
            bw_range = new FreqRange();
        }
    }

    public static class RFInfoPkt {
        public String rf_flow_id;
        public double rf_center_freq;
        public double rf_bandwidth;
        public double if_center_freq;
        public boolean spectrum_inverted;
        public SensorInfo sensor;
        public List<PathDelay> ext_path_delays;
        public RFCapabilities capabilities;
        public CF.PropertiesHolder additional_info;
        public RFInfoPkt(){
            rf_flow_id = null;
            rf_center_freq = 0.0;
            rf_bandwidth = 0.0;
            if_center_freq = 0.0;
            spectrum_inverted = false;
            sensor = new SensorInfo();
            ext_path_delays = new ArrayList<PathDelay>();
            capabilities = new RFCapabilities();
            additional_info = new CF.PropertiesHolder();
        }
    }

    public static class PositionInfo {
        public boolean valid;
        public String datum;
        public double lat;
        public double lon;
        public double alt;
        public PositionInfo(){
            valid = false;
            datum = null;
            lat = 0.0;
            lon = 0.0;
            alt = 0.0;
        }
    }

    public static class GPSInfo {
        public String source_id;
        public String rf_flow_id;
        public String mode;
        public long fom;
        public long tfom;
        public long datumID;
        public double time_offset;
        public double freq_offset;
        public double time_variance;
        public double freq_variance;
        public short satellite_count;
        public float snr;
        public String status_message;
        public BULKIO.PrecisionUTCTime timestamp;
        public CF.PropertiesHolder additional_info;
        public GPSInfo(){
            source_id = null;
            rf_flow_id = null;
            mode = null;
            fom = 0;
            tfom = 0;
            datumID = 0;
            time_offset = 0.0;
            freq_offset = 0.0;
            time_variance = 0.0;
            freq_variance = 0.0;
            satellite_count = 0;
            snr = 0.0f;
            status_message = null;
            timestamp = new BULKIO.PrecisionUTCTime();
            additional_info = new CF.PropertiesHolder();
        }
    }

    public static class GpsTimePos {
        public PositionInfo position;
        public BULKIO.PrecisionUTCTime timestamp;
        public GpsTimePos(){
            position = new PositionInfo();
            timestamp = new BULKIO.PrecisionUTCTime();
        }
    }

    public static class CartesianPositionInfo {
        public boolean valid;
        public String datum;
        public double x;
        public double y;
        public double z;
        public CartesianPositionInfo(){
            valid = false;
            datum = null;
            x = 0.0;
            y = 0.0;
            z = 0.0;
        }
    }

    public static class AttitudeInfo {
        public boolean valid;
        public double pitch;
        public double yaw;
        public double roll;
        public AttitudeInfo(){
            valid = false;
            pitch = 0.0;
            yaw = 0.0;
            roll = 0.0;
        }
    }

    public static class VelocityInfo {
        public boolean valid;
        public String datum;
        public String coordinate_system;
        public double x;
        public double y;
        public double z;
        public VelocityInfo(){
            valid = false;
            datum = null;
            coordinate_system = null;
            x = 0.0;
            y = 0.0;
            z = 0.0;
        }
    }

    public static class AccelerationInfo {
        public boolean valid;
        public String datum;
        public String coordinate_system;
        public double x;
        public double y;
        public double z;
        public AccelerationInfo(){
            valid = false;
            datum = null;
            coordinate_system = null;
            x = 0.0;
            y = 0.0;
            z = 0.0;
        }
    }

    public static class NavigationPacket {
        public String source_id;
        public String rf_flow_id;
        public PositionInfo  position;
        public CartesianPositionInfo cposition;
        public VelocityInfo  velocity;
        public AccelerationInfo  acceleration;
        public AttitudeInfo  attitude;
        public BULKIO.PrecisionUTCTime timestamp;
        public CF.PropertiesHolder additional_info;
        public NavigationPacket(){
            source_id = null;
            rf_flow_id = null;
            position = new PositionInfo();
            cposition = new CartesianPositionInfo();
            velocity = new VelocityInfo();
            acceleration = new AccelerationInfo();
            attitude = new AttitudeInfo();
            timestamp = new BULKIO.PrecisionUTCTime();
            additional_info = new CF.PropertiesHolder();
        }
    }
    
    public static class frontend_tuner_allocation_struct extends StructDef {
        public final StringProperty tuner_type =
            new StringProperty(
                "FRONTEND::tuner_allocation::tuner_type", //id
                "tuner_type", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final StringProperty allocation_id =
            new StringProperty(
                "FRONTEND::tuner_allocation::allocation_id", //id
                "allocation_id", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final DoubleProperty center_frequency =
            new DoubleProperty(
                "FRONTEND::tuner_allocation::center_frequency", //id
                "center_frequency", //name
                0.0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final DoubleProperty bandwidth =
            new DoubleProperty(
                "FRONTEND::tuner_allocation::bandwidth", //id
                "bandwidth", //name
                0.0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final DoubleProperty bandwidth_tolerance =
            new DoubleProperty(
                "FRONTEND::tuner_allocation::bandwidth_tolerance", //id
                "bandwidth_tolerance", //name
                10.0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final DoubleProperty sample_rate =
            new DoubleProperty(
                "FRONTEND::tuner_allocation::sample_rate", //id
                "sample_rate", //name
                0.0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final DoubleProperty sample_rate_tolerance =
            new DoubleProperty(
                "FRONTEND::tuner_allocation::sample_rate_tolerance", //id
                "sample_rate_tolerance", //name
                10.0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final BooleanProperty device_control =
            new BooleanProperty(
                "FRONTEND::tuner_allocation::device_control", //id
                "device_control", //name
                true, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final StringProperty group_id =
            new StringProperty(
                "FRONTEND::tuner_allocation::group_id", //id
                "group_id", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final StringProperty rf_flow_id =
            new StringProperty(
                "FRONTEND::tuner_allocation::rf_flow_id", //id
                "rf_flow_id", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
    
        public frontend_tuner_allocation_struct() {
            addElement(this.tuner_type);
            addElement(this.allocation_id);
            addElement(this.center_frequency);
            addElement(this.bandwidth);
            addElement(this.bandwidth_tolerance);
            addElement(this.sample_rate);
            addElement(this.sample_rate_tolerance);
            addElement(this.device_control);
            addElement(this.group_id);
            addElement(this.rf_flow_id);
        }
    
        public String getId() {
            return "FRONTEND::tuner_allocation";
        }
    }
    
    public final StructProperty<frontend_tuner_allocation_struct> frontend_tuner_allocation =
        new StructProperty<frontend_tuner_allocation_struct>(
            "FRONTEND::tuner_allocation", //id
            "frontend_tuner_allocation", //name
            frontend_tuner_allocation_struct.class, //type
            new frontend_tuner_allocation_struct(), //default value
            Mode.READWRITE, //mode
            new Kind[] {Kind.ALLOCATION} //kind
            );
    
    public final StructProperty<frontend_scanner_allocation_struct> frontend_scanner_allocation =
        new StructProperty<frontend_scanner_allocation_struct>(
            "FRONTEND::scanner_allocation", //id
            "frontend_scanner_allocation", //name
            frontend_scanner_allocation_struct.class, //type
            new frontend_scanner_allocation_struct(), //default value
            Mode.READWRITE, //mode
            new Kind[] {Kind.ALLOCATION} //kind
            );

    public static class frontend_listener_allocation_struct extends StructDef {
        public final StringProperty existing_allocation_id =
            new StringProperty(
                "FRONTEND::listener_allocation::existing_allocation_id", //id
                "existing_allocation_id", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final StringProperty listener_allocation_id =
            new StringProperty(
                "FRONTEND::listener_allocation::listener_allocation_id", //id
                "listener_allocation_id", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
    
        public frontend_listener_allocation_struct(String existing_allocation_id, String listener_allocation_id) {
            this();
            this.existing_allocation_id.setValue(existing_allocation_id);
            this.listener_allocation_id.setValue(listener_allocation_id);
        }
    
        public frontend_listener_allocation_struct() {
            addElement(this.existing_allocation_id);
            addElement(this.listener_allocation_id);
        }
    
        public String getId() {
            return "FRONTEND::listener_allocation";
        }
    }
    
    public static class default_frontend_tuner_status_struct_struct extends StructDef {
        public final StringProperty allocation_id_csv =
            new StringProperty(
                "FRONTEND::tuner_status::allocation_id_csv", //id
                "allocation_id_csv", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final DoubleProperty bandwidth =
            new DoubleProperty(
                "FRONTEND::tuner_status::bandwidth", //id
                "bandwidth", //name
                0.0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final DoubleProperty center_frequency =
            new DoubleProperty(
                "FRONTEND::tuner_status::center_frequency", //id
                "center_frequency", //name
                0.0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final BooleanProperty enabled =
            new BooleanProperty(
                "FRONTEND::tuner_status::enabled", //id
                "enabled", //name
                false, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final StringProperty group_id =
            new StringProperty(
                "FRONTEND::tuner_status::group_id", //id
                "group_id", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final StringProperty rf_flow_id =
            new StringProperty(
                "FRONTEND::tuner_status::rf_flow_id", //id
                "rf_flow_id", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final DoubleProperty sample_rate =
            new DoubleProperty(
                "FRONTEND::tuner_status::sample_rate", //id
                "sample_rate", //name
                0.0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final StringProperty tuner_type =
            new StringProperty(
                "FRONTEND::tuner_status::tuner_type", //id
                "tuner_type", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
    
        /**
         * @generated
         */
        public default_frontend_tuner_status_struct_struct() {
            addElement(this.allocation_id_csv);
            addElement(this.bandwidth);
            addElement(this.center_frequency);
            addElement(this.enabled);
            addElement(this.group_id);
            addElement(this.rf_flow_id);
            addElement(this.sample_rate);
            addElement(this.tuner_type);
        }
    
        public String getId() {
            return "frontend_tuner_status_struct";
        }
    }

    public static class frontend_scanner_allocation_struct extends StructDef {
        public final DoubleProperty min_freq =
            new DoubleProperty(
                "FRONTEND::scanner_allocation::min_freq", //id
                "min_freq", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
        public final DoubleProperty max_freq =
            new DoubleProperty(
                "FRONTEND::scanner_allocation::max_freq", //id
                "max_freq", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
        public final StringProperty mode =
            new StringProperty(
                "FRONTEND::scanner_allocation::mode", //id
                "mode", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
        public final StringProperty control_mode =
            new StringProperty(
                "FRONTEND::scanner_allocation::control_mode", //id
                "control_mode", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
        public final DoubleProperty control_limit =
            new DoubleProperty(
                "FRONTEND::scanner_allocation::control_limit", //id
                "control_limit", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
    
        /**
         * @generated
         */
        public frontend_scanner_allocation_struct(Double min_freq, Double max_freq, String mode, String control_mode, Double control_limit) {
            this();
            this.min_freq.setValue(min_freq);
            this.max_freq.setValue(max_freq);
            this.mode.setValue(mode);
            this.control_mode.setValue(control_mode);
            this.control_limit.setValue(control_limit);
        }
    
        /**
         * @generated
         */
        public void set_min_freq(Double min_freq) {
            this.min_freq.setValue(min_freq);
        }
        public Double get_min_freq() {
            return this.min_freq.getValue();
        }
        public void set_max_freq(Double max_freq) {
            this.max_freq.setValue(max_freq);
        }
        public Double get_max_freq() {
            return this.max_freq.getValue();
        }
        public void set_mode(String mode) {
            this.mode.setValue(mode);
        }
        public String get_mode() {
            return this.mode.getValue();
        }
        public void set_control_mode(String control_mode) {
            this.control_mode.setValue(control_mode);
        }
        public String get_control_mode() {
            return this.control_mode.getValue();
        }
        public void set_control_limit(Double control_limit) {
            this.control_limit.setValue(control_limit);
        }
        public Double get_control_limit() {
            return this.control_limit.getValue();
        }
    
        /**
         * @generated
         */
        public frontend_scanner_allocation_struct() {
            addElement(this.min_freq);
            addElement(this.max_freq);
            addElement(this.mode);
            addElement(this.control_mode);
            addElement(this.control_limit);
        }
    
        public String getId() {
            return "FRONTEND::scanner_allocation";
        }
    };
}
