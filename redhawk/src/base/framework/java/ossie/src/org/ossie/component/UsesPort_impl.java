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

import java.util.HashMap;

import org.apache.log4j.Logger;

import CF.PortPOA;

import CF.PortPackage.*;

@Deprecated
/**
 * This class is deprecated, suggest UsesPort instead.
 */
public abstract class UsesPort_impl< E > extends PortPOA { // SUPPRESS CHECKSTYLE Name
    public final static Logger logger = Logger.getLogger(UsesPort_impl.class.getName());
    
    public Object updatingPortsLock;
    protected boolean active;
    protected String name;
    protected HashMap<String, E> outPorts;

    public UsesPort_impl(final String portName) {
        this.outPorts = new HashMap<String, E>();
        this.active = false;
        this.name = portName;
        this.updatingPortsLock = new Object();
    }

    public boolean isActive() {
        return this.active;
    }

    public void setActive(final boolean active) {
        if (active != this.active) logger.debug("Setting port " + this.name + " activation state: " + active);
        this.active = active;
    }

    public String getName() {
        return this.name;
    }

    public HashMap<String, E> getPorts() {
        return this.outPorts;
    }

    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) throws CF.PortPackage.InvalidPort, CF.PortPackage.OccupiedPort {
        logger.debug("Connecting port with id " + connectionId);
        try {
            // don't want to process while command information is coming in
            synchronized (this.updatingPortsLock) { 
                final E port = narrow(connection);
                this.outPorts.put(connectionId, port);
                this.setActive(true);
            }
        } catch (final Throwable t) {
            logger.error("Failed to connect port", t);
        }

    }

    public void disconnectPort(final String connectionId) {
        logger.debug("Disconnecting port with id " + connectionId);
        // don't want to process while command information is coming in
        synchronized (this.updatingPortsLock) {
            this.outPorts.remove(connectionId);
            this.setActive((this.outPorts.size() != 0));
        }
    }

    /**
     * This method is used to narrow from the CORBA object to an appropriate
     * class. The implementer should use <type>Helper.narrow to return the
     * correct narrowed type.
     * 
     * @param connection The connected object to convert
     * @return the narrowed object
     */
    protected abstract E narrow(org.omg.CORBA.Object connection);
}
