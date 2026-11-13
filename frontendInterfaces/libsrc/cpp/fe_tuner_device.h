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
#ifndef FE_TUNER_DEVICE_BASE_H
#define FE_TUNER_DEVICE_BASE_H

#include <ossie/Device_impl.h>
#include <uuid/uuid.h>
#include <redhawk/FRONTEND/Frontend.h>

#include "ossie/prop_helpers.h"
#include "bulkio/bulkio.h"
#include "fe_tuner_struct_props.h"
#include "fe_tuner_port_impl.h"
#include "fe_rfinfo_port_impl.h"
#include "fe_rfsource_port_impl.h"

/*********************************************************************************************/
/**************************              FRONTEND                   **************************/
/*********************************************************************************************/
namespace frontend {

    class AllocationAlreadyExists : public CF::Device::InvalidCapacity {
      public:
        AllocationAlreadyExists(const char* msg, const CF::Properties& props) :
          CF::Device::InvalidCapacity(msg, props)
        {}
    };
    
    inline std::string uuidGenerator() {
        uuid_t new_random_uuid;
        uuid_generate_random(new_random_uuid);
        char new_random_uuid_str[37];
        uuid_unparse(new_random_uuid, new_random_uuid_str);
        return std::string(new_random_uuid_str);
    };

    /* floatingPointCompare is a helper function to handle floating point comparison
     * Return values:
     *   if lhs == rhs: 0.0
     *   if lhs >  rhs: 1.0 or greater
     *   if lhs <  rhs: -1.0 or less
     * Recommended usage is to convert a comparison such as: (lhs OP rhs)
     * to (floatingPointCompare(lhs,rhs) OP 0), where OP is a comparison operator
     * (==, <, >, <=, >=, !=).
     * "places" is used to specify precision. The default is 1, which
     * uses a single decimal place of precision.
     */
    inline double floatingPointCompare(double lhs, double rhs, size_t places = 1){
        return round((lhs-rhs)*pow(10,places));
        /*if(round((lhs-rhs)*(pow(10,places))) == 0)
            return 0; // equal
        if(lhs<rhs)
            return -1; // lhs < rhs
        return 1; // lhs > rhs*/
    }

    /* validateRequest is a helper function to verify a value is within a range, returning
     * true if the value requested_val falls within the range [available_min:available_max]
     * False is returned if min > max
     */
    inline bool validateRequest(double available_min, double available_max, double requested_val){
        if(floatingPointCompare(requested_val,available_min) < 0) return false;
        if(floatingPointCompare(requested_val,available_max) > 0) return false;
        if(floatingPointCompare(available_min,available_max) > 0) return false;
        return true;
    }

    /* validateRequest is a helper function to compare two ranges, returning true if the range
     * [requested_min:requested_max] falls within the range [available_min:available_max]
     * False is returned if min > max for either available for requested values
     */
    inline bool validateRequest(double available_min, double available_max, double requested_min, double requested_max){
        double center_request = (requested_max+requested_min)/2.0;
        if(floatingPointCompare(center_request,available_min) < 0) return false;
        if(floatingPointCompare(center_request,available_max) > 0) return false;
        if(floatingPointCompare(available_min,available_max) > 0) return false;
        if(floatingPointCompare(requested_min,requested_max) > 0) return false;
        return true;
    }

    /* validateRequestVsSRI is a helper function to check that the input data stream can support
     * the allocation request. The output mode (true if complex output) is used when determining
     * the necessary sample rate required to satisfy the request. The entire frequency band of the
     * request must be available for True to be returned, not just the center frequency.
     * True is returned upon success, otherwise FRONTEND::BadParameterException is thrown.
     * If the CHAN_RF and FRONTEND::BANDWIDTH keywords are not found in the sri,
     * FRONTEND::BadParameterException is thrown.
     */
    bool validateRequestVsSRI(const frontend_tuner_allocation_struct& request, const BULKIO::StreamSRI& upstream_sri, bool output_mode);

    /* validateRequestVsDevice is a helper function to check that the input data stream and the
     * device can support an allocation request. The output mode (true if complex output) is used
     * when determining the necessary sample rate required to satisfy the request. The entire
     * frequency band of the request must be available for True to be returned, not just the center
     * frequency.
     * True is returned upon success, otherwise FRONTEND::BadParameterException is thrown.
     * If the CHAN_RF and FRONTEND::BANDWIDTH keywords are not found in the sri,
     * FRONTEND::BadParameterException is thrown.
     */
    bool validateRequestVsDevice(const frontend_tuner_allocation_struct& request, const BULKIO::StreamSRI& upstream_sri,
            bool output_mode, double min_device_center_freq, double max_device_center_freq, double max_device_bandwidth, double max_device_sample_rate);

    /* validateRequestVsRFInfo is a helper function to check that the analog capabilities can support
     * the allocation request. The mode (true if complex) is used when determining the necessary
     * sample rate required to satisfy the request. The entire frequency band of the request must be
     * available for True to be returned, not just the center frequency.
     * True is returned upon success, otherwise FRONTEND::BadParameterException is thrown.
     */
    bool validateRequestVsRFInfo(const frontend_tuner_allocation_struct& request, const frontend::RFInfoPkt& rfinfo, bool mode);

    /* validateRequestVsDevice is a helper function to check that the analog capabilities and the
     * device can support the allocation request. The mode (true if complex) is used when
     * determining the necessary sample rate required to satisfy the request. The entire frequency
     * band of the request must be available for True to be returned, not just the center frequency.
     * True is returned upon success, otherwise FRONTEND::BadParameterException is thrown.
     */
    bool validateRequestVsDevice(const frontend_tuner_allocation_struct& request, const frontend::RFInfoPkt& rfinfo,
            bool mode, double min_device_center_freq, double max_device_center_freq, double max_device_bandwidth, double max_device_sample_rate);

    /* Tuner Allocation IDs struct. This structure contains allocation tracking data.
     *
     */
    
    struct tunerAllocationIdsStruct {
        tunerAllocationIdsStruct(){
            reset();
        }
        std::string control_allocation_id;
        std::vector<std::string> listener_allocation_ids;

        void reset(){
            control_allocation_id.clear();
            listener_allocation_ids.clear();
        }
    };

    /*
     *    Frontend tuner class definition
     */
    template < typename TunerStatusStructType >
    class FrontendTunerDevice : public Device_impl
    {
        ENABLE_LOGGING

        public:
            FrontendTunerDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
            FrontendTunerDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
            FrontendTunerDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
            FrontendTunerDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
            ~FrontendTunerDevice();

            // this is implemented in the generated base class once all properties are known
            virtual void loadProperties();

            // Device specific allocation handling
            virtual CF::Device::UsageType updateUsageState();
            virtual CORBA::Boolean allocateCapacity(const CF::Properties & capacities);
            virtual void deallocateCapacity(const CF::Properties & capacities);

        protected:
            typedef std::map<std::string, size_t> string_number_mapping;
            typedef boost::mutex::scoped_lock exclusive_lock;

            // Member variables exposed as properties
            std::string device_kind;
            std::string device_model;
            frontend::frontend_tuner_allocation_struct frontend_tuner_allocation;
            frontend::frontend_listener_allocation_struct frontend_listener_allocation;
            std::vector<TunerStatusStructType> frontend_tuner_status;

            virtual bool callDeviceSetTuning(size_t tuner_id);
            virtual void checkValidIds(const CF::Properties & capacities);

            // tuner_allocation_ids is exclusively paired with property frontend_tuner_status.
            // tuner_allocation_ids tracks allocation ids while frontend_tuner_status provides tuner information.
            std::vector<frontend::tunerAllocationIdsStruct> tuner_allocation_ids;

            // Provides mapping from unique allocation ID to internal tuner (channel) number
            string_number_mapping allocation_id_to_tuner_id;
            boost::mutex allocation_id_mapping_lock;

            ///////////////////////////////
            // Device specific functions // -- virtual - to be implemented by device developer
            ///////////////////////////////
            virtual void deviceEnable(TunerStatusStructType &fts, size_t tuner_id) = 0;
            virtual void deviceDisable(TunerStatusStructType &fts, size_t tuner_id) = 0;
            virtual bool deviceSetTuning(const frontend_tuner_allocation_struct &request, TunerStatusStructType &fts, size_t tuner_id) = 0;
            virtual bool deviceDeleteTuning(TunerStatusStructType &fts, size_t tuner_id) = 0;

            ///////////////////////////////
            // Mapping and translation helpers. External string identifiers to internal numerical identifiers
            ///////////////////////////////
            virtual std::string getControlAllocationId(size_t tuner_id);
            virtual std::vector<std::string> getListenerAllocationIds(size_t tuner_id);
            virtual long getTunerMapping(std::string allocation_id);
            std::string createAllocationIdCsv(size_t tuner_id);
            virtual void sendEOS(std::string allocation_id);
            virtual bool removeTunerMapping(size_t tuner_id, std::string allocation_id);
            virtual bool removeTunerMapping(size_t tuner_id);
            virtual void assignListener(const std::string& listen_alloc_id, const std::string& alloc_id);
            virtual void removeListener(const std::string& listen_alloc_id);
            virtual void removeAllocationIdRouting(const size_t tuner_id) = 0;
            virtual void setNumChannels(size_t num);
            virtual void setNumChannels(size_t num, std::string tuner_type);
            virtual void addChannels(size_t num, std::string tuner_type);

            // Configure tuner - gets called during allocation
            virtual bool enableTuner(size_t tuner_id, bool enable);
            virtual bool listenerRequestValidation(frontend_tuner_allocation_struct &request, size_t tuner_id);


            ////////////////////////////
            // Other helper functions //
            ////////////////////////////
            BULKIO::StreamSRI create(std::string &stream_id, TunerStatusStructType &frontend_status, double collector_frequency = -1.0) {
                BULKIO::StreamSRI sri;
                sri.hversion = 1;
                sri.xstart = 0.0;
                if ( frontend_status.sample_rate <= 0.0 )
                    sri.xdelta =  1.0;
                else
                    sri.xdelta = 1/frontend_status.sample_rate;
                sri.xunits = BULKIO::UNITS_TIME;
                sri.subsize = 0;
                sri.ystart = 0.0;
                sri.ydelta = 0.0;
                sri.yunits = BULKIO::UNITS_NONE;
                sri.mode = 0;
                sri.blocking=false;
                sri.streamID = stream_id.c_str();
                CORBA::Double colFreq;
                if (collector_frequency < 0)
                    colFreq = frontend_status.center_frequency;
                else
                    colFreq = CORBA::Double(collector_frequency);
                this->addModifyKeyword<CORBA::Double > (&sri, "COL_RF", CORBA::Double(colFreq));
                this->addModifyKeyword<CORBA::Double > (&sri, "CHAN_RF", CORBA::Double(frontend_status.center_frequency));
                this->addModifyKeyword<std::string> (&sri,"FRONTEND::RF_FLOW_ID",frontend_status.rf_flow_id);
                this->addModifyKeyword<CORBA::Double> (&sri,"FRONTEND::BANDWIDTH", CORBA::Double(frontend_status.bandwidth));
                this->addModifyKeyword<std::string> (&sri,"FRONTEND::DEVICE_ID",_identifier);
                return sri;
            }

            template <typename CORBAXX> bool addModifyKeyword(BULKIO::StreamSRI *sri, CORBA::String_member id, CORBAXX myValue, bool addOnly = false) {
                CORBA::Any value;
                value <<= myValue;
                unsigned long keySize = sri->keywords.length();
                if (!addOnly) {
                    for (unsigned int i = 0; i < keySize; i++) {
                        if (!strcmp(sri->keywords[i].id, id)) {
                            sri->keywords[i].value = value;
                            return true;
                        }
                    }
                }
                sri->keywords.length(keySize + 1);
                if (sri->keywords.length() != keySize + 1)
                    return false;
                sri->keywords[keySize].id = CORBA::string_dup(id);
                sri->keywords[keySize].value = value;
                return true;
            }

            // This is not currently used but is available as a debugging tool
            void printSRI(BULKIO::StreamSRI *sri, std::string strHeader = "DEBUG SRI"){
                std::cout << strHeader << ":\n";
                std::cout << "\thversion: " << sri->hversion<< std::endl;
                std::cout << "\txstart: " << sri->xstart<< std::endl;
                std::cout << "\txdelta: " << sri->xdelta<< std::endl;
                std::cout << "\txunits: " << sri->xunits<< std::endl;
                std::cout << "\tsubsize: " << sri->subsize<< std::endl;
                std::cout << "\tystart: " << sri->ystart<< std::endl;
                std::cout << "\tydelta: " << sri->ydelta<< std::endl;
                std::cout << "\tyunits: " << sri->yunits<< std::endl;
                std::cout << "\tmode: " << sri->mode<< std::endl;
                std::cout << "\tstreamID: " << sri->streamID<< std::endl;
                unsigned long keySize = sri->keywords.length();
                for (unsigned int i = 0; i < keySize; i++) {
                    std::cout << "\t KEYWORD KEY/VAL :: " << sri->keywords[i].id << ": " << ossie::any_to_string(sri->keywords[i].value) << std::endl;
                }
                std::cout << std::endl;
            }

        private:
            // this will be overridden by the generated base class once all ports are known
            virtual void construct();

	protected:
            virtual void _deallocateCapacity(const CF::Properties & capacities);
            virtual bool _removeTunerMapping(size_t tuner_id, std::string allocation_id);
            virtual bool _removeTunerMapping(size_t tuner_id);            
    };

    /*
     *    Frontend scanning tuner class definition
     */
    template < typename TunerStatusStructType >
    class FrontendScanningTunerDevice : public FrontendTunerDevice< TunerStatusStructType >
    {
        ENABLE_LOGGING

        public:
            FrontendScanningTunerDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
            FrontendScanningTunerDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
            FrontendScanningTunerDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
            FrontendScanningTunerDevice(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
            ~FrontendScanningTunerDevice();

        protected:
            frontend::frontend_scanner_allocation_struct frontend_scanner_allocation;

            virtual bool callDeviceSetTuning(size_t tuner_id);
            virtual void checkValidIds(const CF::Properties & capacities);
            virtual bool deviceSetTuningScan(const frontend_tuner_allocation_struct &request, const frontend_scanner_allocation_struct &scan_request, TunerStatusStructType &fts, size_t tuner_id) = 0;
            virtual bool deviceSetTuning(const frontend_tuner_allocation_struct &request, TunerStatusStructType &fts, size_t tuner_id) = 0;
            virtual void loadProperties();

        private:
            // this will be overridden by the generated base class once all ports are known
            virtual void construct();
            bool _has_scanner;
    };

}; // end frontend namespace

#endif
