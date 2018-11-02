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
import FRONTEND.DigitalTunerOperations;
import FRONTEND.DigitalTunerHelper;
import FRONTEND.FrontendException;
import FRONTEND.BadParameterException;
import FRONTEND.NotSupportedException;
import org.ossie.component.PortBase;
import org.ossie.redhawk.PortCallError;

public class OutDigitalTunerPort extends QueryableUsesPort<DigitalTunerOperations> implements PortBase {

    /**
     * Map of connection Ids to port objects
     */
    protected Map<String, DigitalTunerOperations> outConnections = new HashMap<String, DigitalTunerOperations>();

    public OutDigitalTunerPort(String portName) {
        super(portName);
        this.outConnections = new HashMap<String, DigitalTunerOperations>();
    }

    protected DigitalTunerOperations narrow(org.omg.CORBA.Object connection) {
        DigitalTunerOperations ops = DigitalTunerHelper.narrow(connection);
        return ops; 
    }

    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) throws CF.PortPackage.InvalidPort, CF.PortPackage.OccupiedPort {
        try {
            synchronized (this.updatingPortsLock) {
                super.connectPort(connection, connectionId);
                final DigitalTunerOperations port = DigitalTunerHelper.narrow(connection);
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
                        for (DigitalTunerOperations p : this.outConnections.values()) {
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
                        for (DigitalTunerOperations p : this.outConnections.values()) {
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
                        for (DigitalTunerOperations p : this.outConnections.values()) {
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
                        for (DigitalTunerOperations p : this.outConnections.values()) {
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
                        for (DigitalTunerOperations p : this.outConnections.values()) {
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
 
    public void setTunerCenterFrequency(String id, double data) throws PortCallError
    {
        this.setTunerCenterFrequency(id, data, "");
    }

    public void setTunerCenterFrequency(String id, double data, String __connection_id__) throws PortCallError
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
                        this.outPorts.get(__connection_id__).setTunerCenterFrequency(id, data);
                    } else {
                        for (DigitalTunerOperations p : this.outConnections.values()) {
                            p.setTunerCenterFrequency(id, data);
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
 
    public double getTunerCenterFrequency(String id) throws PortCallError
    {
        return this.getTunerCenterFrequency(id, "");
    }
    public double getTunerCenterFrequency(String id, String __connection_id__) throws PortCallError
    {
        double retval = 0.0;

        synchronized(updatingPortsLock){
            try {
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            } catch (PortCallError e) {
                throw e;
            }
            if (this.active) {
                try {
                    if (!__connection_id__.isEmpty()) {
                        retval = this.outPorts.get(__connection_id__).getTunerCenterFrequency(id);
                    } else {
                        for (DigitalTunerOperations p : this.outConnections.values()) {
                            retval = p.getTunerCenterFrequency(id);
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
 
    public void setTunerBandwidth(String id, double data) throws PortCallError
    {
        this.setTunerBandwidth(id, data, "");
    }

    public void setTunerBandwidth(String id, double data, String __connection_id__) throws PortCallError
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
                        this.outPorts.get(__connection_id__).setTunerBandwidth(id, data);
                    } else {
                        for (DigitalTunerOperations p : this.outConnections.values()) {
                            p.setTunerBandwidth(id, data);
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
 
    public double getTunerBandwidth(String id) throws PortCallError
    {
        return this.getTunerBandwidth(id, "");
    }
    public double getTunerBandwidth(String id, String __connection_id__) throws PortCallError
    {
        double retval = 0.0;

        synchronized(updatingPortsLock){
            try {
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            } catch (PortCallError e) {
                throw e;
            }
            if (this.active) {
                try {
                    if (!__connection_id__.isEmpty()) {
                        retval = this.outPorts.get(__connection_id__).getTunerBandwidth(id);
                    } else {
                        for (DigitalTunerOperations p : this.outConnections.values()) {
                            retval = p.getTunerBandwidth(id);
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
 
    public void setTunerAgcEnable(String id, boolean data) throws PortCallError
    {
        this.setTunerAgcEnable(id, data, "");
    }

    public void setTunerAgcEnable(String id, boolean data, String __connection_id__) throws PortCallError
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
                        this.outPorts.get(__connection_id__).setTunerAgcEnable(id, data);
                    } else {
                        for (DigitalTunerOperations p : this.outConnections.values()) {
                            p.setTunerAgcEnable(id, data);
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
 
    public boolean getTunerAgcEnable(String id) throws PortCallError
    {
        return this.getTunerAgcEnable(id, "");
    }
    public boolean getTunerAgcEnable(String id, String __connection_id__) throws PortCallError
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
                        retval = this.outPorts.get(__connection_id__).getTunerAgcEnable(id);
                    } else {
                        for (DigitalTunerOperations p : this.outConnections.values()) {
                            retval = p.getTunerAgcEnable(id);
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
 
    public void setTunerGain(String id, float data) throws PortCallError
    {
        this.setTunerGain(id, data, "");
    }

    public void setTunerGain(String id, float data, String __connection_id__) throws PortCallError
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
                        this.outPorts.get(__connection_id__).setTunerGain(id, data);
                    } else {
                        for (DigitalTunerOperations p : this.outConnections.values()) {
                            p.setTunerGain(id, data);
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
 
    public float getTunerGain(String id) throws PortCallError
    {
        return this.getTunerGain(id, "");
    }
    public float getTunerGain(String id, String __connection_id__) throws PortCallError
    {
        float retval = (float)0.0;

        synchronized(updatingPortsLock){
            try {
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            } catch (PortCallError e) {
                throw e;
            }
            if (this.active) {
                try {
                    if (!__connection_id__.isEmpty()) {
                        retval = this.outPorts.get(__connection_id__).getTunerGain(id);
                    } else {
                        for (DigitalTunerOperations p : this.outConnections.values()) {
                            retval = p.getTunerGain(id);
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
 
    public void setTunerReferenceSource(String id, int data) throws PortCallError
    {
        this.setTunerReferenceSource(id, data, "");
    }

    public void setTunerReferenceSource(String id, int data, String __connection_id__) throws PortCallError
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
                        this.outPorts.get(__connection_id__).setTunerReferenceSource(id, data);
                    } else {
                        for (DigitalTunerOperations p : this.outConnections.values()) {
                            p.setTunerReferenceSource(id, data);
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
 
    public int getTunerReferenceSource(String id) throws PortCallError
    {
        return this.getTunerReferenceSource(id, "");
    }
    public int getTunerReferenceSource(String id, String __connection_id__) throws PortCallError
    {
        int retval = 0;

        synchronized(updatingPortsLock){
            try {
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            } catch (PortCallError e) {
                throw e;
            }
            if (this.active) {
                try {
                    if (!__connection_id__.isEmpty()) {
                        retval = this.outPorts.get(__connection_id__).getTunerReferenceSource(id);
                    } else {
                        for (DigitalTunerOperations p : this.outConnections.values()) {
                            retval = p.getTunerReferenceSource(id);
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
 
    public void setTunerEnable(String id, boolean data) throws PortCallError
    {
        this.setTunerEnable(id, data, "");
    }

    public void setTunerEnable(String id, boolean data, String __connection_id__) throws PortCallError
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
                        this.outPorts.get(__connection_id__).setTunerEnable(id, data);
                    } else {
                        for (DigitalTunerOperations p : this.outConnections.values()) {
                            p.setTunerEnable(id, data);
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
 
    public boolean getTunerEnable(String id) throws PortCallError
    {
        return this.getTunerEnable(id, "");
    }
    public boolean getTunerEnable(String id, String __connection_id__) throws PortCallError
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
                        retval = this.outPorts.get(__connection_id__).getTunerEnable(id);
                    } else {
                        for (DigitalTunerOperations p : this.outConnections.values()) {
                            retval = p.getTunerEnable(id);
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
 
    public void setTunerOutputSampleRate(String id, double data) throws PortCallError
    {
        this.setTunerOutputSampleRate(id, data, "");
    }

    public void setTunerOutputSampleRate(String id, double data, String __connection_id__) throws PortCallError
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
                        this.outPorts.get(__connection_id__).setTunerOutputSampleRate(id, data);
                    } else {
                        for (DigitalTunerOperations p : this.outConnections.values()) {
                            p.setTunerOutputSampleRate(id, data);
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
 
    public double getTunerOutputSampleRate(String id) throws PortCallError
    {
        return this.getTunerOutputSampleRate(id, "");
    }
    public double getTunerOutputSampleRate(String id, String __connection_id__) throws PortCallError
    {
        double retval = 0.0;

        synchronized(updatingPortsLock){
            try {
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            } catch (PortCallError e) {
                throw e;
            }
            if (this.active) {
                try {
                    if (!__connection_id__.isEmpty()) {
                        retval = this.outPorts.get(__connection_id__).getTunerOutputSampleRate(id);
                    } else {
                        for (DigitalTunerOperations p : this.outConnections.values()) {
                            retval = p.getTunerOutputSampleRate(id);
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
        return DigitalTunerHelper.id();
    }

    public String getDirection() {
        return "Uses";
    }
}
