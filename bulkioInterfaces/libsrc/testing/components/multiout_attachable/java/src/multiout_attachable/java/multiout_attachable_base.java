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

import java.util.List;
import java.util.Properties;

import org.apache.log4j.Logger;

import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;

import CF.InvalidObjectReference;

import org.ossie.component.*;
import org.ossie.properties.*;
import bulkio.connection_descriptor_struct;

/**
 * This is the component code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general component housekeeping
 *
 * Source: multiout_attachable.spd.xml
 *
 * @generated
 */
public abstract class multiout_attachable_base extends ThreadedResource {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(multiout_attachable_base.class.getName());

    /**
     * The property packets_ingested
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final UShortProperty packets_ingested =
        new UShortProperty(
            "packets_ingested", //id
            "packets_ingested", //name
            (short)0, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    
    /**
     * The property callback_stats
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    /**
     * The structure for property callback_stats
     * 
     * @generated
     */
    public static class callback_stats_struct extends StructDef {
        public final UShortProperty num_sdds_attaches =
            new UShortProperty(
                "num_sdds_attaches", //id
                null, //name
                (short)0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final UShortProperty num_sdds_detaches =
            new UShortProperty(
                "num_sdds_detaches", //id
                null, //name
                (short)0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final UShortProperty num_vita49_attaches =
            new UShortProperty(
                "num_vita49_attaches", //id
                null, //name
                (short)0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final UShortProperty num_vita49_detaches =
            new UShortProperty(
                "num_vita49_detaches", //id
                null, //name
                (short)0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final UShortProperty num_new_sri_callbacks =
            new UShortProperty(
                "num_new_sri_callbacks", //id
                null, //name
                (short)0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final UShortProperty num_sri_change_callbacks =
            new UShortProperty(
                "num_sri_change_callbacks", //id
                null, //name
                (short)0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
    
        /**
         * @generated
         */
        public callback_stats_struct(Short num_sdds_attaches, Short num_sdds_detaches, Short num_vita49_attaches, Short num_vita49_detaches, Short num_new_sri_callbacks, Short num_sri_change_callbacks) {
            this();
            this.num_sdds_attaches.setValue(num_sdds_attaches);
            this.num_sdds_detaches.setValue(num_sdds_detaches);
            this.num_vita49_attaches.setValue(num_vita49_attaches);
            this.num_vita49_detaches.setValue(num_vita49_detaches);
            this.num_new_sri_callbacks.setValue(num_new_sri_callbacks);
            this.num_sri_change_callbacks.setValue(num_sri_change_callbacks);
        }
    
        /**
         * @generated
         */
        public callback_stats_struct() {
            addElement(this.num_sdds_attaches);
            addElement(this.num_sdds_detaches);
            addElement(this.num_vita49_attaches);
            addElement(this.num_vita49_detaches);
            addElement(this.num_new_sri_callbacks);
            addElement(this.num_sri_change_callbacks);
        }
    
        public String getId() {
            return "callback_stats";
        }
    };
    
    public final StructProperty<callback_stats_struct> callback_stats =
        new StructProperty<callback_stats_struct>(
            "callback_stats", //id
            null, //name
            callback_stats_struct.class, //type
            new callback_stats_struct(), //default value
            Mode.READWRITE, //mode
            new Kind[] {Kind.CONFIGURE} //kind
            );
    
    /**
     * The property connectionTable
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final StructSequenceProperty<connection_descriptor_struct> connectionTable =
        new StructSequenceProperty<connection_descriptor_struct> (
            "connectionTable", //id
            null, //name
            connection_descriptor_struct.class, //type
            StructSequenceProperty.<connection_descriptor_struct>asList(), //defaultValue
            Mode.READWRITE, //mode
            new Kind[] { Kind.CONFIGURE } //kind
        );
    
    /**
     * The property SDDSStreamDefinitions
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    /**
     * The structure for property SDDSStreamDefinition
     * 
     * @generated
     */
    public static class SDDSStreamDefinition_struct extends StructDef {
        public final StringProperty id =
            new StringProperty(
                "sdds::id", //id
                "id", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final StringProperty multicastAddress =
            new StringProperty(
                "sdds::multicastAddress", //id
                "multicastAddress", //name
                "0.0.0.0", //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ULongProperty vlan =
            new ULongProperty(
                "sdds::vlan", //id
                "vlan", //name
                0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ULongProperty port =
            new ULongProperty(
                "sdds::port", //id
                "port", //name
                0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ULongProperty sampleRate =
            new ULongProperty(
                "sdds::sampleRate", //id
                "sampleRate", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final BooleanProperty timeTagValid =
            new BooleanProperty(
                "sdds::timeTagValid", //id
                "timeTagValid", //name
                false, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final StringProperty privateInfo =
            new StringProperty(
                "sdds::privateInfo", //id
                "privateInfo", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
    
        /**
         * @generated
         */
        public SDDSStreamDefinition_struct(String id, String multicastAddress, Integer vlan, Integer port, Integer sampleRate, Boolean timeTagValid, String privateInfo) {
            this();
            this.id.setValue(id);
            this.multicastAddress.setValue(multicastAddress);
            this.vlan.setValue(vlan);
            this.port.setValue(port);
            this.sampleRate.setValue(sampleRate);
            this.timeTagValid.setValue(timeTagValid);
            this.privateInfo.setValue(privateInfo);
        }
    
        /**
         * @generated
         */
        public SDDSStreamDefinition_struct() {
            addElement(this.id);
            addElement(this.multicastAddress);
            addElement(this.vlan);
            addElement(this.port);
            addElement(this.sampleRate);
            addElement(this.timeTagValid);
            addElement(this.privateInfo);
        }
    
        public String getId() {
            return "SDDSStreamDefinition";
        }
    };
    
    public final StructSequenceProperty<SDDSStreamDefinition_struct> SDDSStreamDefinitions =
        new StructSequenceProperty<SDDSStreamDefinition_struct> (
            "SDDSStreamDefinitions", //id
            null, //name
            SDDSStreamDefinition_struct.class, //type
            StructSequenceProperty.<SDDSStreamDefinition_struct>asList(), //defaultValue
            Mode.READWRITE, //mode
            new Kind[] { Kind.CONFIGURE } //kind
        );
    
    /**
     * The property VITA49StreamDefinitions
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    /**
     * The structure for property VITA49StreamDefinition
     * 
     * @generated
     */
    public static class VITA49StreamDefinition_struct extends StructDef {
        public final StringProperty id =
            new StringProperty(
                "vita49::id", //id
                "id", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final StringProperty ip_address =
            new StringProperty(
                "vita49::ip_address", //id
                "ip_address", //name
                "0.0.0.0", //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ULongProperty vlan =
            new ULongProperty(
                "vita49::vlan", //id
                "vlan", //name
                0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ULongProperty port =
            new ULongProperty(
                "vita49::port", //id
                "port", //name
                0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final BooleanProperty valid_data_format =
            new BooleanProperty(
                "vita49::valid_data_format", //id
                "valid_data_format", //name
                false, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final BooleanProperty packing_method_processing_efficient =
            new BooleanProperty(
                "vita49::packing_method_processing_efficient", //id
                "packing_method_processing_efficient", //name
                false, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final BooleanProperty repeating =
            new BooleanProperty(
                "vita49::repeating", //id
                "repeating", //name
                false, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final LongProperty event_tag_size =
            new LongProperty(
                "vita49::event_tag_size", //id
                "event_tag_size", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final LongProperty channel_tag_size =
            new LongProperty(
                "vita49::channel_tag_size", //id
                "channel_tag_size", //name
                0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final LongProperty item_packing_field_size =
            new LongProperty(
                "vita49::item_packing_field_size", //id
                "item_packing_field_size", //name
                0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final LongProperty data_item_size =
            new LongProperty(
                "vita49::data_item_size", //id
                "data_item_size", //name
                0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final LongProperty repeat_count =
            new LongProperty(
                "vita49::repeat_count", //id
                "repeat_count", //name
                0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final LongProperty vector_size =
            new LongProperty(
                "vita49::vector_size", //id
                "vector_size", //name
                0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
    
        /**
         * @generated
         */
        public VITA49StreamDefinition_struct(String id, String ip_address, Integer vlan, Integer port, Boolean valid_data_format, Boolean packing_method_processing_efficient, Boolean repeating, Integer event_tag_size, Integer channel_tag_size, Integer item_packing_field_size, Integer data_item_size, Integer repeat_count, Integer vector_size) {
            this();
            this.id.setValue(id);
            this.ip_address.setValue(ip_address);
            this.vlan.setValue(vlan);
            this.port.setValue(port);
            this.valid_data_format.setValue(valid_data_format);
            this.packing_method_processing_efficient.setValue(packing_method_processing_efficient);
            this.repeating.setValue(repeating);
            this.event_tag_size.setValue(event_tag_size);
            this.channel_tag_size.setValue(channel_tag_size);
            this.item_packing_field_size.setValue(item_packing_field_size);
            this.data_item_size.setValue(data_item_size);
            this.repeat_count.setValue(repeat_count);
            this.vector_size.setValue(vector_size);
        }
    
        /**
         * @generated
         */
        public VITA49StreamDefinition_struct() {
            addElement(this.id);
            addElement(this.ip_address);
            addElement(this.vlan);
            addElement(this.port);
            addElement(this.valid_data_format);
            addElement(this.packing_method_processing_efficient);
            addElement(this.repeating);
            addElement(this.event_tag_size);
            addElement(this.channel_tag_size);
            addElement(this.item_packing_field_size);
            addElement(this.data_item_size);
            addElement(this.repeat_count);
            addElement(this.vector_size);
        }
    
        public String getId() {
            return "VITA49StreamDefinition";
        }
    };
    
    public final StructSequenceProperty<VITA49StreamDefinition_struct> VITA49StreamDefinitions =
        new StructSequenceProperty<VITA49StreamDefinition_struct> (
            "VITA49StreamDefinitions", //id
            null, //name
            VITA49StreamDefinition_struct.class, //type
            StructSequenceProperty.<VITA49StreamDefinition_struct>asList(), //defaultValue
            Mode.READWRITE, //mode
            new Kind[] { Kind.CONFIGURE } //kind
        );
    
    /**
     * The property received_sdds_attachments
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    /**
     * The structure for property sdds_attachment
     * 
     * @generated
     */
    public static class sdds_attachment_struct extends StructDef {
        public final StringProperty streamId =
            new StringProperty(
                "sdds::streamId", //id
                "streamId", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final StringProperty attachId =
            new StringProperty(
                "sdds::attachId", //id
                "attachId", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ULongProperty port =
            new ULongProperty(
                "sdds::rec_port", //id
                "port", //name
                0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
    
        /**
         * @generated
         */
        public sdds_attachment_struct(String streamId, String attachId, Integer port) {
            this();
            this.streamId.setValue(streamId);
            this.attachId.setValue(attachId);
            this.port.setValue(port);
        }
    
        /**
         * @generated
         */
        public sdds_attachment_struct() {
            addElement(this.streamId);
            addElement(this.attachId);
            addElement(this.port);
        }
    
        public String getId() {
            return "sdds_attachment";
        }
    };
    
    public final StructSequenceProperty<sdds_attachment_struct> received_sdds_attachments =
        new StructSequenceProperty<sdds_attachment_struct> (
            "received_sdds_attachments", //id
            null, //name
            sdds_attachment_struct.class, //type
            StructSequenceProperty.<sdds_attachment_struct>asList(), //defaultValue
            Mode.READWRITE, //mode
            new Kind[] { Kind.CONFIGURE } //kind
        );
    
    /**
     * The property received_vita49_attachments
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    /**
     * The structure for property vita49_attachment
     * 
     * @generated
     */
    public static class vita49_attachment_struct extends StructDef {
        public final StringProperty streamId =
            new StringProperty(
                "vita49::streamId", //id
                "streamId", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final StringProperty attachId =
            new StringProperty(
                "vita49::attachId", //id
                "attachId", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ULongProperty port =
            new ULongProperty(
                "vita49::rec_port", //id
                "port", //name
                0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
    
        /**
         * @generated
         */
        public vita49_attachment_struct(String streamId, String attachId, Integer port) {
            this();
            this.streamId.setValue(streamId);
            this.attachId.setValue(attachId);
            this.port.setValue(port);
        }
    
        /**
         * @generated
         */
        public vita49_attachment_struct() {
            addElement(this.streamId);
            addElement(this.attachId);
            addElement(this.port);
        }
    
        public String getId() {
            return "vita49_attachment";
        }
    };
    
    public final StructSequenceProperty<vita49_attachment_struct> received_vita49_attachments =
        new StructSequenceProperty<vita49_attachment_struct> (
            "received_vita49_attachments", //id
            null, //name
            vita49_attachment_struct.class, //type
            StructSequenceProperty.<vita49_attachment_struct>asList(), //defaultValue
            Mode.READWRITE, //mode
            new Kind[] { Kind.CONFIGURE } //kind
        );
    
    // Provides/inputs
    /**
     * @generated
     */
    public bulkio.InSDDSPort port_dataSDDS_in;

    /**
     * @generated
     */
    public bulkio.InVITA49Port port_dataVITA49_in;

    /**
     * @generated
     */
    public bulkio.InFloatPort port_dataFloat_in;

    // Uses/outputs
    /**
     * @generated
     */
    public bulkio.OutSDDSPort port_dataSDDS_out;

    /**
     * @generated
     */
    public bulkio.OutVITA49Port port_dataVITA49_out;

    /**
     * @generated
     */
    public multiout_attachable_base()
    {
        super();

        // Properties
        addProperty(packets_ingested);
        addProperty(callback_stats);
        addProperty(connectionTable);
        addProperty(SDDSStreamDefinitions);
        addProperty(VITA49StreamDefinitions);
        addProperty(received_sdds_attachments);
        addProperty(received_vita49_attachments);

        // Provides/inputs
        this.port_dataSDDS_in = new bulkio.InSDDSPort("dataSDDS_in");
        this.addPort("dataSDDS_in", this.port_dataSDDS_in);
        this.port_dataVITA49_in = new bulkio.InVITA49Port("dataVITA49_in");
        this.addPort("dataVITA49_in", this.port_dataVITA49_in);
        this.port_dataFloat_in = new bulkio.InFloatPort("dataFloat_in");
        this.addPort("dataFloat_in", this.port_dataFloat_in);

        // Uses/outputs
        this.port_dataSDDS_out = new bulkio.OutSDDSPort("dataSDDS_out");
        this.addPort("dataSDDS_out", this.port_dataSDDS_out);
        this.port_dataVITA49_out = new bulkio.OutVITA49Port("dataVITA49_out");
        this.addPort("dataVITA49_out", this.port_dataVITA49_out);

        this.connectionTable.addChangeListener(new PropertyListener<List<connection_descriptor_struct>>() {
            public void valueChanged (List<connection_descriptor_struct> oldValue, List<connection_descriptor_struct> newValue)
            {
                multiout_attachable_base.this.connectionTableChanged(oldValue, newValue);
            }
        });
    }

    public void start() throws CF.ResourcePackage.StartError
    {
        super.start();
    }

    public void stop() throws CF.ResourcePackage.StopError
    {
        super.stop();
    }

    protected void connectionTableChanged (List<connection_descriptor_struct> oldValue, List<connection_descriptor_struct> newValue)
    {
        this.port_dataSDDS_out.updateConnectionFilter(newValue);
        this.port_dataVITA49_out.updateConnectionFilter(newValue);
    }

    /**
     * The main function of your component.  If no args are provided, then the
     * CORBA object is not bound to an SCA Domain or NamingService and can
     * be run as a standard Java application.
     * 
     * @param args
     * @generated
     */
    public static void main(String[] args) 
    {
        final Properties orbProps = new Properties();
        multiout_attachable.configureOrb(orbProps);

        try {
            ThreadedResource.start_component(multiout_attachable.class, args, orbProps);
        } catch (InvalidObjectReference e) {
            e.printStackTrace();
        } catch (NotFound e) {
            e.printStackTrace();
        } catch (CannotProceed e) {
            e.printStackTrace();
        } catch (InvalidName e) {
            e.printStackTrace();
        } catch (ServantNotActive e) {
            e.printStackTrace();
        } catch (WrongPolicy e) {
            e.printStackTrace();
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
    }
}
