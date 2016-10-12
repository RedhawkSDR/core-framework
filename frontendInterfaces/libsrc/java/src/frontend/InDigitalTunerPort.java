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
import frontend.DigitalTunerDelegate;

public class InDigitalTunerPort extends FRONTEND.DigitalTunerPOA {

    protected String name;

    protected Object portAccess = null;

    protected DigitalTunerDelegate delegate = null;

    public InDigitalTunerPort( String portName,
                                DigitalTunerDelegate d) {
        this.name = portName;
        this.delegate = d;
        this.portAccess = new Object();
    }

    public String getTunerType(String id) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.fe_getTunerType(id);
                } else {
                    throw new FrontendException("InDigitalTunerPort getTunerType(String id) callback delegate not defined");
                }
            }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public boolean getTunerDeviceControl(String id) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.fe_getTunerDeviceControl(id);
                } else {
                    throw new FrontendException("InDigitalTunerPort getTunerDeviceControl(String id) callback delegate not defined");
                }
            }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public String getTunerGroupId(String id) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.fe_getTunerGroupId(id);
                } else {
                    throw new FrontendException("InDigitalTunerPort getTunerGroupId(String id) callback delegate not defined");
                }
            }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public String getTunerRfFlowId(String id) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.fe_getTunerRfFlowId(id);
                } else {
                    throw new FrontendException("InDigitalTunerPort getTunerRfFlowId(String id) callback delegate not defined");
                }
            }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public CF.DataType[] getTunerStatus(String id) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.fe_getTunerStatus(id);
                } else {
                    throw new FrontendException("InDigitalTunerPort getTunerStatus(String id) callback delegate not defined");
                }
            }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public void setTunerCenterFrequency(String id, double freq) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.fe_setTunerCenterFrequency(id, freq);
                } else {
                    throw new FrontendException("InDigitalTunerPort setTunerCenterFrequency(String id, double freq) callback delegate not defined");
                }
            }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public double getTunerCenterFrequency(String id) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.fe_getTunerCenterFrequency(id);
                } else {
                    throw new FrontendException("InDigitalTunerPort getTunerCenterFrequency(String id) callback delegate not defined");
                }
            }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public void setTunerBandwidth(String id, double bw) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.fe_setTunerBandwidth(id, bw);
                } else {
                    throw new FrontendException("InDigitalTunerPort setTunerBandwidth(String id, double bw) callback delegate not defined");
                }
            }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public double getTunerBandwidth(String id) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.fe_getTunerBandwidth(id);
                } else {
                    throw new FrontendException("InDigitalTunerPort getTunerBandwidth(String id) callback delegate not defined");
                }
            }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public void setTunerAgcEnable(String id, boolean enable) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.fe_setTunerAgcEnable(id, enable);
                } else {
                    throw new FrontendException("InDigitalTunerPort setTunerAgcEnable(String id, boolean enable) callback delegate not defined");
                }
           }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public boolean getTunerAgcEnable(String id) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.fe_getTunerAgcEnable(id);
                } else {
                    throw new FrontendException("InDigitalTunerPort getTunerAgcEnable(String id) callback delegate not defined");
                }
           }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public void setTunerGain(String id, float gain) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.fe_setTunerGain(id, gain);
                } else {
                    throw new FrontendException("InDigitalTunerPort setTunerGain(String id, float gain) callback delegate not defined");
                }
           }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public float getTunerGain(String id) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.fe_getTunerGain(id);
                } else {
                    throw new FrontendException("InDigitalTunerPort getTunerGain(String id) callback delegate not defined");
                }
           }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public void setTunerReferenceSource(String id, int source) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.fe_setTunerReferenceSource(id, source);
                } else {
                    throw new FrontendException("InDigitalTunerPort setTunerReferenceSource(String id, int source) callback delegate not defined");
                }
           }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public int getTunerReferenceSource(String id) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.fe_getTunerReferenceSource(id);
                } else {
                    throw new FrontendException("InDigitalTunerPort getTunerReferenceSource(String id) callback delegate not defined");
                }
           }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public void setTunerEnable(String id, boolean enable) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.fe_setTunerEnable(id, enable);
                } else {
                    throw new FrontendException("InDigitalTunerPort setTunerEnable(String id, boolean enable) callback delegate not defined");
                }
           }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public boolean getTunerEnable(String id) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.fe_getTunerEnable(id);
                } else {
                    throw new FrontendException("InDigitalTunerPort getTunerEnable(String id) callback delegate not defined");
                }
           }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public void setTunerOutputSampleRate(String id, double sr) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.fe_setTunerOutputSampleRate(id, sr);
                } else {
                    throw new FrontendException("InDigitalTunerPort setTunerOutputSampleRate(String id, double sr) callback delegate not defined");
                }
           }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public double getTunerOutputSampleRate(String id) throws BadParameterException, FrontendException, NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.fe_getTunerOutputSampleRate(id);
                } else {
                    throw new FrontendException("InDigitalTunerPort getTunerOutputSampleRate(String id) callback delegate not defined");
                }
           }catch(BadParameterException e){
                throw e;
            }catch(FrontendException e){
                throw e;
            }catch(NotSupportedException e){
                throw e;
            }
        }
    }

    public void setDelegate( DigitalTunerDelegate d ) {
        delegate = d;
    }
}
