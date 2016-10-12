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
import frontend.GPSDelegate;
public class InGPSPort extends FRONTEND.GPSPOA {

    protected String name;
 
    protected Object portAccess = null;

    protected GPSDelegate delegate = null;    

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
                    return delegate.fe_getGPSInfo();
                }
            }catch(Exception e){
                System.out.println("InGPSPort gps_info() exception " + e.getMessage());
            }
            return null;
        }
    }

    public void gps_info(GPSInfo data) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null) {
                    delegate.fe_setGPSInfo(data);
                }
            }catch(Exception e){
                System.out.println("InGPSPort gps_info(GPSInfo data) exception " + e.getMessage());
            }
        }
    }

    public GpsTimePos gps_time_pos() {
        synchronized(this.portAccess){
            try{
                if ( delegate != null) {
                    return (delegate.fe_getGpsTimePos());
                }
            }catch(Exception e){
                System.out.println("InGPSPort gps_time_pos() exception " + e.getMessage());
            }
            return null;
        }
    }

    public void gps_time_pos(GpsTimePos data) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null) {
                    delegate.fe_setGpsTimePos(data);
                }
            }catch(Exception e){
                System.out.println("InGPSPort gps_time_pos(GpsTimePos data) exception " + e.getMessage());
            }
        }
    }

    public void setDelegate( GPSDelegate d ) {
        delegate = d;
    }
}
