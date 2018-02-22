package rf_ctrl.java;

import java.util.Properties;
import org.ossie.redhawk.PortCallError;

import FRONTEND.RFInfoPkt;
import FRONTEND.GPSInfo;
import FRONTEND.GpsTimePos;
import FRONTEND.NavigationPacket;
import FRONTEND.FrontendException;
import FRONTEND.BadParameterException;
import FRONTEND.CartesianPositionInfo;
import FRONTEND.NotSupportedException;
import FRONTEND.SensorInfo;
import FRONTEND.VelocityInfo;
import FRONTEND.PathDelay;
import FRONTEND.AccelerationInfo;
import FRONTEND.AntennaInfo;
import FRONTEND.AttitudeInfo;
import FRONTEND.FeedInfo;
import FRONTEND.RFCapabilities;
import FRONTEND.FreqRange;
import FRONTEND.PositionInfo;

/**
 * This is the component code. This file contains the derived class where custom
 * functionality can be added to the component. You may add methods and code to
 * this class to handle property changes, respond to incoming data, and perform
 * general component housekeeping
 *
 * Source: rf_ctrl.spd.xml
 */
public class rf_ctrl extends rf_ctrl_base {
    /**
     * This is the component constructor. In this method, you may add
     * additional functionality to properties such as listening for changes
     * or handling allocation, register message handlers and set up internal
     * state for your component.
     *
     * A component may listen for external changes to properties (i.e., by a
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
     * component class.
     * Accessing the Application and Domain Manager:
     * 
     *  Both the Application hosting this Component and the Domain Manager hosting
     *  the Application are available to the Component.
     *  
     *  To access the Domain Manager:
     *      CF.DomainManager dommgr.setValue(this.getDomainManager().getRef();
     *  To access the Application:
     *      CF.Application app.setValue(this.getApplication().getRef();
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
     *       my_message_struct message.setValue(new my_message_struct();
     *       this.message_out.sendMessage(message);
     *
     *    You may also send a batch of messages at once with the sendMessages()
     *    method.
     */

    public rf_ctrl()
    {
        super();
    }

    public void constructor()
    {
    }


    /**
     *
     * Main processing function
     *
     * General functionality:
     *
     * The serviceFunction() is called repeatedly by the component's processing
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
     *            String stream_id.setValue("testStream");
     *            BULKIO.StreamSRI sri.setValue(new BULKIO.StreamSRI();
     *            sri.mode.setValue(0;
     *            sri.xdelta.setValue(0.0;
     *            sri.ydelta.setValue(1.0;
     *            sri.subsize.setValue(0;
     *            sri.xunits.setValue(1; // TIME_S
     *            sri.streamID.setValue((stream_id != null) ? stream_id : "");
     *
     * PrecisionUTCTime:
     *    To create a PrecisionUTCTime object, use the following code:
     *            BULKIO.PrecisionUTCTime tstamp.setValue(bulkio.time.utils.now();
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
     *    the component developer.
     *
     * Properties:
     *
     *    Properties are accessed through members of the same name; characters
     *    that are invalid for a Java identifier are replaced with "_". The
     *    current value of the property is read with getValue and written with
     *    setValue:
     *
     *        float val.setValue(this.float_prop.getValue();
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
     *    This example assumes that the component has two ports:
     *        - A bulkio.InShortPort provides (input) port called dataShort_in
     *        - A bulkio.OutFloatPort uses (output) port called dataFloat_out
     *    The mapping between the port and the class is found in the component
     *    base class file.
     *    This example also makes use of the following Properties:
     *        - A float value called amplitude with a default value of 2.0
     *        - A boolean called increaseAmplitude with a default value of true
     *
     *    bulkio.InShortPort.Packet data.setValue(this.port_dataShort_in.getPacket(125);
     *
     *    if (data != null) {
     *        float[] outData.setValue(new float[data.getData().length];
     *        for (int i.setValue(0; i < data.getData().length; i++) {
     *            if (this.increaseAmplitude.getValue()) {
     *                outData[i].setValue((float)data.getData()[i] * this.amplitude.getValue();
     *            } else {
     *                outData[i].setValue((float)data.getData()[i];
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
        this.get_rfinfo.setValue("ok");
        try {
            this.port_rfinfo_out.rf_flow_id();
        } catch (PortCallError e) {
            this.get_rfinfo.setValue(e.getMessage());
        }
        this.set_rfinfo.setValue("ok");
        try {
    		String rf_flow_id = new String("hello");
    		this.port_rfinfo_out.rf_flow_id(rf_flow_id);
        } catch (PortCallError e) {
        	this.set_rfinfo.setValue(e.getMessage());
        }
        this.get_current_rf.setValue("ok");
        try {
        	this.port_rfsource_out.current_rf_input();
        } catch (PortCallError e) {
        	this.get_current_rf.setValue(e.getMessage());
        }
        this.set_current_rf.setValue("ok");
        try {
    		RFInfoPkt foo = new RFInfoPkt();
    		foo.rf_flow_id = new String("");
    		foo.sensor = new SensorInfo();
    		foo.sensor.collector = new String("");
    		foo.sensor.antenna = new AntennaInfo();
    		foo.sensor.antenna.description = new String("");
    		foo.sensor.antenna.name = new String("");
    		foo.sensor.antenna.size = new String("");
    		foo.sensor.antenna.type = new String("");
    		foo.sensor.feed = new FeedInfo();
    		foo.sensor.feed.name = new String("");
    		foo.sensor.feed.polarization = new String("");
    		foo.sensor.feed.freq_range = new FreqRange();
    		foo.sensor.feed.freq_range.values = new double[0];
    		foo.sensor.mission = new String("");
    		foo.sensor.rx = new String("");
            foo.ext_path_delays = new PathDelay[0];
            foo.capabilities = new RFCapabilities();
            foo.capabilities.freq_range = new FreqRange();
            foo.capabilities.freq_range.values = new double[0];
            foo.capabilities.bw_range = new FreqRange();
            foo.capabilities.bw_range.values = new double[0];
            foo.additional_info = new CF.DataType[0];
    		this.port_rfsource_out.current_rf_input(foo);
        } catch (PortCallError e) {
        	this.set_current_rf.setValue(e.getMessage());
        }
        this.get_available_rf.setValue("ok");
        try {
        	this.port_rfsource_out.available_rf_inputs();
        } catch (PortCallError e) {
        	this.get_available_rf.setValue(e.getMessage());
        }
        this.set_available_rf.setValue("ok");
        try {
    		RFInfoPkt[] foo = new RFInfoPkt[0];
    		this.port_rfsource_out.available_rf_inputs(foo);
        } catch (PortCallError e) {
        	this.set_available_rf.setValue(e.getMessage());
        }
        this.bad_connection.setValue("ok");
        try {
        	this.port_rfsource_out._get_available_rf_inputs("invalid_connectionid");
        } catch (PortCallError e) {
        	this.bad_connection.setValue(e.getMessage());
        }

        String tmp = new String("");
        this.get_tunertype.setValue("ok");
        try {
        	this.port_digitaltuner_out.getTunerType(tmp);
        } catch (PortCallError e) {
        	this.get_tunertype.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.get_tunerdevicecontrol.setValue("ok");
        try {
    		this.port_digitaltuner_out.getTunerDeviceControl(tmp);
        } catch (PortCallError e) {
        	this.get_tunerdevicecontrol.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.get_tunergroupid.setValue("ok");
        try {
        	this.port_digitaltuner_out.getTunerGroupId(tmp);
        } catch (PortCallError e) {
        	this.get_tunergroupid.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.get_tunerrfflowid.setValue("ok");
        try {
    		this.port_digitaltuner_out.getTunerRfFlowId(tmp);
        } catch (PortCallError e) {
        	this.get_tunerrfflowid.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.get_tunerstatus.setValue("ok");
        try {
        	this.port_digitaltuner_out.getTunerStatus(tmp);
        } catch (PortCallError e) {
        	this.get_tunerstatus.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.get_tunercenterfrequency.setValue("ok");
        try {
    		this.port_digitaltuner_out.getTunerCenterFrequency(tmp);
        } catch (PortCallError e) {
        	this.get_tunercenterfrequency.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.set_tunercenterfrequency.setValue("ok");
        try {
        	this.port_digitaltuner_out.setTunerCenterFrequency(tmp, 1.0);
        } catch (PortCallError e) {
        	this.set_tunercenterfrequency.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.get_tunerbandwidth.setValue("ok");
        try {
    		this.port_digitaltuner_out.getTunerBandwidth(tmp);
        } catch (PortCallError e) {
        	this.get_tunerbandwidth.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.set_tunerbandwidth.setValue("ok");
        try {
        	this.port_digitaltuner_out.setTunerBandwidth(tmp, 1.0);
        } catch (PortCallError e) {
        	this.set_tunerbandwidth.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.get_tuneragcenable.setValue("ok");
        try {
    		this.port_digitaltuner_out.getTunerAgcEnable(tmp);
        } catch (PortCallError e) {
        	this.get_tuneragcenable.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.set_tuneragcenable.setValue("ok");
        try {
        	this.port_digitaltuner_out.setTunerAgcEnable(tmp, false);
        } catch (PortCallError e) {
        	this.set_tuneragcenable.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.get_tunergain.setValue("ok");
        try {
    		this.port_digitaltuner_out.getTunerGain(tmp);
        } catch (PortCallError e) {
        	this.get_tunergain.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.set_tunergain.setValue("ok");
        try {
        	this.port_digitaltuner_out.setTunerGain(tmp, (float)1.0);
        } catch (PortCallError e) {
        	this.set_tunergain.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.get_tunerreferencesource.setValue("ok");
        try {
    		this.port_digitaltuner_out.getTunerReferenceSource(tmp);
        } catch (PortCallError e) {
        	this.get_tunerreferencesource.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.set_tunerreferencesource.setValue("ok");
        try {
        	this.port_digitaltuner_out.setTunerReferenceSource(tmp, 2);
        } catch (PortCallError e) {
        	this.set_tunerreferencesource.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.get_tunerenable.setValue("ok");
        try {
    		this.port_digitaltuner_out.getTunerEnable(tmp);
        } catch (PortCallError e) {
        	this.get_tunerenable.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.set_tunerenable.setValue("ok");
        try {
        	this.port_digitaltuner_out.setTunerEnable(tmp, false);
        } catch (PortCallError e) {
        	this.set_tunerenable.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.get_tuneroutputsamplerate.setValue("ok");
        try {
    		this.port_digitaltuner_out.getTunerOutputSampleRate(tmp);
        } catch (PortCallError e) {
        	this.get_tuneroutputsamplerate.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }
        this.set_tuneroutputsamplerate.setValue("ok");
        try {
        	this.port_digitaltuner_out.setTunerOutputSampleRate(tmp, 1.0);
        } catch (PortCallError e) {
        	this.set_tuneroutputsamplerate.setValue(e.getMessage());
        } catch (FrontendException | BadParameterException | NotSupportedException e) {
            this.get_rfinfo.setValue("unexpected exception");
        }

        this.get_gpsinfo.setValue("ok");
        try {
    		this.port_gps_out.gps_info();
        } catch (PortCallError e) {
        	this.get_gpsinfo.setValue(e.getMessage());
        }
        this.set_gpsinfo.setValue("ok");
        try {
        	GPSInfo _gps = new GPSInfo();
        	_gps.additional_info = new CF.DataType[0];
        	_gps.mode = new String("");
        	_gps.rf_flow_id = new String("");
        	_gps.source_id = new String("");
        	_gps.status_message = new String("");
        	_gps.timestamp = new BULKIO.PrecisionUTCTime();
        	_gps.timestamp = bulkio.time.utils.now();
        	this.port_gps_out.gps_info(_gps);
        } catch (PortCallError e) {
        	this.set_gpsinfo.setValue(e.getMessage());
        }
        this.get_gps_timepos.setValue("ok");
        try {
    		this.port_gps_out.gps_time_pos();
        } catch (PortCallError e) {
        	this.get_gps_timepos.setValue(e.getMessage());
        }
        this.set_gps_timepos.setValue("ok");
        try {
        	GpsTimePos _gps = new GpsTimePos();
        	_gps.position = new PositionInfo();
        	_gps.position.datum = new String("");
        	_gps.timestamp = new BULKIO.PrecisionUTCTime();
        	_gps.timestamp = bulkio.time.utils.now();
        	this.port_gps_out.gps_time_pos(_gps);
        } catch (PortCallError e) {
        	this.set_gps_timepos.setValue(e.getMessage());
        }

        this.get_nav_packet.setValue("ok");
        try {
    		this.port_navdata_out.nav_packet();
        } catch (PortCallError e) {
        	this.get_nav_packet.setValue(e.getMessage());
        }
        this.set_nav_packet.setValue("ok");
        try {
        	NavigationPacket _nav = new NavigationPacket();
        	_nav.acceleration = new AccelerationInfo();
        	_nav.acceleration.coordinate_system = new String("");
        	_nav.acceleration.datum = new String("");
        	_nav.additional_info = new CF.DataType[0];
        	_nav.attitude = new AttitudeInfo();
        	_nav.cposition = new CartesianPositionInfo();
        	_nav.cposition.datum = new String("");
        	_nav.position = new PositionInfo();
        	_nav.position.datum = new String("");
        	_nav.rf_flow_id = new String("");
        	_nav.source_id = new String("");
        	_nav.timestamp = new BULKIO.PrecisionUTCTime();
        	_nav.timestamp = bulkio.time.utils.now();
        	_nav.velocity = new VelocityInfo();
        	_nav.velocity.coordinate_system = new String("");
        	_nav.velocity.datum = new String("");
        	this.port_navdata_out.nav_packet(_nav);
        } catch (PortCallError e) {
        	this.set_nav_packet.setValue(e.getMessage());
        }
        
        return FINISH;
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

}
