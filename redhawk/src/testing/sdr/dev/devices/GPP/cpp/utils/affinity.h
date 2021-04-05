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
#ifndef __GPP_AFFINITY_H__
#define __GPP_AFFINITY_H__

#include <string>
#include <vector>
#include <stdexcept>
#include "ossie/PropertyMap.h"
#include "ossie/affinity.h"

namespace gpp
{
    /**
       Affinity support  - overrides for local node processing - requires numa libararies...

    */
    
    namespace affinity {

      bool   check_numa();

      /**
         Find the socket assocated with a particular network interface

         @param iface  name of the network interface to search for
         @return int  -1  could not find interface,  > -1 - identifies processor socket

       */
      int  find_socket_for_interface( const std::string &iface, const bool findFirst=false, const redhawk::affinity::CpuList &bl=redhawk::affinity::CpuList(0)  );
      

      /**
        get_cpu_list

        Determine the list of cpu ids from numa policy (processor socket or cpu) and cont865ext.  For processor socket context, the list of
        cpus will be determined using numa_node_to_cpus.  For cpu context, the list of cpus will be numa_parse_cpustring.
        
        @param "socket" or "cpu"
        @param context  numalib parsable string to specify node or cpu lists (all returns all cpus)x
      */
      redhawk::affinity::CpuList get_cpu_list( const std::string &pol, const std::string &context );

      /**
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
            socket  Assign task a list of NUMA Nodes 
            nic     Assigns affinity to a processor socket or cpu list from those cpu-s processing interrupts for a NIC
            cgroup  Assign task to a Cgroup.
            cpu     Assign task to a list of CPU Ids

      */
      int set_affinity( const redhawk::affinity::AffinityDirectives &spec, 
                        const pid_t pid, 
                        const redhawk::affinity::CpuList &blacklist = redhawk::affinity::CpuList(0));

    };  // affinity namespace
    
}  // gpp  Namespace

#endif
