/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK codegenTesting.
 *
 * REDHAWK codegenTesting is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK codegenTesting is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
package fei_exception_through.java;

import java.util.Properties;
import org.omg.CORBA.ORB;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
import CF.DeviceManager;
import CF.DevicePackage.InvalidCapacity;
import FRONTEND.BadParameterException;
import FRONTEND.FrontendException;
import FRONTEND.NotSupportedException;
import CF.DevicePackage.InvalidCapacity;
import CF.InvalidObjectReference;
import org.ossie.redhawk.PortCallError;

/**
 * This is the device code. This file contains the derived class where custom
 * functionality can be added to the device. You may add methods and code to
 * this class to handle property changes, respond to incoming data, and perform
 * general device housekeeping
 *
 * Source: fei_exception_through.spd.xml
 */
public class fei_exception_through extends fei_exception_through_base {
    /**
     * This is the device constructor. In this method, you may add
     * additional functionality to properties such as listening for changes
     * or handling allocation, register message handlers and set up internal
     * state for your device.
     *
     * A device may listen for external changes to properties (i.e., by a
     * call to configure) using the PropertyListener interface. Listeners are
     * registered by calling addChangeListener() on the property instance
     * with an object that implements the PropertyListener interface for that
     * data type (e.g., "PropertyListener<Float>" for a float property). More
     * than one listener can be connected to a property.
     *
     *   Example:
     *       // This example makes use of the following properties:
     *       //  - A float value called scaleValue
     *       // The file must import "org.ossie.properties.PropertyListener"
     *       // Add the following import to the top of the file:
     *       import org.ossie.properties.PropertyListener;
     *
     *       //Add the following to the class constructor:
     *       this.scaleValue.addChangeListener(new PropertyListener<Float>() {
     *           public void valueChanged(Float oldValue, Float newValue) {
     *               scaleValueChanged(oldValue, newValue);
     *           }
     *       });
     *
     *       //Add the following method to the class:
     *       private void scaleValueChanged(Float oldValue, Float newValue)
     *       {
     *          logger.debug("Changed scaleValue " + oldValue + " to " + newValue);
     *       }
     *
     * The recommended practice is for the implementation of valueChanged() to
     * contain only glue code to dispatch the call to a private method on the
     * device class.
     *
     * Devices may contain allocation properties with "external" action, which
     * are used in capacity allocation and deallocation. In order to support
     * this capability, allocation properties require additional functionality.
     * This is implemented by calling setAllocator() on the property instance
     * with an object that implements the Allocator interface for that data type.
     *
     *   Example:
     *       // This example makes use of the following properties
     *       //  - A struct property called tuner_alloc
     *       // The following methods are defined elsewhere in your class:
     *       //  - private boolean allocate_tuner(tuner_alloc_struct capacity)
     *       //  - private void deallocate_tuner(tuner_alloc_struct capacity)
     *       // The file must import "org.ossie.properties.Allocator"
     *
     *       this.tuner_alloc.setAllocator(new Allocator<tuner_alloc_struct>() {
     *           public boolean allocate(tuner_alloc_struct capacity) {
     *               return allocate_tuner(capacity);
     *           }
     *           public void deallocate(tuner_alloc_struct capacity) {
     *               deallocate_tuner(capacity);
     *           }
     *       });
     *
     * The recommended practice is for the allocate() and deallocate() methods
     * to contain only glue code to dispatch the call to private methods on the
     * device class.
     * Accessing the Device Manager and Domain Manager:
     * 
     *  Both the Device Manager hosting this Device and the Domain Manager hosting
     *  the Device Manager are available to the Device.
     *  
     *  To access the Domain Manager:
     *      CF.DomainManager dommgr = this.getDomainManager().getRef();
     *  To access the Device Manager:
     *      CF.DeviceManager devmgr = this.getDeviceManager().getRef();
     *
     * Messages:
     *
     *   To send or receive messages, you must have at least one message
     *   prototype described as a struct property of kind "message."
     *
     *   Receiving:
     *
     *   To receive a message, you must have an input port of type MessageEvent
     *   (marked as "bi-dir" in the Ports editor). For each message type the
     *   component supports, you must register a message handler callback with
     *   the message input port. Message handlers implement the MessageListener
     *   interface.
     *
     *   A callback is registered by calling registerMessage() on the message
     *   input port with the message ID, the message struct's Class object and
     *   an object that implements the MessageListener interface for that
     *   message struct (e.g., "MessageListener<my_message_struct>" for a
     *   message named "my_message").
     *
     *     Example:
     *       // Assume the component has a message type called "my_message" and
     *       // an input MessageEvent port called "message_in".
     *       // Add the following to the top of the file:
     *       import org.ossie.events.MessageListener;
     *
     *       // Register the callback in the class constructor:
     *       this.message_in.registerMessage("my_message", my_message_struct.class, new MessageListener<my_message_struct>() {
     *           public void messageReceived(String messageId, my_message_struct messageData) {
     *               my_message_received(messageData);
     *           }
     *       });
     *
     *       // Implement the message handler method:
     *       private void my_message_received(my_message_struct messageData) {
     *           // Respond to the message
     *       }
     *
     *   The recommended practice is for the implementation of messageReceived()
     *   to contain only glue code to dispatch the call to a private method on
     *   the component class.
     *
     *   Sending:
     *
     *   To send a message, you must have an output port of type MessageEvent.
     *   Create an instance of the message struct type and call sendMessage()
     *   to send a single message.
     *
     *     Example:
     *       // Assume the component has a message type called "my_message" and
     *       // an output MessageEvent port called "message_out".
     *       my_message_struct message = new my_message_struct();
     *       this.message_out.sendMessage(message);
     *
     *    You may also send a batch of messages at once with the sendMessages()
     *    method.
     */

    public fei_exception_through()
    {
        super();
    }

    public void constructor()
    {
    /**************************************************************************
    
     For a tuner device, the structure frontend_tuner_status needs to match the number
     of tuners that this device controls and what kind of device it is.
     The options for devices are: TX, RX, RX_DIGITIZER, CHANNELIZER, DDC, RC_DIGITIZER_CHANNELIZER
     
     For example, if this device has 5 physical
     tuners, each an RX_DIGITIZER, then the code in the construct function should look like this:

     this.setNumChannels(5, "RX_DIGITIZER");
     
     The incoming request for tuning contains a string describing the requested tuner
     type. The string for the request must match the string in the tuner status.
     
    **************************************************************************/
    this.setNumChannels(1, "RX_DIGITIZER");
    }


    /**
     *
     * Main processing function
     *
     * General functionality:
     *
     * The serviceFunction() is called repeatedly by the device's processing
     * thread, which runs independently of the main thread. Each invocation
     * should perform a single unit of work, such as reading and processing one
     * data packet.
     *
     * The return status of serviceFunction() determines how soon the next
     * invocation occurs:
     *   - NORMAL: the next call happens immediately
     *   - NOOP:   the next call happens after a pre-defined delay (100 ms)
     *   - FINISH: no more calls occur
     *
     * StreamSRI:
     *    To create a StreamSRI object, use the following code:
     *            String stream_id = "testStream";
     *            BULKIO.StreamSRI sri = new BULKIO.StreamSRI();
     *            sri.mode = 0;
     *            sri.xdelta = 0.0;
     *            sri.ydelta = 1.0;
     *            sri.subsize = 0;
     *            sri.xunits = 1; // TIME_S
     *            sri.streamID = (stream_id != null) ? stream_id : "";
     *
     * PrecisionUTCTime:
     *    To create a PrecisionUTCTime object, use the following code:
     *            BULKIO.PrecisionUTCTime tstamp = bulkio.time.utils.now();
     *
     * Ports:
     *
     *    Each port instance is accessed through members of the following form:
     *
     *        this.port_<PORT NAME>
     *
     *    Input BULKIO data is obtained by calling getPacket on the provides
     *    port. The getPacket method takes one argument: the time to wait for
     *    data to arrive, in milliseconds. A timeout of 0 causes getPacket to
     *    return immediately, while a negative timeout indicates an indefinite
     *    wait. If no data is queued and no packet arrives during the waiting
     *    period, getPacket returns null.
     *
     *    Output BULKIO data is sent by calling pushPacket on the uses port. In
     *    the case of numeric data, the pushPacket method takes a primitive
     *    array (e.g., "float[]"), a timestamp, an end-of-stream flag and a
     *    stream ID. You must make at least one call to pushSRI to associate a
     *    StreamSRI with the stream ID before calling pushPacket, or receivers
     *    may drop the data.
     *
     *    When all processing on a stream is complete, a call should be made to
     *    pushPacket with the end-of-stream flag set to "true".
     *
     *    Interactions with non-BULKIO ports are left up to the discretion of
     *    the device developer.
     *
     * Properties:
     *
     *    Properties are accessed through members of the same name; characters
     *    that are invalid for a Java identifier are replaced with "_". The
     *    current value of the property is read with getValue and written with
     *    setValue:
     *
     *        float val = this.float_prop.getValue();
     *        ...
     *        this.float_prop.setValue(1.5f);
     *
     *    Primitive data types are stored using the corresponding Java object
     *    wrapper class. For example, a property of type "float" is stored as a
     *    Float. Java will automatically box and unbox primitive types where
     *    appropriate.
     *
     *    Numeric properties support assignment via setValue from any numeric
     *    type. The standard Java type coercion rules apply (e.g., truncation
     *    of floating point values when converting to integer types).
     *
     * Example:
     *
     *    This example assumes that the device has two ports:
     *        - A bulkio.InShortPort provides (input) port called dataShort_in
     *        - A bulkio.OutFloatPort uses (output) port called dataFloat_out
     *    The mapping between the port and the class is found in the device
     *    base class file.
     *    This example also makes use of the following Properties:
     *        - A float value called amplitude with a default value of 2.0
     *        - A boolean called increaseAmplitude with a default value of true
     *
     *    bulkio.InShortPort.Packet data = this.port_dataShort_in.getPacket(125);
     *
     *    if (data != null) {
     *        float[] outData = new float[data.getData().length];
     *        for (int i = 0; i < data.getData().length; i++) {
     *            if (this.increaseAmplitude.getValue()) {
     *                outData[i] = (float)data.getData()[i] * this.amplitude.getValue();
     *            } else {
     *                outData[i] = (float)data.getData()[i];
     *            }
     *        }
     *
     *        // NOTE: You must make at least one valid pushSRI call
     *        if (data.sriChanged()) {
     *            this.port_dataFloat_out.pushSRI(data.getSRI());
     *        }
     *        this.port_dataFloat_out.pushPacket(outData, data.getTime(), data.getEndOfStream(), data.getStreamID());
     *    }
     *
     */
    protected int serviceFunction() {
        logger.debug("serviceFunction() example log message");

        return NOOP;
    }

    /**
     * Set additional options for ORB startup. For example:
     *
     *   orbProps.put("com.sun.CORBA.giop.ORBFragmentSize", Integer.toString(fragSize));
     *
     * @param orbProps
     */
    public static void configureOrb(final Properties orbProps) {
    }


    /*************************************************************
    Functions supporting tuning allocation
    *************************************************************/
    public void deviceEnable(frontend_tuner_status_struct_struct fts, int tuner_id)
    {
        /************************************************************
        modify fts, which corresponds to this.frontend_tuner_status.getValue().get(tuner_id)
        Make sure to set the 'enabled' member of fts to indicate that tuner as enabled
        ************************************************************/
        System.out.println("deviceEnable(): Enable the given tuner  *********");
        fts.enabled.setValue(true);
        return;
    }
    public void deviceDisable(frontend_tuner_status_struct_struct fts, int tuner_id)
    {
        /************************************************************
        modify fts, which corresponds to this.frontend_tuner_status.getValue().get(tuner_id)
        Make sure to reset the 'enabled' member of fts to indicate that tuner as disabled
        ************************************************************/
        System.out.println("deviceDisable(): Disable the given tuner  *********");
        fts.enabled.setValue(false);
        return;
    }
    public boolean deviceSetTuning(final frontend.FETypes.frontend_tuner_allocation_struct request, frontend_tuner_status_struct_struct fts, int tuner_id)
    {
        /************************************************************
        modify fts, which corresponds to this.frontend_tuner_status.getValue().get(tuner_id)
        
        The bandwidth, center frequency, and sampling rate that the hardware was actually tuned
        to needs to populate fts (to make sure that it meets the tolerance requirement. For example,
        if the tuned values match the requested values, the code would look like this:
        
        fts.bandwidth.setValue(request.bandwidth.getValue());
        fts.center_frequency.setValue(request.center_frequency.getValue());
        fts.sample_rate.setValue(request.sample_rate.getValue());
        
        return true if the tuning succeeded, and false if it failed
        ************************************************************/
        System.out.println("deviceSetTuning(): Evaluate whether or not a tuner is added  *********");
        return true;
    }
    public boolean deviceSetTuningScan(final frontend.FETypes.frontend_tuner_allocation_struct request, final frontend.FETypes.frontend_scanner_allocation_struct scan_request, frontend_tuner_status_struct_struct fts, int tuner_id)
    {
        /************************************************************
        modify fts, which corresponds to this.frontend_tuner_status.getValue().get(tuner_id)
        
        The bandwidth, center frequency, and sampling rate that the hardware was actually tuned
        to needs to populate fts (to make sure that it meets the tolerance requirement. For example,
        if the tuned values match the requested values, the code would look like this:
        
        fts.bandwidth.setValue(request.bandwidth.getValue());
        fts.center_frequency.setValue(request.center_frequency.getValue());
        fts.sample_rate.setValue(request.sample_rate.getValue());
        
        return true if the tuning succeeded, and false if it failed
        ************************************************************/
        System.out.println("deviceSetTuning(): Evaluate whether or not a tuner is added  *********");
        return true;
    }
    public boolean deviceDeleteTuning(frontend_tuner_status_struct_struct fts, int tuner_id)
    {
        /************************************************************
        modify fts, which corresponds to this.frontend_tuner_status.getValue().get(tuner_id)
        return true if the tune deletion succeeded, and false if it failed
        ************************************************************/
        System.out.println("deviceDeleteTuning(): Deallocate an allocated tuner  *********");
        return true;
    }

    public void setScanStrategy(final String allocation_id, final FRONTEND.ScanningTunerPackage.ScanStrategy scan_strategy)
    {
        try {
            this.port_DigitalTuner_out.setScanStrategy(allocation_id, scan_strategy);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public void setScanStartTime(final String allocation_id, BULKIO.PrecisionUTCTime start_time)
    {
        try {
            this.port_DigitalTuner_out.setScanStartTime(allocation_id, start_time);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public FRONTEND.ScanningTunerPackage.ScanStatus getScanStatus(final String allocation_id)
    {
        try {
            return this.port_DigitalTuner_out.getScanStatus(allocation_id);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public String getTunerType(final String allocation_id)
    {
        try {
            return this.port_DigitalTuner_out.getTunerType(allocation_id);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public boolean getTunerDeviceControl(final String allocation_id)
    {
        try {
            return this.port_DigitalTuner_out.getTunerDeviceControl(allocation_id);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public String getTunerGroupId(final String allocation_id)
    {
        try {
            return this.port_DigitalTuner_out.getTunerGroupId(allocation_id);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public String getTunerRfFlowId(final String allocation_id) throws FRONTEND.FrontendException, BadParameterException, NotSupportedException
    {
        try {
            return this.port_DigitalTuner_out.getTunerRfFlowId(allocation_id);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public void setTunerCenterFrequency(final String allocation_id, double freq) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, NotSupportedException
    {
        try {
            this.port_DigitalTuner_out.setTunerCenterFrequency(allocation_id, freq);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public double getTunerCenterFrequency(final String allocation_id) throws FRONTEND.FrontendException, BadParameterException, NotSupportedException
    {
        try {
            return this.port_DigitalTuner_out.getTunerCenterFrequency(allocation_id);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public void setTunerBandwidth(final String allocation_id, double bw) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, NotSupportedException
    {
        try {
            this.port_DigitalTuner_out.setTunerBandwidth(allocation_id, bw);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public double getTunerBandwidth(final String allocation_id) throws FRONTEND.FrontendException, BadParameterException, NotSupportedException
    {
        try {
            return this.port_DigitalTuner_out.getTunerBandwidth(allocation_id);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public void setTunerAgcEnable(final String allocation_id, boolean enable) throws FRONTEND.NotSupportedException, FrontendException, BadParameterException
    {
        try {
            this.port_DigitalTuner_out.setTunerAgcEnable(allocation_id, enable);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public boolean getTunerAgcEnable(final String allocation_id) throws FRONTEND.NotSupportedException, FrontendException, BadParameterException
    {
        try {
            return this.port_DigitalTuner_out.getTunerAgcEnable(allocation_id);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public void setTunerGain(final String allocation_id, float gain) throws FRONTEND.NotSupportedException, FrontendException, BadParameterException
    {
        try {
            this.port_DigitalTuner_out.setTunerGain(allocation_id, gain);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public float getTunerGain(final String allocation_id) throws FRONTEND.NotSupportedException, FrontendException, BadParameterException
    {
        try {
            return this.port_DigitalTuner_out.getTunerGain(allocation_id);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public void setTunerReferenceSource(final String allocation_id, int source) throws FRONTEND.NotSupportedException, FrontendException, BadParameterException
    {
        try {
            this.port_DigitalTuner_out.setTunerReferenceSource(allocation_id, source);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public int getTunerReferenceSource(final String allocation_id) throws FRONTEND.NotSupportedException, FrontendException, BadParameterException
    {
        try {
            return this.port_DigitalTuner_out.getTunerReferenceSource(allocation_id);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public void setTunerEnable(final String allocation_id, boolean enable) throws FRONTEND.FrontendException, BadParameterException, NotSupportedException
    {
        try {
            this.port_DigitalTuner_out.setTunerEnable(allocation_id, enable);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public boolean getTunerEnable(final String allocation_id) throws FRONTEND.FrontendException, BadParameterException, NotSupportedException
    {
        try {
            return this.port_DigitalTuner_out.getTunerEnable(allocation_id);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public void setTunerOutputSampleRate(final String allocation_id, double sr) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, NotSupportedException
    {
        try {
            this.port_DigitalTuner_out.setTunerOutputSampleRate(allocation_id, sr);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public double getTunerOutputSampleRate(final String allocation_id) throws FRONTEND.FrontendException, BadParameterException, NotSupportedException
    {
        try {
            return this.port_DigitalTuner_out.getTunerOutputSampleRate(allocation_id);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public FRONTEND.GPSInfo get_gps_info(final String port_name)
    {
        try {
            return this.port_GPS_out.gps_info();
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public void set_gps_info(final String port_name, final FRONTEND.GPSInfo gps_info)
    {
        try {
            this.port_GPS_out.gps_info(gps_info);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public FRONTEND.GpsTimePos get_gps_time_pos(final String port_name)
    {
        try {
            return this.port_GPS_out.gps_time_pos();
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public void set_gps_time_pos(final String port_name, final FRONTEND.GpsTimePos gps_time_pos)
    {
        try {
            this.port_GPS_out.gps_time_pos(gps_time_pos);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public FRONTEND.NavigationPacket get_nav_packet(final String port_name)
    {
        try {
            return this.port_NavData_out.nav_packet();
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public void set_nav_packet(final String port_name, final FRONTEND.NavigationPacket nav_info)
    {
        try {
            this.port_NavData_out.nav_packet(nav_info);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    /*************************************************************
    Functions servicing the RFInfo port(s)
    - port_name is the port over which the call was received
    *************************************************************/
    public String get_rf_flow_id(final String port_name)
    {
        try {
            return this.port_RFInfo_out.rf_flow_id();
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public void set_rf_flow_id(final String port_name, final String id)
    {
        try {
            this.port_RFInfo_out.rf_flow_id(id);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public FRONTEND.RFInfoPkt get_rfinfo_pkt(final String port_name)
    {
        try {
            return this.port_RFInfo_out.rfinfo_pkt();
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public void set_rfinfo_pkt(final String port_name, final FRONTEND.RFInfoPkt pkt)
    {
        try {
            this.port_RFInfo_out.rfinfo_pkt(pkt);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public FRONTEND.RFInfoPkt[] get_available_rf_inputs(final String port_name)
    {
        try {
            return this.port_RFSource_out.available_rf_inputs();
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public void set_available_rf_inputs(final String port_name, final FRONTEND.RFInfoPkt[] inputs)
    {
        try {
            this.port_RFSource_out.available_rf_inputs(inputs);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public FRONTEND.RFInfoPkt get_current_rf_input(final String port_name)
    {
        try {
            return this.port_RFSource_out.current_rf_input();
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }

    public void set_current_rf_input(final String port_name, final FRONTEND.RFInfoPkt pkt)
    {
        try {
            this.port_RFSource_out.current_rf_input(pkt);
        } catch (PortCallError e) {
            throw new RuntimeException("Port call error: "+e.getMessage());
        }
    }
}
