/*
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
 */

package org.ossie.events;

import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;
import org.omg.CosEventChannelAdmin.*;
import org.omg.CosEventComm.PushConsumer;
import org.omg.CosEventComm.PushSupplier;
import org.omg.CosEventComm.PushSupplierHelper;
import org.omg.CosEventComm.PushSupplierPOA;
import org.omg.PortableServer.POAPackage.ObjectNotActive;
import org.omg.PortableServer.POAPackage.ServantAlreadyActive;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;

import org.apache.log4j.Logger;

import org.ossie.component.QueryableUsesPort;
import org.ossie.component.PortBase;
import org.ossie.component.RHLogger;
import org.ossie.properties.StructDef;

public class MessageSupplierPort extends QueryableUsesPort<EventChannelOperations> implements EventChannelOperations, PortBase {

    /**
     * Internal class to adapt PushSupplier CORBA interface for disconnection
     * notification
     */
    private class SupplierAdapter extends PushSupplierPOA {
        public SupplierAdapter(String connectionId)
        {
            this.connectionId = connectionId;
        }

        public void disconnect_push_supplier()
        {
            MessageSupplierPort.this.removeConnection(this.connectionId, false);
        }

        final String connectionId;
    }

    public RHLogger _portLog;
    private Logger logger = Logger.getLogger(MessageSupplierPort.class.getName());
    private Map<String,PushConsumer> consumers = new HashMap<String,PushConsumer>();
    private Map<String,SupplierAdapter> suppliers = new HashMap<String,SupplierAdapter>();

    public MessageSupplierPort(String portName)
    {
        super(portName);
    }

    public MessageSupplierPort(String portName, Logger logger)
    {
        this(portName);
        this.logger = logger;
    }

    public MessageSupplierPort(String portName, RHLogger logger)
    {
        this(portName);
        this._portLog = logger;
    }

    public void setLogger(Logger logger)
    {
        this.logger = logger;
    }

    public void setLogger(RHLogger logger)
    {
        this._portLog = logger;
    }

    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) throws CF.PortPackage.InvalidPort, CF.PortPackage.OccupiedPort {
        // Give a specific exception message for nil
        if (connection == null) {
            throw new CF.PortPackage.InvalidPort((short)1, "Nil object reference");
        }

        // Attempt to narrow the reference to an event channel; note this does
        // not require the lock
        EventChannel channel = null;
        try {
            channel = EventChannelHelper.narrow(connection);
        } catch (final org.omg.CORBA.BAD_PARAM ex) {
            // In this context, a CORBA.BAD_PARAM exception indicates that the
            // object is of the wrong type
            throw new CF.PortPackage.InvalidPort((short)1, "Object is not an event channel");
        } catch (final org.omg.CORBA.SystemException ex) {
            // If the object is not obviously the desired type, narrow will
            // invoke _is_a, which may throw a CORBA exception if a remote
            // object is unreachable (e.g., dead)
            throw new CF.PortPackage.InvalidPort((short)1, "Object unreachable");
        }

        // Try to get a proxy consumer, which should only fail if there is some
        // sort of communication issue
        ProxyPushConsumer proxy_consumer = null;
        try {
            final SupplierAdmin supplier_admin = channel.for_suppliers();
            proxy_consumer = supplier_admin.obtain_push_consumer();
        } catch (final Exception ex) {
            throw new CF.PortPackage.InvalidPort((short)1, "Event channel unreachable");
        }

        // Create a supplier to receive notification when the consumer leaves
        SupplierAdapter supplier = new SupplierAdapter(connectionId);
        try {
            org.omg.CORBA.Object supplier_obj = this.activateChild(supplier);
            PushSupplier supplier_ref = PushSupplierHelper.narrow(supplier_obj);
            proxy_consumer.connect_push_supplier(supplier_ref);
        } catch (final Exception ex) {
            throw new CF.PortPackage.InvalidPort((short)1, "Unable to create an event supplier");
        }

        synchronized (this.updatingPortsLock) { 
            // Store the original channel in the parent object's container
            this.outPorts.put(connectionId, channel);
            this.suppliers.put(connectionId, supplier);
            this.consumers.put(connectionId, proxy_consumer);
            this.active = true;
        }
    }

    public void disconnectPort(final String connectionId)
    {
        this.removeConnection(connectionId, true);
    }

    /**
     * Sends pre-serialized messages to all connections.
     *
     * @param data  messages serialized to a CORBA Any
     */
    public void push(final Any data)
    {
        try {
            this._push(data,"");
        } catch( final org.omg.CORBA.MARSHAL ex ) {
            this.logger.warn("Could not deliver the message. Maximum message size exceeded");
        }
    }

    /**
     * Sends pre-serialized messages to a specific connection.
     *
     * @param data          messages serialized to a CORBA Any
     * @param connectionId  target connection
     * @throws IllegalArgumentException  If connectionId does not match any
     *                                   connection
     * @since 2.2
     */
    public void push(final Any data, String connectionId)
    {
        try {
            this._push(data, connectionId);
        } catch( final org.omg.CORBA.MARSHAL ex ) {
            this.logger.warn("Could not deliver the message. Maximum message size exceeded");
        }
    }

    public void _push(final Any data, String connectionId)
    {
        synchronized(this.updatingPortsLock) {
            if (!this.active) {
                return;
            }

            if (!connectionId.isEmpty()) {
                if (!_hasConnection(connectionId)) {
                    throw new IllegalArgumentException("invalid connection: '"+connectionId+"'");
                }
            }

            for (Map.Entry<String,PushConsumer> entry : consumers.entrySet()) {
                if (!_isConnectionSelected(entry.getKey(), connectionId)) {
                    continue;
                }

                PushConsumer consumer = entry.getValue();
                try {
                    consumer.push(data);
                } catch (final org.omg.CosEventComm.Disconnected ex) {
                    removeConnection( consumer );
                    continue;
                } catch( final org.omg.CORBA.COMM_FAILURE ex ) {
                    removeConnection( consumer );
                    continue;
                } catch( final org.omg.CORBA.OBJECT_NOT_EXIST ex ) {
                    removeConnection( consumer );
                    continue;
                } catch( final org.omg.CORBA.MARSHAL ex ) {
                    throw ex;
                } catch (final Exception e) {
                    continue;
                }
            }
        }
    }
    
    /**
     * Sends a single message to all connections.
     *
     * @param message  message structure to send
     */
    public void sendMessage(final StructDef message)
    {
        this.sendMessage(message, "");
    }

    /**
     * Sends a single message to a specific connection.
     *
     * @param message       message structure to send
     * @param connectionId  target connection
     * @throws IllegalArgumentException  If connectionId does not match any
     *                                   connection
     */
    public void sendMessage(StructDef message, String connectionId)
    {
        this.sendMessages(Arrays.asList(message), connectionId);
    }

    /**
     * Sends a collection of messages to all connections.
     *
     * @param messages  collection of message structures to send
     */
    public void sendMessages(final Collection<? extends StructDef> messages)
    {
        this.sendMessages(messages, "");
    }

    /**
     * Sends a collection of messages to a specific connection.
     *
     * @param messages      collection of message structures to send
     * @param connectionId  target connection
     * @throws IllegalArgumentException  If connectionId does not match any
     *                                   connection
     */
    public void sendMessages(Collection<? extends StructDef> messages, String connectionId)
    {
        final CF.DataType[] properties = new CF.DataType[messages.size()];
        int index = 0;
        for (StructDef message : messages) {
            properties[index++] = new CF.DataType(message.getId(), message.toAny());
        }
        final Any any = ORB.init().create_any();
        CF.PropertiesHelper.insert(any, properties);
        try {
            this._push(any, connectionId);
        } catch( final org.omg.CORBA.MARSHAL ex ) {
            // Sending them all at once failed, try send the messages individually
            if (messages.size() == 1) {
                this.logger.warn("Could not deliver the message. Maximum message size exceeded");
            } else {
                this.logger.warn("Could not deliver the message. Maximum message size exceeded, trying individually.");

                for (CF.DataType prop : properties) {
                    final Any a = ORB.init().create_any();
                    CF.PropertiesHelper.insert(a, new CF.DataType[]{prop});
                    this.push(a, connectionId);
                }
            }
        }
    }

    protected EventChannelOperations narrow(org.omg.CORBA.Object connection) 
    {
        return EventChannelHelper.narrow(connection);
    }

    private void removeConnection(PushConsumer consumer )
    {
        String connectionId=null;
        synchronized (this.updatingPortsLock) {
            for(Map.Entry<String,PushConsumer> entry: this.consumers.entrySet()){
                if( entry.getValue() == consumer ){
                    connectionId = entry.getKey();
                    break; //breaking because its one to one map
                }
            }
        }
        
        if ( connectionId != null && connectionId.isEmpty() == false ) {
            removeConnection( connectionId, false );
        }
    }


    private void removeConnection(String connectionId, boolean notifyConsumer)
    {
        SupplierAdapter supplier = null;
        PushConsumer proxy_consumer = null;
        synchronized (this.updatingPortsLock) {
            // Remove the original EventChannel object from the parent class'
            // container
            this.outPorts.remove(connectionId);

            proxy_consumer = this.consumers.get(connectionId);
            if (proxy_consumer == null) {
                return;
            }
            this.consumers.remove(connectionId);
            this.active = !this.consumers.isEmpty();

            supplier = this.suppliers.get(connectionId);
            this.suppliers.remove(connectionId);
        }
        if (notifyConsumer) {
            proxy_consumer.disconnect_push_consumer();
        }
        
        if (supplier != null ) {
            try {
                this.deactivateChild(supplier);
            }
            catch( Exception e ) {
                // pass
            }
        }
    } 

    private org.omg.CORBA.Object activateChild(org.omg.PortableServer.Servant servant)
    {
        try {
            org.omg.PortableServer.POA poa = this._default_POA();
            byte[] oid = poa.activate_object(servant);
            return poa.id_to_reference(oid);
        } catch (final ServantAlreadyActive exc) {
            throw new AssertionError("Servant already active");
        } catch (final ObjectNotActive exc) {
            throw new AssertionError("Object not active");
        } catch (final WrongPolicy exc) {
            throw new AssertionError("Wrong policy");
        }
    }

    private void deactivateChild(org.omg.PortableServer.Servant servant)
    {
        try {
            org.omg.PortableServer.POA poa = servant._default_POA();
            byte[] oid = poa.servant_to_id(servant);
            poa.deactivate_object(oid);
        } catch (final ServantNotActive exc) {
            throw new AssertionError("Servant not active");
        } catch (final ObjectNotActive exc) {
            throw new AssertionError("Object not active");
        } catch (final WrongPolicy exc) {
            throw new AssertionError("Wrong policy");
        }
    }

    private boolean _hasConnection(String connectionId)
    {
        return consumers.containsKey(connectionId);
    }

    private boolean _isConnectionSelected(String connectionId, String targetId)
    {
        if (targetId.isEmpty()) {
            return true;
        }
        return connectionId.equals(targetId);
    }

    @Deprecated
    public org.omg.CosEventChannelAdmin.ConsumerAdmin for_consumers() 
    {
        return null;
    }

    @Deprecated
    public void destroy() 
    {
    }

    @Deprecated
    public org.omg.CosEventChannelAdmin.SupplierAdmin for_suppliers() 
    {
        return null;
    }

    public String getRepid()
    {
        return ExtendedEvent.MessageEventHelper.id();
    }

    public String getDirection()
    {
        return CF.PortSet.DIRECTION_USES;
    }
}
