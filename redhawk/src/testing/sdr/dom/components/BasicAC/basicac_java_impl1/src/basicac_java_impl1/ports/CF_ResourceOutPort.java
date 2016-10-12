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

package basicac_java_impl1.ports;


import java.util.HashMap;
import java.util.Map;

import org.ossie.component.UsesPort;
import CF.*;

/**
 * @generated
 */
public class CF_ResourceOutPort extends UsesPort<ResourceOperations> implements ResourceOperations {

    /**
     * Map of connection Ids to port objects
     * @generated
     */
    private Map<String, ResourceOperations> outConnections = new HashMap<String, ResourceOperations>();

    /**
     * @generated
     */
    public CF_ResourceOutPort(String portName) 
    {
        super(portName);

        this.outConnections = new HashMap<String, ResourceOperations>();
        //begin-user-code
        //end-user-code
    }

    /**
     * @generated
     */
    protected ResourceOperations narrow(org.omg.CORBA.Object connection) 
    {
        //begin-user-code
        //end-user-code
        return ResourceHelper.narrow(connection);
    }

    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) {
        try {
            // don't want to process while command information is coming in
            synchronized (this.updatingPortsLock) { 
                final ResourceOperations port = ResourceHelper.narrow(connection);
                this.outConnections.put(connectionId, port);
                this.active = true;
            }
        } catch (final Throwable t) {
            t.printStackTrace();
        }

    }

    public void disconnectPort(final String connectionId) {
        // don't want to process while command information is coming in
        synchronized (this.updatingPortsLock) {
            this.outConnections.remove(connectionId);
            this.active = (this.outConnections.size() != 0);
        }
    }

    /**
     * @generated
     */
    public boolean started() 
    {
        boolean retval = false;
        
        synchronized(this.updatingPortsLock) {    // don't want to process while command information is coming in
            if (this.active) {
                //begin-user-code
                //end-user-code
                
                for (ResourceOperations p : this.outConnections.values()) {
                       retval = p.started();
                }
            }
        }    // don't want to process while command information is coming in
        
        //begin-user-code
        //end-user-code
        
        return retval;
    }
    /**
     * @generated
     */
    public void configure(CF.DataType[] configProperties) throws CF.PropertySetPackage.InvalidConfiguration, CF.PropertySetPackage.PartialConfiguration
    {
        synchronized(this.updatingPortsLock) {    // don't want to process while command information is coming in
            if (this.active) {
                //begin-user-code
                //end-user-code
                
                for (ResourceOperations p : this.outConnections.values()) {
                       p.configure(configProperties);
                }
            }
        }    // don't want to process while command information is coming in
        
        //begin-user-code
        //end-user-code
        
        return;
    }
    /**
     * @generated
     */
    public void query(CF.PropertiesHolder configProperties) throws CF.UnknownProperties
    {
        synchronized(this.updatingPortsLock) {    // don't want to process while command information is coming in
            if (this.active) {
                //begin-user-code
                //end-user-code
                
                for (ResourceOperations p : this.outConnections.values()) {
                       p.query(configProperties);
                }
            }
        }    // don't want to process while command information is coming in
        
        //begin-user-code
        //end-user-code
        
        return;
    }
    /**
     * @generated
     */
    public org.omg.CORBA.Object getPort(String name) throws CF.PortSupplierPackage.UnknownPort
    {
        org.omg.CORBA.Object retval = null;
        
        synchronized(this.updatingPortsLock) {    // don't want to process while command information is coming in
            if (this.active) {
                //begin-user-code
                //end-user-code
                
                for (ResourceOperations p : this.outConnections.values()) {
                       retval = p.getPort(name);
                }
            }
        }    // don't want to process while command information is coming in
        
        //begin-user-code
        //end-user-code
        
        return retval;
    }
    /**
     * @generated
     */
    public void start() throws CF.ResourcePackage.StartError
    {
        synchronized(this.updatingPortsLock) {    // don't want to process while command information is coming in
            if (this.active) {
                //begin-user-code
                //end-user-code
                
                for (ResourceOperations p : this.outConnections.values()) {
                       p.start();
                }
            }
        }    // don't want to process while command information is coming in
        
        //begin-user-code
        //end-user-code
        
        return;
    }
    /**
     * @generated
     */
    public void initialize() throws CF.LifeCyclePackage.InitializeError
    {
        synchronized(this.updatingPortsLock) {    // don't want to process while command information is coming in
            if (this.active) {
                //begin-user-code
                //end-user-code
                
                for (ResourceOperations p : this.outConnections.values()) {
                       p.initialize();
                }
            }
        }    // don't want to process while command information is coming in
        
        //begin-user-code
        //end-user-code
        
        return;
    }
    /**
     * @generated
     */
    public void releaseObject() throws CF.LifeCyclePackage.ReleaseError
    {
        synchronized(this.updatingPortsLock) {    // don't want to process while command information is coming in
            if (this.active) {
                //begin-user-code
                //end-user-code
                
                for (ResourceOperations p : this.outConnections.values()) {
                       p.releaseObject();
                }
            }
        }    // don't want to process while command information is coming in
        
        //begin-user-code
        //end-user-code
        
        return;
    }
    /**
     * @generated
     */
    public void runTest(int testid, CF.PropertiesHolder testValues) throws CF.UnknownProperties, CF.TestableObjectPackage.UnknownTest
    {
        synchronized(this.updatingPortsLock) {    // don't want to process while command information is coming in
            if (this.active) {
                //begin-user-code
                //end-user-code
                
                for (ResourceOperations p : this.outConnections.values()) {
                       p.runTest(testid, testValues);
                }
            }
        }    // don't want to process while command information is coming in
        
        //begin-user-code
        //end-user-code
        
        return;
    }
    /**
     * @generated
     */
    public String identifier() 
    {
        String retval = "";
        
        synchronized(this.updatingPortsLock) {    // don't want to process while command information is coming in
            if (this.active) {
                //begin-user-code
                //end-user-code
                
                for (ResourceOperations p : this.outConnections.values()) {
                       retval = p.identifier();
                }
            }
        }    // don't want to process while command information is coming in
        
        //begin-user-code
        //end-user-code
        
        return retval;
    }
    /**
     * @generated
     */
    public void stop() throws CF.ResourcePackage.StopError
    {
        synchronized(this.updatingPortsLock) {    // don't want to process while command information is coming in
            if (this.active) {
                //begin-user-code
                //end-user-code
                
                for (ResourceOperations p : this.outConnections.values()) {
                       p.stop();
                }
            }
        }    // don't want to process while command information is coming in
        
        //begin-user-code
        //end-user-code
        
        return;
    }

    /**
     * @generated
     */
    private CF.DataType[] Sequence_configure_0;
}