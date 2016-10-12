#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <string>
#include <sched.h>
#include <boost/algorithm/string.hpp>
#ifdef HAVE_LIBNUMA
#include <numa.h>
#endif
#ifdef HAVE_OMNIORB4_CORBA_H
#include "omniORB4/CORBA.h"
#endif
#include "ossie/affinity.h"
#include "ossie/debug.h"

//
// Predfined Property Settings for Affinity requests 
//
// id = affinity::exec_directive_class 
//   Affinity directive classifications for to use, valid entries are:
//    scoket - assign processing affinity to a numa node
//    cpu - assign processing affinity to a list of cpu ids
//    nic - assign processing affinity for cpu id or nodes that service hardware interrupts for the nic
//    cgroup - assign processing affinity to a prexisting cgroup name
//
// id = affinity::exec_directive_value
//   The exec_directive_value meaning for the exec_directive_class values
//     socket : parsable node string as defined in the numa library page
//     cpu : parsable cpu idstring as defined in the numa library page
//     nic : interface string naem, assign processing affinity for cpu id or nodes that service hardware interrupts for the nic
//     cgroup : name of the cgroup to add the resource to (i.e process id of resource). Follows rhel/centos group directory 
//              path ( "/cgroup/cgroup_name/task" )

static std::string _AFFINITY_PROPS_( "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
<!DOCTYPE properties PUBLIC \"-//JTRS//DTD SCA V2.2.2 PRF//EN\" \"properties.dtd\"> \
<properties> \
    <simple id=\"affinity::exec_directive_class\" mode=\"readwrite\" name=\"exec_directive_class\" type=\"string\" optional=\"false\"> \
      <enumerations> \
        <enumeration label=\"socket\" value=\"socket\"/> \
        <enumeration label=\"nic\" value=\"nic\"/> \
        <enumeration label=\"cpu\" value=\"cpu\"/> \
        <enumeration label=\"cgroup\" value=\"cgroup\"/> \
      </enumerations> \
      <kind kindtype=\"property\"/> \
      <kind kindtype=\"configure\"/> \
      <action type=\"external\"/> \
    </simple> \
   <simple id=\"affinity::exec_directive_value\" mode=\"readwrite\" name=\"exec_directive_value\" type=\"string\" optional=\"false\"> \
      <description>The context specification for the exec_directive_class.   See numa library manpage for node (socket) and cpu list specifications.  For cgroup option then a pre-existing cgroup name is required.</description> \
      <kind kindtype=\"property\"/> \
      <kind kindtype=\"configure\"/> \
      <action type=\"external\"/> \
    </simple> \
</properties> \
 ");

namespace  redhawk {

  namespace affinity {

    //
    // promote nic affinity to a socket if all associated cpus for the interface are blacklisted
    //
    static   bool                _promote_nic_to_socket = true;

    //
    // disable affinity processing
    //
    static   bool                _affinity_enabled  = true;

    //
    // Allow for alternate affinity processing
    //
    static   SetAffinityFunc     _affinity_override_func = NULL;

    static  rh_logger::LoggerPtr  _affinity_logger = rh_logger::Logger::getLogger("redhawk::affinity");


    //
    // get root directory of mount point for CGROUP processing
    //
    const std::string get_cgroup_root() {
      std::string cgroup_root("/cgroup");
      if ( std::getenv("REDHAWK_CGROUP_ROOT") != NULL ) {
        cgroup_root = std::getenv("REDHAWK_CGROUP_ROOT");
      }
      return cgroup_root;
    }

    //
    // get root directory of mount point for CPUSET processing
    //
    const std::string get_cpuset_root() {
      std::string croot("/dev/cpuset");
      if ( std::getenv("REDHAWK_CPUSET_ROOT") != NULL ) {
        croot = std::getenv("REDHAWK_CPUSET_ROOT");
      }
      return croot;
    }

    //
    // Return XML formatted property definition for well known affinity processing
    //
    std::string get_property_definitions() {
      return  _AFFINITY_PROPS_;
    }

    //
    // check if affinity is disabled
    //
    bool is_disabled() {
      bool retval=false;
      if ( !_affinity_enabled  || std::getenv("REDHAWK_DISABLE_AFFINITY") != NULL ) {
        //RH_DEBUG(_affinity_logger, "Affinity processing is disabled.");
        retval=true;
      }
      return retval;
    }

    //
    // set affinity to be disabled
    //
    void set_affinity_state( const bool onoff) {
      _affinity_enabled = onoff;
    }

    //
    // set affinity logger
    //
    void set_affinity_logger( rh_logger::LoggerPtr newLogger) {
      _affinity_logger = newLogger;
    }

    rh_logger::LoggerPtr get_affinity_logger() {
      return _affinity_logger;
    }

    void set_nic_promotion( const bool onoff ) {
      _promote_nic_to_socket = onoff;
    }

    const bool get_nic_promotion() {
      return _promote_nic_to_socket;
    }

    void override_set( SetAffinityFunc newFunc ) {
      _affinity_override_func = newFunc;
    }


    /*
       identify_cpus 

       From a specified network interface determine the list of CPUs that service interrupts
       for that interface.

     */
    CpuList identify_cpus( const std::string &iface ) {

      CpuList  cpus;
      std::string pintr("/proc/interrupts");
      std::ifstream in(pintr.c_str(), std::ifstream::in );
      if ( in.fail() ) {
        RH_ERROR(_affinity_logger, "Unable to access /proc/interrupts");
        return cpus;
      }

      std::string line;
      while( std::getline( in, line ) ) {
        // check if the device is our interface
        RH_TRACE(_affinity_logger, "Processing /proc/interrupts.... line:" << line);
        if ( line.rfind(iface) != std::string::npos ) {
          std::istringstream iss(line);
          int parts=0;
          do {
            std::string tok;
            iss>>tok;
	    // skip interrupt number and iface
	    if ( parts > 0 and tok != iface ) {
              std::istringstream iss(tok);
              int icnt;
              iss >> icnt;
              if ( icnt > 0 ) {
                RH_TRACE(_affinity_logger, "identify cpus: Adding CPU : " << parts-1);
                cpus.push_back(parts-1);
              }
	    }
	    parts++;
          }while(iss);

        }
      }

      CpuList::iterator citer=cpus.begin();
      for (; citer != cpus.end(); citer++) {
        RH_DEBUG(_affinity_logger, "identified CPUS iface/cpu ...:" << iface << "/" << *citer);
      }
    
      return cpus;
    }

    int   find_socket_for_interface ( const std::string &iface , const bool findFirst, const CpuList &bl ){

      int retval=-1;
      
      // Determine cpu list by interrupts assigned for the specified NIC
      CpuList cpulist = identify_cpus(iface);
      if ( cpulist.size() > 0 ) {
        int psoc=-1;
#if HAVE_LIBNUMA
        int soc=-1;
        for( int i=0; i < (int)cpulist.size();i++ ) {
          RH_DEBUG(_affinity_logger, "Finding (processor socket) for NIC:" << iface << " socket :" << numa_node_of_cpu(cpulist[i]) );
          if ( std::count( bl.begin(), bl.end(), cpulist[i] ) != 0 ) continue;

          soc = numa_node_of_cpu(cpulist[i]);
          if ( soc != psoc && psoc != -1 && !findFirst ) {
            RH_WARN(_affinity_logger, "More than 1 socket servicing NIC:" << iface);
            psoc=-1;
            break;
          }
          psoc=soc;
          if( findFirst ) break;
        }
#endif
        retval=psoc;
      }
      

      return retval;

    }  

    /*
        Return list of cpus either associated with a node (i.e. processor socket), or 
        from properly formatted string support by numa library parse methods
     */
    CpuList get_cpu_list( const std::string &list_type,  const std::string &context )
      throw (AffinityFailed)
    {
      CpuList cpu_list;
      if ( is_disabled() ) {
        return cpu_list;
      }
      
#ifdef HAVE_LIBNUMA
      if ( list_type == "socket" || list_type == "node" ) {
          std::string nodestr = context;
          struct bitmask *node_mask = numa_parse_nodestring((char *)nodestr.c_str()); 
          if ( !node_mask )  {
            throw AffinityFailed("Processor Socket affinity failed, unable to parse:  " + nodestr);
          }

          bitmask *cpu_mask = numa_allocate_cpumask(); 
          if ( !cpu_mask ) {
            throw AffinityFailed("Unable to allocate cpu mask");
          } 

          // for each bit set in the mask then get cpu list
          int nbytes = numa_bitmask_nbytes(node_mask);
          for (int i=0; i < nbytes*8; i++ ){
            if ( numa_bitmask_isbitset( node_mask, i ) ) {
              numa_node_to_cpus( i, cpu_mask );
              
              // foreach cpu identified add to list
              int nb = numa_bitmask_nbytes(cpu_mask);
              for (int j=0; j < nb*8; j++ ){
                if ( numa_bitmask_isbitset( cpu_mask, j ) ) {
                  cpu_list.push_back( j );
                }
              }
            }
          }

      }

      if ( list_type == "cpu" ) {
        std::string cpustr = context;
        struct bitmask *cpu_mask = numa_parse_cpustring((char*)cpustr.c_str());    
          if ( !cpu_mask ) {
            throw AffinityFailed("CPU affinity failed, unable to parse: <" + cpustr + ">" );
          }

          // foreach cpu identified add to list
          int nb = numa_bitmask_nbytes(cpu_mask);
          for (int j=0; j < nb*8; j++ ){
            if ( numa_bitmask_isbitset( cpu_mask, j ) ) {
              cpu_list.push_back( j );
            }
          }
      }
#endif
      return cpu_list;
    }

    /*
       Determine if a CF Properties set has affinity namespaced properties
     */
    bool has_affinity( const CF::Properties& options ) {
      bool retval=false;

      RH_TRACE(_affinity_logger, "Affinity Options,  list size:" << options.length() );
      for (uint32_t i = 0; i < options.length(); ++i) {
        const std::string id=(const char *)options[i].id;
        RH_TRACE(_affinity_logger, "Affinity Options - option..." << id << "/" << ossie::any_to_string(options[i].value));        
        if ( boost::istarts_with(id, redhawk::affinity::AFFINITY_ID ) == true ) {
          RH_TRACE(_affinity_logger, "Affinity Option - AFFINITY options..." << id << "/" << ossie::any_to_string(options[i].value));
          retval=true;
          break;
        }
      }
      return retval;
    }



    bool has_nic_affinity( const CF::Properties& options )
    {
      bool retval = false;
      AffinityDirectives spec;

      if ( is_disabled() ) {
        return 0;
      }
      
      const redhawk::PropertyMap ops(options);
      std::string aid  = AFFINITY_ID;
      if ( ops.contains(aid) == false ) {
        aid  = boost::to_upper_copy( aid );
        if ( ops.contains(aid) == false ) {
          return retval;
        }
      }
         
      std::string pol;
      const redhawk::PropertyMap affinity_map(ops[aid].asProperties());
      redhawk::PropertyMap::const_iterator iter = affinity_map.begin();
      for ( ; iter != affinity_map.end(); iter++ ) {
        
        if ( iter->getId() == "affinity::exec_directive_class" ) {
          RH_TRACE(_affinity_logger, "has_nic_affinity affinity::exec_directive_class ...:" << affinity_map["affinity::exec_directive_class"].toString() );
          RH_TRACE(_affinity_logger, "has_nic_affinity affinity::exec_directive_value ...:" << affinity_map["affinity::exec_directive_value"].toString() );
          try {
            pol = affinity_map["affinity::exec_directive_class"].toString();
            if ( pol == "nic" ) {
              retval=true;
              break;
            }
          }
          catch(...){
          }
        }

        if ( iter->getId() == "nic" ) {
          retval = true;
          break;
          RH_DEBUG(_affinity_logger, "has_nic_affinity (oldstyle) nic ..:"  );
        }

      }

      return retval;
    }


    AffinityDirectives convert_properties(const CF::Properties& options ) 
      throw (AffinityFailed)
    {
      AffinityDirectives spec;

      const redhawk::PropertyMap ops(options);
      std::string aid  = AFFINITY_ID;
      if ( ops.contains(aid) == false ) {
        aid  = boost::to_upper_copy( aid );
        if ( ops.contains(aid) == false ) {
          throw AffinityFailed("No Affinity property provided");
        }
      }
         
      AffinityDirective pol;
      const redhawk::PropertyMap affinity_map(ops[aid].asProperties());
      redhawk::PropertyMap::const_iterator iter = affinity_map.begin();
      for ( ; iter != affinity_map.end(); iter++ ) {
        
        if ( iter->getId() == "affinity::exec_directive_class" ) {
          RH_TRACE(_affinity_logger, "set_affinity affinity::exec_directive_class ...:" << affinity_map["affinity::exec_directive_class"].toString() );
          RH_TRACE(_affinity_logger, "set_affinity affinity::exec_directive_value ...:" << affinity_map["affinity::exec_directive_value"].toString() );
          try {
            pol.first = affinity_map["affinity::exec_directive_class"].toString();
            pol.second = affinity_map["affinity::exec_directive_value"].toString();
            spec.push_back( pol );
          }
          catch(...){
          }
        }

        if ( iter->getId() == "nic" ) {
          pol.first = "nic";
          pol.second = affinity_map["nic"].toString();
          spec.push_back( pol );
          RH_DEBUG(_affinity_logger, "set_affinity (oldstyle) nic ..:" << pol.first << "/" << pol.second );
        }

      }

      if ( affinity_map.contains("socket") ) {
        pol.first = "socket";
        pol.second = affinity_map["socket"].toString();
        RH_DEBUG(_affinity_logger, "set_affinity. processor socket ..:" << pol.first << "/" << pol.second );
        spec.push_back( pol );
      }

      if ( affinity_map.contains("cpu") ) {
        pol.first = "cpu";
        pol.second = affinity_map["cpu"].toString();
        RH_DEBUG(_affinity_logger, "set_affinity. cpu ..:" << pol.first << "/" << pol.second );
        spec.push_back( pol );
      }

      if ( affinity_map.contains("cpuset") ) {
        pol.first = "cpuset";
        pol.second = affinity_map["cpuset"].toString();
        spec.push_back( pol );
      }

      if ( affinity_map.contains("cgroup") ) {
        pol.first = "cgroup";
        pol.second = affinity_map["cgroup"].toString();
        RH_DEBUG(_affinity_logger, "set_affinity. cgroup ..:" << pol.first << "/" << pol.second );
        spec.push_back( pol );
      }

      return spec;
    }


    /*
       Called from waveform and node deployers that pass in affinity directives via a
       CF::Properties set

       Translates the CF::Properties to a affinity directives, then call set_affinity with this dictionary

     */
    int set_affinity( const CF::Properties& options, const pid_t pid, const CpuList &blacklist ) 
      throw (AffinityFailed)
    {
      AffinityDirectives spec;

      if ( is_disabled() ) {
        return 0;
      }
      
      spec = convert_properties( options );

      return set_affinity(spec, pid, blacklist );
    }

    int set_affinity( const AffinityDirectives &spec, const pid_t pid, const CpuList &blacklist) 
      throw (AffinityFailed)
    {
      if ( is_disabled() ) {
        return 0;
      }

      if ( _affinity_override_func ) return (_affinity_override_func)( spec, pid, blacklist );

      CpuList::const_iterator citer=blacklist.begin();
      for (; citer != blacklist.end(); citer++) {
        RH_DEBUG(_affinity_logger, "BlackList ...:" << *citer);
      }

      AffinityDirectives::const_iterator piter = spec.begin();
      for ( int cnt=0; piter != spec.end(); piter++, cnt++ ) {
        AffinityDirective affinity_spec = *piter;
        RH_DEBUG(_affinity_logger, " cnt:" << cnt << " Processing Affinity pid: " << pid << " " << affinity_spec.first << ":" << affinity_spec.second );

#ifdef HAVE_LIBNUMA
        //
        // nic -- Determine cpu list by interrupts assigned for the specified NIC
        //
        if ( affinity_spec.first == "nic" ) {
          std::string iface = affinity_spec.second;
          // Determine cpu list by interrupts assigned for the specified NIC
          CpuList cpulist = identify_cpus(iface);
          
          // if no cpus identified then issue warning
          if ( cpulist.size() > 0  ) {

            // check if black list is specified... if not then use node based affinity
            if ( blacklist.size() == 0 && getpid() == pid ) {   // are we the same process,  then use node binding method
              bitmask *node_mask = numa_allocate_nodemask();
              if ( !node_mask )  {
                throw AffinityFailed("Unable to allocate node mask");
              }

              for( int i=0; i < (int)cpulist.size();i++ ) {
                RH_DEBUG(_affinity_logger, "Setting NIC (processor socket select) available socket :" << numa_node_of_cpu(cpulist[i]) );
                numa_bitmask_setbit(node_mask, numa_node_of_cpu(cpulist[i]) );
              }

              RH_DEBUG(_affinity_logger, "Setting NIC (processor socket select) affinity constraint: :" << iface );
              numa_bind(node_mask);
              numa_bitmask_free(node_mask);
            }
            else {

              int cpus=0;
              for( int i=0; i < (int)cpulist.size();i++ ) {
                // check if cpu id is not in blacklist
                if ( std::count( blacklist.begin(), blacklist.end(), cpulist[i] ) == 0  ) cpus++;
              }

              //
              // For nic based affinity and blacklisted cpus... 
              //
              // Find all the cpus that service interrupts for the specified interfaces
              //
              //   if interface is serviced by a single cpu and promote to socket flag is on
              //    get list of cpus for the processor socket servicing the interface...
              //    apply blacklist
              //    specify affinity with remaining cpu list
              //
              //   if interface is serviced by single cpu and blacklisted, and promote to socket flag is on
              //    get list of cpus for the processor socket servicing the interface...
              //    apply blacklist
              //    specify affinity with remaining cpu list
              //    
              //   otherwise ... 
              //    from the list of cpus for the interface, apply blacklist then apply as affinity 
              //
              if ( (cpulist.size() == 1 && get_nic_promotion()) || ( cpus == 0 && get_nic_promotion() ) ) {
                int cpuid = cpulist[0];
                std::ostringstream os;
                os << numa_node_of_cpu( cpuid );
                CpuList tlist = get_cpu_list( "socket", os.str() );
                RH_INFO(_affinity_logger, "Promoting NIC affinity to PID:" << pid << " SOCKET:" << os.str() );

                cpulist.clear();
                for( int i=0; i < (int)tlist.size();i++ ) {
                  if ( tlist[i] == cpuid ) continue;
                  cpulist.push_back( tlist[i] );
                }
              }
              
              cpus=0;
              for( int i=0; i < (int)cpulist.size();i++ ) {
                // check if cpu id is not in blacklist
                if ( std::count( blacklist.begin(), blacklist.end(), cpulist[i] ) == 0  ) cpus++;
              }
            
              if ( cpus > 0 ) {  // use cpulist to bind process
                bitmask *cpu_mask = numa_allocate_cpumask();  
                if ( !cpu_mask )  {
                  throw AffinityFailed("Unable to allocate node mask");
                }

                for( int i=0; i < (int)cpulist.size();i++ ) {
                  // check if cpu id is blacklisted
                  if ( std::count( blacklist.begin(), blacklist.end(), cpulist[i] ) == 0  ) {
                    RH_DEBUG(_affinity_logger, "Setting NIC (cpu select) available :" << cpulist[i] );
                    numa_bitmask_setbit(cpu_mask, cpulist[i]);
                  }
                }

                RH_DEBUG(_affinity_logger, "Setting NIC (cpu select) affinity constraint: :" << iface );
                if ( numa_sched_setaffinity( pid, cpu_mask) ) {
                  std::ostringstream e;
                  e << "Binding to NIC with cpu affinity, nic=" << iface;
                  throw AffinityFailed(e.str());
                }
                numa_bitmask_free(cpu_mask);
              }
              else {
                RH_WARN(_affinity_logger, "Setting NIC (cpu select), no cpu available all blacklisted :" << iface );
                std::ostringstream e;
                e << "Binding to NIC, no cpus available all blacklisted :" << iface;
                throw AffinityFailed(e.str());
              }
            }
          }
          else {
            RH_WARN(_affinity_logger, "Setting NIC, unable to set directive:" << iface );
            std::ostringstream e;
            e << "Binding to NIC, unable to set directive, cannot determine socket or cpu list from interrupt mapping, directive:" << iface;
            throw AffinityFailed(e.str());
          }
        }

        // socket -- assign via socket (numa node) number
        if ( affinity_spec.first == "socket" ) {
          std::string nodestr = affinity_spec.second;
          struct bitmask *node_mask = numa_parse_nodestring((char *)nodestr.c_str()); 
          if ( !node_mask )  {
            throw AffinityFailed("Processor Socket affinity failed, unable to parse:  " + nodestr);
          }
        
          // plain  node binding if no cpus are listed.
          if ( blacklist.size() == 0 ) {
            // bind to node... let system scheduler do its magic
            RH_DEBUG(_affinity_logger, "Setting PROCESSOR SOCKET affinity to constraint :" << nodestr );
            numa_bind( node_mask );
          }
          else { // remove blacklisted cpus from node binding
            bitmask *cpu_mask = numa_allocate_cpumask(); 
            if ( !cpu_mask ) {
              throw AffinityFailed("Unable to allocate cpu mask");
            } 

            // check if node is active, if so then get a cpu id
            int nbytes = numa_bitmask_nbytes(node_mask);
            for (int i=0; i < nbytes*8; i++ ){
              if ( numa_bitmask_isbitset( node_mask, i ) ) {
                numa_node_to_cpus( i, cpu_mask );
              }
            }

            // check if cpu id is blacklisted
            CpuList::const_iterator biter = blacklist.begin();
            for ( ; biter != blacklist.end() ; biter++ ) {
              RH_DEBUG(_affinity_logger, "Setting PROCESSOR SOCKET (cpu select) blacklist :" << *biter );
              numa_bitmask_clearbit(cpu_mask, *biter);
            }

            RH_DEBUG(_affinity_logger, "Setting PROCESSOR SOCKET (cpu select) affinity, pid/constraint:" << pid << "/" << nodestr );
            if ( numa_sched_setaffinity( pid, cpu_mask) ) {
              std::ostringstream e;
              e << "Binding to PROCESSOR SOCKET with blacklisted cpus, node=" << nodestr;
              throw AffinityFailed(e.str());
            }
            numa_bitmask_free(cpu_mask);
          }
        
          numa_bitmask_free(node_mask);

        }

        // cpu -- assign via cpu id
        if ( affinity_spec.first == "cpu" ) {
          std::string cpustr = affinity_spec.second;
          struct bitmask *cpu_mask = numa_parse_cpustring((char*)cpustr.c_str());    
          if ( !cpu_mask ) {
            throw AffinityFailed("CPU affinity failed, unable to parse: <" + cpustr + ">" );
          } 

          // apply black list 
          CpuList::const_iterator biter = blacklist.begin();
          for ( ; biter != blacklist.end() ; biter++ ) {
              RH_DEBUG(_affinity_logger, "Setting CPU affinity, blacklist :" << *biter );
            numa_bitmask_clearbit(cpu_mask, *biter);
          }

          RH_DEBUG(_affinity_logger, "Setting CPU affinity to constraint :" << cpustr );
          if ( numa_sched_setaffinity( pid, cpu_mask ) ) {
            std::ostringstream e;
            e << "Binding to CPU: " << cpustr;
            throw AffinityFailed(e.str());
          }

        }

#else
      RH_WARN(_affinity_logger, "Missing affinity support from Redhawk libraries, ... ignoring numa affinity based requests ");
#endif

        // cpuset -- assign via cpuset
        if ( affinity_spec.first == "cpuset" ) {
          std::string cpuset_name = affinity_spec.second;
          pid_t pid = getpid();
          std::string croot(get_cpuset_root());
          std::string sname( croot+"/"+cpuset_name+"/task");
          std::ofstream os(sname.c_str(), std::ofstream::out);
          if ( os.fail() ){
            std::ostringstream e;
            e << "CPUSET affinity failed, could not open cpuset : " << cpuset_name;
            throw AffinityFailed(e.str());
          }
          os << pid << std::endl;
          os.close();
          RH_DEBUG(_affinity_logger, "Setting CPUSET  affinity to constraint :" << cpuset_name );
        }

        // cgroup - assign to cgroup
        if ( affinity_spec.first == "cgroup" ) {
          std::string cgroup_name = affinity_spec.second;
          pid_t pid = getpid();
          std::string croot(get_cgroup_root());
          std::string sname( croot + "/" + cgroup_name+"/task");
          std::ofstream os(sname.c_str(), std::ofstream::out);
          if ( os.fail() ){
            std::ostringstream e;
            e << "CGROUP affinity failed, for cgroup: " << cgroup_name;
            throw AffinityFailed(e.str());
          }
          os << pid << std::endl;
          os.close();
          RH_DEBUG(_affinity_logger, "Setting CGROUP  affinity to constraint :" << cgroup_name );
        }

      }


      return 0;

    }

  };

};
