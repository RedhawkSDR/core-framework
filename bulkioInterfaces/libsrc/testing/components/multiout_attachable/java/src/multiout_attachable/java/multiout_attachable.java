/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
package multiout_attachable.java;

import java.util.Properties;
import java.util.Iterator;
import java.util.Vector;
import java.util.List;
import org.ossie.properties.*;
import org.apache.log4j.Logger;

/**
 * This is the component code. This file contains the derived class where custom
 * functionality can be added to the component. You may add methods and code to
 * this class to handle property changes, respond to incoming data, and perform
 * general component housekeeping
 *
 * Source: multiout_attachable.spd.xml
 */

public class multiout_attachable extends multiout_attachable_base {

    public final static Logger logger = Logger.getLogger(multiout_attachable.class.getName());

    public class SDDSAttachDetachCallback implements bulkio.InSDDSPort.Callback  {

        SDDSAttachDetachCallback () {
        }

        public String attach(BULKIO.SDDSStreamDefinition streamDef, String userid) {
            logger.debug("SDDSAttachDetachCallback.attach() streamId " + streamDef.id + " userid " + userid);
            multiout_attachable.this.callback_stats.getValue().num_sdds_attaches.setValue(multiout_attachable.this.callback_stats.getValue().num_sdds_attaches.getValue()+1);
            logger.debug("SDDSAttachDetachCallback.attach() updated num_sdds_attaches " + multiout_attachable.this.callback_stats.getValue().num_sdds_attaches.getValue());
            String aid = java.util.UUID.randomUUID().toString();
            sdds_attachment_struct newAttachment = new sdds_attachment_struct(streamDef.id, aid, new Integer(streamDef.port));
            multiout_attachable.this.received_sdds_attachments.getValue().add(newAttachment);
            logger.debug("SDDSAttachDetachCallback.attach() number of attachments " + multiout_attachable.this.received_sdds_attachments.getValue().size());
            logger.debug("SDDSAttachDetachCallback.attach() returning attachId " + aid);

            return aid;
        }

        public void detach(String attachId)  {
            logger.debug("SDDSAttachDetachCallback.detach() attachId " + attachId);
            List<sdds_attachment_struct> attList = multiout_attachable.this.received_sdds_attachments.getValue();
            Iterator<sdds_attachment_struct> iter = attList.iterator();
            while (iter.hasNext()) {
                    sdds_attachment_struct nextStreamAttachment = iter.next();
                    logger.debug("SDDSAttachDetachCallback.detach() checking stream attachment with attachId " + nextStreamAttachment.attachId.getValue());
                    if (nextStreamAttachment.attachId.getValue().equals(attachId)) {
                            iter.remove();
                            multiout_attachable.this.callback_stats.getValue().num_sdds_detaches.setValue(multiout_attachable.this.callback_stats.getValue().num_sdds_detaches.getValue()+1);
                            logger.debug("SDDSAttachDetachCallback.detach() updated num_sdds_detaches " + multiout_attachable.this.callback_stats.getValue().num_sdds_detaches.getValue());
                            multiout_attachable.this.callback_stats.setValue(multiout_attachable.this.callback_stats.getValue());
                    }
            }
            logger.debug("SDDSAttachDetachCallback.detach() updated number of attachments " + multiout_attachable.this.received_sdds_attachments.getValue().size());
        }
    };

    public class VITA49AttachDetachCallback implements bulkio.InVITA49Port.Callback  {
       
        VITA49AttachDetachCallback () {
        }

        public String attach(BULKIO.VITA49StreamDefinition streamDef, String userid) {
            logger.debug("VITA49AttachDetachCallback.attach() streamId " + streamDef.id + " userid " + userid);
            multiout_attachable.this.callback_stats.getValue().num_vita49_attaches.setValue(multiout_attachable.this.callback_stats.getValue().num_vita49_attaches.getValue()+1);
            logger.debug("VITA49AttachDetachCallback.attach() updated num_vita49_attaches " + multiout_attachable.this.callback_stats.getValue().num_vita49_attaches.getValue());
            String aid = java.util.UUID.randomUUID().toString();
            vita49_attachment_struct newAttachment = new vita49_attachment_struct(streamDef.id, aid, new Integer(streamDef.port));
            multiout_attachable.this.received_vita49_attachments.getValue().add(newAttachment);
            logger.debug("VITA49AttachDetachCallback.attach() number of attachments " + multiout_attachable.this.received_vita49_attachments.getValue().size());
            logger.debug("VITA49AttachDetachCallback.attach() returning attachId " + aid);

            return aid;
        }

        public void detach(String attachId)  {
            logger.debug("VITA49AttachDetachCallback.detach() attachId " + attachId);
            Iterator<vita49_attachment_struct> iter = multiout_attachable.this.received_vita49_attachments.getValue().iterator();
            while (iter.hasNext()) {
                    vita49_attachment_struct nextStreamAttachment = iter.next();
                    if (nextStreamAttachment.attachId.getValue().equals(attachId)) {
                            iter.remove();
                            multiout_attachable.this.callback_stats.getValue().num_vita49_detaches.setValue(multiout_attachable.this.callback_stats.getValue().num_vita49_detaches.getValue()+1);
                            logger.debug("VITA49AttachDetachCallback.detach() updated num_vita49_detaches " + multiout_attachable.this.callback_stats.getValue().num_vita49_detaches.getValue());
                    }
            }
            logger.debug("VITA49AttachDetachCallback.detach() updated number of attachments " + multiout_attachable.this.received_vita49_attachments.asList().size());
        }
    };

    public class SriCallback implements bulkio.SriListener  {
        SriCallback() {
        }

        public void newSRI( BULKIO.StreamSRI sri){
            // Query SRIs to ensure deadlock doesn't occur
            BULKIO.StreamSRI[] sddsSriList = multiout_attachable.this.port_dataSDDS_in.activeSRIs();
            BULKIO.StreamSRI[] vita49SriList = multiout_attachable.this.port_dataVITA49_in.activeSRIs();

            multiout_attachable.this.callback_stats.getValue().num_new_sri_callbacks.setValue(multiout_attachable.this.callback_stats.getValue().num_new_sri_callbacks.getValue()+1);
            logger.debug("SriCallback.newSRI() num_new_sri_callbacks " + multiout_attachable.this.callback_stats.getValue().num_new_sri_callbacks.getValue());
        }

        public boolean changedSRI (BULKIO.StreamSRI sri){
            // Query SRIs to ensure deadlock doesn't occur
            BULKIO.StreamSRI[] sddsSriList = multiout_attachable.this.port_dataSDDS_in.activeSRIs();
            BULKIO.StreamSRI[] vita49SriList = multiout_attachable.this.port_dataVITA49_in.activeSRIs();

            multiout_attachable.this.callback_stats.getValue().num_sri_change_callbacks.setValue(multiout_attachable.this.callback_stats.getValue().num_sri_change_callbacks.getValue()+1);
            logger.debug("SriCallback.changedSRI() num_sri_change_callbacks " + multiout_attachable.this.callback_stats.getValue().num_sri_change_callbacks.getValue());
            return true;
        }
    };

    /**
     * This is the component constructor. In this method, you may add additional
     * functionality to properties, such as listening for changes and handling
     * allocation, and set up internal state for your component.
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
     */
    public multiout_attachable() {
        super();
        this.SDDSStreamDefinitions.addChangeListener(new PropertyListener<List<SDDSStreamDefinition_struct>>(){
          public void valueChanged( List<SDDSStreamDefinition_struct> oldValue, List<SDDSStreamDefinition_struct> newValue){
              sddsStreamDefChanged(oldValue,newValue);
          }
        });

        this.VITA49StreamDefinitions.addChangeListener(new PropertyListener<List<VITA49StreamDefinition_struct>>(){
          public void valueChanged( List<VITA49StreamDefinition_struct> oldValue, List<VITA49StreamDefinition_struct> newValue){
              vita49StreamDefChanged(oldValue,newValue);
          }
        });

        this.port_dataSDDS_in.setLogger(this.logger);
        this.port_dataSDDS_out.setLogger(this.logger);
        this.port_dataSDDS_in.setAttachDetachCallback(new SDDSAttachDetachCallback());
        this.port_dataVITA49_in.setAttachDetachCallback(new VITA49AttachDetachCallback());
        this.port_dataSDDS_in.setSriListener(new SriCallback());
        this.port_dataVITA49_in.setSriListener(new SriCallback());
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
     *    the component developer.
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
     *    This example assumes that the component has two ports:
     *        - A bulkio.InShortPort provides (input) port called dataShort_in
     *        - A bulkio.OutFloatPort uses (output) port called dataFloat_out
     *    The mapping between the port and the class is found in the component
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

        bulkio.InFloatPort.Packet data = this.port_dataFloat_in.getPacket(125);
        if (data == null) {
            return NOOP;
        }
        if (data != null && data.sriChanged()) {
            logger.debug("serviceFunction() pushing out new SRI for streamId " + data.getSRI().streamID);
            this.port_dataSDDS_out.pushSRI(data.getSRI(), bulkio.time.utils.now());
            this.port_dataVITA49_out.pushSRI(data.getSRI(), bulkio.time.utils.now());
        }

        multiout_attachable.this.packets_ingested.setValue(multiout_attachable.this.packets_ingested.getValue()+1);
        return NORMAL;
    }

    protected void vita49StreamDefChanged(List<VITA49StreamDefinition_struct> oldValue, List<VITA49StreamDefinition_struct> newValue)
    {
        // Grab all the streamIds
        Vector<String> oldStreamIds = new Vector<String>();
        Vector<String> newStreamIds = new Vector<String>();
        Iterator<VITA49StreamDefinition_struct> iter = oldValue.iterator();
        Iterator<VITA49StreamDefinition_struct> iter2 = newValue.iterator();
        logger.debug("vita49StreamDefChanged() oldValue.size() " + oldValue.size());
        logger.debug("vita49StreamDefChanged() newValue.size() " + newValue.size());

        while(iter.hasNext()){
            oldStreamIds.add(iter.next().id.getValue());
        }
        for (String sid: oldStreamIds){
            logger.debug("vita49StreamDefChanged(): oldStreamId: " + sid);
        }
        while(iter2.hasNext()){
            newStreamIds.add(iter2.next().id.getValue());
        }
        for (String sid: newStreamIds){
            logger.debug("vita49StreamDefChanged(): newStreamId: " + sid);
        }

        // Find which old streams need to be detached
        iter = oldValue.iterator();
        while(iter.hasNext()){
                String streamId = iter.next().id.getValue();
                if (newStreamIds.indexOf(streamId) == -1) {
                        try{
                            logger.debug("vita49StreamDefChanged() calling removeStream for " + streamId);
                            this.port_dataVITA49_out.removeStream(streamId);
                        }catch (BULKIO.dataVITA49Package.AttachError e){
                            logger.error("vita49StreamDefChanged() AttachError for streamID " + streamId); 
                        }catch (BULKIO.dataVITA49Package.StreamInputError e){
                            logger.error("vita49StreamDefChanged() StreamInputError for streamID " + streamId); 
                        }catch (BULKIO.dataVITA49Package.DetachError e){
                            logger.error("vita49StreamDefChanged() DetachError for streamID " + streamId); 
                        }
                }
        }

        // Find which new streams need to be attached
        iter = newValue.iterator();
        while (iter.hasNext()){
                VITA49StreamDefinition_struct nextNewDef = iter.next();
                // Check if the entry is new
                boolean isNew = false;
                if (oldStreamIds.indexOf(nextNewDef.id.getValue()) == -1){
                    logger.debug("vita49StreamDefChanged() streamId " + nextNewDef.id.getValue() + " is new");
                    isNew = true;
                }
                
                // Check if the entry was updated
                boolean isUpdated = false;
                iter2 = oldValue.iterator();
                while (iter2.hasNext()) {
                        VITA49StreamDefinition_struct oldStreamDef = iter2.next();
                        if (oldStreamDef.id.getValue().equals(nextNewDef.id.getValue())) {
                                long val1 = nextNewDef.port.getValue();
                                long val2 = oldStreamDef.port.getValue();
                                
                                logger.debug("Incoming streamId " + nextNewDef.id.getValue() + " - port: " + val1);
                                logger.debug("     Old streamId " + oldStreamDef.id.getValue() + " - port: " + val2);
                                isUpdated = (val1 != val2);
                                logger.debug("vita49StreamDefChanged() streamId " + nextNewDef.id.getValue() + " is updated?: " + isUpdated);
                        }
                }

            BULKIO.VITA49StreamDefinition newDef = new BULKIO.VITA49StreamDefinition();
            newDef.id = nextNewDef.id.getValue();
            newDef.ip_address = nextNewDef.ip_address.getValue();
            newDef.port = nextNewDef.port.getValue();
            newDef.protocol = BULKIO.TransportProtocol.VITA49_TCP_TRANSPORT;
            newDef.valid_data_format = nextNewDef.valid_data_format.getValue();
            newDef.vlan = nextNewDef.vlan.getValue();
            newDef.data_format = new BULKIO.VITA49DataPacketPayloadFormat();
            newDef.data_format.channel_tag_size = nextNewDef.channel_tag_size.getValue();
            newDef.data_format.complexity = BULKIO.VITA49DataComplexity.VITA49_COMPLEX_CARTESIAN;
            newDef.data_format.data_item_format = BULKIO.VITA49DataDigraph.VITA49_32F;
            newDef.data_format.data_item_size = nextNewDef.data_item_size.getValue();
            newDef.data_format.event_tag_size = nextNewDef.event_tag_size.getValue();
            newDef.data_format.item_packing_field_size = nextNewDef.item_packing_field_size.getValue();
            newDef.data_format.packing_method_processing_efficient = nextNewDef.packing_method_processing_efficient.getValue();
            newDef.data_format.repeat_count = nextNewDef.repeat_count.getValue();
            newDef.data_format.repeating = nextNewDef.repeating.getValue();
            newDef.data_format.vector_size = nextNewDef.vector_size.getValue();

            try{
                if (isNew){
                    logger.debug("vita49StreamDefChanged() calling addStream for streamId " + newDef.id);
                    this.port_dataVITA49_out.addStream(newDef);
                }
                if (isUpdated){
                    logger.debug("vita49StreamDefChanged() calling updateStream for streamId " + newDef.id);
                    this.port_dataVITA49_out.updateStream(newDef);
                }
            }catch (BULKIO.dataVITA49Package.AttachError e){
                logger.error("vita49StreamDefChanged() AttachError for streamID " + newDef.id); 
            }catch (BULKIO.dataVITA49Package.StreamInputError e){
                logger.error("vita49StreamDefChanged() StreamInputError for streamID " + newDef.id); 
            }catch (BULKIO.dataVITA49Package.DetachError e){
                logger.error("vita49StreamDefChanged() DetachError for streamID " + newDef.id); 
            }
        }
        this.VITA49StreamDefinitions.setValue(newValue);
    }

    protected void sddsStreamDefChanged(List<SDDSStreamDefinition_struct> oldValue, List<SDDSStreamDefinition_struct> newValue)
    {
        // Grab all the attachIds
        Vector<String> oldStreamIds = new Vector<String>();
        Vector<String> newStreamIds = new Vector<String>();
        Iterator<SDDSStreamDefinition_struct> iter = oldValue.iterator();
        Iterator<SDDSStreamDefinition_struct> iter2 = newValue.iterator();

        logger.debug("sddsStreamDefChanged() oldValue.size() " + oldValue.size());
        logger.debug("sddsStreamDefChanged() newValue.size() " + newValue.size());

        while(iter.hasNext()){
            oldStreamIds.add(iter.next().id.getValue());
        }
        for (String sid: oldStreamIds){
            logger.debug("sddsStreamDefChanged(): oldStreamId: " + sid);
        }
        while(iter2.hasNext()){
            newStreamIds.add(iter2.next().id.getValue());
        }
        for (String sid: newStreamIds){
            logger.debug("sddsStreamDefChanged(): newStreamId: " + sid);
        }

        // Find which old streams need to be detached
        iter = oldValue.iterator();
        while(iter.hasNext()){
            String streamId = iter.next().id.getValue();
            if (newStreamIds.indexOf(streamId) == -1) {
                try{
                    logger.debug("sddsStreamDefChanged() calling removeStream for streamId " + streamId);
                    this.port_dataSDDS_out.removeStream(streamId);
                }catch (BULKIO.dataSDDSPackage.AttachError e){
                    logger.error("sddsStreamDefChanged() AttachError for streamID " + streamId); 
                }catch (BULKIO.dataSDDSPackage.StreamInputError e){
                    logger.error("sddsStreamDefChanged() StreamInputError for streamID " + streamId); 
                }catch (BULKIO.dataSDDSPackage.DetachError e){
                    logger.error("sddsStreamDefChanged() DetachError for streamID " + streamId); 
                }
            }
        }

        // Find which new streams need to be attached
        iter = newValue.iterator();
        while (iter.hasNext()){
                SDDSStreamDefinition_struct nextNewDef = iter.next();
                // Check if the entry is new
                boolean isNew = false;
                if (oldStreamIds.indexOf(nextNewDef.id.getValue()) == -1){
                    logger.debug("sddsStreamDefChanged() streamId " + nextNewDef.id.getValue() + " is new");
                    isNew = true;
                }

                // Check if the entry was updated
                boolean isUpdated = false;
                iter2 = oldValue.iterator();
                while (iter2.hasNext()) {
                        SDDSStreamDefinition_struct oldStreamDef = iter2.next();
                        if (oldStreamDef.id.getValue().equals(nextNewDef.id.getValue())) {
                                logger.debug("sddsStreamDefChanged() streamId " + nextNewDef.id.getValue() + " is updated");
                                isUpdated = (nextNewDef.port.getValue() != oldStreamDef.port.getValue());
                        }
                }

            BULKIO.SDDSStreamDefinition newDef = new BULKIO.SDDSStreamDefinition();
            newDef.id = nextNewDef.id.getValue();
            newDef.port = nextNewDef.port.getValue();
            newDef.vlan = nextNewDef.vlan.getValue();
            newDef.dataFormat = BULKIO.SDDSDataDigraph.SDDS_CF;
            newDef.multicastAddress = nextNewDef.multicastAddress.getValue();
            newDef.privateInfo = nextNewDef.privateInfo.getValue();
            newDef.sampleRate = nextNewDef.sampleRate.getValue();
            newDef.timeTagValid = nextNewDef.timeTagValid.getValue();

            try{
                if (isNew){
                    logger.debug("sddsStreamDefChanged() calling addStream for streamId " + newDef.id);
                    this.port_dataSDDS_out.addStream(newDef);
                }
                if (isUpdated){
                    logger.debug("sddsStreamDefChanged() calling updateStream for streamId " + newDef.id);
                    this.port_dataSDDS_out.updateStream(newDef);
                }
            }catch (BULKIO.dataSDDSPackage.AttachError e){
                logger.error("sddsStreamDefChanged() AttachError for streamID " + newDef.id); 
            }catch (BULKIO.dataSDDSPackage.StreamInputError e){
                logger.error("sddsStreamDefChanged() StreamInputError for streamID " + newDef.id); 
            }catch (BULKIO.dataSDDSPackage.DetachError e){
                logger.error("sddsStreamDefChanged() DetachError for streamID " + newDef.id); 
            }
        }
        this.SDDSStreamDefinitions.setValue(newValue);
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
