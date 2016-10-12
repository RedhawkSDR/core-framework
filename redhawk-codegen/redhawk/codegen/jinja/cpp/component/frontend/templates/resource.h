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
//% extends "pull/resource.h"

/*{% block updateUsageState %}*/
/*{%   if not 'FrontendTuner' in component.implements %}*/
${super()}
/*{%-  endif %}*/
/*{% endblock %}*/

/*{% block extensions %}*/

    protected:
/*{% if 'FrontendTuner' in component.implements %}*/
        std::string getTunerType(const std::string& allocation_id);
        bool getTunerDeviceControl(const std::string& allocation_id);
        std::string getTunerGroupId(const std::string& allocation_id);
        std::string getTunerRfFlowId(const std::string& allocation_id);
/*{% endif %}*/
/*{% if 'AnalogTuner' in component.implements %}*/
        double getTunerCenterFrequency(const std::string& allocation_id);
        void setTunerCenterFrequency(const std::string& allocation_id, double freq);
        double getTunerBandwidth(const std::string& allocation_id);
        void setTunerBandwidth(const std::string& allocation_id, double bw);
        bool getTunerAgcEnable(const std::string& allocation_id);
        void setTunerAgcEnable(const std::string& allocation_id, bool enable);
        float getTunerGain(const std::string& allocation_id);
        void setTunerGain(const std::string& allocation_id, float gain);
        long getTunerReferenceSource(const std::string& allocation_id);
        void setTunerReferenceSource(const std::string& allocation_id, long source);
        bool getTunerEnable(const std::string& allocation_id);
        void setTunerEnable(const std::string& allocation_id, bool enable);
/*{% endif %}*/
/*{% if 'DigitalTuner' in component.implements %}*/
        double getTunerOutputSampleRate(const std::string& allocation_id);
        void setTunerOutputSampleRate(const std::string& allocation_id, double sr);
/*{% endif %}*/
/*{% if 'GPS' in component.implements %}*/
        frontend::GPSInfo get_gps_info(const std::string& port_name);
        void set_gps_info(const std::string& port_name, const frontend::GPSInfo &gps_info);
        frontend::GpsTimePos get_gps_time_pos(const std::string& port_name);
        void set_gps_time_pos(const std::string& port_name, const frontend::GpsTimePos &gps_time_pos);
/*{% endif %}*/
/*{% if 'NavData' in component.implements %}*/
        frontend::NavigationPacket get_nav_packet(const std::string& port_name);
        void set_nav_packet(const std::string& port_name, const frontend::NavigationPacket &nav_info);
/*{% endif %}*/
/*{% if 'RFInfo' in component.implements %}*/
        std::string get_rf_flow_id(const std::string& port_name);
        void set_rf_flow_id(const std::string& port_name, const std::string& id);
        frontend::RFInfoPkt get_rfinfo_pkt(const std::string& port_name);
        void set_rfinfo_pkt(const std::string& port_name, const frontend::RFInfoPkt& pkt);
/*{% endif %}*/
/*{% if 'RFSource' in component.implements %}*/
        std::vector<frontend::RFInfoPkt> get_available_rf_inputs(const std::string& port_name);
        void set_available_rf_inputs(const std::string& port_name, const std::vector<frontend::RFInfoPkt> &inputs);
        frontend::RFInfoPkt get_current_rf_input(const std::string& port_name);
        void set_current_rf_input(const std::string& port_name, const frontend::RFInfoPkt &pkt);
/*{% endif %}*/

    private:
/*{% if 'FrontendTuner' in component.implements %}*/
        ////////////////////////////////////////
        // Required device specific functions // -- to be implemented by device developer
        ////////////////////////////////////////

        // these are pure virtual, must be implemented here
        void deviceEnable(frontend_tuner_status_struct_struct &fts, size_t tuner_id);
        void deviceDisable(frontend_tuner_status_struct_struct &fts, size_t tuner_id);
        bool deviceSetTuning(const frontend::frontend_tuner_allocation_struct &request, frontend_tuner_status_struct_struct &fts, size_t tuner_id);
        bool deviceDeleteTuning(frontend_tuner_status_struct_struct &fts, size_t tuner_id);

/*{% endif %}*/
        void construct();
/*{% endblock %}*/

