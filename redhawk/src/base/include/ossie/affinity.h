/*
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
 */
#ifndef __RH_AFFINITY_H__
#define __RH_AFFINITY_H__

#include <string>
#include <vector>
#include <stdexcept>
#include "ossie/PropertyMap.h"
#include "ossie/logging/rh_logger.h"

namespace redhawk
{
    /*
       Affinity support during application and node deployments

    */
    
    namespace affinity {

      //
      // Exception through during affinity methods 
      //
      class AffinityFailed : public std::runtime_error {
        
      public:
      AffinityFailed( const std::string &w ) :
        std::runtime_error(w) {};

      };

      /*
        Type the defines a list of CPU id
      */
      typedef std::vector<int>                      CpuList;

      /*
         Affinity policy record... policy name and value
       */
      typedef std::pair< std::string, std::string > AffinityDirective;

      /*
         List Affinity policy records 
       */
      typedef std::vector< AffinityDirective >      AffinityDirectives;

      /*
         Affinity processing function signature to allow for overrides
       */
      typedef  int (*SetAffinityFunc)( const AffinityDirectives &spec, const pid_t pid, const CpuList &blacklist);
      
      /*
         Control location of cgroup and cpuset mounts points if those are preconfigured and available to
         to a Redhawk node.
       */
      const std::string get_cgroup_root();
      const std::string get_cpuset_root();

      /*
         Returns a xml formatted string that represents a SCA properties for recognized affinity properties. 
         These properties could appear in the <affinity> section under the <componentinstantiation> section for 
         a resource delpoyment.
       */
      std::string get_property_definitions() ;

      /*
         Affinity property id that identifies namespace for all affinity base property directives
       */
      const std::string AFFINITY_ID("affinity");

     /*
         Find the socket assocated with a particular network interface

         @param iface  name of the network interface to search for
         @return int  -1  could not find interface,  > -1 - identifies processor socket

       */
      int  find_socket_for_interface( const std::string &iface, const bool findFirst=false, const CpuList &bl=CpuList(0)  );
      

      /*
        get_cpu_list

        Determine the list of cpu ids from numa policy (node (processor socket) or cpu) and context.  
        For socket context, the list of cpus will be determined using numa_parse_nodestring and numa_node_to_cpus.  
        For cpu context, the list of cpus will be numa_parse_cpustring.
        
        @param pol  socket or cpu
        @param context  numalib parsable string to specify socket or cpu lists (all=returns all cpus)
      */
      CpuList get_cpu_list( const std::string &pol, const std::string &context );
      
      /*
         check if affinity processing is disabled (controlled via environment REDHAWK_AFFINITY_DISABLED variable or api call)
       */
      bool is_disabled();

      /*
         enable/disable affnity processing
       */
      void set_affinity_state( const bool onoff );

      /*
         set the affinity logger for debug... 
      */
      void set_affinity_logger( rh_logger::LoggerPtr newLogger );

      /*
         get teh affinity logger for debug...
      */
      rh_logger::LoggerPtr get_affinity_logger( );

      /*
         Check if a properties set contains affinity properties.
      */
      bool has_affinity( const CF::Properties &options );


      /*
         Check if a properties set contains a nic affinity directive.
      */
      bool has_nic_affinity( const CF::Properties &options );

      /*
         Control promotion of nic affinity to a processor socket context 
         if all cpus for the nic are blacklisted.
       */
      void set_nic_promotion( const bool onoff );
      const bool get_nic_promotion() ;

      /*
          Override set_affinity call with a customized version
       */ 
      void override_set( SetAffinityFunc newFunc );

      /*
         convert a properties set that contains affinity namespaced properties to a dictionary format
         acceptable for the set_affinity method
       */
      AffinityDirectives convert_properties(const CF::Properties& options );

      /*
         set_affinity 
       
         Affinity execution contexts are supported by the underlying operating system. The creation of certain contexts, 
         (cgroup and cpusets), is outside the scope of the method and requires manual configuration by administrators and integrators. 

         For the CGROUP directive, the task will be assigned to the group by placing the task's process id 
         into the tasks file under the cgroup's directory.  Proper priviledges are required for the process 
         executing this method.  There is also an assumption the the cgroup partition will be mounted 
         under the /cgroup 

         For node and application deployments. Redhawk resources can add the following <affinity> section to the 
         <componentplacement> 
            <componentinstantiation> 
              <affinity>
               <simpleref refid="affinity::exec_directive_class" value="cpu" />
               <simpleref refid="affinity::exec_directive_value" value="5"/>
             </affinity>
           </componentinstantiation> 
         </componentplacement> 

          affinity::exec_directive_class and affinity::exec_directive_value are a pair. 
            socket   Assign task a list of NUMA Nodes 
            nic     Assigns affinity to a Numa node based or cpu list from those cpu-s processing interrupts for a NIC
            cgroup  Assign task to a Cgroup.
            cpu     Assign task to a list of CPU Ids

      */
      int set_affinity( const CF::Properties &options, const pid_t pid, const CpuList &blacklist = CpuList(0) );

      int set_affinity( const AffinityDirectives &spec, const pid_t pid, const CpuList &blacklist = CpuList(0));

    };  // affinity namespace
    
}  // redhawk  Namespace

#endif
