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
import FRONTEND.GPSOperations;
import FRONTEND.GPSHelper;
import FRONTEND.GPSInfo;
import FRONTEND.GpsTimePos;


public class OutGPSPort extends QueryableUsesPort<GPSOperations> implements GPSOperations {

    protected String name;
 
    protected Object updatingPortsLock;

    /**
     * Map of connection Ids to port objects
     */
    protected Map<String, GPSOperations> outConnections = new HashMap<String, GPSOperations>();

    public OutGPSPort( String portName) {
        super(portName);
        this.name = portName;
        this.outConnections = new HashMap<String, GPSOperations>();
    }

    protected GPSOperations narrow(org.omg.CORBA.Object connection)
    {
        GPSOperations ops = GPSHelper.narrow(connection);
        return ops;
    }

    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) throws CF.PortPackage.InvalidPort, CF.PortPackage.OccupiedPort
    {
        try {
            synchronized (this.updatingPortsLock) {
                super.connectPort(connection, connectionId);
                final GPSOperations port = GPSHelper.narrow(connection);
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

    public GPSInfo gps_info()
    {
        GPSInfo retval = null;

        synchronized(updatingPortsLock){
            if (this.active) {
                for (GPSOperations p : this.outConnections.values()) {
                    retval = p.gps_info();
                }
            }
        }
        return retval;
    }

    public void gps_info(GPSInfo data)
    {
        synchronized(updatingPortsLock){
            if (this.active) {
                for (GPSOperations p : this.outConnections.values()) {
                    p.gps_info(data);
                }
            }
        }
    }

    public GpsTimePos gps_time_pos()
    {
        GpsTimePos retval = null;

        synchronized(this.updatingPortsLock) { 
            if (this.active) {
                for (GPSOperations p : this.outConnections.values()) {
                    retval = p.gps_time_pos();
                }
            }
        }
        return retval;
    }

    public void gps_time_pos(GpsTimePos data)
    {
        synchronized(this.updatingPortsLock) {
            if (this.active) {
                for (GPSOperations p : this.outConnections.values()) {
                    p.gps_time_pos(data);
                }
            }
        }
    }
}

