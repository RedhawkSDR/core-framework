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

import FRONTEND.FrontendException;
import FRONTEND.BadParameterException;
import FRONTEND.NotSupportedException;
import FRONTEND.FrontendTunerHelper;
import frontend.FrontendTunerDelegate;
import org.ossie.component.PortBase;

public class InFrontendTunerPort extends FRONTEND.FrontendTunerPOA implements PortBase {

    protected String name;

    protected Object portAccess = null;

    protected FrontendTunerDelegate delegate = null;

    public InFrontendTunerPort( String portName) {
        this(portName, null);
    }

    public InFrontendTunerPort( String portName,
                                FrontendTunerDelegate d) {
        this.name = portName;
        this.delegate = d;
        this.portAccess = new Object();
    }


    public String getTunerType(String id) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerType(id);
                } else {
                    throw new RuntimeException("InFrontendTunerPort getTunerType(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public boolean getTunerDeviceControl(String id) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerDeviceControl(id);
                } else {
                    throw new RuntimeException("InFrontendTunerPort getTunerDeviceControl(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public String getTunerGroupId(String id) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerGroupId(id);
                } else {
                    throw new RuntimeException("InFrontendTunerPort getTunerGroupId(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public String getTunerRfFlowId(String id) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerRfFlowId(id);
                } else {
                    throw new RuntimeException("InFrontendTunerPort getTunerRfFlowId(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public CF.DataType[] getTunerStatus(String id) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerStatus(id);
                } else {
                    throw new RuntimeException("InFrontendTunerPort getTunerStatus(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setDelegate( FrontendTunerDelegate d ) {
        delegate = d;
    }

    public String getRepid() {
        return FrontendTunerHelper.id();
    }

    public String getDirection() {
        return "Provides";
    }
}
