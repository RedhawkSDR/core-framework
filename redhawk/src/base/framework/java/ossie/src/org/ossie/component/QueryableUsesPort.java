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

import java.util.Map;
import java.util.HashMap;

import org.ossie.component.UsesPort;
import ExtendedCF.*;
import org.ossie.redhawk.PortCallError;
import org.ossie.component.RHLogger;
import java.util.List;
import java.util.ArrayList;
import java.util.ListIterator;

public abstract class QueryableUsesPort< E > extends QueryablePortPOA { // SUPPRESS CHECKSTYLE Name
    
    public Object updatingPortsLock;
    protected boolean active;
    protected String name;
    protected HashMap<String, E> outPorts;

    public QueryableUsesPort(final String portName) {
        this.outPorts = new HashMap<String, E>();
        this.active = false;
        this.name = portName;
        this.updatingPortsLock = new Object();
    }

    public RHLogger _portLog = null;
    public void setLogger(RHLogger logger)
    {
        this._portLog = logger;
    }

    public List<String> getConnectionIds() {
        List<String> retval = new ArrayList<String>();
        for (String key : outPorts.keySet()) {
            retval.add(key);
        }
        return retval;
    }

    public void __evaluateRequestBasedOnConnections(String __connection_id__, boolean returnValue, boolean inOut, boolean out) throws PortCallError {
        if (__connection_id__.isEmpty() && (outPorts.size() > 1)) {
            if (out || inOut || returnValue) {
                throw new PortCallError("Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.", this.getConnectionIds());
            }
        }
        if (outPorts.isEmpty()) {
            if (out || inOut || returnValue) {
                throw new PortCallError("No connections available.", this.getConnectionIds());
            } else {
                if (!__connection_id__.isEmpty()) {
                    throw new PortCallError("The requested connection id ("+__connection_id__+") does not exist.", this.getConnectionIds());
                }
            }
        }
        if ((!__connection_id__.isEmpty()) && (!outPorts.isEmpty())) {
            if (!outPorts.containsKey(__connection_id__)) {
                throw new PortCallError("The requested connection id ("+__connection_id__+") does not exist.", this.getConnectionIds());
            }
        }
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

    public HashMap<String, E> getPorts() {
        return this.outPorts;
    }

    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) throws CF.PortPackage.InvalidPort, CF.PortPackage.OccupiedPort {
        try {
            // don't want to process while command information is coming in
            synchronized (this.updatingPortsLock) { 
                final E port = narrow(connection);
                this.outPorts.put(connectionId, port);
                this.active = true;
            }
        } catch (final Throwable t) {
            t.printStackTrace();
        }

    }

    public void disconnectPort(final String connectionId) {
        // don't want to process while command information is coming in
        synchronized (this.updatingPortsLock) {
            this.outPorts.remove(connectionId);
            this.active = (this.outPorts.size() != 0);
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

    public UsesConnection[] connections() {
        synchronized (this.updatingPortsLock) {
            final UsesConnection[] connList = new UsesConnection[this.outPorts.size()];
            int index = 0;
            for (Map.Entry<String,E> entry : this.outPorts.entrySet()) {
                org.omg.CORBA.Object obj = (org.omg.CORBA.Object)entry.getValue();
                connList[index++] = new UsesConnection(entry.getKey(), obj);
            }
            return connList;
        }
    }
}
