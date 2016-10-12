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

/**
 *
 * Identification: $Revision$
 */
package org.ossie.component;

import CF.PortPOA;

@Deprecated
/**
 * This class is deprecated, suggest class ProvidesPort instead.
 */
public class ProvidesPort_impl extends PortPOA { // SUPPRESS CHECKSTYLE Name
    public Object updatingPortsLock;
    protected Object newMessage;
    protected String name;

    public ProvidesPort_impl(final String portName) {
        this.name = portName;
    }

    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) {
        synchronized (this.updatingPortsLock) {
            // Nothing to do on the provides side
        } // don't want to process while command information is coming in
    }

    public void disconnectPort(final String connectionId) {
        synchronized (this.updatingPortsLock) {
            // Nothing to do on the provides side
        } // don't want to process while command information is coming in
    }
}
