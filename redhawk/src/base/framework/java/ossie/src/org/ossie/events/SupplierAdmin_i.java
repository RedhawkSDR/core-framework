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

import org.omg.CORBA.ORB;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.Servant;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;

import org.ossie.events.MessageConsumerPort;

import org.ossie.component.*;

public class SupplierAdmin_i extends CosEventChannelAdmin.SupplierAdminPOA implements CosEventChannelAdmin.SupplierAdminOperations {

    protected CosEventChannelAdmin.SupplierAdmin supplier_admin;
    protected MessageConsumerPort parent;

    public SupplierAdmin_i(MessageConsumerPort _parent) {
        this.parent = _parent;
    }

    public CosEventChannelAdmin.SupplierAdmin setup(final ORB orb, final POA poa) {
        CosEventChannelAdmin.SupplierAdminPOATie tie = new CosEventChannelAdmin.SupplierAdminPOATie(this, poa);
        tie._this(orb);
        try {
            supplier_admin = CosEventChannelAdmin.SupplierAdminHelper.narrow(poa.servant_to_reference((Servant)tie));
        } catch (ServantNotActive e) {
            return null;
        } catch (WrongPolicy e) {
            return null;
        }
        if (this.parent.local_consumer_ref == null) {
            this.parent.local_consumer = new Consumer_i();
            this.parent.local_consumer_ref = this.parent.local_consumer.setup(this.parent._orb(), this.parent._poa());
        }
        return supplier_admin;
    }

    public CosEventChannelAdmin.ProxyPushConsumer obtain_push_consumer() {
        CosEventChannelAdmin.ProxyPushConsumer retval = null;
        synchronized (this.parent.updatingPortsLock) {
            retval = CosEventChannelAdmin.ProxyPushConsumerHelper.narrow(this.parent.local_consumer_ref);
        }
        
        return retval;
        
    }

    public CosEventChannelAdmin.ProxyPullConsumer obtain_pull_consumer() {
        CosEventChannelAdmin.ProxyPullConsumer retval = null;
        
        return retval;
    }

}
