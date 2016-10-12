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

import org.omg.CORBA.Any;
import org.omg.CosEventChannelAdmin.*;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import org.ossie.events.Consumer_i;
import org.ossie.component.*;
import org.ossie.properties.StructDef;

/**
 * @generated
 */
public class MessageConsumerPort extends ExtendedEvent.MessageEventPOA implements ExtendedEvent.MessageEventOperations {
    protected HashMap<String, ProxyPushSupplierOperations> outConnections_channel;
    public Consumer_i local_consumer;
    public org.omg.CosEventChannelAdmin.ProxyPushConsumer local_consumer_ref;
    public SupplierAdmin_i supplier_admin;
    public org.omg.CosEventChannelAdmin.SupplierAdmin supplier_admin_ref;
    protected HashMap<String, EventCallback> callbacks;
    
    public Object updatingPortsLock;
    protected boolean active;
    protected String name;
    protected HashMap<String, ExtendedEvent.MessageEventPOA> outPorts;

    public boolean isActive() {
        return this.active;
    }

    public void setActive(final boolean active) {
        this.active = active;
    }

    public String getName() {
        return this.name;
    }

    public HashMap<String, ExtendedEvent.MessageEventPOA> getPorts() {
        return this.outPorts;
    }

    public EventCallback callback;

    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) throws CF.PortPackage.InvalidPort, CF.PortPackage.OccupiedPort {
        try {
            // don't want to process while command information is coming in
            synchronized (this.updatingPortsLock) {
                if (this.local_consumer == null) {
                    this.local_consumer = new Consumer_i();
                    this.local_consumer_ref = this.local_consumer.setup(this._orb(), this._poa());
                    if (this.local_consumer_ref == null) {
                        throw new CF.PortPackage.InvalidPort((short) 0, "Unable to create an event consumer");
                    }
                    if (this.callbacks != null) {
                        Set<String> keys = this.callbacks.keySet();
                        for (String key : keys) {
                            this.local_consumer.registerMessage(key, this.callbacks.get(key));
                        }
                    }
                }
                final EventChannelOperations port = EventChannelHelper.narrow(connection);
                final ConsumerAdminOperations consumer_admin = port.for_consumers();
                final ProxyPushSupplierOperations proxy_supplier = consumer_admin.obtain_push_supplier();
                proxy_supplier.connect_push_consumer(this.local_consumer_ref);
                this.outConnections_channel.put(connectionId, proxy_supplier);
                this.active = true;
            }
        } catch (final Throwable t) {
            t.printStackTrace();
        }

    }

    public void disconnectPort(final String connectionId) {
        synchronized (this.updatingPortsLock) {
            final ProxyPushSupplierOperations proxy_supplier = outConnections_channel.get(connectionId);
            if (proxy_supplier == null)
                return;
            outConnections_channel.remove(connectionId);
            proxy_supplier.disconnect_push_supplier();
        }
    }
    
    public Class< ? > parent;
    public StructDef data_structure;
    
    /**
     * @generated
     */
    public MessageConsumerPort(String portName) 
    {
        this.outPorts = new HashMap<String, ExtendedEvent.MessageEventPOA>();
        this.active = false;
        this.name = portName;
        this.updatingPortsLock = new Object();
        this.outConnections_channel = new HashMap<String, ProxyPushSupplierOperations>();
        this.local_consumer = null;
        this.supplier_admin = null;
        this.callbacks = new HashMap<String, EventCallback>();

        //begin-user-code
        //end-user-code
    }
    
    public void registerMessage(final String message_id, EventCallback _callback) {
        this.callbacks.put(message_id, _callback);
        if (this.local_consumer != null) {
            this.local_consumer.registerMessage(message_id, _callback);
        }
    }
    
    public void push(final Any data) 
    {
        synchronized(this.updatingPortsLock) {    // don't want to process while command information is coming in
            if (this.active) {
                for (ProxyPushSupplierOperations p : this.outConnections_channel.values()) {
                    try {
                        ((ProxyPushConsumerOperations) p).push(data);
                    } catch (final org.omg.CosEventComm.Disconnected ex) {
                        continue;
                    }
                }
            }
        }    // don't want to process while command information is coming in
    }

    /**
     * @generated
     */
    public org.omg.CosEventChannelAdmin.ConsumerAdmin for_consumers() 
    {
        org.omg.CosEventChannelAdmin.ConsumerAdmin retval = null;
        
        return retval;
    }
    /**
     * @generated
     */
    public void destroy() 
    {
        
        //begin-user-code
        //end-user-code
        
        return;
    }

    /**
     * @generated
     */
    public org.omg.CosEventChannelAdmin.SupplierAdmin for_suppliers() 
    {
        synchronized(this.updatingPortsLock) {
            if (this.supplier_admin == null) {
                this.supplier_admin = new SupplierAdmin_i(this);
                this.supplier_admin_ref = this.supplier_admin.setup(this._orb(), this._poa());
            }
            for (Map.Entry<String, EventCallback> entry : this.callbacks.entrySet()) {
                this.local_consumer.registerMessage(entry.getKey(), entry.getValue());
            }
        }
        return this.supplier_admin_ref;
    }
}
