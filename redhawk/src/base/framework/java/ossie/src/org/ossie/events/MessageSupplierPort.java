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

import org.ossie.component.UsesPort;
import org.ossie.component.PortBase;
import org.ossie.properties.StructDef;

public class MessageSupplierPort extends UsesPort<EventChannelOperations> implements EventChannelOperations, PortBase {

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

    public void setLogger(Logger logger)
    {
        this.logger = logger;
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
            this.suppliers.put(connectionId, supplier);
            this.consumers.put(connectionId, proxy_consumer);
            this.active = true;
        }
    }

    public void disconnectPort(final String connectionId)
    {
        this.removeConnection(connectionId, true);
    }
    
    public void push(final Any data) 
    {
        synchronized(this.updatingPortsLock) {
            if (!this.active) {
                return;
            }

            for (PushConsumer consumer : this.consumers.values()) {
                try {
                    consumer.push(data);
                } catch (final org.omg.CosEventComm.Disconnected ex) {
                    continue;
                }
            }
        }
    }

    public void sendMessage(final StructDef message)
    {
        this.sendMessages(Arrays.asList(message));
    }

    public void sendMessages(final Collection<StructDef> messages) {
        final CF.DataType[] properties = new CF.DataType[messages.size()];
        int index = 0;
        for (StructDef message : messages) {
            properties[index++] = new CF.DataType(message.getId(), message.toAny());
        }
        final Any any = ORB.init().create_any();
        CF.PropertiesHelper.insert(any, properties);
        this.push(any);
    }

    protected EventChannelOperations narrow(org.omg.CORBA.Object connection) 
    {
        return EventChannelHelper.narrow(connection);
    }

    private void removeConnection(String connectionId, boolean notifyConsumer)
    {
        SupplierAdapter supplier = null;
        PushConsumer proxy_consumer = null;
        synchronized (this.updatingPortsLock) {
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
        this.deactivateChild(supplier);
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
		return "IDL:ExtendedEvent/MessageEvent:1.0";
	}

	public String getDirection()
	{
		return "Uses";
	}
}
