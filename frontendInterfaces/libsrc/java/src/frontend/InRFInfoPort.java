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

public class InRFInfoPort extends FRONTEND.RFInfoPOA {

    protected String name;
 
    protected Object portAccess = null;

    protected RFInfoDelegate delegate = null;    

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
                    return delegate.fe_getRFFlowId();
                }
            }catch(Exception e){
                System.out.println("InRFInfoPort rf_flow_id() exception " + e.getMessage());
            }
            return null;
        }
    }

    public void rf_flow_id(String data) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null) {
                    delegate.fe_setRFFlowId(data);
                }
            }catch(Exception e){
                System.out.println("InRFInfoPort rf_flow_id(String data) exception " + e.getMessage());
            }
        }
    }

    public RFInfoPkt rfinfo_pkt() {
        synchronized(this.portAccess){
            try{
                if ( delegate != null) {
                    return (delegate.fe_getRFInfoPkt());
                } else { 
                    return null;
                }
            }catch(Exception e){
                System.out.println("InRFInfoPort rfinfo_pkt() exception " + e.getMessage());
            }
            return null;
        }
    }

    public void rfinfo_pkt(RFInfoPkt data) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null) {
                    delegate.fe_setRFInfoPkt(data);
                }
            }catch(Exception e){
                System.out.println("InRFInfoPort rfinfo_pkt(RFInfoPkt data) exception " + e.getMessage());
            }
        }
    }

    public void setDelegate( RFInfoDelegate d ) {
        delegate = d;
    }
}
