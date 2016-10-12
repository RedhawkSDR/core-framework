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

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.omg.CORBA.Any;
import org.omg.CosEventChannelAdmin.*;
import org.omg.CosEventComm.PushSupplier;
import org.omg.PortableServer.POAPackage.ObjectNotActive;
import org.omg.PortableServer.POAPackage.ServantAlreadyActive;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;

import org.apache.log4j.Logger;

import org.ossie.component.PortBase;
import org.ossie.properties.StructDef;

/**
 * A port that can receive REDHAWK messages via point-to-point connections or
 * distributed over event channels.
 */
@SuppressWarnings("deprecation")
public class MessageConsumerPort extends ExtendedEvent.MessageEventPOA implements PortBase {
    public Object updatingPortsLock = new Object();

    protected HashMap<String, EventCallback> callbacks = new HashMap<String, EventCallback>();
    protected boolean active = false;
    protected String name;

    /**
     * Internal class to adapt ProxyPushConsumer CORBA interface on a
     * per-supplier basis.
     */
    private class ProxyMessageConsumer extends ProxyPushConsumerPOA {
        public ProxyMessageConsumer()
        {
        }

        public ProxyMessageConsumer(String connectionId)
        {
            this.connectionId = connectionId;
        }

        public void push(final org.omg.CORBA.Any data)
        {
            MessageConsumerPort.this.messagesReceived(data);
        }

        public void connect_push_supplier(PushSupplier supplier)
        {
            this.supplier = supplier;
        }

        public void disconnect_push_consumer()
        {
            MessageConsumerPort.this.removeConsumer(this);
        }

        public void disconnect()
        {
            if (this.supplier != null) {
                this.supplier.disconnect_push_supplier();
            }
        }

        public String getConnectionId()
        {
            return this.connectionId;
        }

        private PushSupplier supplier = null;
        private String connectionId = null;
    }

    /**
     * Internal class to adapt SupplierAdmin CORBA interface and dispatch to
     * MessageSupplierPort.
     */
    private class SupplierAdminProxy extends SupplierAdminPOA {
        public ProxyPushConsumer obtain_push_consumer() {
            return MessageConsumerPort.this.createConsumer();
        }

        public ProxyPullConsumer obtain_pull_consumer() {
            // Not implemented: only push consumer/supplier
            throw new org.omg.CORBA.NO_IMPLEMENT("Message consumer port only supports push suppliers");
        }
    }

    private List<ProxyMessageConsumer> consumers = new LinkedList<ProxyMessageConsumer>();
    private SupplierAdminProxy supplierAdmin = null;
    private Logger logger = Logger.getLogger(MessageConsumerPort.class.getName());

    /**
     * Create a new message consumer port with the default class logger.
     */
    public MessageConsumerPort(String portName) 
    {
        this.name = portName;
    }
    
    /**
     * Create a new message consumer port, with a user-supplied logger.
     */
   public MessageConsumerPort(String portName, Logger logger) 
    {
        this(portName);
        this.logger = logger;
    }
    
    public boolean isActive() {
        return this.active;
    }

    public void setActive(final boolean active) {
        this.active = active;
    }

    public String getName() {
        return this.name;
    }

    public void setLogger(Logger logger)
    {
        this.logger = logger;
    }

	public String getRepid()
	{
		return "IDL:ExtendedEvent/MessageEvent:1.0";
	}

	public String getDirection()
	{
		return "Bidir";
	}

    /**
     * Register a listener for a message.
     *
     * @param messageId  the identifier of the message
     * @param clazz      the class object for the message payload
     * @param listener   the listener to be called when a message is received
     * @param <E>        the type of the message
     *
     * @since 2.0
     */
    public <E extends StructDef> void registerMessage(String messageId, Class<E> clazz, MessageListener<E> listener)
    {
        synchronized (this.callbacks) {
            this.callbacks.put(messageId, new MessageAdapter<E>(clazz, listener));
        }
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

        // Try to get a proxy supplier, which should only fail if there is some
        // sort of communication issue
        ProxyPushSupplier proxy_supplier = null;
        try {
            final ConsumerAdmin consumer_admin = channel.for_consumers();
            proxy_supplier = consumer_admin.obtain_push_supplier();
        } catch (final Exception ex) {
            throw new CF.PortPackage.InvalidPort((short)1, "Event channel unreachable");
        }

        // Create a proxy consumer to handle messages from this event channel,
        // and connect it with the proxy supplier
        ProxyMessageConsumer consumer = new ProxyMessageConsumer(connectionId);
        try {
            org.omg.CORBA.Object consumer_obj = this.activateChild(consumer);
            ProxyPushConsumer consumer_ref = ProxyPushConsumerHelper.narrow(consumer_obj);
            proxy_supplier.connect_push_consumer(consumer_ref);
        } catch (final Exception ex) {
            throw new CF.PortPackage.InvalidPort((short)1, "Unable to create an event consumer");
        }
        consumer.connect_push_supplier(proxy_supplier);

        synchronized (this.updatingPortsLock) {
            this.consumers.add(consumer);
            this.active = true;
        }
    }

    public void disconnectPort(final String connectionId) {
        ProxyMessageConsumer consumer = null;
        synchronized (this.updatingPortsLock) {
            consumer = this.findConsumerByConnectionId(connectionId);
            if (consumer == null) {
                return;
            }
            this.consumers.remove(consumer);
        }
        consumer.disconnect();
        this.deactivateChild(consumer);
    }

    public org.omg.CosEventChannelAdmin.SupplierAdmin for_suppliers() 
    {
        synchronized(this.updatingPortsLock) {
            if (this.supplierAdmin == null) {
                SupplierAdminProxy supplier_admin = new SupplierAdminProxy();
                this.activateChild(supplier_admin);
                this.supplierAdmin = supplier_admin;
            }
        }

        // Java's CORBA implementation has a broken _this(); work around the
        // problem by going directly to the POA to create a reference, then
        // narrow the result
        org.omg.CORBA.Object object = null;
        try {
            object = this.supplierAdmin._default_POA().servant_to_reference(this.supplierAdmin);
        } catch (final ServantNotActive sna) {
            throw new AssertionError("Servant not active");
        } catch (final WrongPolicy wp) {
            throw new AssertionError("Wrong POA policy");
        }
        return SupplierAdminHelper.narrow(object);
    }

    public org.omg.CosEventChannelAdmin.ConsumerAdmin for_consumers() 
    {
        // Not implemented: only remote suppliers are supported
        throw new org.omg.CORBA.NO_IMPLEMENT("Message consumer port only supports suppliers");
    }

    public void destroy() 
    {
        // Not implemented: cannot be destroyed
        throw new org.omg.CORBA.NO_IMPLEMENT("Message consumer port cannot be destroyed");
    }

    private ProxyMessageConsumer findConsumerByConnectionId(final String connectionId)
    {
        for (ProxyMessageConsumer consumer : this.consumers) {
            if (connectionId.equals(consumer.getConnectionId())) {
                return consumer;
            }
        }
        return null;
    }

    private ProxyPushConsumer createConsumer()
    {
        ProxyMessageConsumer consumer = new ProxyMessageConsumer();
        org.omg.CORBA.Object proxy = this.activateChild(consumer);
        synchronized (this.updatingPortsLock) {
            this.consumers.add(consumer);
        }
        return ProxyPushConsumerHelper.narrow(proxy);
    }

    private org.omg.CORBA.Object activateChild(org.omg.PortableServer.Servant servant)
    {
        try {
            byte[] oid = this._poa().activate_object(servant);
            return this._poa().id_to_reference(oid);
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

    private void removeConsumer(ProxyMessageConsumer consumer)
    {
        synchronized (this.updatingPortsLock) {
            this.consumers.remove(consumer);
        }
        this.deactivateChild(consumer);
    }

    private void messagesReceived(final org.omg.CORBA.Any data)
    {
        // Ignore all payloads except CF.Properties
        if (!data.type().equivalent(CF.PropertiesHelper.type())) {
            return;
        }

        for (final CF.DataType message : CF.PropertiesHelper.extract(data)) {
            this.dispatchMessage(message.id, message.value);
        }
    }

    private void dispatchMessage(final String messageId, final org.omg.CORBA.Any messageData)
    {
        EventCallback message_callback = null;
        synchronized (this.callbacks) {
            message_callback = this.callbacks.get(messageId);

            // If no callback is registered, present a meaningful warning
            if (message_callback == null) {
                String warning = "No callbacks registered for messages with id '"+messageId+"'.";
                if (this.callbacks.isEmpty()) {
                    warning += " No callbacks are registered";
                } else if (this.callbacks.size() == 1) {
                    warning += " The only registered callback is for message with id: "+this.callbacks.keySet().iterator().next();
                } else {
                    warning += " The available message callbacks are for messages with any of the following id:";
                    for (final String available : this.callbacks.keySet()) {
                        warning += " " + available;
                    }
                }
                this.logger.warn(warning);
                return;
            }
        }

        message_callback.message(messageId, messageData);       
    }

    /**
     * @deprecated  As of REDHAWK 2.0, replaced by {@link #registerMessage(String,Class,MessageListener)}
     */
    @Deprecated
    public void registerMessage(final String message_id, EventCallback _callback) {
        this.callbacks.put(message_id, _callback);
    }

    @Deprecated
    public void push(final Any data) 
    {
        // This method was non-functional in prior releases
    }

    @Deprecated
    public HashMap<String, ExtendedEvent.MessageEventPOA> getPorts() {
        return new HashMap<String, ExtendedEvent.MessageEventPOA>();
    }

    @Deprecated
    public Consumer_i local_consumer = null;

    @Deprecated
    public org.omg.CosEventChannelAdmin.ProxyPushConsumer local_consumer_ref = null;

    @Deprecated
    public SupplierAdmin_i supplier_admin = null;

    @Deprecated
    public org.omg.CosEventChannelAdmin.SupplierAdmin supplier_admin_ref = null;

    @Deprecated
    public EventCallback callback;

    @Deprecated
    public Class< ? > parent;

    @Deprecated
    public StructDef data_structure;
    
}
