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
import FRONTEND.DigitalTunerHelper;
import frontend.DigitalTunerDelegate;
import org.ossie.component.PortBase;

public class InDigitalTunerPort extends FRONTEND.DigitalTunerPOA implements PortBase {

    protected String name;

    protected Object portAccess = null;

    protected DigitalTunerDelegate delegate = null;

    public InDigitalTunerPort( String portName) {
        this(portName, null);
    }

    public InDigitalTunerPort( String portName,
                                DigitalTunerDelegate d) {
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
                    throw new RuntimeException("InDigitalTunerPort getTunerType(String id) callback delegate not defined");
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
                    throw new RuntimeException("InDigitalTunerPort getTunerDeviceControl(String id) callback delegate not defined");
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
                    throw new RuntimeException("InDigitalTunerPort getTunerGroupId(String id) callback delegate not defined");
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
                    throw new RuntimeException("InDigitalTunerPort getTunerRfFlowId(String id) callback delegate not defined");
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
                    throw new RuntimeException("InDigitalTunerPort getTunerStatus(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setTunerCenterFrequency(String id, double freq) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.setTunerCenterFrequency(id, freq);
                } else {
                    throw new RuntimeException("InDigitalTunerPort setTunerCenterFrequency(String id, double freq) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public double getTunerCenterFrequency(String id) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerCenterFrequency(id);
                } else {
                    throw new RuntimeException("InDigitalTunerPort getTunerCenterFrequency(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setTunerBandwidth(String id, double bw) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.setTunerBandwidth(id, bw);
                } else {
                    throw new RuntimeException("InDigitalTunerPort setTunerBandwidth(String id, double bw) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public double getTunerBandwidth(String id) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerBandwidth(id);
                } else {
                    throw new RuntimeException("InDigitalTunerPort getTunerBandwidth(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setTunerAgcEnable(String id, boolean enable) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.setTunerAgcEnable(id, enable);
                } else {
                    throw new RuntimeException("InDigitalTunerPort setTunerAgcEnable(String id, boolean enable) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public boolean getTunerAgcEnable(String id) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerAgcEnable(id);
                } else {
                    throw new RuntimeException("InDigitalTunerPort getTunerAgcEnable(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setTunerGain(String id, float gain) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.setTunerGain(id, gain);
                } else {
                    throw new RuntimeException("InDigitalTunerPort setTunerGain(String id, float gain) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public float getTunerGain(String id) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerGain(id);
                } else {
                    throw new RuntimeException("InDigitalTunerPort getTunerGain(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setTunerReferenceSource(String id, int source) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.setTunerReferenceSource(id, source);
                } else {
                    throw new RuntimeException("InDigitalTunerPort setTunerReferenceSource(String id, int source) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public int getTunerReferenceSource(String id) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerReferenceSource(id);
                } else {
                    throw new RuntimeException("InDigitalTunerPort getTunerReferenceSource(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setTunerEnable(String id, boolean enable) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.setTunerEnable(id, enable);
                } else {
                    throw new RuntimeException("InDigitalTunerPort setTunerEnable(String id, boolean enable) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public boolean getTunerEnable(String id) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerEnable(id);
                } else {
                    throw new RuntimeException("InDigitalTunerPort getTunerEnable(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setTunerOutputSampleRate(String id, double sr) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.setTunerOutputSampleRate(id, sr);
                } else {
                    throw new RuntimeException("InDigitalTunerPort setTunerOutputSampleRate(String id, double sr) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public double getTunerOutputSampleRate(String id) {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerOutputSampleRate(id);
                } else {
                    throw new RuntimeException("InDigitalTunerPort getTunerOutputSampleRate(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setDelegate( DigitalTunerDelegate d ) {
        delegate = d;
    }

    public String getRepid() {
        return DigitalTunerHelper.id();
    }

    public String getDirection() {
        return "Provides";
    }
}
