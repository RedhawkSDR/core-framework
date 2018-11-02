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
import FRONTEND.NavDataOperations;
import FRONTEND.NavDataHelper;
import FRONTEND.NavigationPacket;
import org.ossie.component.PortBase;
import org.ossie.redhawk.PortCallError;

public class OutNavDataPort extends QueryableUsesPort<NavDataOperations> implements PortBase {

    /**
     * Map of connection Ids to port objects
     */
    protected Map<String, NavDataOperations> outConnections = new HashMap<String, NavDataOperations>();

    public OutNavDataPort( String portName) {
        super(portName);
        this.outConnections = new HashMap<String, NavDataOperations>();
    }

    protected NavDataOperations narrow(org.omg.CORBA.Object connection)
    {
        NavDataOperations ops = NavDataHelper.narrow(connection);
        return ops;
    }

    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) throws CF.PortPackage.InvalidPort, CF.PortPackage.OccupiedPort
    {
        try {
            synchronized (this.updatingPortsLock) {
                super.connectPort(connection, connectionId);
                final NavDataOperations port = NavDataHelper.narrow(connection);
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

    public NavigationPacket nav_packet() throws PortCallError
    {
        return this._get_nav_packet("");
    }
    public NavigationPacket _get_nav_packet(String __connection_id__) throws PortCallError
    {
        NavigationPacket retval = null;

        synchronized(updatingPortsLock){
            try {
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            } catch (PortCallError e) {
                throw e;
            }
            if (this.active) {
                try {
                    if (!__connection_id__.isEmpty()) {
                        retval = this.outPorts.get(__connection_id__).nav_packet();
                    } else {
                        for (NavDataOperations p : this.outConnections.values()) {
                            retval = p.nav_packet();
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

    public void nav_packet(NavigationPacket data) throws PortCallError
    {
        this.nav_packet(data, "");
    }

    public void nav_packet(NavigationPacket data, String __connection_id__) throws PortCallError
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
                        this.outPorts.get(__connection_id__).nav_packet(data);
                    } else {
                        for (NavDataOperations p : this.outConnections.values()) {
                            p.nav_packet(data);
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
        return NavDataHelper.id();
    }

    public String getDirection() {
        return "Uses";
    }
}

