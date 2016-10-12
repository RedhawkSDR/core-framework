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
//% set classname = component.userclass.name
//% set baseclass = component.baseclass.name
//% set artifactType = component.artifacttype
package ${component.package};

import java.util.Properties;

/*{% if component.isafrontendtuner %}*/
import org.omg.CORBA.ORB;
import org.omg.PortableServer.POA;
import CF.DeviceManager;
import CF.DevicePackage.InvalidCapacity;
import CF.InvalidObjectReference;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
/*{% endif %}*/

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
     * This is the ${artifactType} constructor. In this method, you may add additional
     * functionality to properties, such as listening for changes and handling
     * allocation, and set up internal state for your ${artifactType}.
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
     *          logger.debug("Changed scaleValue " + oldValue + " to " + newValue);
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
     */
/*{% if component.isafrontendtuner %}*/
    public ${classname}(DeviceManager devMgr, String compId, String label, String softwareProfile, ORB orb, POA poa) throws InvalidObjectReference, ServantNotActive, WrongPolicy, CF.DevicePackage.InvalidCapacity {
        super(devMgr, compId, label, softwareProfile, orb, poa);
/*{% else %}*/
    public ${classname}() {
        super();
/*{% endif %}*/
    }

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
     * Example:
     *
     *    This example assumes that the ${artifactType} has two ports:
     *        - A bulkio.InShortPort provides (input) port called dataShort_in
     *        - A bulkio.InFloatPort uses (output) port called dataFloat_out
     *    The mapping between the port and the class is found in the ${artifactType}
     *    base class file.
     *    This example also makes use of the following Properties:
     *        - A float value called amplitude with a default value of 2.0
     *        - A boolean called increaseAmplitude with a default value of true
     *
     *    InShortPort.Packet data = this.port_dataShort_in.getPacket(125);
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

/*{% if component.isafrontendtuner %}*/
    ///////////////////////////////////////////////////
    ////// tuner configuration functions // -- overrides base class implementations
    ///////////////////////////////////////////////////////

    public boolean removeTuner(int tuner_id) {
        "removeTuner(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********"
        return BOOLEAN_VALUE_HERE;
    }

    ////////////////////////////////////////
    ////// Required device specific functions // -- implemented by device developer
    ////////////////////////////////////////////

    protected boolean push_EOS_on_listener(String listener_allocation_id){
        "push_EOS_on_listener(): DEVELOPER MUST IMPLEMENT THIS METHOD *********" 
        return BOOLEAN_VALUE_HERE;
    }
    protected boolean _valid_tuner_type(String tuner_type){
        "_valid_tuner_type(): DEVELOPER MUST IMPLEMENT THIS METHOD *********" 
        return BOOLEAN_VALUE_HERE;
    }
    protected boolean _valid_center_frequency(double req_freq, int tuner_id){
        "_valid_center_frequency(): DEVELOPER MUST IMPLEMENT THIS METHOD *********" 
        return BOOLEAN_VALUE_HERE;
    }
    protected boolean _valid_bandwidth(double req_bw, int tuner_id){
        "_valid_bandwidth(): DEVELOPER MUST IMPLEMENT THIS METHOD *********" 
        return BOOLEAN_VALUE_HERE;
    }
    protected boolean _valid_sample_rate(double req_sr, int tuner_id){
        "_valid_sample_rate(): DEVELOPER MUST IMPLEMENT THIS METHOD *********" 
        return BOOLEAN_VALUE_HERE;
    }
    protected boolean _dev_enable(int tuner_id){
        "_dev_enable(): DEVELOPER MUST IMPLEMENT THIS METHOD *********" 
        return BOOLEAN_VALUE_HERE;
    }
    protected boolean _dev_disable(int tuner_id){
        "_dev_disable(): DEVELOPER MUST IMPLEMENT THIS METHOD *********" 
        return BOOLEAN_VALUE_HERE;
    }
    protected boolean _dev_set_all(double req_freq, double req_bw, double req_sr, int tuner_id){
        "_dev_set_all(): DEVELOPER MUST IMPLEMENT THIS METHOD *********" 
        return BOOLEAN_VALUE_HERE;
    }
    protected boolean _dev_set_center_frequency(double req_freq, int tuner_id){
        "_dev_set_center_frequency(): DEVELOPER MUST IMPLEMENT THIS METHOD *********" 
        return BOOLEAN_VALUE_HERE;
    }
    protected boolean _dev_set_bandwidth(double req_bw, int tuner_id){
        "_dev_set_bandwidth(): DEVELOPER MUST IMPLEMENT THIS METHOD *********" 
        return BOOLEAN_VALUE_HERE;
    }
    protected boolean _dev_set_sample_rate(double req_sr, int tuner_id){
        "_dev_set_sample_rate(): DEVELOPER MUST IMPLEMENT THIS METHOD *********" 
        return BOOLEAN_VALUE_HERE;
    }
    protected boolean _dev_get_all(double freq, double bw, double sr, int tuner_id){
        "_dev_get_all(): DEVELOPER MUST IMPLEMENT THIS METHOD *********" 
        return BOOLEAN_VALUE_HERE;
    }
    protected double _dev_get_center_frequency(int tuner_id){
        "_dev_get_center_frequency(): DEVELOPER MUST IMPLEMENT THIS METHOD *********" 
        return DOUBLE_VALUE_HERE;
    }
    protected double _dev_get_bandwidth(int tuner_id){
        "_dev_get_bandwidth(): DEVELOPER MUST IMPLEMENT THIS METHOD *********" 
        return DOUBLE_VALUE_HERE;
    }
    protected double _dev_get_sample_rate(int tuner_id){
        "_dev_get_sample_rate(): DEVELOPER MUST IMPLEMENT THIS METHOD *********" 
        return DOUBLE_VALUE_HERE;
    }
/*{% endif %}*/

/*{% set foundInFrontendInterface = False %}*/
/*{% set foundInAnalogInterface = False %}*/
/*{% set foundInDigitalInterface = False %}*/
/*{% set foundInGPSPort = False %}*/
/*{% set foundInNavDataPort = False %}*/
/*{% set foundInRFInfoPort = False %}*/
/*{% set foundInRFSourcePort = False %}*/
/*{% for port in component.ports if port is provides %}*/
/*{%     if port.javatype == "frontend.InDigitalTunerPort" or
            port.javatype == "frontend.InAnalogTunerPort" or
            port.javatype == "frontend.InFrontendTunerPort" %}*/
    /*#- add FEI FrontendTuner callback functions #*/
/*{%         if foundInFrontendInterface == False %}*/
/*
    public String fe_getTunerType(String id){
        "fe_getTunerType(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public boolean fe_getTunerDeviceControl(String id){
        "fe_getTunerDeviceControl(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public String fe_getTunerGroupId(String id){
        "fe_getTunerGroupId(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
   public String fe_getTunerRfFlowId(String id){
        "fe_getTunerRfFlowId(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public CF.DataType[] fe_getTunerStatus(String id){
        "fe_getTunerStatus(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/
/*{%             set foundInFrontendInterface = True %}*/
/*{%         endif %}*/
/*{%     endif %}*/
/*{%     if port.javatype == "frontend.InDigitalTunerPort" or
            port.javatype == "frontend.InAnalogTunerPort" %}*/
    /*#- add FEI AnalogTuner callback functions #*/
/*{%         if foundInAnalogInterface == False %}*/
/*
    public double fe_getTunerCenterFrequency(String id){
        "fe_getTunerCenterFrequency(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public void fe_setTunerCenterFrequency(String id, double freq){
        "fe_setTunerCenterFrequency(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public double fe_getTunerBandwidth(String id){
        "fe_getTunerBandwidth(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public void fe_setTunerBandwidth(String id,double bw){
        "fe_setTunerBandwidth(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public boolean fe_getTunerAgcEnable(String id){
        "fe_getTunerAgcEnable(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public void fe_setTunerAgcEnable(String id,boolean enable){
        "fe_setTunerAgcEnable(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public float fe_getTunerGain(String id){
        "fe_getTunerGain(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public void fe_setTunerGain(String id,float gain){
        "fe_setTunerGain(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public int fe_getTunerReferenceSource(String id){
        "fe_getTunerReferenceSource(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public void fe_setTunerReferenceSource(String id,int source){
        "fe_setTunerReferenceSource(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public boolean fe_getTunerEnable(String id){
        "fe_getTunerEnable(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public void fe_setTunerEnable(String id,boolean enable){
        "fe_setTunerEnable(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*{%             set foundInAnalogInterface = True %}*/
/*{%         endif %}*/
/*{% endif %}*/
/*{%     if port.javatype == "frontend.InDigitalTunerPort" %}*/
    /*#- add FEI DigitalTuner callback functions #*/
/*{%         if foundInDigitalInterface == False %}*/
/*
    public double fe_getTunerOutputSampleRate(String id){
        "fe_getTunerOutputSampleRate(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public void fe_setTunerOutputSampleRate(String id,double sr){
        "fe_setTunerOutputSampleRate(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/
/*{%             set foundInDigitalInterface = True %}*/
/*{%         endif %}*/
/*{%     endif %}*/
/*{%     if port.javatype == "frontend.InGPSPort" %}*/
/*{%         if foundInGPSPort == False %}*/
/*
    public FRONTEND.GPSInfo fe_getGPSInfo(){
        "fe_getGPSInfo(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public void fe_setGPSInfo(FRONTEND.GPSInfo data){
        "fe_setGPSInfo(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public FRONTEND.GpsTimePos fe_getGpsTimePos(){
        "fe_getGpsTimePos(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public void fe_setGpsTimePos(FRONTEND.GpsTimePos data){
        "fe_setGpsTimePos(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/
/*{%             set foundInGPSPort = True %}*/
/*{%         endif %}*/
/*{%     endif %}*/
/*{%     if port.javatype == "frontend.InNavDataPort" %}*/
/*{%         if foundInNavDataPort == False %}*/
/*
    public FRONTEND.NavigationPacket fe_getNavPkt(){
        "fe_getNavPkt(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public void fe_setNavPkt(FRONTEND.NavigationPacket data){
        "fe_setNavPkt(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/
/*{%             set foundInNavDataPort = True %}*/
/*{%         endif %}*/
/*{%     endif %}*/
/*{%     if port.javatype == "frontend.InRFInfoPort" %}*/
/*{%         if foundInRFInfoPort == False %}*/
/*
    public String fe_getRFFlowId(){
        "fe_getRFFlowId(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public void fe_setRFFlowId(String data){
        "fe_setRFFlowId(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public FRONTEND.RFInfoPkt fe_getRFInfoPkt(){
        "fe_getRFInfoPkt(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public void fe_setRFInfoPkt(FRONTEND.RFInfoPkt data){
        "fe_setRFInfoPkt(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/
/*{%             set foundInRFInfoPort = True %}*/
/*{%         endif %}*/
/*{%     endif %}*/
/*{%     if port.javatype == "frontend.InRFSourcePort" %}*/
/*{%         if foundInRFSourcePort == False %}*/
/*
    public FRONTEND.RFInfoPkt[] fe_getAvailableRFInputs(){
        "fe_getAvailableRFInputs(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public void fe_setAvailableRFInputs(FRONTEND.RFInfoPkt[] data){
        "fe_setAvailableRFInputs(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public FRONTEND.RFInfoPkt fe_getCurrentRFInput(){
        "fe_getCurrentRFInput(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/

/*
    public void fe_setCurrentRFInput(FRONTEND.RFInfoPkt data){
        "fe_setCurrentRFInput(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
    }
*/
/*{%             set foundInRFSourcePort = True %}*/
/*{%         endif %}*/
/*{%     endif %}*/
/*{% endfor %}*/
/*{% block extensions %}*/
/*# Allow for child class extensions #*/
/*{% endblock %}*/

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
