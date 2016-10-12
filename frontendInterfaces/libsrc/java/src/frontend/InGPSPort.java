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

import FRONTEND.GpsTimePos;
import FRONTEND.GPSInfo;
import FRONTEND.GPSHelper;
import frontend.GPSDelegate;
import org.ossie.component.PortBase;

public class InGPSPort extends FRONTEND.GPSPOA implements PortBase {

    protected String name;
 
    protected Object portAccess = null;

    protected GPSDelegate delegate = null;    

    public InGPSPort( String portName) {
        this(portName, null);
    }

    public InGPSPort( String portName,
                         GPSDelegate d) {
        this.name = portName;
        this.delegate = d;
        this.portAccess = new Object();
    }

    public GPSInfo gps_info() {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ) {
                    return delegate.get_gps_info(this.name);
                } else {
                    throw new RuntimeException("InGPSPort get_gps_info() callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void gps_info(GPSInfo data) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null) {
                    delegate.set_gps_info(this.name, data);
                } else {
                    throw new RuntimeException("InGPSPort set_gps_info(GPSInfo data) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public GpsTimePos gps_time_pos() {
        synchronized(this.portAccess){
            try{
                if ( delegate != null) {
                    return (delegate.get_gps_time_pos(this.name));
                } else {
                    throw new RuntimeException("InGPSPort get_gps_time_pos() callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void gps_time_pos(GpsTimePos data) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null) {
                    delegate.set_gps_time_pos(this.name, data);
                } else {
                    throw new RuntimeException("InGPSPort set_gps_time_pos(GpsTimePos data) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setDelegate( GPSDelegate d ) {
        delegate = d;
    }

    public String getRepid() {
        return GPSHelper.id();
    }

    public String getDirection() {
        return "Provides";
    }
}
