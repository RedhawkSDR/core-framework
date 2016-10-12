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
/**************************************************************************

    This is the device code. This file contains the child class where
    custom functionality can be added to the device. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/
#include <stdexcept>
#include <linux/limits.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/utsname.h>
#include <boost/filesystem/path.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#ifdef HAVE_LIBNUMA
#include <numa.h>
#endif
#include "ossie/Events.h"
#include "ossie/affinity.h"

#include "GPP.h"
#include "utils/affinity.h"
#include "utils/SymlinkReader.h"
#include "utils/ReferenceWrapper.h"
#include "states/ProcMeminfo.h"
#include "statistics/CpuUsageStats.h"
#include "reports/NicThroughputThresholdMonitor.h"
#include "reports/FreeMemoryThresholdMonitor.h"

#define PROCESSOR_NAME "DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b"
#define OS_NAME        "DCE:4a23ad60-0b25-4121-a630-68803a498f75"
#define OS_VERSION     "DCE:0f3a9a37-a342-43d8-9b7f-78dc6da74192"




PREPARE_LOGGING(GPP_i)

extern GPP_i *devicePtr;

inline bool operator== (const GPP_i::component_description& s1, 
			const GPP_i::component_description& s2) {
    if (s1.appName!=s2.appName)
        return false;
    if (s1.identifier!=s2.identifier)
        return false;
    return true;
};


GPP_i::GPP_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    GPP_base(devMgr_ior, id, lbl, sftwrPrfl)
{
  _init();
}

GPP_i::GPP_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    GPP_base(devMgr_ior, id, lbl, sftwrPrfl, compDev)
{
  _init();
}

GPP_i::GPP_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    GPP_base(devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
  _init();
}

GPP_i::GPP_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    GPP_base(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev)
{
  _init();
}

GPP_i::~GPP_i()
{

}

void GPP_i::_init() {

  sig_fd = -1;

  //
  // add our local set affinity method that performs numa library calls
  //
  redhawk::affinity::override_set( &gpp::affinity::set_affinity );

  //
  // get canonical machine context
  //
  hostName = boost::asio::ip::host_name();

  struct utsname _uts;
  if (uname(&_uts) != -1) {
    std::string machine(_uts.machine);
    if ((machine== "i386") or (machine== "i686")) {
      machine = "x86";
    }
    processor_name = machine;
    os_name = _uts.sysname;
    os_version = _uts.release;
  }

  // figure out the location of the binary
  std::ostringstream lpath;
  lpath << "/proc/"<<getpid()<<"/exe";
  try {
    std::string binary_file = SymlinkReader::ReadLink(lpath.str());
    binary_location = binary_file.substr(0, binary_file.find_last_of('/')+1);    
  }
  catch(...){
  }

  // default cycle time setting for updating data model, metrics and state
  threshold_cycle_time = 500;

  //
  // Add property change listeners and allocation modifiers
  //

  // Add property change listeners for affinity information...
  addPropertyChangeListener( "affinity", this, &GPP_i::_affinity_changed );

  // add property change listener
  addPropertyChangeListener("reserved_capacity_per_component", this, &GPP_i::reservedChanged);

  // tie allocation modifier callbacks to identifiers

  // nic allocation 
  setAllocationImpl("nic_allocation", this, &GPP_i::allocateCapacity_nic_allocation, &GPP_i::deallocateCapacity_nic_allocation);

  // load capacity allocations
  setAllocationImpl("DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056", this, &GPP_i::allocate_loadCapacity, &GPP_i::deallocate_loadCapacity);

  // check  memory capacity allocations 
  setAllocationImpl("DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", this, &GPP_i::allocate_memCapacity, &GPP_i::deallocate_memCapacity);

  //setAllocationImpl("diskCapacity", this, &GPP_i::allocate_diskCapacity, &GPP_i::deallocate_diskCapacity);

}

void GPP_i::postConstruction (std::string &profile, 
                                     std::string &registrar_ior, 
                                     const std::string &idm_channel_ior,
                                     const std::string &nic,
                                     const int sigfd )
{

  Device_impl::postConstruction(profile,registrar_ior,idm_channel_ior,nic,sigfd );
  // if sigfd > 0 then signalfd method was establish via command line USESIGFD
  if ( sigfd > 0 ) {
    sig_fd = sigfd;
  }
  else {
    // require signalfd to be configured before orb init call.... 
    throw std::runtime_error("unable configure signal handler");
  }

}


void GPP_i::process_ODM(const CORBA::Any &data) {
    boost::mutex::scoped_lock lock(pidLock);
    const ExtendedEvent::ResourceStateChangeEventType* app_state_change;
    if (data >>= app_state_change) {
        std::string appName = ossie::corba::returnString(app_state_change->sourceName);
        if (app_state_change->stateChangeTo == ExtendedEvent::STARTED) {
          RH_NL_TRACE("GPP", "ODM CHANNEL EVENT --> APP STARTED app: " << appName );
	  for (std::vector<component_description>::iterator it=reservations.begin();it!=reservations.end();it++) {
	    if ((*it).appName == appName) {
	      tableReservation(*it);
	      break;
	    }
	  }
        } else if (app_state_change->stateChangeTo == ExtendedEvent::STOPPED) {
          RH_NL_TRACE("GPP", "ODM CHANNEL EVENT --> APP STOPPED app: " << appName );
	  for (std::vector<component_description>::iterator it=tabled_reservations.begin();it!=tabled_reservations.end();it++) {
	    if ((*it).appName == appName) {
	      restoreReservation(*it);
	      break;
	    }
	  }
	}
    }
}

int GPP_i::_setupExecPartitions( const CpuList &bl_cpus ) {

#if HAVE_LIBNUMA
  // fill in the exec partitions for each numa node identified on the system
    CpuUsageStats::CpuList cpus;
    std::string nodestr("all");
    struct bitmask *node_mask = numa_parse_nodestring((char *)nodestr.c_str());     

    bitmask *cpu_mask = numa_allocate_cpumask(); 
    
    // for each node bit set in the mask then get cpu list
    int nbytes = numa_bitmask_nbytes(node_mask);
    for (int i=0; i < nbytes*8; i++ ){
      exec_socket  soc;
      cpus.clear();
      if ( numa_bitmask_isbitset( node_mask, i ) ) {
        soc.id = i;
        numa_node_to_cpus( i, cpu_mask );
              
        // foreach cpu identified add to list
        int nb = numa_bitmask_nbytes(cpu_mask);
        for (int j=0; j < nb*8; j++ ){
          int count =  std::count( bl_cpus.begin(), bl_cpus.end(), j );
          if ( numa_bitmask_isbitset( cpu_mask, j ) && count == 0 ) {
            cpus.push_back( j );
          }
        }
        CpuUsageStats cpu_usage(cpus);
        soc.cpus = cpus;
        soc.stats = cpu_usage;
        soc.idle_threshold = thresholds.cpu_idle;      
        soc.load_capacity.max =  cpus.size() * 1.0;
        soc.load_capacity.measured = 0.0;
        soc.load_capacity.allocated = 0.0;
        execPartitions.push_back( soc );
      }
    }

#endif

    if ( execPartitions.size()  ) {
      ExecPartitionList::iterator iter =  execPartitions.begin();
      std::ostringstream ss;
      ss  << boost::format("%-6s %-4s %-7s %-7s %-7s ") % "SOCKET" % "CPUS" % "USER"  % "SYSTEM"  % "IDLE"   << std::endl;
      LOG_INFO(GPP_i, ss.str()  );
      ss.clear();
      ss.str("");
      for ( ; iter != execPartitions.end(); iter++ ) {
        iter->update();  iter->update();
        ss  << boost::format("%-6d %-4d %-7.2f %-7.2f %-7.2f ") % iter->id % iter->stats.get_ncpus() % iter->stats.get_user_percent()  % iter->stats.get_system_percent()  % iter->stats.get_idle_percent() ;
        LOG_INFO(GPP_i, ss.str()  );    
        ss.clear();
        ss.str("");
      }
    }

    return 0;
}



void  GPP_i::initializeMemoryMonitor()
{
  // add available memory monitor, mem_free defaults to MB
  addThresholdMonitor( new FreeMemoryThresholdMonitor(_identifier, MakeCref<CORBA::LongLong, float>(thresholds.mem_free), 
                                                      ConversionWrapper<CORBA::LongLong, float>(memCapacity, 1048576, std::divides<float>() ) ) );
}

void
GPP_i::initializeNetworkMonitor()
{
    nic_facade.reset( new NicFacade(advanced.maximum_throughput_percentage,
                                    nic_interfaces,
                                    available_nic_interfaces,
                                    networkMonitor,
                                    nic_metrics,
                                    nic_allocation_status) );

    data_model.push_back( nic_facade );

    std::vector<std::string> nic_devices( nic_facade->get_devices() );
    for( size_t i=0; i<nic_devices.size(); ++i )
    {
        LOG_INFO(GPP_i, __FUNCTION__ << ": Adding interface (" << nic_devices[i] << ")" );
        addThresholdMonitor(
            new NicThroughputThresholdMonitor(_identifier,
                                              nic_devices[i],
                                              MakeCref<CORBA::Long, float>(modified_thresholds.nic_usage),
                                              boost::bind(&NicFacade::get_throughput_by_device, nic_facade, nic_devices[i]) ) );
    }
}

void
GPP_i::initializeCpuMonitor()
{
  // add memory state reader
  ProcMeminfoPtr mem_state( new ProcMeminfo() );

  // add cpu utilization calculator
  RH_NL_INFO("GPP", " initialize CPU Montior --- wl size " << wl_cpus.size());
  CpuUsageStatsPtr cpu_usage_stats( new CpuUsageStats( wl_cpus ) );

  // provide required system metrics to this GPP
  system_monitor.reset( new SystemMonitor( cpu_usage_stats,
                                           mem_state ) );
  // seed system monitor
  for ( int i=0; i<5; i++ ) { 
    system_monitor->report(); 
    boost::this_thread::sleep( boost::posix_time::milliseconds( 200 ) );
  }

  data_model.push_back( system_monitor );

  //  observer to monitor when cpu idle pass threshold value
  addThresholdMonitor( new CpuThresholdMonitor(_identifier, &modified_thresholds.cpu_idle, *cpu_usage_stats, false ) );
}

void
GPP_i::addThresholdMonitor( ThresholdMonitor* threshold_monitor )
{
	boost::shared_ptr<ThresholdMonitor> t( threshold_monitor );
	t->attach_listener( boost::bind(&GPP_i::send_threshold_event, this, _1) );
	threshold_monitors.push_back( t );
}


//
//  Device LifeCycle API
// 

void GPP_i::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
  RH_NL_TRACE("GPP", "initialize()");
  
  //
  // subscribe to ODM_Channel for receiving state changes from Application object
  //
  try {
    mymgr = redhawk::events::Manager::GetManager(this);
    odm_consumer = mymgr->Subscriber("ODM_Channel");
    odm_consumer->setDataArrivedListener(this, &GPP_i::process_ODM);
  } catch ( ... ) {
    LOG_WARN(GPP_i, "Unable to register with EventChannelManager, disabling domain event notification.");
  }

  
  //
  // Setup affinity settings context
  //
  _affinity_changed( NULL, &affinity );

  //
  // setup execution partitions for performing socket based deployments, we need to know the current black list
  //
  _setupExecPartitions( bl_cpus );
    
  //
  // setup the data model for the GPP 
  //
  threshold_monitors.clear();
  initializeCpuMonitor();
  initializeMemoryMonitor();
  initializeNetworkMonitor();

  std::for_each( data_model.begin(), data_model.end(), boost::bind( &Updateable::update, _1 ) );
  std::for_each( execPartitions.begin(), execPartitions.end(), boost::bind( &Updateable::update, _1 ) );

  //
  // System wide metrics
  //
  loadCapacity_counter = 0;
  idle_capacity_modifier = 100.0 * reserved_capacity_per_component/((float)processor_cores);
  modified_thresholds = thresholds;

  // enable monitors to push out state change events..
  MonitorSequence::iterator iter=threshold_monitors.begin();
  for( ; iter != threshold_monitors.end(); iter++ ) {
    if  ( *iter ) (*iter)->enable_dispatch();
  }

  // use by service function to mark update time for monitors, states, and stats
  time_mark = boost::posix_time::microsec_clock::local_time();

  GPP_base::start();
  GPP_base::initialize();

}

void GPP_i::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError) {
    if ( odm_consumer ) odm_consumer.reset();
    GPP_base::releaseObject();
}


CF::ExecutableDevice::ProcessID_Type GPP_i::execute (const char* name, const CF::Properties& options, const CF::Properties& parameters)
    throw (CORBA::SystemException, CF::Device::InvalidState, CF::ExecutableDevice::InvalidFunction, 
           CF::ExecutableDevice::InvalidParameters, CF::ExecutableDevice::InvalidOptions, 
           CF::InvalidFileName, CF::ExecutableDevice::ExecuteFail)
{

    boost::recursive_mutex::scoped_lock lock;
    try
    {
        lock = boost::recursive_mutex::scoped_lock(load_execute_lock);
    }
    catch( const boost::thread_resource_error& e )
    {
        std::stringstream errstr;
        errstr << "Error acquiring lock (errno=" << e.native_error() << " msg=\"" << e.what() << "\")";
        LOG_ERROR(GPP_i, __FUNCTION__ << ": " << errstr.str() );
        throw CF::Device::InvalidState(errstr.str().c_str());
    }

    std::vector<std::string> prepend_args;
    std::string naming_context_ior;
    const redhawk::PropertyMap& tmp_params = redhawk::PropertyMap::cast(parameters);
    naming_context_ior = tmp_params["NAMING_CONTEXT_IOR"].toString();
    std::string app_id;
    std::string component_id = tmp_params["COMPONENT_IDENTIFIER"].toString();
    std::string name_binding = tmp_params["NAME_BINDING"].toString();
    CF::Application_var _app = CF::Application::_nil();
    CORBA::Object_var obj = ossie::corba::Orb()->string_to_object(naming_context_ior.c_str());
    if (CORBA::is_nil(obj)) {
        LOG_WARN(GPP_i, "Invalid application registrar IOR");
    } else {
        CF::ApplicationRegistrar_var _appRegistrar = CF::ApplicationRegistrar::_nil();
        _appRegistrar = CF::ApplicationRegistrar::_narrow(obj);
        if (CORBA::is_nil(_appRegistrar)) {
            LOG_WARN(GPP_i, "Invalid application registrar IOR");
        } else {
            _app = _appRegistrar->app();
            if (not CORBA::is_nil(_app)) {
                app_id = ossie::corba::returnString(_app->name());
            }
        }
    }
    if (this->useScreen) {
        std::string ld_lib_path(getenv("LD_LIBRARY_PATH"));
        setenv("GPP_LD_LIBRARY_PATH",ld_lib_path.c_str(),1);
        
        DIR *dir;
        struct dirent *ent;
        std::string search_bin("screen");
    
        std::string path(getenv( "PATH" ));
        bool foundScreen = false;
        while (not foundScreen) {
            size_t sub = path.find(":");
            if (path.size() == 0)
                break;
            std::string substr = path.substr(0, sub);
            if ((dir = opendir (substr.c_str())) != NULL) {
                while ((ent = readdir (dir)) != NULL) {
                    std::string filename(ent->d_name);
                    if (filename == search_bin) {
                        prepend_args.push_back(substr+"/"+filename);
                        foundScreen = true;
                        break;
                    }
                }
                closedir (dir);
            }
            if (sub != std::string::npos)
                path = path.substr(sub+1, std::string::npos);
            else
                path.clear();
        }
        prepend_args.push_back("-D");
        prepend_args.push_back("-m");
        prepend_args.push_back("-c");
        prepend_args.push_back(binary_location+"gpp.screenrc");
        if ((not component_id.empty()) and (not name_binding.empty())) {
            if (component_id.find("DCE:") != std::string::npos) {
                component_id = component_id.substr(4, std::string::npos);
            }
            size_t waveform_boundary = component_id.find(":");
            std::string component_inst_id, waveform_name;
            component_inst_id = component_id.substr(0, waveform_boundary);
            waveform_name = component_id.substr(waveform_boundary+1, std::string::npos);
            prepend_args.push_back("-S");
            prepend_args.push_back(waveform_name+"."+name_binding);
            prepend_args.push_back("-t");
            prepend_args.push_back(waveform_name+"."+name_binding);
        }
    }
    CF::ExecutableDevice::ProcessID_Type ret_pid;
    try {
        ret_pid = ExecutableDevice_impl::do_execute(name, options, parameters, prepend_args);
        this->addPid(ret_pid, app_id, component_id);
        this->addReservation( getComponentDescription(ret_pid) );
    } catch ( ... ) {
        throw;
    }
    return ret_pid;
}

void GPP_i::terminate (CF::ExecutableDevice::ProcessID_Type processId) throw (CORBA::SystemException, CF::ExecutableDevice::InvalidProcess, CF::Device::InvalidState)
{
    boost::recursive_mutex::scoped_lock lock(load_execute_lock);
    try {
      ExecutableDevice_impl::terminate(processId);
      this->removeReservation( getComponentDescription(processId)) ;
    }
    catch(...){
    }
    this->removePid(processId);
}


//
//
//  Executable/Device method overrides...
//
//

void GPP_i::updateUsageState()
{
  if (system_monitor->get_idle_percent() < modified_thresholds.cpu_idle) {
    if ( system_monitor->get_idle_average() < modified_thresholds.cpu_idle) 
      setUsageState(CF::Device::BUSY);
  }
  else if (system_monitor->get_mem_free() < (unsigned long)modified_thresholds.mem_free)
    setUsageState(CF::Device::BUSY);
  else if (this->getPids().size() == 0)
    setUsageState(CF::Device::IDLE);
  else
    setUsageState(CF::Device::ACTIVE);
}


/**
  override ExecutableDevice::set_resource_affinity to handle localized settings.
 */
void GPP_i::set_resource_affinity( const CF::Properties& options, const pid_t rsc_pid, const char *rsc_name, const std::vector<int> &bl )
 {
   // check if we override incoming affinity requests...
   if ( affinity.force_override ) {
     if ( redhawk::affinity::is_disabled() == false ) {
       LOG_WARN(GPP_i, "Enforcing GPP affinity property settings to resource, GPP/pid: " <<  label() << "/" << rsc_pid );
       if ( _apply_affinity( affinity, rsc_pid, rsc_name ) < 0 ) {
         throw redhawk::affinity::AffinityFailed("Failed to apply GPP affinity settings to resource");
       }
     }
     else {
       LOG_WARN(GPP_i, "Affinity processing disabled, unable to apply GPP affinity settings to resource, GPP/rsc/pid: " <<  label() << "/" << rsc_name << "/" << rsc_pid );
     }
   }
   else if ( affinity.deploy_per_socket && redhawk::affinity::has_nic_affinity(options) == false ) {

     if ( execPartitions.size() == 0 ) {
       LOG_WARN(GPP_i, "Skipping deploy_per_socket request. Reason: No execute partitions found, check numa settings. GPP/resource: " <<  label() << "/" << rsc_name );
       return;
     }

       // get next available socket to deploy on....
     if ( redhawk::affinity::is_disabled() == false ) {
       int psoc = _get_deploy_on_partition();
       if ( psoc < 0 ) {
         throw redhawk::affinity::AffinityFailed("Failed to apply PROCESSOR SOCKET affinity settings to resource");
       }
       LOG_DEBUG(GPP_i, "Enforcing PROCESSOR SOCKET affinity settings to resource, GPP/pid/socket: " <<  label() << "/" << rsc_pid << "/" << psoc );
       std::ostringstream os;
       os << psoc;
       if ( _apply_affinity( rsc_pid, rsc_name, "socket", os.str(), bl_cpus ) < 0 ) {
         throw redhawk::affinity::AffinityFailed("Failed to apply GPP affinity settings to resource");
       }
     }       
   }
   else {

     redhawk::affinity::CpuList blcpus;
     try {
       //blcpus = gpp::affinity::get_cpu_list( "cpu", affinity.blacklist_cpus );
       blcpus.resize(bl_cpus.size());
       std::copy( bl_cpus.begin(), bl_cpus.end(), blcpus.begin() );
     }
     catch( redhawk::affinity::AffinityFailed &ex) {
       LOG_ERROR(GPP_i, "Unable to process blacklist cpu specification reason:"  << ex.what() );
       throw;
     }

     //
     // Same flow as ExecutableDevice::set_resource_affinity
     //   
     try {
       if ( redhawk::affinity::has_affinity( options ) ) {
         LOG_DEBUG(GPP_i, "Has Affinity Options....GPP/Resource:" << label() << "/" << rsc_name);
         if ( redhawk::affinity::is_disabled() ) {
           LOG_WARN(GPP_i, "Resource has affinity directives but processing disabled, ExecDevice/Resource:" << 
                    label() << "/" << rsc_name);
         }
         else {
           LOG_DEBUG(GPP_i, "Calling set resource affinity.... GPP/Resource:" <<
                     label() << "/" << rsc_name);
           redhawk::affinity::set_affinity( options, rsc_pid, blcpus );
         }
       }
       else {
         LOG_TRACE(GPP_i, "No Affinity Options Found....GPP/Resource:" << label() << "/" << rsc_name);
       }
     }
     catch( redhawk::affinity::AffinityFailed &e) {
       LOG_WARN(GPP_i, "AFFINITY REQUEST FAILED: " << e.what() );
       throw;
     }

   }

}



int GPP_i::serviceFunction()
{

  // Check if any children died....
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(sig_fd, &readfds);
  struct timeval tv = {0, 50};
  struct signalfd_siginfo si;
  ssize_t s;

  if ( sig_fd > -1 ) {
    // don't care about writefds and exceptfds:
    select(sig_fd+1, &readfds, NULL, NULL, &tv);
    if (FD_ISSET(sig_fd, &readfds)) {
      LOG_TRACE(GPP_i, "Checking for signals from SIGNALFD......" << sig_fd);
      s = read(sig_fd, &si, sizeof(struct signalfd_siginfo));
      if (s != sizeof(struct signalfd_siginfo)){
        LOG_ERROR(GPP_i, "SIGCHLD handling error ...");
      }
 
      if ( si.ssi_signo == SIGCHLD) {
        LOG_TRACE(GPP_i, "Child died , pid .................................." << si.ssi_pid);
        sigchld_handler(si.ssi_signo);
      }
    }
  }

  boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
  boost::posix_time::time_duration dur = now -time_mark;
  if ( dur.total_milliseconds() < threshold_cycle_time ) {
    return NOOP;
  }
  
  time_mark = now;

  //
  // update any threshold limits that are based on the current system state
  //
  establishModifiedThresholds();
    
  // update data model for the GPP
  try {      
    std::for_each( data_model.begin(), data_model.end(), boost::bind( &Updateable::update, _1 ) );
    std::for_each( execPartitions.begin(), execPartitions.end(), boost::bind( &exec_socket::update, _1 ) );
    calculateSystemMemoryLoading();
  }
  catch( const boost::thread_resource_error& e ){
    std::stringstream errstr;
    errstr << "Error acquiring lock (errno=" << e.native_error() << " msg=\"" << e.what() << "\")";
    LOG_ERROR(GPP_i, __FUNCTION__ << ": " << errstr.str() );
    return NOOP;
  }

  if ( execPartitions.size() ) {
    // dump execute partition status... 
    ExecPartitionList::iterator iter =  execPartitions.begin();
    std::ostringstream ss;
    ss  << boost::format("%-6s %-4s %-7s %-7s %-7s ") % "SOCKET" % "CPUS" % "USER"  % "SYSTEM"  % "IDLE";
    LOG_DEBUG(GPP_i, ss.str()  );
    ss.clear();
    ss.str("");
    for ( ; iter != execPartitions.end(); iter++ ) {
    
      ss  << boost::format("%-6d %-4d %-7.2f %-7.2f %-7.2f ") % iter->id % iter->stats.get_ncpus() % iter->stats.get_user_percent()  % iter->stats.get_system_percent()  % iter->stats.get_idle_percent() ;
      LOG_DEBUG(GPP_i, ss.str()  );    
      ss.clear();
      ss.str("");
    }
  }

  for( size_t i=0; i<threshold_monitors.size(); ++i ) {
    threshold_monitors[i]->update();
    LOG_TRACE(GPP_i, __FUNCTION__ << ": resource_id=" << threshold_monitors[i]->get_resource_id() << 
              " threshold=" << threshold_monitors[i]->get_threshold() << " measured=" << threshold_monitors[i]->get_measured());
  }

  // update monitors to see if thresholds are exceeded
  std::for_each( threshold_monitors.begin(), threshold_monitors.end(), boost::bind( &Updateable::update, _1 ) );

  // update device usages state for the GPP
  updateUsageState();
    
  return NORMAL;
}


//
//
//  Property Callback Methods
//
//

void GPP_i::reservedChanged(const float *oldValue, const float *newValue)
{
  if(  newValue ) {
    reserved_capacity_per_component = *newValue;
    idle_capacity_modifier = 100.0 * reserved_capacity_per_component/((float)this->processor_cores);

    ExecPartitionList::iterator iter = execPartitions.begin();
    for( ; iter != execPartitions.end(); iter++ ) {
      iter->idle_cap_mod = 100.0 * reserved_capacity_per_component / ((float)iter->cpus.size());
    }
  }
}


void GPP_i::_affinity_changed( const affinity_struct *ovp, const affinity_struct *nvp ) {

  if ( ovp && nvp && *ovp == *nvp ) return;
  
  const affinity_struct nv = *nvp;

  RH_NL_TRACE("GPP", "BEGIN Affinty Settings Changed.............");

  if ( ovp )  {
    const affinity_struct ov = *ovp;
    LOG_DEBUG(GPP_i, "OV:                " );
    LOG_DEBUG(GPP_i, "OV: ov.policy/context " << ov.exec_directive_class << "/" << ov.exec_directive_value );
    LOG_DEBUG(GPP_i, "OV: ov.blacklist size " << ov.blacklist_cpus.size() );
    LOG_DEBUG(GPP_i, "OV: ov.force_override " << ov.force_override );
    LOG_DEBUG(GPP_i, "OV: ov.disabled       " << ov.disabled );
  }

  LOG_DEBUG(GPP_i, "NV:                " );
  LOG_DEBUG(GPP_i, "NV: nv.policy/context " << nv.exec_directive_class << "/" << nv.exec_directive_value );
  LOG_DEBUG(GPP_i, "NV: nv.blacklist size " << nv.blacklist_cpus.size() );
  LOG_DEBUG(GPP_i, "NV: nv.force_override " << nv.force_override );
  LOG_DEBUG(GPP_i, "NV: nv.disabled       " << nv.disabled );

  // change affinity struct to affinity spec..
  redhawk::affinity::AffinityDirective value;
  if ( nv.exec_directive_class == "socket" ) {value.first ="socket"; value.second = nv.exec_directive_value; }
  if ( nv.exec_directive_class == "cpu" ) { value.first ="cpu"; value.second = nv.exec_directive_value; }
  if ( nv.exec_directive_class == "nic" ) { value.first ="nic"; value.second = nv.exec_directive_value; }
  if ( nv.exec_directive_class == "cgroup" ){ value.first ="cgroup"; value.second = nv.exec_directive_value; }


  // change affinity struct to affinity spec..
  bl_cpus.clear();
  try {
    redhawk::affinity::CpuList cpus = gpp::affinity::get_cpu_list( "cpu", nv.blacklist_cpus );
    bl_cpus.resize(cpus.size());
    std::copy( cpus.begin(), cpus.end(), bl_cpus.begin());
  }
  catch( redhawk::affinity::AffinityFailed &ex) {
    RH_NL_ERROR("GPP", "Unable to process blacklist cpu specification reason:"  << ex.what() );
  }

  {
    std::stringstream os;
    CpuList::const_iterator iter = bl_cpus.begin();
    for ( ; iter != bl_cpus.end(); iter++ ) {
      os << *iter;
      if ( iter +1 != bl_cpus.end() ) os << ",";
    }
    RH_NL_DEBUG("GPP", "Affinity Requested: Blacklist " << bl_cpus.size() << " str == " << os.str() );
  }

  // 
  // need to determine if cpu monitor list to watch is different...than current list
  // Right now we monitor all cpus on the host, but we can limit the view
  // by adding another property to the affinity_struct ... that follows format
  // from numa_parse_XXX methods
  //
  //
  redhawk::affinity::CpuList cpus;
  try {
    cpus = gpp::affinity::get_cpu_list( "node","all" );
  }
  catch( redhawk::affinity::AffinityFailed &ex) {
    RH_NL_ERROR("GPP", "Unable to process white listed cpu specification, reason:"  << ex.what() );
  }

  wl_cpus.clear();
  for( uint32_t i=0; i< cpus.size(); i++ ) {
    if ( std::count( bl_cpus.begin(), bl_cpus.end(), cpus[i] ) == 0 ) wl_cpus.push_back( cpus[i] );
  }

  RH_NL_DEBUG("GPP", "Affinity Force Override, force_override=" << nv.force_override);
  RH_NL_INFO("GPP", "Affinity Disable State,  disabled=" << nv.disabled);
  if ( nv.disabled ) {
    RH_NL_INFO("GPP", "Disabling affinity processing requests.");
    redhawk::affinity::set_affinity_state( nv.disabled );
  }

  // apply changes to member variable
  affinity = nv;

  _set_processor_monitor_list( wl_cpus );

  // reset idle idle capcity monitor... idle threshold gets calculated during each loop of svc func.
  processor_cores = wl_cpus.size();
  if ( processor_cores == 0 ) processor_cores = boost::thread::hardware_concurrency();
  idle_capacity_modifier = 100.0 * reserved_capacity_per_component/((float)processor_cores);
  
  // reset capacity modifier for all execution partitions
  ExecPartitionList::iterator iter = execPartitions.begin();
  for( ; iter != execPartitions.end(); iter++ ) {
    iter->idle_cap_mod = 100.0 * reserved_capacity_per_component / ((float)iter->cpus.size());
  }

  RH_NL_TRACE("GPP", "END Affinty Settings Changed.............");
  
}

//
//
//  Allocation Callback Methods
//
//

bool GPP_i::allocateCapacity_nic_allocation(const nic_allocation_struct &alloc)
{
    boost::mutex::scoped_lock lock(pidLock);
    std::string  except_msg("Invalid allocation");
    bool success=false;
    LOG_TRACE(GPP_i, __FUNCTION__ << ": Allocating nic_allocation (identifier=" << alloc.identifier << ")");
    try
    {
        LOG_TRACE(GPP_i, __FUNCTION__ << ": ALLOCATION: { identifier: \"" << alloc.identifier << "\", data_rate: " << alloc.data_rate << ", data_size: " << alloc.data_size << ", multicast_support: \"" << alloc.multicast_support << "\", ip_addressable: \"" << alloc.ip_addressable << "\", interface: \"" << alloc.interface << "\" }");
        success = nic_facade->allocate_capacity(alloc);
        
        if( success )
        {
            nic_allocation_status_struct_struct status;
            for( size_t i=0; i<nic_allocation_status.size(); ++i )
            {
                if( nic_allocation_status[i].identifier == alloc.identifier )
                {
                  status = nic_allocation_status[i];
                  // need to check if processor socket servicing interface has enough idle capacity
                  if ( _check_exec_partition( status.interface ) == true ) {
                      LOG_TRACE(GPP_i, __FUNCTION__ << ": SUCCESS: { identifier: \"" << status.identifier << "\", data_rate: " << status.data_rate << ", data_size: " << status.data_size << ", multicast_support: \"" << status.multicast_support << "\", ip_addressable: \"" << status.ip_addressable << "\", interface: \"" << status.interface << "\" }");
                    break;
                  }
                  else {
                    except_msg = "Not enough processor capacity on socket that services NIC:" + status.interface;
                    throw -1;
                  }
                   
                }
            }
        }
        
        return success;
    }
    //catch( const NicAllocator::InvalidAllocation& e )
    catch( ... )
    {
      // if we made an allocation the unroll
      try {
        if ( success ) nic_facade->deallocate_capacity(alloc);
      }
      catch(...)
        {};

        CF::Properties errprops;
        errprops.length(1);
        errprops[0].id = "nic_allocation";
        errprops[0].value <<= alloc;
        throw CF::Device::InvalidCapacity(except_msg.c_str(), errprops);
    }
}


void GPP_i::deallocateCapacity_nic_allocation(const nic_allocation_struct &alloc)
{
  boost::mutex::scoped_lock lock(pidLock);
  LOG_TRACE(GPP_i, __FUNCTION__ << ": Deallocating nic_allocation (identifier=" << alloc.identifier << ")");
  try {
      LOG_DEBUG(GPP_i, __FUNCTION__ << ": { identifier: \"" << alloc.identifier << "\", data_rate: " << alloc.data_rate << ", data_size: " << alloc.data_size << ", multicast_support: \"" << alloc.multicast_support << "\", ip_addressable: \"" << alloc.ip_addressable << "\", interface: \"" << alloc.interface << "\" }");
      nic_facade->deallocate_capacity(alloc);
    }
  catch( ... )
    {
      CF::Properties errprops;
      errprops.length(1);
      errprops[0].id = "nic_allocation";
      errprops[0].value <<= alloc;
      throw CF::Device::InvalidCapacity("Invalid allocation", errprops);
    }
}

void GPP_i::deallocateCapacity (const CF::Properties& capacities) throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CORBA::SystemException)
{
    GPP_base::deallocateCapacity(capacities);
}
CORBA::Boolean GPP_i::allocateCapacity (const CF::Properties& capacities) throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CF::Device::InsufficientCapacity, CORBA::SystemException)
{
    bool retval = GPP_base::allocateCapacity(capacities);
    return retval;
}


bool GPP_i::allocate_diskCapacity(const double &value) {

  if (isBusy()) {
    return false;
  }
   
  return true;
}

void GPP_i::deallocate_diskCapacity(const double &value) {
  return;
}

bool GPP_i::allocate_memCapacity(const int64_t &value) {

  if (isBusy()) {
    return false;
  }

  // get current available free memory from system
  if ( system_monitor->get_mem_free() < (uint64_t)value ) 
    return false;

  return true;
}

void GPP_i::deallocate_memCapacity(const int64_t &value) {
  return;
}



bool GPP_i::allocate_loadCapacity(const double &value) {

  if (isBusy()) {
    return false;
  }
    
  if ( (loadCapacity_counter - value ) < 0.0  )
    return false;
                                          
  loadCapacity_counter -= value;
  updateUsageState();
  return true;
}

void GPP_i::deallocate_loadCapacity(const double &value) {
  loadCapacity_counter += value;
  updateThresholdMonitors();
  updateUsageState();
  return;
}



//
//
// Support Methods
//
//


void GPP_i::send_threshold_event(const threshold_event_struct& message)
{
  LOG_INFO(GPP_i, __FUNCTION__ << ": " << message.message );
  MessageEvent_out->sendMessage(message);
}

void GPP_i::sendChildNotification(const std::string &comp_id, const std::string &app_id)
{
  LOG_INFO(GPP_i, "Child termination notification on the IDM channel : comp:" << comp_id  << " app:" <<app_id);
  redhawk::events::DomainEventWriter writer(idm_publisher);
  writer.sendComponentTermination( _identifier.c_str(), app_id.c_str(), comp_id.c_str() );
}



void GPP_i::updateThresholdMonitors()
{
  boost::mutex::scoped_lock lock(pidLock);
  MonitorSequence::iterator iter=threshold_monitors.begin();
  for( ; iter != threshold_monitors.end(); iter++ ) {
    ThresholdMonitorPtr monitor=*iter;
    monitor->update();
    LOG_TRACE(GPP_i, __FUNCTION__ << ": resource_id=" << monitor->get_resource_id() << " threshold=" << monitor->get_threshold() << " measured=" << monitor->get_measured());
  }
}


void GPP_i::establishModifiedThresholds()
{
  boost::mutex::scoped_lock lock(pidLock);
  this->modified_thresholds.cpu_idle = this->thresholds.cpu_idle + (this->idle_capacity_modifier * this->reservations.size()) + this->loadCapacity_counter;
  LOG_TRACE(GPP_i, __FUNCTION__ << "ModifyThreshold : " << std::endl << 
           " modified_threshold=" << modified_thresholds.cpu_idle << std::endl << 
           " system: idle: " << system_monitor->get_idle_percent() << std::endl << 
           "         idle avg: " << system_monitor->get_idle_average() << std::endl << 
           " threshold: " << thresholds.cpu_idle << std::endl <<
           " modifier: " << idle_capacity_modifier << std::endl <<
           " reservations: " << reservations.size() << std::endl <<
           " loadCapacity_counter: " << loadCapacity_counter );
}

void GPP_i::calculateSystemMemoryLoading() {
  LOG_TRACE(GPP_i, __FUNCTION__ << ": memCapacity=" << memCapacity << " sys_monitor.get_mem_free=" << system_monitor->get_mem_free() );
  memCapacity = system_monitor->get_mem_free();
}


void GPP_i::sigchld_handler(int sig)
{
    int status;
    pid_t child_pid;
        
    while( (child_pid = waitpid(-1, &status, WNOHANG)) > 0 )
    {
      try {
        component_description retval;
        if ( devicePtr) {
          retval = devicePtr->getComponentDescription(child_pid);
          sendChildNotification(retval.identifier, retval.appName);
        }
        break;
      } catch ( ... ) {
        try {
          sendChildNotification("Unknown", "Unknown");
        } catch ( ... ) {
        }
      }
    }
    
    if( child_pid == -1 && errno != ECHILD )
    {
        // Error
        perror("waitpid");
    }

}


std::vector<int> GPP_i::getPids()
{
    boost::mutex::scoped_lock lock(pidLock);
    std::vector<int> keys;
    for (ProcessMap::iterator it=pids.begin();it!=pids.end();it++) {
        keys.push_back(it->first);
    }
    return keys;
}

void GPP_i::addPid(int pid, std::string appName, std::string identifier)
{
    boost::mutex::scoped_lock lock(pidLock);
    if (pids.find(pid) == pids.end()) {
        component_description tmp;
        tmp.appName = appName;
        tmp.identifier = identifier;
        pids[pid] = tmp;
    }
}

GPP_i::component_description GPP_i::getComponentDescription(int pid)
{
    boost::mutex::scoped_lock lock(pidLock);
    ProcessMap::iterator it=pids.find(pid);
    if (it == pids.end())
        throw std::invalid_argument("pid not found");
    return it->second;
}

void GPP_i::removePid(int pid)
{
    boost::mutex::scoped_lock lock(pidLock);
    ProcessMap::iterator it=pids.find(pid);
    if (it == pids.end())
        return;
    pids.erase(it);
}

void GPP_i::addReservation( const component_description &component)
{
    boost::mutex::scoped_lock lock(pidLock);
    this->reservations.push_back(component);
}

void GPP_i::removeReservation( const component_description &component)
{
    boost::mutex::scoped_lock lock(pidLock);
    ProcessList::iterator it = std::find(this->reservations.begin(), this->reservations.end(), component);
    if (it != this->reservations.end()) {
        this->reservations.erase(it);
    }
    it = std::find(this->tabled_reservations.begin(), this->tabled_reservations.end(), component);
    if (it != this->tabled_reservations.end()) {
        this->tabled_reservations.erase(it);
    }
}

void GPP_i::tableReservation( const component_description &component)
{
    ProcessList::iterator it = std::find(this->reservations.begin(), this->reservations.end(), component);
    if (it != this->reservations.end()) {
        this->tabled_reservations.push_back(*it);
        this->reservations.erase(it);
    }
}

void GPP_i::restoreReservation( const component_description &component)
{
    ProcessList::iterator it = std::find(this->tabled_reservations.begin(), this->tabled_reservations.end(), component);
    if (it != this->tabled_reservations.end()) {
        this->reservations.push_back(*it);
        this->tabled_reservations.erase(it);
    }
}


int GPP_i::_apply_affinity( const affinity_struct &nv,
                            const pid_t rsc_pid, 
                            const char *rsc_name  ) {

  return _apply_affinity( rsc_pid, rsc_name, nv.exec_directive_class, nv.exec_directive_value, bl_cpus );
}


int GPP_i::_apply_affinity(  const pid_t rsc_pid, 
                             const char *rsc_name,
                             const std::string &dir_class,
                             const std::string &dir_value,
                             const CpuList &bl_cpus) {

  int retval=0;
  // change affinity struct to affinity spec..
  redhawk::affinity::AffinityDirectives pol;
  redhawk::affinity::AffinityDirective value;
  value.first = dir_class;
  value.second = dir_value;
  pol.push_back(value);

  redhawk::affinity::CpuList blcpus;
  try {
    //bl_cpus = gpp::affinity::get_cpu_list( "cpu", nv.blacklist_cpus );
    blcpus.resize(bl_cpus.size());
    std::copy( bl_cpus.begin(), bl_cpus.end(), blcpus.begin() );
  }
  catch( redhawk::affinity::AffinityFailed &ex) {
    RH_NL_ERROR("GPP", "Unable to process blacklist cpu specification reason:"  << ex.what() );
    throw;
  }

  RH_NL_DEBUG("GPP", "Affinity Override Requested (processor socket),  pid/name: " << rsc_pid << "/" << rsc_name <<" pol/value "  << value.first << "/" << value.second );
  
  // apply affinity changes to the process
  try {
    if ( redhawk::affinity::set_affinity( pol, rsc_pid, blcpus) != 0 )  {
      RH_NL_WARN("GPP", "Unable to set affinity for process, pid/name: " << rsc_pid << "/" << rsc_name );
    }
  }
  catch( redhawk::affinity::AffinityFailed &ex) {
    RH_NL_ERROR("GPP", "Unable to set affinity, reason:"  << ex.what() );
    throw;
  }

  return retval;
}


void GPP_i::_set_processor_monitor_list( const CpuList &cpus ) {
  std::ostringstream os;  
  if ( cpus.size() == 0 ) {
    os << "all";
  }
  else {
    CpuList::const_iterator iter = cpus.begin();
    for ( ; iter != cpus.end(); iter++ ) {
      os << *iter;
      if ( iter +1 != cpus.end() ) os << ",";
    }
  }

  RH_NL_DEBUG("GPP", "Processor white list " << wl_cpus.size() << " str == " << os.str() );
  processor_monitor_list = os.str();
}


bool GPP_i::_check_exec_partition( const std::string &iface ){

  bool retval=false;
  if ( execPartitions.size() == 0 ) {
    RH_NL_INFO("GPP", " _check_exec_partition: no exec partitions available.. iface:" << iface );
    retval=true;
  }
  else {
    int  soc=gpp::affinity::find_socket_for_interface( iface );
    RH_NL_INFO("GPP", " _check_exec_partition: iface:" << iface << "/" << soc );
    if  ( soc > -1  ) {
    
      if ( (uint32_t)soc < execPartitions.size() ) {
        const exec_socket &ep = execPartitions[soc];
        // get modified idle threshold value
        double m_idle_thresh = ep.idle_threshold + ( ep.idle_cap_mod * reservations.size()) + 
          (float)loadCapacity_counter/(float)ep.cpus.size();
        RH_NL_DEBUG("GPP", " Checking Execution Partition for an NIC interface iface/socket " << iface << "/" << soc << ") IDLE: actual/avg/threshold limit/modified " <<  
                   ep.get_idle_percent() << "/" << ep.get_idle_average() << "/" << ep.idle_threshold << "/" << m_idle_thresh );
        if ( ep.get_idle_percent() > m_idle_thresh ) {
          retval = true;
        }
        if ( ep.get_idle_average() > m_idle_thresh ) {
          retval = true;
        }
      }
    }
  }
  
  return retval;
}

int  GPP_i::_get_deploy_on_partition() {

  int psoc=-1;
  ExecPartitionList::iterator  iter = execPartitions.begin();
  for( ;  iter != execPartitions.end(); iter++ ) {
    // get modified idle threshold value
    double m_idle_thresh = iter->idle_threshold + ( iter->idle_cap_mod * reservations.size()) + 
      (float)loadCapacity_counter/(float)iter->cpus.size();
    RH_NL_DEBUG("GPP", " Looking for execute partition (processor socket:" << iter->id << ") IDLE: actual/avg/threshold limit/modified " <<  
		iter->get_idle_percent() << "/" << iter->get_idle_average() << "/" << iter->idle_threshold << "/" << m_idle_thresh );
    if ( iter->get_idle_percent() > m_idle_thresh ) {
      psoc=iter->id;
      break;
    }
    if ( iter->get_idle_average() > m_idle_thresh ) {
      psoc=iter->id;
      break;
    }
  }

  if ( psoc > -1 ) { RH_NL_INFO("GPP", " Deploy resource on selected SOCKET PARTITON, socket:" << psoc ); }
  return psoc;
}
