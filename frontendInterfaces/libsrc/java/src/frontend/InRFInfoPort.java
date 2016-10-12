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

import frontend.RFInfoDelegate;
import FRONTEND.RFInfoPkt;
import FRONTEND.RFInfoHelper;
import org.ossie.component.PortBase;

public class InRFInfoPort extends FRONTEND.RFInfoPOA implements PortBase {

    protected String name;
 
    protected Object portAccess = null;

    protected RFInfoDelegate delegate = null;    

    public InRFInfoPort( String portName) {
        this(portName, null);
    }

    public InRFInfoPort( String portName,
                         RFInfoDelegate d) {
        this.name = portName;
        this.delegate = d;
        this.portAccess = new Object();
    }

    public String rf_flow_id() {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){ 
                    return delegate.get_rf_flow_id(this.name);
                } else {
                    throw new RuntimeException("InRFInfoPort get_rf_flow_id() callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void rf_flow_id(String data) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null) {
                    delegate.set_rf_flow_id(this.name, data);
                } else {
                    throw new RuntimeException("InRFInfoPort set_rf_flow_id(String data) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public RFInfoPkt rfinfo_pkt() {
        synchronized(this.portAccess){
            try{
                if ( delegate != null) {
                    return (delegate.get_rfinfo_pkt(this.name));
                } else {
                    throw new RuntimeException("InRFInfoPort get_rfinfo_pkt() callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void rfinfo_pkt(RFInfoPkt data) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null) {
                    delegate.set_rfinfo_pkt(this.name, data);
                } else {
                    throw new RuntimeException("InRFInfoPort set_rfinfo_pkt(RFInfoPkt data) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setDelegate( RFInfoDelegate d ) {
        delegate = d;
    }

    public String getRepid() {
        return RFInfoHelper.id();
    }

    public String getDirection() {
        return "Provides";
    }
}
