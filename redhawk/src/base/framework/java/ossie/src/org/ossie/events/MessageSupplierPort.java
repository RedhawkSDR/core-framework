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
import java.util.Hashtable;

import CosEventChannelAdmin.*;
import org.ossie.events.Consumer_i;
import org.ossie.component.*;
import org.ossie.properties.StructDef;

public class MessageSupplierPort extends UsesPort<EventChannelOperations> implements EventChannelOperations {
    protected Hashtable<String, ProxyPushConsumerOperations> outConnections_channel;

    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) {
        try {
            // don't want to process while command information is coming in
            synchronized (this.updatingPortsLock) { 
                final EventChannelOperations port = narrow(connection);
                final SupplierAdminOperations supplier_admin = port.for_suppliers();
                final ProxyPushConsumerOperations proxy_consumer = supplier_admin.obtain_push_consumer();
                this.outConnections_channel.put(connectionId, proxy_consumer);
                this.active = true;
            }
        } catch (final Throwable t) {
            t.printStackTrace();
        }

    }
    
    public MessageSupplierPort(String portName) 
    {
        super(portName);
        this.outConnections_channel = new Hashtable<String, ProxyPushConsumerOperations>();
    }
    
    public void push(final Any data) 
    {
        synchronized(this.updatingPortsLock) {    // don't want to process while command information is coming in
            if (this.active) {
                for (ProxyPushConsumerOperations p : this.outConnections_channel.values()) {
                    try {
                        ((ProxyPushConsumerOperations) p).push(data);
                    } catch (final COS.CosEventComm.Disconnected ex) {
                        continue;
                    }
                }
            }
        }    // don't want to process while command information is coming in
    }

    protected EventChannelOperations narrow(org.omg.CORBA.Object connection) 
    {
        return EventChannelHelper.narrow(connection);
    }

    public CosEventChannelAdmin.ConsumerAdmin for_consumers() 
    {
        CosEventChannelAdmin.ConsumerAdmin retval = null;
        return retval;
    }
    public void destroy() 
    {
        return;
    }
    public CosEventChannelAdmin.SupplierAdmin for_suppliers() 
    {
        CosEventChannelAdmin.SupplierAdmin retval = null;
        return retval;
    }
}
