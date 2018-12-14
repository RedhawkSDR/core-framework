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
/*{% block license %}*/
/*# Allow child templates to include license #*/
/*{% endblock %}*/
//% set classname = component.userclass.name
//% set baseclass = component.baseclass.name
//% set artifactType = component.artifacttype
package ${component.package};

import java.util.Properties;
import org.ossie.component.RHLogger;
/*{% block mainadditionalimports %}*/
/*# Allow for child class imports #*/
/*{% endblock %}*/

/**
 * This is the ${artifactType} code. This file contains the derived class where custom
 * functionality can be added to the ${artifactType}. You may add methods and code to
 * this class to handle property changes, respond to incoming data, and perform
 * general ${artifactType} housekeeping
 *
 * Source: ${component.profile.spd}
 */
public class ${classname} extends ${baseclass} {
    /**
     * This is the ${artifactType} constructor. In this method, you may add
     * additional functionality to properties such as listening for changes
     * or handling allocation, register message handlers and set up internal
     * state for your ${artifactType}.
     *
     * A ${artifactType} may listen for external changes to properties (i.e., by a
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
     *          _baseLog.debug("Changed scaleValue " + oldValue + " to " + newValue);
     *       }
     *
     * The recommended practice is for the implementation of valueChanged() to
     * contain only glue code to dispatch the call to a private method on the
     * ${artifactType} class.
//% if component is device
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
//% endif
//% if component is device
     * Accessing the Device Manager and Domain Manager:
     * 
     *  Both the Device Manager hosting this Device and the Domain Manager hosting
     *  the Device Manager are available to the Device.
     *  
     *  To access the Domain Manager:
     *      CF.DomainManager dommgr = this.getDomainManager().getRef();
     *  To access the Device Manager:
     *      CF.DeviceManager devmgr = this.getDeviceManager().getRef();
//% else
     * Accessing the Application and Domain Manager:
     * 
     *  Both the Application hosting this Component and the Domain Manager hosting
     *  the Application are available to the Component.
     *  
     *  To access the Domain Manager:
     *      CF.DomainManager dommgr = this.getDomainManager().getRef();
     *  To access the Application:
     *      CF.Application app = this.getApplication().getRef();
//% endif
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

    public ${classname}()
    {
        super();
    }

    public void constructor()
    {
/*{% if 'FrontendTuner' in component.implements %}*/
        /**************************************************************************
    
         For a tuner device, the structure frontend_tuner_status needs to match the number
         of tuners that this device controls and what kind of device it is.
         The options for devices are: TX, RX, RX_DIGITIZER, CHANNELIZER, DDC, RC_DIGITIZER_CHANNELIZER
     
         For example, if this device has 5 physical
         tuners, 3 RX_DIGITIZER and 2 CHANNELIZER, then the code in the construct function 
         should look like this:

         this.addChannels(3, "RX_DIGITIZER");
         this.addChannels(2, "CHANNELIZER");
     
         The incoming request for tuning contains a string describing the requested tuner
         type. The string for the request must match the string in the tuner status.
     
        **************************************************************************/
        this.addChannels(1, "RX_DIGITIZER");
/*{% endif %}*/
    }

/*{% if component is device %}*/
/*{%   block updateUsageState %}*/
    /**************************************************************************

         This is called automatically after allocateCapacity or deallocateCapacity are called.
         Your implementation should determine the current state of the device:

            setUsageState(CF.DevicePackage.UsageType.IDLE);   // not in use
            setUsageState(CF.DevicePackage.UsageType.ACTIVE); // in use, with capacity remaining for allocation
            setUsageState(CF.DevicePackage.UsageType.BUSY);   // in use, with no capacity remaining for allocation

     ***************************************************************************/
    protected void updateUsageState()
    {
    }
/*{%   endblock %}*/
/*{% endif %}*/

    /**
     *
     * Main processing function
     *
     * General functionality:
     *
     * The serviceFunction() is called repeatedly by the ${artifactType}'s processing
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
/*{% if component is device %}*/
/*{% if 'FrontendTuner' in component.implements %}*/
     *
     *    To create a StreamSRI object based on tuner status structure index 'idx':
     *            BULKIO.StreamSRI sri = this.create("my_stream_id", this.frontend_tuner_status.getValue().get(idx));
/*{% endif %}*/
/*{% endif %}*/
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
     *    the ${artifactType} developer.
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
     * Logging:
     *
     *    The member _baseLog is a logger whose base name is the component (or device) instance name.
     *    New logs should be created based on this logger name.
     *
     *    To create a new logger,
     *        RHLogger my_logger = this._baseLog.getChildLogger("foo");
     *
     *    Assuming component instance name abc_1, my_logger will then be created with the 
     *    name "abc_1.user.foo".
     *
     * Example:
     *
     *    This example assumes that the ${artifactType} has two ports:
     *        - A bulkio.InShortPort provides (input) port called dataShort_in
     *        - A bulkio.OutFloatPort uses (output) port called dataFloat_out
     *    The mapping between the port and the class is found in the ${artifactType}
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
        _baseLog.debug("serviceFunction() example log message");

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

/*{% block extensions %}*/
/*# Allow for child class extensions #*/
/*{% endblock %}*/
}
