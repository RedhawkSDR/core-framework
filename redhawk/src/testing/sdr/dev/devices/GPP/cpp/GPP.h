/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK GPP.
 *
 * REDHAWK GPP is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK GPP is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef GPP_IMPL_H
#define GPP_IMPL_H

#include "GPP_base.h"
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <sys/resource.h>

#include "utils/Updateable.h"
#include "reports/ThresholdMonitor.h"
#include "states/State.h"
#include "statistics/Statistics.h"
#include "statistics/CpuUsageStats.h"
#include "reports/SystemMonitorReporting.h"
#include "reports/CpuThresholdMonitor.h"
#include "reports/NicThroughputThresholdMonitor.h"
#include "NicFacade.h"
#include "ossie/Events.h"

class ThresholdMonitor;
class NicFacade;


#if BOOST_FILESYSTEM_VERSION < 3
#define BOOST_PATH_STRING(x) (x)
#else
#define BOOST_PATH_STRING(x) (x).string()
#endif

typedef boost::shared_mutex Lock;
typedef boost::unique_lock< Lock > WriteLock;
typedef boost::shared_lock< Lock > ReadLock;




class GPP_i : public GPP_base
{
  ENABLE_LOGGING;
    
 public:
  static std::string format_up_time(unsigned long secondsUp);


    public:
        GPP_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        GPP_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        GPP_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        GPP_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~GPP_i();

        int serviceFunction();
        void initializeNetworkMonitor();
        void initializeResourceMonitors();
        void send_threshold_event(const threshold_event_struct& message);
        
        void initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException);
        bool allocate_loadCapacity(const double &value);
        void deallocate_loadCapacity(const double &value);
        bool allocate_diskCapacity(const double &value);
        void deallocate_diskCapacity(const double &value);
        bool allocate_memCapacity(const CORBA::LongLong &value);
        void deallocate_memCapacity(const CORBA::LongLong &value);
        bool allocate_reservation_request(const redhawk__reservation_request_struct &value);
        void deallocate_reservation_request(const redhawk__reservation_request_struct &value);
        bool allocate_mcastegress_capacity(const CORBA::Long &value);
        void deallocate_mcastegress_capacity(const CORBA::Long &value);
        bool allocate_mcastingress_capacity(const CORBA::Long &value);
        void deallocate_mcastingress_capacity(const CORBA::Long &value);

        CF::ExecutableDevice::ProcessID_Type execute ( const char* name, 
                                                       const CF::Properties& options, 
                                                       const CF::Properties& parameters)
          throw (CORBA::SystemException, CF::Device::InvalidState, CF::ExecutableDevice::InvalidFunction, 
                 CF::ExecutableDevice::InvalidParameters, CF::ExecutableDevice::InvalidOptions, 
                 CF::InvalidFileName, CF::ExecutableDevice::ExecuteFail);

        CF::ExecutableDevice::ProcessID_Type do_execute (const char* name, const CF::Properties& options,
                                                         const CF::Properties& parameters, 
                                                         const std::vector<std::string> prepend_args) 
          throw (CF::ExecutableDevice::ExecuteFail,
                 CF::InvalidFileName, CF::ExecutableDevice::InvalidOptions,
                 CF::ExecutableDevice::InvalidParameters,
                 CF::ExecutableDevice::InvalidFunction, CF::Device::InvalidState,
                 CORBA::SystemException);


        void terminate (CF::ExecutableDevice::ProcessID_Type processId)
            throw (CORBA::SystemException, CF::ExecutableDevice::InvalidProcess, CF::Device::InvalidState);
        void updateThresholdMonitors();
        void sendChildNotification(const std::string &comp_id, const std::string &app_id);
        bool allocateCapacity_nic_allocation(const nic_allocation_struct &value);
        void deallocateCapacity_nic_allocation(const nic_allocation_struct &value);
        void deallocateCapacity (const CF::Properties& capacities) throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CORBA::SystemException);
        CORBA::Boolean allocateCapacity (const CF::Properties& capacities) throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CF::Device::InsufficientCapacity, CORBA::SystemException);
        void releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError);


        void postConstruction( std::string &softwareProfile,
                          std::string &registrar_ior,
                          const std::string &idm_channel_ior="",
                          const std::string &nic="",
                          const int  sigfd=-1 );

        int sigchld_handler( int sig );

        int redirected_io_handler( );
        
        std::vector<component_monitor_struct> get_component_monitor();
        
        struct proc_values {
            float mem_rss;
            CORBA::ULong num_threads;
            int pgrpid;
        };
        
        struct grp_values : proc_values {
            int num_processes;
            std::vector<int> pids;
        };
        
        void update_grp_child_pids();
        std::map<int,proc_values> parsed_stat;
        std::map<int,grp_values> grp_children;

        struct  proc_redirect {
          int         pid;
          int         cout;
          int         cerr;
          std::string fname;
          proc_redirect( int _pid, int _cout, int _cerr=-1 );
          proc_redirect( const std::string &fname, int _pid, int _cout, int _cerr=-1 );
          void close();
        };

        struct component_description {
	  static const int pstat_history_len=5;
          int         pid;
          std::string appName;
          std::string identifier;
          bool        app_started;
          float       reservation;
          float       core_usage;
          bool        terminated;
          uint64_t    pstat_history[pstat_history_len];
          uint8_t     pstat_idx;
          std::vector<int> pids;
          GPP_i       *parent;

	  component_description();
          component_description( const std::string &appId);
	  void          add_history( int64_t ptime );
	  void          add_history();
	  int64_t       get_pstat_usage( const bool refresh=true );
	  int64_t       get_pstat_usage( uint64_t &p2, uint64_t &p1 );
	  int64_t       get_process_time();
        };

        struct application_reservation {
          std::vector<unsigned short> component_pids;
          std::map<std::string, float> reservation;
          float usage;
        };

        void constructor();

        protected:

        
          struct LoadCapacity {
            float max;
            float measured;
            float allocated;
          };

          //
          // base execution unit for partitioning a host system
          //
          struct exec_socket : public Updateable {
            int                              id;
            CpuUsageStats::CpuList           cpus;
            CpuUsageStats                    stats;
            LoadCapacity                     load_capacity;              // future
            double                           idle_threshold;
            double                           idle_cap_mod;

          exec_socket() : idle_threshold(0.0), idle_cap_mod(0.0) {};

            void update() {
              stats.compute_statistics();
            };

            double get_idle_percent() const {
              return stats.get_idle_percent();
            }

            double get_idle_average() const {
              return stats.get_idle_average();
            }

            bool is_available() const {
              return stats.get_idle_percent() > idle_threshold ;
            };
          };

          typedef std::vector< exec_socket >      ExecPartitionList;


          friend bool operator==( const component_description &, 
                                  const component_description & );

          std::vector<int> getPids();
          component_description getComponentDescription(int pid);
          void                  markPidTerminated(const int pid );


          void  set_resource_affinity( const CF::Properties& options,
                                       const pid_t rsc_pid,
                                       const char  *rsc_name,
                                       const std::vector<int> &bl= std::vector<int>(0) );           


          void process_ODM(const CORBA::Any &data);

          void updateUsageState();
          void setShadowThresholds(const thresholds_struct &newVals );

          typedef boost::shared_ptr<NicThroughputThresholdMonitor>   NicMonitorPtr;
          typedef boost::shared_ptr<ThresholdMonitor>           ThresholdMonitorPtr;
          typedef std::vector< uint32_t >                       CpuList;
          typedef std::vector< boost::shared_ptr<Updateable> >  UpdateableSequence;
          typedef std::vector<boost::shared_ptr<State> >        StateSequence;
          typedef std::vector<boost::shared_ptr<Statistics> >   StatisticsSequence;
          typedef std::vector<boost::shared_ptr<Reporting> >    ReportingSequence;
          typedef std::vector< ThresholdMonitorPtr >            MonitorSequence;
          typedef std::vector< NicMonitorPtr >                  NicMonitorSequence;
          typedef boost::shared_ptr<SystemMonitor>              SystemMonitorPtr;
          typedef std::map<int, component_description >         ProcessMap;
          typedef std::deque< component_description >           ProcessList;
          typedef std::deque< proc_redirect >                   ProcessFds;
          typedef std::map<std::string, application_reservation>    ApplicationReservationMap;

          void addProcess(int pid, 
                      const std::string &appName, 
                      const std::string &identifier,
                      const float       req_reservation );
          void removeProcess(int pid );
          void addThresholdMonitor( ThresholdMonitorPtr threshold_monitor );
          void reservedChanged(const float *oldValue, const float *newValue);
          void mcastnicThreshold_changed(const CORBA::Long *oldValue, const CORBA::Long *newValue);
          void thresholds_changed(const thresholds_struct *oldValue, const thresholds_struct *newValue);
          void update();

          ProcessList                                         pids;
          size_t                                              n_reservations;
          Lock                                                pidLock;
          Lock                                                fdsLock;
          ProcessFds                                          redirectedFds;
          bool                                                _handle_io_redirects;
          std::string                                         _componentOutputLog;

          Lock                                                nicLock;
          NicFacadePtr                                        nic_facade;
          MonitorSequence                                     threshold_monitors;
          NicMonitorSequence                                  nic_monitors;
          SystemMonitorPtr                                    system_monitor;
          ProcessLimitsPtr                                    process_limits;
          ExecPartitionList                                   execPartitions;
          ApplicationReservationMap                           applicationReservations;
        
          Lock                                                monitorLock;
          UpdateableSequence                                  data_model;
          thresholds_struct                                   __thresholds;             
          thresholds_struct                                   modified_thresholds;
          uint64_t                                            thresh_mem_free_units;
          uint64_t                                            mem_free_units;
          uint64_t                                            mem_cap_units;
          int64_t                                             memCapacityThreshold;
          double                                              memInitCapacityPercent;
          uint64_t                                            memInitVirtFree;
          float                                               idle_capacity_modifier;
          CpuList                                             wl_cpus;            // list of allowable cpus to run on .... empty == all, derived from affnity blacklist property and host machine
          CpuList                                             bl_cpus;            // list of blacklist cpus to avoid
          double                                             mcastnicIngressThresholdValue;
          double                                             mcastnicEgressThresholdValue;

          std::string                                         binary_location;    // path to this program.
        
          boost::posix_time::ptime                            time_mark;          // time marker for update
          redhawk::events::SubscriberPtr                      odm_consumer;       // interface that receives ODM_Channel events
          redhawk::events::ManagerPtr                         mymgr;              // interface to manage event channel access

          std::string                                         _busy_reason;
          boost::posix_time::ptime                            _busy_timestamp;          // time when busy reason was initially set
          boost::posix_time::ptime                            _busy_mark;               // track message output

        private:

          //
          // set the busy reason property for the GPP..  
          //
          void  _resetReason();
          void  _setReason( const std::string &reason, const std::string &event, const bool enable_timestamp = true );

          bool  _component_cleanup( const int pid, const int exit_status );

          //
          // setup execution partitions for launching components
          // 
          int   _setupExecPartitions( const CpuList &blacklist );

          //
          // apply specific affinity settings to a pid
          //
          int   _apply_affinity(  const pid_t rsc_pid, 
                                  const char *rsc_name,
                                  const std::string &affinity_class,
                                  const std::string &affinity_value,
                                  const CpuList &bl_cpus );
        
          //
          // apply affinity settings for affinity struct property
          //
          int   _apply_affinity( const affinity_struct &affinity, const pid_t rsc_pid, const char *rsc_name  );

          //
          // get the next available partition to use for luanching resources
          //
          int   _get_deploy_on_partition();

          //
          // Check if execution partition for a NIC interface has enough processing capacity 
          //
          bool _check_exec_partition( const std::string &iface );

          //
          // Callback when affinity processing structure is changed
          //
          void _affinity_changed(const affinity_struct *ov, const affinity_struct *nv );

          //
          // Callback when componentOutputLog is changed
          //
          void _component_output_changed(const std::string *ov, const std::string *nv );

          //
          // Set vlan list attribute
          //
          void _set_vlan_property();

          //
          // Determine list of CPUs that are monitored
          //
          void _set_processor_monitor_list( const CpuList &cl );

          //
          // expand special characters in consoleOutputLog
          //
          std::string _expand_parameters( const std::string &path );

          //
          // Common method called by all CTORs
          //
          void _init();
        
          //
          // check file and thread limits for the process and system
          //
          bool _check_limits( const thresholds_struct &threshold);
          //
          // check threshold limits for nic interfaces to determine busy state
          //
          bool _check_nic_thresholds();

          std::string user_id;
          int limit_check_count;

          ossie::ProcessThread                                _signalThread;
          ossie::ProcessThread                                _redirectedIO;
        };

#endif // GPP_IMPL_H
