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
import FRONTEND.NavDataHelper;
import frontend.NavDataDelegate;
import org.ossie.component.PortBase;

// ----------------------------------------------------------------------------------------
// InNavDataPort definition
// ----------------------------------------------------------------------------------------
public class InNavDataPort extends FRONTEND.NavDataPOA implements PortBase {

    protected String name;

    protected Object portAccess = null;

    protected NavDataDelegate delegate = null;

    public InNavDataPort( String portName) {
        this(portName, null);
    }

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
                    return delegate.get_nav_packet(this.name);
                } else {
                    throw new RuntimeException("InNavDataPort get_nav_packet() callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void nav_packet(NavigationPacket data){
        synchronized(this.portAccess){
            try{
                if ( delegate != null) {
                    delegate.set_nav_packet(this.name, data);
                } else {
                    throw new RuntimeException("InNavDataPort set_nav_packet(NavigationPacket data) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setDelegate( NavDataDelegate d ) {
        delegate = d;
    }

    public String getRepid() {
        return NavDataHelper.id();
    }

    public String getDirection() {
        return "Provides";
    }
}
