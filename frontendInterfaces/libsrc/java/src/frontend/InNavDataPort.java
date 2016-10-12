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

import FRONTEND.NavigationPacket;
import frontend.NavDataDelegate;

// ----------------------------------------------------------------------------------------
// InNavDataPort definition
// ----------------------------------------------------------------------------------------
public class InNavDataPort extends FRONTEND.NavDataPOA{

    protected String name;

    protected Object portAccess = null;

    protected NavDataDelegate delegate = null;

    public InNavDataPort( String portName,
                          NavDataDelegate d){
        this.name = portName;
        this.delegate = d;
        this.portAccess = new Object();
    }

    public NavigationPacket nav_packet() {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ) {
                    return delegate.fe_getNavPkt();
                }
            }catch(Exception e){
                System.out.println("InNavDataPort nav_packet() exception " + e.getMessage());
            }
            return null;
        }
    }

    public void nav_packet(NavigationPacket data){
        synchronized(this.portAccess){
            try{
                if ( delegate != null) {
                    delegate.fe_setNavPkt(data);
                }
            }catch(Exception e){
                System.out.println("InNavDataPort nav_packet(NavigationPacket data) exception " + e.getMessage());
            }
        }
    }

    public void setDelegate( NavDataDelegate d ) {
        delegate = d;
    }
}
