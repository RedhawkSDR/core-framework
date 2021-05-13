#ifndef FEI_CAP_I_IMPL_H
#define FEI_CAP_I_IMPL_H

#include "fei_cap_base.h"

class fei_cap_i : public fei_cap_base
{
    ENABLE_LOGGING
    public:
        fei_cap_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        fei_cap_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        fei_cap_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        fei_cap_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~fei_cap_i();

        void constructor();

        int serviceFunction();

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
        void configureTuner(const std::string& allocation_id, const CF::Properties& tunerSettings);
        CF::Properties* getTunerSettings(const std::string& allocation_id);
        std::string get_rf_flow_id(const std::string& port_name);
        void set_rf_flow_id(const std::string& port_name, const std::string& id);
        frontend::RFInfoPkt get_rfinfo_pkt(const std::string& port_name);
        void set_rfinfo_pkt(const std::string& port_name, const frontend::RFInfoPkt& pkt);

        std::vector<RDC_i*> RDCs;
        std::map<std::string, CF::Device::Allocations_var> _delegatedAllocations;

        CF::Device::Allocations* allocate (const CF::Properties& capacities)
            throw (CF::Device::InvalidState, CF::Device::InvalidCapacity,
                   CF::Device::InsufficientCapacity, CORBA::SystemException);
        void deallocate (const char* alloc_id)
            throw (CF::Device::InvalidState, CF::Device::InvalidCapacity,
                   CORBA::SystemException);
        CORBA::Boolean allocateCapacity(const CF::Properties & capacities)
            throw (CORBA::SystemException, CF::Device::InvalidCapacity, CF::Device::InvalidState);
        void deallocateCapacity (const CF::Properties& capacities) throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CORBA::SystemException);

    private:
        ////////////////////////////////////////
        // Required device specific functions // -- to be implemented by device developer
        ////////////////////////////////////////

        // these are pure virtual, must be implemented here
        void deviceEnable(frontend_tuner_status_struct_struct &fts, size_t tuner_id);
        void deviceDisable(frontend_tuner_status_struct_struct &fts, size_t tuner_id);
        bool deviceSetTuning(const frontend::frontend_tuner_allocation_struct &request, frontend_tuner_status_struct_struct &fts, size_t tuner_id);
        bool deviceDeleteTuning(frontend_tuner_status_struct_struct &fts, size_t tuner_id);

};

#endif // FEI_CAP_I_IMPL_H
