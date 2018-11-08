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
import org.ossie.component.PortBase;
import org.ossie.redhawk.PortCallError;

public class OutGPSPort extends QueryableUsesPort<GPSOperations> implements PortBase {

    /**
     * Map of connection Ids to port objects
     */
    protected Map<String, GPSOperations> outConnections = new HashMap<String, GPSOperations>();

    public OutGPSPort( String portName) {
        super(portName);
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

    public GPSInfo gps_info() throws PortCallError
    {
        return this._get_gps_info("");
    }

    public GPSInfo _get_gps_info(String __connection_id__) throws PortCallError
    {
        GPSInfo retval = null;

        synchronized(updatingPortsLock){
            try {
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            } catch (PortCallError e) {
                throw e;
            }
            if (this.active) {
                try {
                    if (!__connection_id__.isEmpty()) {
                        retval = this.outPorts.get(__connection_id__).gps_info();
                    } else {
                        for (GPSOperations p : this.outConnections.values()) {
                            retval = p.gps_info();
                        }
                    }
                } catch(org.omg.CORBA.SystemException e) {
                    throw e;
                } catch(Throwable e) {
                    throw new RuntimeException(e);
                }
            }
        }
        return retval;
    }

    public void gps_info(GPSInfo data) throws PortCallError
    {
        this.gps_info(data, "");
    }

    public void gps_info(GPSInfo data, String __connection_id__) throws PortCallError
    {
        synchronized(updatingPortsLock){
            try {
                __evaluateRequestBasedOnConnections(__connection_id__, false, false, false);
            } catch (PortCallError e) {
                throw e;
            }
            if (this.active) {
                try {
                    if (!__connection_id__.isEmpty()) {
                        this.outPorts.get(__connection_id__).gps_info(data);
                    } else {
                        for (GPSOperations p : this.outConnections.values()) {
                            p.gps_info(data);
                        }
                    }
                } catch(org.omg.CORBA.SystemException e) {
                    throw e;
                } catch(Throwable e) {
                    throw new RuntimeException(e);
                }
            }
        }
    }

    public GpsTimePos gps_time_pos() throws PortCallError
    {
        return this._get_gps_time_pos("");
    }

    public GpsTimePos _get_gps_time_pos(String __connection_id__) throws PortCallError
    {
        GpsTimePos retval = null;

        synchronized(updatingPortsLock){
            try {
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            } catch (PortCallError e) {
                throw e;
            }
            if (this.active) {
                try {
                    if (!__connection_id__.isEmpty()) {
                        retval = this.outPorts.get(__connection_id__).gps_time_pos();
                    } else {
                        for (GPSOperations p : this.outConnections.values()) {
                            retval = p.gps_time_pos();
                        }
                    }
                } catch(org.omg.CORBA.SystemException e) {
                    throw e;
                } catch(Throwable e) {
                    throw new RuntimeException(e);
                }
            }
        }
        return retval;
    }

    public void gps_time_pos(GpsTimePos data) throws PortCallError
    {
        this.gps_time_pos(data, "");
    }

    public void gps_time_pos(GpsTimePos data, String __connection_id__) throws PortCallError
    {
        synchronized(updatingPortsLock){
            try {
                __evaluateRequestBasedOnConnections(__connection_id__, false, false, false);
            } catch (PortCallError e) {
                throw e;
            }
            if (this.active) {
                try {
                    if (!__connection_id__.isEmpty()) {
                        this.outPorts.get(__connection_id__).gps_time_pos(data);
                    } else {
                        for (GPSOperations p : this.outConnections.values()) {
                            p.gps_time_pos(data);
                        }
                    }
                } catch(org.omg.CORBA.SystemException e) {
                    throw e;
                } catch(Throwable e) {
                    throw new RuntimeException(e);
                }
            }
        }
    }

    public String getRepid() {
        return GPSHelper.id();
    }

    public String getDirection() {
        return "Uses";
    }
}

