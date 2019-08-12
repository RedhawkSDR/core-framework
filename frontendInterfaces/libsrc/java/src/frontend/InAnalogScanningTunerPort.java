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
import FRONTEND.AnalogScanningTunerHelper;
import frontend.AnalogScanningTunerDelegate;
import org.ossie.component.PortBase;
import org.ossie.component.RHLogger;
import FRONTEND.ScanningTunerPackage.ScanStatus;
import FRONTEND.ScanningTunerPackage.ScanStrategy;
import BULKIO.PrecisionUTCTime;

public class InAnalogScanningTunerPort extends FRONTEND.AnalogScanningTunerPOA implements PortBase {

    protected String name;

    protected Object portAccess = null;

    protected AnalogScanningTunerDelegate delegate = null;

    public InAnalogScanningTunerPort(String portName) {
        this(portName, null);
    }

    public InAnalogScanningTunerPort(String portName,
                             AnalogScanningTunerDelegate d){
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
                    throw new RuntimeException("InAnalogScanningTunerPort getTunerType(String id) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort getTunerDeviceControl(String id) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort getTunerGroupId(String id) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort getTunerRfFlowId(String id) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort getTunerStatus(String id) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort setTunerCenterFrequency(String id, double freq) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort getTunerCenterFrequency(String id) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort setTunerBandwidth(String id, double bw) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort getTunerBandwidth(String id) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort setTunerAgcEnable(String id, boolean enable) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort getTunerAgcEnable(String id) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort setTunerGain(String id, float gain) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort getTunerGain(String id) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort setTunerReferenceSource(String id, int source) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort getTunerReferenceSource(String id) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort setTunerEnable(String id, boolean enable) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort getTunerEnable(String id) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort getScanStatus(String id) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort setScanStartTime(String id, BULKIO.PrecisionUTCTime start_time) callback delegate not defined");
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
                    throw new RuntimeException("InAnalogScanningTunerPort setScanStrategy(String id, FRONTEND.ScanningTunerPackage.ScanStrategy scan_strategy) callback delegate not defined");
                }
            } catch(org.omg.CORBA.SystemException | org.omg.CORBA.UserException e) {
                throw e;
            } catch(Throwable e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void setDelegate( AnalogScanningTunerDelegate d ) {
        delegate = d;
    }

    public String getRepid() {
        return AnalogScanningTunerHelper.id();
    }

    public String getDirection() {
        return "Provides";
    }
}
