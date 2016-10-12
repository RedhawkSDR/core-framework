/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK frontendInterfaces.
 *
 * REDHAWK frontendInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK frontendInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
package frontend;

import java.util.HashMap;
import java.util.Map;
import org.ossie.component.QueryableUsesPort;
import FRONTEND.FrontendTunerOperations;
import FRONTEND.FrontendTunerHelper;
import FRONTEND.FrontendException;
import FRONTEND.BadParameterException;
import FRONTEND.NotSupportedException;

public class OutFrontendTunerPort extends QueryableUsesPort<FrontendTunerOperations> implements FrontendTunerOperations {

    protected String name;

    protected Object updatingPortsLock;

    /**
     * Map of connection Ids to port objects
     */
    protected Map<String, FrontendTunerOperations> outConnections = new HashMap<String, FrontendTunerOperations>();

    public OutFrontendTunerPort(String portName) { 
        super(portName);
        this.name = portName;
        this.outConnections = new HashMap<String, FrontendTunerOperations>();
    }

    protected FrontendTunerOperations narrow(org.omg.CORBA.Object connection) {
        FrontendTunerOperations ops = FrontendTunerHelper.narrow(connection);
        return ops; 
    }

    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) throws CF.PortPackage.InvalidPort, CF.PortPackage.OccupiedPort {
        try {
            synchronized (this.updatingPortsLock) {
                super.connectPort(connection, connectionId);
                final FrontendTunerOperations port = FrontendTunerHelper.narrow(connection);
                this.outConnections.put(connectionId, port);
                this.active = true;
            }
        } catch (final Throwable t) {
            t.printStackTrace();
        }

    }

    public void disconnectPort(final String connectionId) {
        synchronized (this.updatingPortsLock) {
            super.disconnectPort(connectionId);
            this.outConnections.remove(connectionId);
            this.active = (this.outConnections.size() != 0);
        }
    }

    public String getTunerType(String id) throws FrontendException, BadParameterException, NotSupportedException {
        String retval = "";

        synchronized(this.updatingPortsLock) {
            if (this.active) {
                for (FrontendTunerOperations p : this.outConnections.values()) {
                    retval = p.getTunerType(id);
                }
            }
        }
        return retval;
    }
 
    public boolean getTunerDeviceControl(String id) throws FrontendException, BadParameterException, NotSupportedException {
        boolean retval = false;

        synchronized(this.updatingPortsLock) { 
            if (this.active) {
                for (FrontendTunerOperations p : this.outConnections.values()) {
                    retval = p.getTunerDeviceControl(id);
                }
            }
        }
        return retval;
    }
 
    public String getTunerGroupId(String id) throws FrontendException, BadParameterException, NotSupportedException {
        String retval = "";

        synchronized(this.updatingPortsLock) {
            if (this.active) {
                for (FrontendTunerOperations p : this.outConnections.values()) {
                    retval = p.getTunerGroupId(id);
                }
            }
        } 
        return retval;
    }
 
    public String getTunerRfFlowId(String id) throws FrontendException, BadParameterException, NotSupportedException {
        String retval = "";

        synchronized(this.updatingPortsLock) {
            if (this.active) {
                
                for (FrontendTunerOperations p : this.outConnections.values()) {
                    retval = p.getTunerRfFlowId(id);
                }
            }
        } 
        
        return retval;
    }
 
    public CF.DataType[] getTunerStatus(String id) throws FrontendException, BadParameterException, NotSupportedException {
        CF.DataType[] retval = null;

        synchronized(this.updatingPortsLock) { 
            if (this.active) {
                for (FrontendTunerOperations p : this.outConnections.values()) {
                    retval = p.getTunerStatus(id);
                }
            }
        }
        return retval;
    }
 }
