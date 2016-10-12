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
import org.omg.CORBA.ORB;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.Servant;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
import org.ossie.events.EventCallback;
import org.ossie.properties.AnyUtils;
import java.util.HashMap;

import CF.DataType;

public class Consumer_i extends org.omg.CosEventChannelAdmin.ProxyPushConsumerPOA implements org.omg.CosEventChannelAdmin.ProxyPushConsumerOperations {

    protected org.omg.CosEventChannelAdmin.ProxyPushConsumer push_consumer;
    protected HashMap<String, EventCallback> callbacks;

    public Consumer_i() {
        callbacks = new HashMap<String, EventCallback>();
    }

    public org.omg.CosEventChannelAdmin.ProxyPushConsumer setup(final ORB orb, final POA poa) {
        org.omg.CosEventChannelAdmin.ProxyPushConsumerPOATie tie = new org.omg.CosEventChannelAdmin.ProxyPushConsumerPOATie(this, poa);
        tie._this(orb);
        try {
            push_consumer = org.omg.CosEventChannelAdmin.ProxyPushConsumerHelper.narrow(poa.servant_to_reference((Servant)tie));
        } catch (ServantNotActive e) {
            return null;
        } catch (WrongPolicy e) {
            return null;
        }
        return push_consumer;
    }

    public void push(final Any data) {
        CF.DataType [] msgs = (DataType[])AnyUtils.convertAny(data);
        for (final DataType msg : msgs) {
            if (this.callbacks.get(msg.id) != null) {
                Any msg_container = msg.value;
                this.callbacks.get(msg.id).message(msg.id, msg_container);
            }
        }
    }
    
    public void registerMessage(final String message_id, EventCallback _callback) {
        this.callbacks.put(message_id, _callback);
    }
    
    public void connect_push_supplier(final org.omg.CosEventComm.PushSupplier push_supplier) {
        
    }

    public void disconnect_push_consumer() {
        
    }

}
