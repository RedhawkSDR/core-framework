#ifndef FEI_EXCEPTION_THROUGH_I_IMPL_H
#define FEI_EXCEPTION_THROUGH_I_IMPL_H

#include "fei_exception_through_base.h"

class fei_exception_through_i : public fei_exception_through_base
{
    ENABLE_LOGGING
    public:
        fei_exception_through_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        fei_exception_through_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        fei_exception_through_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        fei_exception_through_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~fei_exception_through_i();

        void constructor();

        int serviceFunction();
        void busyStatusChanged(bool oldValue, bool newValue);

    protected:
        std::string getTunerType(const std::string& allocation_id);
        bool getTunerDeviceControl(const std::string& allocation_id);
        std::string getTunerGroupId(const std::string& allocation_id);
        std::string getTunerRfFlowId(const std::string& allocation_id);
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
        double getTunerOutputSampleRate(const std::string& allocation_id);
        void setTunerOutputSampleRate(const std::string& allocation_id, double sr);
        frontend::ScanStatus getScanStatus(const std::string& allocation_id);
        void setScanStartTime(const std::string& allocation_id, const BULKIO::PrecisionUTCTime& start_time);
        void setScanStrategy(const std::string& allocation_id, const frontend::ScanStrategy* scan_strategy);
        frontend::GPSInfo get_gps_info(const std::string& port_name);
        void set_gps_info(const std::string& port_name, const frontend::GPSInfo &gps_info);
        frontend::GpsTimePos get_gps_time_pos(const std::string& port_name);
        void set_gps_time_pos(const std::string& port_name, const frontend::GpsTimePos &gps_time_pos);
        frontend::NavigationPacket get_nav_packet(const std::string& port_name);
        void set_nav_packet(const std::string& port_name, const frontend::NavigationPacket &nav_info);
        std::string get_rf_flow_id(const std::string& port_name);
        void set_rf_flow_id(const std::string& port_name, const std::string& id);
        frontend::RFInfoPkt get_rfinfo_pkt(const std::string& port_name);
        void set_rfinfo_pkt(const std::string& port_name, const frontend::RFInfoPkt& pkt);
        std::vector<frontend::RFInfoPkt> get_available_rf_inputs(const std::string& port_name);
        void set_available_rf_inputs(const std::string& port_name, const std::vector<frontend::RFInfoPkt> &inputs);
        frontend::RFInfoPkt get_current_rf_input(const std::string& port_name);
        void set_current_rf_input(const std::string& port_name, const frontend::RFInfoPkt &pkt);
    private:
        ////////////////////////////////////////
        // Required device specific functions // -- to be implemented by device developer
        ////////////////////////////////////////

        // these are pure virtual, must be implemented here
        void deviceEnable(frontend_tuner_status_struct_struct &fts, size_t tuner_id);
        void deviceDisable(frontend_tuner_status_struct_struct &fts, size_t tuner_id);
        bool deviceSetTuningScan(const frontend::frontend_tuner_allocation_struct &request, const frontend::frontend_scanner_allocation_struct &scan_request, frontend_tuner_status_struct_struct &fts, size_t tuner_id);
        bool deviceSetTuning(const frontend::frontend_tuner_allocation_struct &request, frontend_tuner_status_struct_struct &fts, size_t tuner_id);
        bool deviceDeleteTuning(frontend_tuner_status_struct_struct &fts, size_t tuner_id);

};

#endif // FEI_EXCEPTION_THROUGH_I_IMPL_H
