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
import org.ossie.component.PortBase;
import org.ossie.redhawk.PortCallError;

public class OutFrontendTunerPort extends QueryableUsesPort<FrontendTunerOperations> implements PortBase {

    /**
     * Map of connection Ids to port objects
     */
    protected Map<String, FrontendTunerOperations> outConnections = new HashMap<String, FrontendTunerOperations>();

    public OutFrontendTunerPort(String portName) { 
        super(portName);
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

    public String getTunerType(String id) throws PortCallError
    {
        return this.getTunerType(id, "");
    }
    public String getTunerType(String id, String __connection_id__) throws PortCallError
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
                        retval = this.outPorts.get(__connection_id__).getTunerType(id);
                    } else {
                        for (FrontendTunerOperations p : this.outConnections.values()) {
                            retval = p.getTunerType(id);
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
 
    public boolean getTunerDeviceControl(String id) throws PortCallError
    {
        return this.getTunerDeviceControl(id, "");
    }
    public boolean getTunerDeviceControl(String id, String __connection_id__) throws PortCallError
    {
        boolean retval = false;

        synchronized(updatingPortsLock){
            try {
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            } catch (PortCallError e) {
                throw e;
            }
            if (this.active) {
                try {
                    if (!__connection_id__.isEmpty()) {
                        retval = this.outPorts.get(__connection_id__).getTunerDeviceControl(id);
                    } else {
                        for (FrontendTunerOperations p : this.outConnections.values()) {
                            retval = p.getTunerDeviceControl(id);
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
 
    public String getTunerGroupId(String id) throws PortCallError
    {
        return this.getTunerGroupId(id, "");
    }
    public String getTunerGroupId(String id, String __connection_id__) throws PortCallError
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
                        retval = this.outPorts.get(__connection_id__).getTunerGroupId(id);
                    } else {
                        for (FrontendTunerOperations p : this.outConnections.values()) {
                            retval = p.getTunerGroupId(id);
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
 
    public String getTunerRfFlowId(String id) throws PortCallError
    {
        return this.getTunerRfFlowId(id, "");
    }
    public String getTunerRfFlowId(String id, String __connection_id__) throws PortCallError
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
                        retval = this.outPorts.get(__connection_id__).getTunerRfFlowId(id);
                    } else {
                        for (FrontendTunerOperations p : this.outConnections.values()) {
                            retval = p.getTunerRfFlowId(id);
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
 
    public CF.DataType[] getTunerStatus(String id) throws PortCallError
    {
        return this.getTunerStatus(id, "");
    }
    public CF.DataType[] getTunerStatus(String id, String __connection_id__) throws PortCallError
    {
        CF.DataType[] retval = null;

        synchronized(updatingPortsLock){
            try {
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            } catch (PortCallError e) {
                throw e;
            }
            if (this.active) {
                try {
                    if (!__connection_id__.isEmpty()) {
                        retval = this.outPorts.get(__connection_id__).getTunerStatus(id);
                    } else {
                        for (FrontendTunerOperations p : this.outConnections.values()) {
                            retval = p.getTunerStatus(id);
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

    public String getRepid() {
        return FrontendTunerHelper.id();
    }

    public String getDirection() {
        return "Uses";
    }
 }
