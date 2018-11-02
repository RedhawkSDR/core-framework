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
import FRONTEND.RFSourceOperations;
import FRONTEND.RFSourceHelper;
import FRONTEND.RFInfoPkt;
import org.ossie.component.PortBase;
import org.ossie.redhawk.PortCallError;

public class OutRFSourcePort extends QueryableUsesPort<RFSourceOperations> implements PortBase {

    /**
     * Map of connection Ids to port objects
     */
    protected Map<String, RFSourceOperations> outConnections = new HashMap<String, RFSourceOperations>();

    public OutRFSourcePort( String portName) {
        super(portName);
        this.outConnections = new HashMap<String, RFSourceOperations>();
    }

    protected RFSourceOperations narrow(org.omg.CORBA.Object connection) {
        RFSourceOperations ops = RFSourceHelper.narrow(connection);
        return ops;
    }

    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) throws CF.PortPackage.InvalidPort, CF.PortPackage.OccupiedPort {
        try {
            synchronized (this.updatingPortsLock) {
                super.connectPort(connection, connectionId);
                final RFSourceOperations port = RFSourceHelper.narrow(connection);
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

    public RFInfoPkt[] available_rf_inputs() throws PortCallError
    {
        return this._get_available_rf_inputs("");
    }
    public RFInfoPkt[] _get_available_rf_inputs(String __connection_id__) throws PortCallError
    {
        RFInfoPkt[] retval = null;

        synchronized(updatingPortsLock){
            try {
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            } catch (PortCallError e) {
                throw e;
            }
            if (this.active) {
                try {
                    if (!__connection_id__.isEmpty()) {
                        retval = this.outPorts.get(__connection_id__).available_rf_inputs();
                    } else {
                        for (RFSourceOperations p : this.outConnections.values()) {
                            retval = p.available_rf_inputs();
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

    public void available_rf_inputs(RFInfoPkt[] data) throws PortCallError
    {
        this.available_rf_inputs(data, "");
    }

    public void available_rf_inputs(RFInfoPkt[] data, String __connection_id__) throws PortCallError
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
                        this.outPorts.get(__connection_id__).available_rf_inputs(data);
                    } else {
                        for (RFSourceOperations p : this.outConnections.values()) {
                            p.available_rf_inputs(data);
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

    public RFInfoPkt current_rf_input() throws PortCallError
    {
        return this._get_current_rf_input("");
    }
    public RFInfoPkt _get_current_rf_input(String __connection_id__) throws PortCallError
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
                        retval = this.outPorts.get(__connection_id__).current_rf_input();
                    } else {
                        for (RFSourceOperations p : this.outConnections.values()) {
                            retval = p.current_rf_input();
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

    public void current_rf_input(RFInfoPkt data) throws PortCallError
    {
        this.current_rf_input(data, "");
    }

    public void current_rf_input(RFInfoPkt data, String __connection_id__) throws PortCallError
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
                        this.outPorts.get(__connection_id__).current_rf_input(data);
                    } else {
                        for (RFSourceOperations p : this.outConnections.values()) {
                            p.current_rf_input(data);
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
        return RFSourceHelper.id();
    }

    public String getDirection() {
        return "Uses";
    }
}

