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
// see ossie/affinity.h for detailed explaination, local install to handle execution of affinity requests
//

namespace  gpp {

  namespace affinity {

    const std::string get_cgroup_root() {
      return redhawk::affinity::get_cgroup_root();
    }

    const std::string get_cpuset_root() {
      return redhawk::affinity::get_cpuset_root();
    }

    bool is_disabled() {
      return redhawk::affinity::is_disabled();
    }

    void set_nic_promotion( const bool onoff ) {
      redhawk::affinity::set_nic_promotion(onoff);
    }

    const bool get_nic_promotion() {
      return redhawk::affinity::get_nic_promotion();
    }


   /**
       identify_cpus

       From a specified network interface determine the list of CPUs that service interrupts
       for that interface.

     */
    redhawk::affinity::CpuList identify_cpus( const std::string &iface ) {

      redhawk::affinity::CpuList  cpus;
      std::string pintr("/proc/interrupts");
      std::ifstream in(pintr.c_str(), std::ifstream::in );
      if ( in.fail() ) {
        RH_NL_ERROR("gpp::affinity", "Unable to access /proc/interrupts");
        return cpus;
      }

      std::string line;
      while( std::getline( in, line ) ) {
        // check if the device is our interface
        RH_NL_TRACE("gpp::affinity", "Processing /proc/interrupts.... line:" << line);
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
                RH_NL_TRACE("gpp::affinity", "identify cpus: Adding CPU : " << parts-1);
                cpus.push_back(parts-1);
              }
            }
            parts++;
          }while(iss);

        }
      }

      redhawk::affinity::CpuList::iterator citer=cpus.begin();
      for (; citer != cpus.end(); citer++) {
        RH_NL_DEBUG("gpp::affinity", "identified CPUS iface/cpu ...:" << iface << "/" << *citer);
      }

      return cpus;
    }


    int   find_socket_for_interface ( const std::string &iface , const bool findFirst, const redhawk::affinity::CpuList &bl ){

      int retval=-1;
      
      // Determine cpu list by interrupts assigned for the specified NIC
      redhawk::affinity::CpuList cpulist = identify_cpus(iface);
      if ( cpulist.size() > 0 ) {
        int psoc=-1;
#ifdef HAVE_LIBNUMA
        int soc=-1;
        for( int i=0; i < (int)cpulist.size();i++ ) {
          RH_NL_DEBUG("gpp::affinity", "Finding (processor socket) for NIC:" << iface << " socket :" << numa_node_of_cpu(cpulist[i]) );
          if ( std::count(  bl.begin(), bl.end(), cpulist[i] ) != 0 ) continue;
          soc = numa_node_of_cpu(cpulist[i]);
          if ( soc != psoc && psoc != -1 && !findFirst ) {
            RH_NL_WARN("gpp::affinity", "More than 1 socket servicing NIC:" << iface);
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


    redhawk::affinity::CpuList get_cpu_list( const std::string &list_type,  const std::string &context )
      throw (redhawk::affinity::AffinityFailed)
    {
      redhawk::affinity::CpuList cpu_list;
      if ( is_disabled() ) {
        return cpu_list;
      }
      
#ifdef HAVE_LIBNUMA
      if ( list_type == "socket" || list_type == "node" ) {
          std::string nodestr = context;
          struct bitmask *node_mask = numa_parse_nodestring((char *)nodestr.c_str()); 
          if ( !node_mask )  {
            throw redhawk::affinity::AffinityFailed("Processor Socket affinity failed, unable to parse:  " + nodestr);
          }

          bitmask *cpu_mask = numa_allocate_cpumask(); 
          if ( !cpu_mask ) {
            throw redhawk::affinity::AffinityFailed("Unable to allocate cpu mask");
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
            throw redhawk::affinity::AffinityFailed("CPU affinity failed, unable to parse: <" + cpustr + ">" );
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


    int set_affinity( const redhawk::affinity::AffinityDirectives &spec, const pid_t pid, const redhawk::affinity::CpuList &blacklist) 
      throw (redhawk::affinity::AffinityFailed)
    {
      if ( is_disabled() ) {
        return 0;
      }

      redhawk::affinity::CpuList::const_iterator citer=blacklist.begin();
      for (; citer != blacklist.end(); citer++) {
        RH_NL_DEBUG("gpp::affinity", "BlackList ...:" << *citer);
      }

      redhawk::affinity::AffinityDirectives::const_iterator piter = spec.begin();
      for ( int cnt=0; piter != spec.end(); piter++, cnt++ ) {
        redhawk::affinity::AffinityDirective affinity_spec = *piter;
        RH_NL_DEBUG("gpp::affinity", " cnt:" << cnt << " Processing Affinity pid: " << pid << " " << affinity_spec.first << ":" << affinity_spec.second );

#ifdef HAVE_LIBNUMA
        // nic -- Determine cpu list by interrupts assigned for the specified NIC
        if ( affinity_spec.first == "nic" ) {
          std::string iface = affinity_spec.second;
          // Determine cpu list by interrupts assigned for the specified NIC
          redhawk::affinity::CpuList cpulist = identify_cpus(iface);
          
          // if no cpus identified then issue warning
          if ( cpulist.size() > 0  ) {

            // check if black list is specified... if not then use numa node based affinity
            if ( blacklist.size() == 0 && getpid() == pid ) {   // are we the same process,  then use node binding method
              bitmask *node_mask = numa_allocate_nodemask();
              if ( !node_mask )  {
                throw redhawk::affinity::AffinityFailed("Unable to allocate node mask");
              }

              for( int i=0; i < (int)cpulist.size();i++ ) {
                RH_NL_DEBUG("gpp::affinity", "Setting NIC (processor socket select) available sockets :" << numa_node_of_cpu(cpulist[i]) );
                numa_bitmask_setbit(node_mask, numa_node_of_cpu(cpulist[i]) );
              }

              RH_NL_DEBUG("gpp::affinity", "Setting NIC (processor socket select) affinity constraint: :" << iface );
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
              if ( (cpulist.size() == 1 && get_nic_promotion() ) || ( cpus == 0 && get_nic_promotion() ) ) {
                int cpuid = cpulist[0];
                std::ostringstream os;
                os << numa_node_of_cpu( cpuid );
                redhawk::affinity::CpuList tlist = get_cpu_list( "socket", os.str() );
                RH_NL_INFO("gpp::affinity", "Promoting NIC affinity to PID:" << pid << " SOCKET:" << os.str() );
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
                  throw redhawk::affinity::AffinityFailed("Unable to allocate node mask");
                }

                for( int i=0; i < (int)cpulist.size();i++ ) {
                  // check if cpu id is blacklisted
                  if ( std::count( blacklist.begin(), blacklist.end(), cpulist[i] ) == 0  ) {
                    RH_NL_DEBUG("gpp::affinity", "Setting NIC (cpu select) available :" << cpulist[i] );
                    numa_bitmask_setbit(cpu_mask, cpulist[i]);
                  }
                }

                RH_NL_DEBUG("gpp::affinity", "Setting NIC (cpu select) affinity constraint: :" << iface );
                if ( numa_sched_setaffinity( pid, cpu_mask) ) {
                  std::ostringstream e;
                  e << "Binding to NIC with cpu affinity, nic=" << iface;
                  throw redhawk::affinity::AffinityFailed(e.str());
                }
                numa_bitmask_free(cpu_mask);
              }
              else {
                RH_NL_WARN("gpp::affinity", "Setting NIC (cpu select), no cpu available all blacklisted :" << iface );
                std::ostringstream e;
                e << "Binding to NIC, no cpus available all blacklisted :" << iface;
                throw redhawk::affinity::AffinityFailed(e.str());
              }
            }
          }
          else {
            RH_NL_WARN("gpp::affinity", "Setting NIC, unable to set directive:" << iface );
            std::ostringstream e;
            e << "Binding to NIC, unable to set directive, cannot determine processor socket or cpu list from interrupt mapping, directive:" << iface;
            throw redhawk::affinity::AffinityFailed(e.str());
          }
        }

        // socket  -- assign via processor socket
        if ( affinity_spec.first == "socket" ) {
          std::string nodestr = affinity_spec.second;
          struct bitmask *node_mask = numa_parse_nodestring((char *)nodestr.c_str()); 
          if ( !node_mask )  {
            throw redhawk::affinity::AffinityFailed("Processor socket affinity failed, unable to parse:  " + nodestr);
          }
        
          // plain  node binding if no cpus are listed.
          if ( blacklist.size() == 0 ) {
            // bind to node... let system scheduler do its magic
            RH_NL_DEBUG("gpp::affinity", "Setting PROCESSOR SOCKET affinity to constraint :" << nodestr );
            numa_bind( node_mask );
          }
          else { // remove blacklisted cpus from node binding
            bitmask *cpu_mask = numa_allocate_cpumask(); 
            if ( !cpu_mask ) {
              throw redhawk::affinity::AffinityFailed("Unable to allocate cpu mask");
            } 

            // check if node is active, if so then get a cpu id
            int nbytes = numa_bitmask_nbytes(node_mask);
            for (int i=0; i < nbytes*8; i++ ){
              if ( numa_bitmask_isbitset( node_mask, i ) ) {
                numa_node_to_cpus( i, cpu_mask );
              }
            }

            // check if cpu id is blacklisted
            redhawk::affinity::CpuList::const_iterator biter = blacklist.begin();
            for ( ; biter != blacklist.end() ; biter++ ) {
              RH_NL_DEBUG("gpp::affinity", "Setting PROCESSOR SOCKET (cpu select) blacklist :" << *biter );
              numa_bitmask_clearbit(cpu_mask, *biter);
            }
#if 0
            {
              // TEST for sched_setaffinity to resolve that not all threads are being confined to cpu set
              cpu_set_t  cset;
              CPU_ZERO(&cset);
              int nbytes = numa_bitmask_nbytes(cpu_mask);
              for (int i=0; i < nbytes*8; i++ ){
                if ( numa_bitmask_isbitset( cpu_mask, i ) ) {
                  RH_NL_DEBUG("gpp::affinity", "PTHREAD setting affinity to cpu :" << i );
                  CPU_SET(i,&cset);
                }
              }

              //if ( !pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset) ) {y
              if ( !sched_setaffinity(pid, sizeof(cpu_set_t), &cset) ) {
                   RH_NL_ERROR("gpp::affinity", "Setting PROCESSOR SOCKET (cpu select), unable to set processor affinity");
              }


            }
#endif

            RH_NL_DEBUG("gpp::affinity", "Setting PROCESSOR SOCKET (cpu select) affinity, pid/constraint:" << pid << "/" << nodestr );
            if ( numa_sched_setaffinity( pid, cpu_mask) ) {
              std::ostringstream e;
              e << "Binding to PROCESSOR SOCKET with blacklisted cpus, socket=" << nodestr;
              throw redhawk::affinity::AffinityFailed(e.str());
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
            throw redhawk::affinity::AffinityFailed("CPU affinity failed, unable to parse: <" + cpustr + ">" );
          } 

          // apply black list 
          redhawk::affinity::CpuList::const_iterator biter = blacklist.begin();
          for ( ; biter != blacklist.end() ; biter++ ) {
              RH_NL_DEBUG("gpp::affinity", "Setting CPU affinity, blacklist :" << *biter );
            numa_bitmask_clearbit(cpu_mask, *biter);
          }

          RH_NL_DEBUG("gpp::affinity", "Setting CPU affinity to constraint :" << cpustr );
          if ( numa_sched_setaffinity( pid, cpu_mask ) ) {
            std::ostringstream e;
            e << "Binding to CPU: " << cpustr;
            throw redhawk::affinity::AffinityFailed(e.str());
          }

        }

#else
      RH_NL_WARN("gpp::affinity", "Missing affinity support from Redhawk libraries, ... ignoring numa affinity based requests ");
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
            throw redhawk::affinity::AffinityFailed(e.str());
          }
          os << pid << std::endl;
          os.close();
          RH_NL_DEBUG("gpp::affinity", "Setting CPUSET  affinity to constraint :" << cpuset_name );
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
            throw redhawk::affinity::AffinityFailed(e.str());
          }
          os << pid << std::endl;
          os.close();
          RH_NL_DEBUG("gpp::affinity", "Setting CGROUP  affinity to constraint :" << cgroup_name );
        }

      }


      return 0;

    }

  };

};
