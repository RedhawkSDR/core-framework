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
package org.ossie.component;

import org.omg.CosNaming.NamingContext;
import org.omg.CosNaming.NamingContextHelper;

import org.omg.CORBA.ORB;
import CF.ResourceOperations;
import CF.Application;
import CF.ApplicationRegistrar;
import CF.ApplicationRegistrarHelper;
import CF.DomainManager;
import org.ossie.component.ThreadedResource;
import org.ossie.component.ThreadedComponent;
import org.ossie.redhawk.ApplicationContainer;
import org.ossie.redhawk.NetworkContainer;

public abstract class Component extends ThreadedResource implements ResourceOperations,ThreadedComponent {
    protected ApplicationContainer _app;
    protected NetworkContainer _net;
    
    public Component() {
        super();
        this._app = null;
        this._net = null;
    }
    
    public ApplicationContainer getApplication() {
        return this._app;
    }
    
    public NetworkContainer getNetwork() {
        return this._net;
    }
    
    public void setAdditionalParameters(String ApplicationRegistrarIOR, String nic) {
        super.setAdditionalParameters(ApplicationRegistrarIOR, nic);
        final org.omg.CORBA.Object obj = this.orb.string_to_object(ApplicationRegistrarIOR);
        this._net = new NetworkContainer(nic);
        ApplicationRegistrar appReg = null;
        try {
            appReg = ApplicationRegistrarHelper.narrow(obj);
        } catch (Exception e) {}
        if (appReg!=null) {
            this._app = new ApplicationContainer(appReg.app());
        }
    }
};
