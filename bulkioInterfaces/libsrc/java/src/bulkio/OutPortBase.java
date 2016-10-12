/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
package bulkio;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.log4j.Logger;

import ExtendedCF.UsesConnection;

import BULKIO.PortUsageType;
import BULKIO.StreamSRI;
import BULKIO.UsesPortStatistics;

import org.ossie.component.PortBase;

public abstract class OutPortBase<E> extends BULKIO.UsesPortStatisticsProviderPOA implements org.ossie.component.PortBase {
    /**
     * Name within the component
     */
    protected String name;

    /**
     * Is this port active?
     */
    protected boolean active;

    /**
     * Lock for updates to port state
     */
    protected final Object updatingPortsLock = new Object();

    /**
     * Map of connection Ids to port objects
     */
    protected final Map<String, E> outConnections = new HashMap<String, E>();

    /**
     * Map of connection ID to statistics
     */
    protected final Map<String, linkStatistics> stats = new HashMap<String, linkStatistics>();

    /**
     * Target for port logging
     */
    protected Logger logger;

    /**
     * Event listener when connect/disconnet events happen
     */
    protected ConnectionEventListener callback = null;

    /**
     * Map of stream IDs to streamSRIs
     */
    protected final Map<String,SriMapStruct> currentSRIs = new HashMap<String,SriMapStruct>();

    protected OutPortBase(String portName) {
        this(portName, null, null);
    }

    protected OutPortBase(String portName, Logger logger, ConnectionEventListener connectionListener) {
        this.name = portName;
        this.active = false;
        this.logger = logger;
        this.callback = connectionListener;
    }

    /**
     * Returns true if this port is active.
     */
    public boolean isActive() {
        return this.active;
    }

    /**
     * Set the active state of this port.
     */
    public void setActive(final boolean active) {
        this.active = active;
    }

    /**
     * Returns the name of this port.
     */
    public String getName() {
        return this.name;
    }

    /**
     * Replaces the current logger.
     */
    public void setLogger(Logger newlogger){
        synchronized (this.updatingPortsLock) {
	    this.logger = newlogger;
	}
    }

    /**
     * Sets the listener to receive connect and disconnect notifications.
     */
    public void setConnectionEventListener(ConnectionEventListener newListener){
        synchronized (this.updatingPortsLock) {
	    this.callback = newListener;
	}
    }

    /**
     * @deprecated
     */
    public HashMap<String,E> getPorts() {
        return new HashMap<String,E>();
    }

    /**
     * Returns the current state.
     */
    public PortUsageType state() {
        synchronized (this.updatingPortsLock) {
            if (this.outConnections.size() > 0) {
                return PortUsageType.ACTIVE;
            } else {
                return PortUsageType.IDLE;
            }
        }
    }

    /**
     * Returns the current connections.
     */
    public UsesConnection[] connections() {
        final List<UsesConnection> connList = new ArrayList<UsesConnection>();
        synchronized (this.updatingPortsLock) {
            for (Entry<String, E> ent : this.outConnections.entrySet()) {
                org.omg.CORBA.Object my_obj = (org.omg.CORBA.Object)ent.getValue();
                if (my_obj instanceof omnijni.ObjectImpl) {
                    String ior = omnijni.ORB.object_to_string(my_obj);
                    my_obj = this._orb().string_to_object(ior);
                }
                connList.add(new UsesConnection(ent.getKey(), my_obj));
            }
        }
        return connList.toArray(new UsesConnection[connList.size()]);
    }

    /**
     * Enables tracking of statistics.
     */
    public void enableStats(final boolean enable)
    {
        for (String connId : outConnections.keySet()) {
            stats.get(connId).setEnabled(enable);
        }
    };

    /**
     * Returns the current statistics.
     */
    public UsesPortStatistics[] statistics() {
        final List<UsesPortStatistics> portStats = new ArrayList<UsesPortStatistics>();
        synchronized (this.updatingPortsLock) {
            for (String connId : this.outConnections.keySet()) {
                portStats.add(new UsesPortStatistics(connId, this.stats.get(connId).retrieve()));
            }
        }

        return portStats.toArray(new UsesPortStatistics[portStats.size()]);
    }

    /**
     * Returns the SRIs for all current streams.
     */
    public StreamSRI[] activeSRIs()
    {
        ArrayList<StreamSRI> sriList = new ArrayList<StreamSRI>();
        for(Map.Entry<String,SriMapStruct> entry: this.currentSRIs.entrySet()) {
            SriMapStruct srimap = entry.getValue();
            sriList.add(srimap.sri);
        }
        return sriList.toArray(new StreamSRI[0]);
    }

	public String getRepid()
	{
		return "IDL:CORBA/Object:1.0";
	}

	public String getDirection()
	{
		return "Uses";
	}
}
