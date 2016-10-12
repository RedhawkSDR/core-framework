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

import FRONTEND.RFInfoPkt;
import frontend.RFSourceDelegate;

public class InRFSourcePort extends FRONTEND.RFSourcePOA {

    protected String name;
 
    protected Object portAccess = null;

    protected RFSourceDelegate delegate = null;    

    public InRFSourcePort( String portName,
                         RFSourceDelegate d) {
        this.name = portName;
        this.delegate = d;
        this.portAccess = new Object();
    }

    public RFInfoPkt[] available_rf_inputs() {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){ 
                    return delegate.fe_getAvailableRFInputs();
                }
            }catch(Exception e){
                System.out.println("InRFSourcePort available_rf_inputs() exception " + e.getMessage());
            }
            return null;
        }
    }

    public void available_rf_inputs(RFInfoPkt[] data) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null){ 
                    delegate.fe_setAvailableRFInputs(data);
                }
            }catch(Exception e){
                System.out.println("InRFSourcePort available_rf_inputs(RFInfoPkt[] data) exception " + e.getMessage());
            }
        }
    }

    public RFInfoPkt current_rf_input() {
        synchronized(this.portAccess){
            try{
                if ( delegate != null){
                    return (delegate.fe_getCurrentRFInput());
                }
            }catch(Exception e){
                System.out.println("InRFSourcePort current_rf_input() exception " + e.getMessage());
            }
            return null;
        }
    }

    public void current_rf_input(RFInfoPkt data) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null) {
                    delegate.fe_setCurrentRFInput(data);
                }
            }catch(Exception e){
                System.out.println("InRFSourcePort current_rf_input(RFInfoPkt data) exception " + e.getMessage());
            }
        }
    }

    public void setDelegate( RFSourceDelegate d ) {
        delegate = d;
    }
}
