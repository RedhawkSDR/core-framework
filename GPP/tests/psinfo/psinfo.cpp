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
#include "states/ProcStat.h"
#include "states/ProcMeminfo.h"
#include "statistics/CpuUsageStats.h"

#include <iostream>
#include <stdexcept>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp> 

#ifdef HAVE_LIBNUMA
#include <numa.h>
#endif



int main(int argc, char* argv[])
{

  try {

    ProcMeminfo meminfo;
    meminfo.update_state();
    ProcMeminfo::Contents  c=meminfo.get();
    ProcMeminfo::Contents::const_iterator iter=c.begin();
    std::cout << "MEMINFO:" << std::endl;
    for ( ; iter != c.end(); iter++ ) std::cout << iter->first << " = "  << iter->second  << std::endl;
      

    ProcStat pstat;
    
    pstat.update_state();
    ProcStat::Contents data;
    data = pstat.get();
    std::stringstream ss;
    std::cout << " cpus" << data.cpus.size() << std::endl;

    ss  << boost::format("%-7s  %-7s %-7s %-7s %-7s %-7s %-7s %-7s %-7s %-7s %-7s  ") % "CPU ID" % "USER"  % "NICE"   % "SYSTEM"  % "IDLE"   % "IOWAIT"  % "IRQ"  % "SOFTIRQ"   % "STEAL"  % "GUEST"   % "GNICE" ;
    std::cout << ss.str() << std::endl;
    ss.clear();
    ss.str("");
    ss  << boost::format("%-5s:%-2d ") % data.all.id % data.all.idx;
    for ( int j=0; j < data.all.jiffies.size(); j++ ) ss  << boost::format("%-7d  ") % data.all.jiffies[j];
    std::cout << ss.str() << std::endl;
    ss.clear();
    ss.str("");

    for ( int i=0; i < data.cpus.size(); i++ ) {
      ss  << boost::format("%-5s:%-2d ") % data.cpus[i].id % data.cpus[i].idx;
      for ( int j=0; j < data.cpus[i].jiffies.size(); j++ ) ss  << boost::format("%-7lld  ") % data.cpus[i].jiffies[j];
      std::cout << ss.str() << std::endl;
      ss.clear();
      ss.str("");

    }

    {
    CpuUsageStats cpu_usage;

    cpu_usage.compute_statistics();
    std::cout << " USAGE NCPUS << " << cpu_usage.get_ncpus() << std::endl;
    
    ss  << boost::format("%-7s %-7s %-7s %-7s") % "USER"  % "SYSTEM"  % "IDLE" % "AVG" ;
    std::cout << ss.str() << std::endl;
    ss.clear();
    ss.str("");
    
    ss  << boost::format("%-7.2f %-7.2f %-7.2f %-7.2f ") % cpu_usage.get_user_percent()  % cpu_usage.get_system_percent()  % cpu_usage.get_idle_percent() %  cpu_usage.get_idle_history();
    std::cout << ss.str() << std::endl;
    ss.clear();
    ss.str("");

    for ( int i=0; i<10; i++ ) {
      boost::this_thread::sleep( boost::posix_time::milliseconds( 200 ) );
      cpu_usage.compute_statistics();
    }
    std::cout << " (2) USAGE NCPUS << " << cpu_usage.get_ncpus() << std::endl;
    
    ss  << boost::format("%-7s %-7s %-7s %-7s") % "USER"  % "SYSTEM"  % "IDLE" % "AVG" ;
    std::cout << ss.str() << std::endl;
    ss.clear();
    ss.str("");
    
    ss  << boost::format("%-7.2f %-7.2f %-7.2f %-7.2f ") % cpu_usage.get_user_percent()  % cpu_usage.get_system_percent()  % cpu_usage.get_idle_percent() %  cpu_usage.get_idle_history();
    std::cout << ss.str() << std::endl;
    ss.clear();
    ss.str("");
    }

    CpuUsageStats::CpuList cpus;
    cpus.push_back(0);
    cpus.push_back(1);
    cpus.push_back(2);
    cpus.push_back(3);
    cpus.push_back(4);
    CpuUsageStats cpu_usage(cpus);

    cpu_usage.compute_statistics();
    std::cout << " (list) USAGE NCPUS << " << cpu_usage.get_ncpus() << std::endl;
    
    ss  << boost::format("%-7s %-7s %-7s %-7s") % "USER"  % "SYSTEM"  % "IDLE" % "AVG" ;
    std::cout << ss.str() << std::endl;
    ss.clear();
    ss.str("");

    ss  << boost::format("%-7.2f %-7.2f %-7.2f %-7.2f ") % cpu_usage.get_user_percent()  % cpu_usage.get_system_percent()  % cpu_usage.get_idle_percent() %  cpu_usage.get_idle_history();
    std::cout << ss.str() << std::endl;
    ss.clear();
    ss.str("");
    
    for ( int i=0; i<5; i++ ) {
      boost::this_thread::sleep( boost::posix_time::milliseconds( 200 ) );
      cpu_usage.compute_statistics();
    }
    std::cout << " (2 - list) USAGE NCPUS << " << cpu_usage.get_ncpus() << std::endl;
    
    ss  << boost::format("%-7s %-7s %-7s %-7s") % "USER"  % "SYSTEM"  % "IDLE" % "AVG" ;
    std::cout << ss.str() << std::endl;
    ss.clear();
    ss.str("");
    
    ss  << boost::format("%-7.2f %-7.2f %-7.2f %-7.2f ") % cpu_usage.get_user_percent()  % cpu_usage.get_system_percent()  % cpu_usage.get_idle_percent() %  cpu_usage.get_idle_history();
    std::cout << ss.str() << std::endl;
    ss.clear();
    ss.str("");


#ifdef HAVE_LIBNUMA
    {
    CpuUsageStats::CpuList cpus;
    std::string nodestr("0");
    struct bitmask *node_mask = numa_parse_nodestring((char *)nodestr.c_str());     

    bitmask *cpu_mask = numa_allocate_cpumask(); 
    
    // for each bit set in the mask then get cpu list
    int nbytes = numa_bitmask_nbytes(node_mask);
    for (int i=0; i < nbytes*8; i++ ){
      if ( numa_bitmask_isbitset( node_mask, i ) ) {
        numa_node_to_cpus( i, cpu_mask );
              
        // foreach cpu identified add to list
        int nb = numa_bitmask_nbytes(cpu_mask);
        for (int j=0; j < nb*8; j++ ){
          if ( numa_bitmask_isbitset( cpu_mask, j ) ) {
            cpus.push_back( j );
          }
        }
      }
    }

    CpuUsageStats cpu_usage(cpus);

    cpu_usage.compute_statistics();
    std::cout << " (list) USAGE NCPUS(node:"<< nodestr<< ") "<< cpu_usage.get_ncpus() << std::endl;
    
    ss  << boost::format("%-7s %-7s %-7s %-7s") % "USER"  % "SYSTEM"  % "IDLE" % "AVG" ;
    std::cout << ss.str() << std::endl;
    ss.clear();
    ss.str("");
    
    ss  << boost::format("%-7.2f %-7.2f %-7.2f %-7.2f ") % cpu_usage.get_user_percent()  % cpu_usage.get_system_percent()  % cpu_usage.get_idle_percent() %  cpu_usage.get_idle_history();
    std::cout << ss.str() << std::endl;
    ss.clear();
    ss.str("");

    for ( int i=0; i<5; i++ ) {
      boost::this_thread::sleep( boost::posix_time::milliseconds( 200 ) );
      cpu_usage.compute_statistics();
    }
    std::cout << " (2 - list) USAGE NCPUS(node:"<< nodestr<< ") "<< cpu_usage.get_ncpus() << std::endl;
    
    ss  << boost::format("%-7s %-7s %-7s %-7s") % "USER"  % "SYSTEM"  % "IDLE" % "AVG" ;
    std::cout << ss.str() << std::endl;
    ss.clear();
    ss.str("");
    
    ss  << boost::format("%-7.2f %-7.2f %-7.2f %-7.2f ") % cpu_usage.get_user_percent()  % cpu_usage.get_system_percent()  % cpu_usage.get_idle_percent() %  cpu_usage.get_idle_history();
    std::cout << ss.str() << std::endl;
    ss.clear();
    ss.str("");
    }

#endif

    

  }
  catch(std::exception &ex) {
    std::cerr << "caught: "<< ex.what()  << std::endl;
  }
  catch(...) {
    std::cerr << " ut ohhh... something bad happened" << std::endl;
  }



  return 0;
}
