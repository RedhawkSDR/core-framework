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
import FRONTEND.RFInfoOperations;
import FRONTEND.RFInfoHelper;
import FRONTEND.RFInfoPkt;
import org.ossie.component.PortBase;
import org.ossie.redhawk.PortCallError;

public class OutRFInfoPort extends QueryableUsesPort<RFInfoOperations> implements PortBase {

    /**
     * Map of connection Ids to port objects
     */
    protected Map<String, RFInfoOperations> outConnections = new HashMap<String, RFInfoOperations>();

    public OutRFInfoPort( String portName) {
        super(portName);
        this.outConnections = new HashMap<String, RFInfoOperations>();
    }

    protected RFInfoOperations narrow(org.omg.CORBA.Object connection) {
        RFInfoOperations ops = RFInfoHelper.narrow(connection);
        return ops;
    }

    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) throws CF.PortPackage.InvalidPort, CF.PortPackage.OccupiedPort {
        try {
            synchronized (this.updatingPortsLock) {
                super.connectPort(connection, connectionId);
                final RFInfoOperations port = RFInfoHelper.narrow(connection);
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

    public String rf_flow_id() throws PortCallError
    {
        return this._get_rf_flow_id("");
    }
    public String _get_rf_flow_id(String __connection_id__) throws PortCallError
    {
        String retval = "";

        synchronized(updatingPortsLock){
            try {
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            } catch (PortCallError e) {
                throw e;
            }
            if (this.active) {
                try {
                    if (!__connection_id__.isEmpty()) {
                        retval = this.outPorts.get(__connection_id__).rf_flow_id();
                    } else {
                        for (RFInfoOperations p : this.outConnections.values()) {
                            retval = p.rf_flow_id();
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

    public void rf_flow_id(String data) throws PortCallError
    {
        this.rf_flow_id(data, "");
    }

    public void rf_flow_id(String data, String __connection_id__) throws PortCallError
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
                        this.outPorts.get(__connection_id__).rf_flow_id(data);
                    } else {
                        for (RFInfoOperations p : this.outConnections.values()) {
                            p.rf_flow_id(data);
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

    public RFInfoPkt rfinfo_pkt() throws PortCallError
    {
        return this._get_rfinfo_pkt("");
    }
    public RFInfoPkt _get_rfinfo_pkt(String __connection_id__) throws PortCallError
    {
        RFInfoPkt retval = null;

        synchronized(updatingPortsLock){
            try {
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            } catch (PortCallError e) {
                throw e;
            }
            if (this.active) {
                try {
                    if (!__connection_id__.isEmpty()) {
                        retval = this.outPorts.get(__connection_id__).rfinfo_pkt();
                    } else {
                        for (RFInfoOperations p : this.outConnections.values()) {
                            retval = p.rfinfo_pkt();
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

    public void rfinfo_pkt(RFInfoPkt data) throws PortCallError
    {
        this.rfinfo_pkt(data, "");
    }

    public void rfinfo_pkt(RFInfoPkt data, String __connection_id__) throws PortCallError
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
                        this.outPorts.get(__connection_id__).rfinfo_pkt(data);
                    } else {
                        for (RFInfoOperations p : this.outConnections.values()) {
                            p.rfinfo_pkt(data);
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
        return RFInfoHelper.id();
    }

    public String getDirection() {
        return "Uses";
    }
}

