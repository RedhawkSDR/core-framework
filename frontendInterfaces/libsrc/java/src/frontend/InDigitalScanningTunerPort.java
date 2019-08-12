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
import FRONTEND.DigitalScanningTunerHelper;
import frontend.DigitalScanningTunerDelegate;
import FRONTEND.ScanningTunerPackage.ScanStatus;
import FRONTEND.ScanningTunerPackage.ScanStrategy;
import BULKIO.PrecisionUTCTime;
import org.ossie.component.PortBase;
import org.ossie.component.RHLogger;

public class InDigitalScanningTunerPort extends FRONTEND.DigitalScanningTunerPOA implements PortBase {

    protected String name;

    protected Object portAccess = null;

    protected DigitalScanningTunerDelegate delegate = null;

    public InDigitalScanningTunerPort( String portName) {
        this(portName, null);
    }

    public InDigitalScanningTunerPort( String portName,
                                DigitalScanningTunerDelegate d) {
        this.name = portName;
        this.delegate = d;
        this.portAccess = new Object();
    }

    public RHLogger _portLog = null;
    public void setLogger(RHLogger logger)
    {
        this._portLog = logger;
    }

    public String getTunerType(String id) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerType(id);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort getTunerType(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public boolean getTunerDeviceControl(String id) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerDeviceControl(id);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort getTunerDeviceControl(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public String getTunerGroupId(String id) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerGroupId(id);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort getTunerGroupId(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public String getTunerRfFlowId(String id) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerRfFlowId(id);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort getTunerRfFlowId(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public CF.DataType[] getTunerStatus(String id) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerStatus(id);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort getTunerStatus(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setTunerCenterFrequency(String id, double freq) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.setTunerCenterFrequency(id, freq);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort setTunerCenterFrequency(String id, double freq) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public double getTunerCenterFrequency(String id) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerCenterFrequency(id);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort getTunerCenterFrequency(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setTunerBandwidth(String id, double bw) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.setTunerBandwidth(id, bw);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort setTunerBandwidth(String id, double bw) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public double getTunerBandwidth(String id) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerBandwidth(id);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort getTunerBandwidth(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setTunerAgcEnable(String id, boolean enable) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.setTunerAgcEnable(id, enable);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort setTunerAgcEnable(String id, boolean enable) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public boolean getTunerAgcEnable(String id) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerAgcEnable(id);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort getTunerAgcEnable(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setTunerGain(String id, float gain) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.setTunerGain(id, gain);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort setTunerGain(String id, float gain) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public float getTunerGain(String id) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerGain(id);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort getTunerGain(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setTunerReferenceSource(String id, int source) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.setTunerReferenceSource(id, source);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort setTunerReferenceSource(String id, int source) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public int getTunerReferenceSource(String id) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerReferenceSource(id);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort getTunerReferenceSource(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setTunerEnable(String id, boolean enable) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.setTunerEnable(id, enable);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort setTunerEnable(String id, boolean enable) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public boolean getTunerEnable(String id) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerEnable(id);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort getTunerEnable(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setTunerOutputSampleRate(String id, double sr) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.setTunerOutputSampleRate(id, sr);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort setTunerOutputSampleRate(String id, double sr) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public double getTunerOutputSampleRate(String id) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getTunerOutputSampleRate(id);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort getTunerOutputSampleRate(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public FRONTEND.ScanningTunerPackage.ScanStatus getScanStatus(String id) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    return delegate.getScanStatus(id);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort getScanStatus(String id) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setScanStartTime(String id, BULKIO.PrecisionUTCTime start_time) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.setScanStartTime(id, start_time);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort setScanStartTime(String id, BULKIO.PrecisionUTCTime start_time) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setScanStrategy(String id, FRONTEND.ScanningTunerPackage.ScanStrategy scan_strategy) throws FRONTEND.FrontendException, FRONTEND.BadParameterException, FRONTEND.NotSupportedException {
        synchronized(this.portAccess){
            try{
                if ( delegate != null ){
                    delegate.setScanStrategy(id, scan_strategy);
                } else {
                    throw new RuntimeException("InDigitalScanningTunerPort setScanStrategy(String id, FRONTEND.ScanningTunerPackage.ScanStrategy scan_strategy) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setDelegate( DigitalScanningTunerDelegate d ) {
        delegate = d;
    }

    public String getRepid() {
        return DigitalScanningTunerHelper.id();
    }

    public String getDirection() {
        return "Provides";
    }
}
