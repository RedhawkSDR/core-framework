/*#
 * This file is protected by Copyright. Please refer to the COPYRIGHT file 
 * distributed with this source distribution.
 * 
 * This file is part of REDHAWK core.
 * 
 * REDHAWK core is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version.
 * 
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 #*/
//% extends "pull/resource.java"

/*{% block mainadditionalimports %}*/
/*{% if 'FrontendTuner' in component.implements %}*/
import org.omg.CORBA.ORB;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
import CF.DeviceManager;
import CF.DevicePackage.InvalidCapacity;
import CF.DevicePackage.InvalidCapacity;
import CF.InvalidObjectReference;
/*{% endif %}*/
/*{% endblock %}*/

/*{% block updateUsageState %}*/
/*{%   if component.superclass.name == "ThreadedDevice" %}*/
    ${super()}
/*{%-  endif %}*/
/*{% endblock %}*/

/*{% block extensions %}*/
/*{% if 'FrontendTuner' in component.implements %}*/

    /*************************************************************
    Functions supporting tuning allocation
    *************************************************************/
    public void deviceEnable(frontend_tuner_status_struct_struct fts, int tuner_id)
    {
        /************************************************************
        modify fts, which corresponds to this.frontend_tuner_status.getValue().get(tuner_id)
        Make sure to set the 'enabled' member of fts to indicate that tuner as enabled
        ************************************************************/
        System.out.println("deviceEnable(): Enable the given tuner  *********");
        fts.enabled.setValue(true);
        return;
    }
    public void deviceDisable(frontend_tuner_status_struct_struct fts, int tuner_id)
    {
        /************************************************************
        modify fts, which corresponds to this.frontend_tuner_status.getValue().get(tuner_id)
        Make sure to reset the 'enabled' member of fts to indicate that tuner as disabled
        ************************************************************/
        System.out.println("deviceDisable(): Disable the given tuner  *********");
        fts.enabled.setValue(false);
        return;
    }
/*{% if 'ScanningTuner' in component.implements %}*/
    public boolean deviceSetTuningScan(final frontend.FETypes.frontend_tuner_allocation_struct request, final frontend.FETypes.frontend_scanner_allocation_struct scan_request, frontend_tuner_status_struct_struct fts, int tuner_id)
    {
        /************************************************************

        This function is called when the allocation request contains a scanner allocation

        modify fts, which corresponds to this.frontend_tuner_status.getValue().get(tuner_id)
        
        The bandwidth, center frequency, and sampling rate that the hardware was actually tuned
        to needs to populate fts (to make sure that it meets the tolerance requirement. For example,
        if the tuned values match the requested values, the code would look like this:
        
        fts.bandwidth.setValue(request.bandwidth.getValue());
        fts.center_frequency.setValue(request.center_frequency.getValue());
        fts.sample_rate.setValue(request.sample_rate.getValue());
        
        return true if the tuning succeeded, and false if it failed
        ************************************************************/
        System.out.println("deviceSetTuning(): Evaluate whether or not a tuner is added  *********");
        return true;
    }
/*{% endif %}*/
    public boolean deviceSetTuning(final frontend.FETypes.frontend_tuner_allocation_struct request, frontend_tuner_status_struct_struct fts, int tuner_id)
    {
        /************************************************************
/*{% if 'ScanningTuner' in component.implements %}*/

        This function is called when the allocation request does not contain a scanner allocation

/*{% endif %}*/
        modify fts, which corresponds to this.frontend_tuner_status.getValue().get(tuner_id)
        
        The bandwidth, center frequency, and sampling rate that the hardware was actually tuned
        to needs to populate fts (to make sure that it meets the tolerance requirement. For example,
        if the tuned values match the requested values, the code would look like this:
        
        fts.bandwidth.setValue(request.bandwidth.getValue());
        fts.center_frequency.setValue(request.center_frequency.getValue());
        fts.sample_rate.setValue(request.sample_rate.getValue());
        
        return true if the tuning succeeded, and false if it failed
        ************************************************************/
        System.out.println("deviceSetTuning(): Evaluate whether or not a tuner is added  *********");
        return true;
    }
    public boolean deviceDeleteTuning(frontend_tuner_status_struct_struct fts, int tuner_id)
    {
        /************************************************************
        modify fts, which corresponds to this.frontend_tuner_status.getValue().get(tuner_id)
        return true if the tune deletion succeeded, and false if it failed
        ************************************************************/
        System.out.println("deviceDeleteTuning(): Deallocate an allocated tuner  *********");
        return true;
    }
/*{% endif %}*/
/*{% endblock %}*/

/*{% block fei_port_delegations %}*/
/*{% if 'FrontendTuner' in component.implements %}*/
    /*************************************************************
    Functions servicing the tuner control port
    *************************************************************/
    public String getTunerType(final String allocation_id) throws FRONTEND.FrontendException
    {
        int idx = getTunerMapping(allocation_id);
        if (idx < 0) throw new FRONTEND.FrontendException("Invalid allocation id");
        return frontend_tuner_status.getValue().get(idx).tuner_type.getValue();
    }

    public boolean getTunerDeviceControl(final String allocation_id) throws FRONTEND.FrontendException
    {
        int idx = getTunerMapping(allocation_id);
        if (idx < 0) throw new FRONTEND.FrontendException("Invalid allocation id");
        if (getControlAllocationId(idx) == allocation_id)
            return true;
        return false;
    }

    public String getTunerGroupId(final String allocation_id) throws FRONTEND.FrontendException
    {
        int idx = getTunerMapping(allocation_id);
        if (idx < 0) throw new FRONTEND.FrontendException("Invalid allocation id");
        return frontend_tuner_status.getValue().get(idx).group_id.getValue();
    }

    public String getTunerRfFlowId(final String allocation_id) throws FRONTEND.FrontendException
    {
        int idx = getTunerMapping(allocation_id);
        if (idx < 0) throw new FRONTEND.FrontendException("Invalid allocation id");
        return frontend_tuner_status.getValue().get(idx).rf_flow_id.getValue();
    }
/*{% endif %}*/
/*{% if 'AnalogTuner' in component.implements %}*/

    public void setTunerCenterFrequency(final String allocation_id, double freq) throws FRONTEND.FrontendException, FRONTEND.BadParameterException
    {
        int idx = getTunerMapping(allocation_id);
        if (idx < 0) throw new FRONTEND.FrontendException("Invalid allocation id");
        if (!allocation_id.equals(getControlAllocationId(idx)))
            throw new FRONTEND.FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner"));
        if (freq<0) throw new FRONTEND.BadParameterException("Center frequency cannot be less than 0");
        // set hardware to new value. Raise an exception if it's not possible
        this.frontend_tuner_status.getValue().get(idx).center_frequency.setValue(freq);
    }

    public double getTunerCenterFrequency(final String allocation_id) throws FRONTEND.FrontendException
    {
        int idx = getTunerMapping(allocation_id);
        if (idx < 0) throw new FRONTEND.FrontendException("Invalid allocation id");
        return frontend_tuner_status.getValue().get(idx).center_frequency.getValue();
    }

    public void setTunerBandwidth(final String allocation_id, double bw) throws FRONTEND.FrontendException, FRONTEND.BadParameterException
    {
        int idx = getTunerMapping(allocation_id);
        if (idx < 0) throw new FRONTEND.FrontendException("Invalid allocation id");
        if (!allocation_id.equals(getControlAllocationId(idx)))
            throw new FRONTEND.FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner"));
        if (bw<0) throw new FRONTEND.BadParameterException("Bandwidth cannot be less than 0");
        // set hardware to new value. Raise an exception if it's not possible
        this.frontend_tuner_status.getValue().get(idx).bandwidth.setValue(bw);
    }

    public double getTunerBandwidth(final String allocation_id) throws FRONTEND.FrontendException
    {
        int idx = getTunerMapping(allocation_id);
        if (idx < 0) throw new FRONTEND.FrontendException("Invalid allocation id");
        return frontend_tuner_status.getValue().get(idx).bandwidth.getValue();
    }

    public void setTunerAgcEnable(final String allocation_id, boolean enable) throws FRONTEND.NotSupportedException
    {
        throw new FRONTEND.NotSupportedException("setTunerAgcEnable not supported");
    }

    public boolean getTunerAgcEnable(final String allocation_id) throws FRONTEND.NotSupportedException
    {
        throw new FRONTEND.NotSupportedException("getTunerAgcEnable not supported");
    }

    public void setTunerGain(final String allocation_id, float gain) throws FRONTEND.NotSupportedException
    {
        throw new FRONTEND.NotSupportedException("setTunerGain not supported");
    }

    public float getTunerGain(final String allocation_id) throws FRONTEND.NotSupportedException
    {
        throw new FRONTEND.NotSupportedException("getTunerGain not supported");
    }

    public void setTunerReferenceSource(final String allocation_id, int source) throws FRONTEND.NotSupportedException
    {
        throw new FRONTEND.NotSupportedException("setTunerReferenceSource not supported");
    }

    public int getTunerReferenceSource(final String allocation_id) throws FRONTEND.NotSupportedException
    {
        throw new FRONTEND.NotSupportedException("getTunerReferenceSource not supported");
    }

    public void setTunerEnable(final String allocation_id, boolean enable) throws FRONTEND.FrontendException
    {
        int idx = getTunerMapping(allocation_id);
        if (idx < 0) throw new FRONTEND.FrontendException("Invalid allocation id");
        if (!allocation_id.equals(getControlAllocationId(idx)))
            throw new FRONTEND.FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner"));
        // set hardware to new value. Raise an exception if it's not possible
        this.frontend_tuner_status.getValue().get(idx).enabled.setValue(enable);
    }

    public boolean getTunerEnable(final String allocation_id) throws FRONTEND.FrontendException
    {
        int idx = getTunerMapping(allocation_id);
        if (idx < 0) throw new FRONTEND.FrontendException("Invalid allocation id");
        return frontend_tuner_status.getValue().get(idx).enabled.getValue();
    }
/*{% endif %}*/
/*{% if 'DigitalTuner' in component.implements %}*/

    public void setTunerOutputSampleRate(final String allocation_id, double sr) throws FRONTEND.FrontendException, FRONTEND.BadParameterException
    {
        int idx = getTunerMapping(allocation_id);
        if (idx < 0) throw new FRONTEND.FrontendException("Invalid allocation id");
        if (!allocation_id.equals(getControlAllocationId(idx)))
            throw new FRONTEND.FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner"));
        if (sr<0) throw new FRONTEND.BadParameterException("Sample rate cannot be less than 0");
        // set hardware to new value. Raise an exception if it's not possible
        this.frontend_tuner_status.getValue().get(idx).sample_rate.setValue(sr);
    }

    public double getTunerOutputSampleRate(final String allocation_id) throws FRONTEND.FrontendException
    {
        int idx = getTunerMapping(allocation_id);
        if (idx < 0) throw new FRONTEND.FrontendException("Invalid allocation id");
        return frontend_tuner_status.getValue().get(idx).sample_rate.getValue();
    }
/*{% endif %}*/
/*{% if 'ScanningTuner' in component.implements %}*/

    public FRONTEND.ScanningTunerPackage.ScanStatus getScanStatus(String allocation_id) throws FRONTEND.FrontendException, FRONTEND.BadParameterException
    {
        int idx = getTunerMapping(allocation_id);
        if (idx < 0) throw new FRONTEND.FrontendException("Invalid allocation id");
        FRONTEND.ScanningTunerPackage.ScanStatus status = null;
        return status;
    }

    public void setScanStartTime(String allocation_id, BULKIO.PrecisionUTCTime start_time) throws FRONTEND.FrontendException, FRONTEND.BadParameterException
    {
        int idx = getTunerMapping(allocation_id);
        if (idx < 0) throw new FRONTEND.FrontendException("Invalid allocation id");
        if (!allocation_id.equals(getControlAllocationId(idx)))
            throw new FRONTEND.FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner"));
    }

    public void setScanStrategy(String allocation_id, FRONTEND.ScanningTunerPackage.ScanStrategy scan_strategy) throws FRONTEND.FrontendException, FRONTEND.BadParameterException
    {
        int idx = getTunerMapping(allocation_id);
        if (idx < 0) throw new FRONTEND.FrontendException("Invalid allocation id");
        if (!allocation_id.equals(getControlAllocationId(idx)))
            throw new FRONTEND.FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner"));
    }

/*{% endif %}*/
/*{% if 'GPS' in component.implements %}*/

    public FRONTEND.GPSInfo get_gps_info(final String port_name)
    {
        FRONTEND.GPSInfo gps_info = null;
        return gps_info;
    }

    public void set_gps_info(final String port_name, final FRONTEND.GPSInfo gps_info)
    {
    }

    public FRONTEND.GpsTimePos get_gps_time_pos(final String port_name)
    {
        FRONTEND.GpsTimePos gps_time_pos = null;
        return gps_time_pos;
    }

    public void set_gps_time_pos(final String port_name, final FRONTEND.GpsTimePos gps_time_pos)
    {
    }
/*{% endif %}*/
/*{% if 'NavData' in component.implements %}*/

    public FRONTEND.NavigationPacket get_nav_packet(final String port_name)
    {
        FRONTEND.NavigationPacket nav_info = null;
        return nav_info;
    }

    public void set_nav_packet(final String port_name, final FRONTEND.NavigationPacket nav_info)
    {
    }
/*{% endif %}*/
/*{% if 'RFInfo' in component.implements %}*/

    /*************************************************************
    Functions servicing the RFInfo port(s)
    - port_name is the port over which the call was received
    *************************************************************/
    public String get_rf_flow_id(final String port_name)
    {
        return new String("none");
    }

    public void set_rf_flow_id(final String port_name, final String id)
    {
    }

    public FRONTEND.RFInfoPkt get_rfinfo_pkt(final String port_name)
    {
        FRONTEND.RFInfoPkt pkt = null;
        return pkt;
    }

    public void set_rfinfo_pkt(final String port_name, final FRONTEND.RFInfoPkt pkt)
    {
    }
/*{% endif %}*/
/*{% if 'RFSource' in component.implements %}*/

    public FRONTEND.RFInfoPkt[] get_available_rf_inputs(final String port_name)
    {
        FRONTEND.RFInfoPkt[] inputs = null;
        return inputs;
    }

    public void set_available_rf_inputs(final String port_name, final FRONTEND.RFInfoPkt[] inputs)
    {
    }

    public FRONTEND.RFInfoPkt get_current_rf_input(final String port_name)
    {
        FRONTEND.RFInfoPkt pkt = null;
        return pkt;
    }

    public void set_current_rf_input(final String port_name, final FRONTEND.RFInfoPkt pkt)
    {
    }
/*{% endif %}*/
/*{% endblock %}*/
