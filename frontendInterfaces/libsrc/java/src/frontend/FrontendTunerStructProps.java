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

import java.util.HashMap;
import java.util.Properties;

import org.ossie.component.*;
import org.ossie.properties.*;

public class FrontendTunerStructProps {

    /**
     * The structure for property FRONTEND::tuner_allocation
     */
    public static class frontend_tuner_allocation_struct extends StructDef {
        /**
         * The property FRONTEND::tuner_allocation::tuner_type
         * Example Tuner Types: TX, RX, CHANNELIZER, DDC, RX_DIGITIZER, RX_DIGTIZIER_CHANNELIZER
         */
        public final StringProperty tuner_type =
            new StringProperty(
                "FRONTEND::tuner_allocation::tuner_type", //id
                "tuner_type", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property FRONTEND::tuner_allocation::allocation_id
         * The allocation_id set by the caller. Used by the caller to reference the device uniquely
         */
        public final StringProperty allocation_id =
            new StringProperty(
                "FRONTEND::tuner_allocation::allocation_id", //id
                "allocation_id", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property FRONTEND::tuner_allocation::center_frequency
         * Requested center frequency.
         */
        public final DoubleProperty center_frequency =
            new DoubleProperty(
                "FRONTEND::tuner_allocation::center_frequency", //id
                "center_frequency", //name
                0.0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property FRONTEND::tuner_allocation::bandwidth
         * Requested Bandwidth
         */
        public final DoubleProperty bandwidth =
            new DoubleProperty(
                "FRONTEND::tuner_allocation::bandwidth", //id
                "bandwidth", //name
                0.0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property FRONTEND::tuner_allocation::bandwidth_tolerance
         * Allowable Percent above requested bandwidth  (ie - 100 would be up to twice)
         */
        public final DoubleProperty bandwidth_tolerance =
            new DoubleProperty(
                "FRONTEND::tuner_allocation::bandwidth_tolerance", //id
                "bandwidth_tolerance", //name
                10.0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property FRONTEND::tuner_allocation::sample_rate
         * Requested sample rate. This can be ignored for such devices as analog tuners
         */
        public final DoubleProperty sample_rate =
            new DoubleProperty(
                "FRONTEND::tuner_allocation::sample_rate", //id
                "sample_rate", //name
                0.0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property FRONTEND::tuner_allocation::sample_rate_tolerance
         * Allowable Percent above requested sample rate (ie - 100 would be up to twice)
         */
        public final DoubleProperty sample_rate_tolerance =
            new DoubleProperty(
                "FRONTEND::tuner_allocation::sample_rate_tolerance", //id
                "sample_rate_tolerance", //name
                10.0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property FRONTEND::tuner_allocation::device_control
         * True: Has control over the device to make changes
         * False: Does not need control and can just attach to any currently tasked device that satisfies the parameters (essentually a listener)
         */
        public final BooleanProperty device_control =
            new BooleanProperty(
                "FRONTEND::tuner_allocation::device_control", //id
                "device_control", //name
                true, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property FRONTEND::tuner_allocation::group_id
         * Unique identifier that specifies a group of device. Must match group_id on the device
         */
        public final StringProperty group_id =
            new StringProperty(
                "FRONTEND::tuner_allocation::group_id", //id
                "group_id", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property FRONTEND::tuner_allocation::rf_flow_id
         * Optional. Specifies a certain RF flow to allocate against. If left empty, it will match all frontend devices.
         */
        public final StringProperty rf_flow_id =
            new StringProperty(
                "FRONTEND::tuner_allocation::rf_flow_id", //id
                "rf_flow_id", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
    
        public frontend_tuner_allocation_struct(String tuner_type, String allocation_id, Double center_frequency, Double bandwidth, Double bandwidth_tolerance, Double sample_rate, Double sample_rate_tolerance, Boolean device_control, String group_id, String rf_flow_id) {
            this();
            this.tuner_type.setValue(tuner_type);
            this.allocation_id.setValue(allocation_id);
            this.center_frequency.setValue(center_frequency);
            this.bandwidth.setValue(bandwidth);
            this.bandwidth_tolerance.setValue(bandwidth_tolerance);
            this.sample_rate.setValue(sample_rate);
            this.sample_rate_tolerance.setValue(sample_rate_tolerance);
            this.device_control.setValue(device_control);
            this.group_id.setValue(group_id);
            this.rf_flow_id.setValue(rf_flow_id);
        }
    
        /**
         * @generated
         */
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
    };
    
    /**
     * The structure for property FRONTEND::listener_allocation
     */
    public static class frontend_listener_allocation_struct extends StructDef {
        /**
         * The property FRONTEND::listener_allocation::existing_allocation_id
         * If the meaning of this property isn't clear, a description should be added.
         */
        public final StringProperty existing_allocation_id =
            new StringProperty(
                "FRONTEND::listener_allocation::existing_allocation_id", //id
                "existing_allocation_id", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property FRONTEND::listener_allocation::listener_allocation_id
         * If the meaning of this property isn't clear, a description should be added.
         */
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
    };
    
    /**
     * The structure for default_frontend_tuner_status_struct
     */
    public static class default_frontend_tuner_status_struct_struct extends StructDef {
        /**
         * The property FRONTEND::tuner_status::tuner_type
         * Example Tuner Types: TX, RX, CHANNELIZER, DDC, RX_DIGITIZER, RX_DIGTIZIER_CHANNELIZER
         */
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
         * The property FRONTEND::tuner_status::allocation_id_csv
         * The allocation_id's set by the caller
         */
        public final StringProperty allocation_id_csv =
            new StringProperty(
                "FRONTEND::tuner_status::allocation_id_csv", //id
                "allocation_id_csv", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property FRONTEND::tuner_status::center_frequency
         * Requested center frequency.
         */
        public final DoubleProperty center_frequency =
            new DoubleProperty(
                "FRONTEND::tuner_status::center_frequency", //id
                "center_frequency", //name
                0.0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property FRONTEND::tuner_status::bandwidth
         * Requested Bandwidth
         */
        public final DoubleProperty bandwidth =
            new DoubleProperty(
                "FRONTEND::tuner_status::bandwidth", //id
                "bandwidth", //name
                0.0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property FRONTEND::tuner_status::sample_rate
         * Requested sample rate. This can be ignored for such devices as analog tuners
         */
        public final DoubleProperty sample_rate =
            new DoubleProperty(
                "FRONTEND::tuner_status::sample_rate", //id
                "sample_rate", //name
                0.0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property FRONTEND::tuner_status::group_id
         * Unique identifier that specifies a group of device. Must match group_id on the device
         */
        public final StringProperty group_id =
            new StringProperty(
                "FRONTEND::tuner_status::group_id", //id
                "group_id", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property FRONTEND::tuner_status::rf_flow_id
         * Optional. Specifies a certain RF flow to allocate against. If left empty, it will match all frontend devices.
         */
        public final StringProperty rf_flow_id =
            new StringProperty(
                "FRONTEND::tuner_status::rf_flow_id", //id
                "rf_flow_id", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property FRONTEND::tuner_status::enabled
         * If the meaning of this property isn't clear, a description should be added.
         */
        public final BooleanProperty enabled =
            new BooleanProperty(
                "FRONTEND::tuner_status::enabled", //id
                "enabled", //name
                false, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );

        public default_frontend_tuner_status_struct_struct(String tuner_type, String allocation_id_csv, Double center_frequency, Double bandwidth, Double sample_rate, String group_id, String rf_flow_id, Boolean enabled) {
            this();
            this.tuner_type.setValue(tuner_type);
            this.allocation_id_csv.setValue(allocation_id_csv);
            this.center_frequency.setValue(center_frequency);
            this.bandwidth.setValue(bandwidth);
            this.sample_rate.setValue(sample_rate);
            this.group_id.setValue(group_id);
            this.rf_flow_id.setValue(rf_flow_id);
            this.enabled.setValue(enabled);
        }
    
        public default_frontend_tuner_status_struct_struct() {
            addElement(this.tuner_type);
            addElement(this.allocation_id_csv);
            addElement(this.center_frequency);
            addElement(this.bandwidth);
            addElement(this.sample_rate);
            addElement(this.group_id);
            addElement(this.rf_flow_id);
            addElement(this.enabled);
        }
    };

}
