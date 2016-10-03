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
#ifndef SYSTEM_MONITOR_REPORTING_H_
#define SYSTEM_MONITOR_REPORTING_H_
#include <vector>
#include <stdint.h>
#include <boost/shared_ptr.hpp>
#include "Reporting.h"
#include "statistics/CpuUsageStats.h"
#include "states/ProcMeminfo.h"
#include "states/Limits.h"

class SystemMonitor : public Reporting
{
 public: 
  typedef boost::shared_ptr< CpuUsageStats >  CpuStatsPtr;
  typedef boost::shared_ptr< ProcMeminfo >    MemInfoPtr;
  typedef CpuUsageStats::CpuList              CpuList;

  struct loadavg {
      double  one_min;
      double  five_min;
      double fifteen_min;
  };

  struct Report {
    uint64_t virtual_memory_total;
    uint64_t virtual_memory_used;
    uint64_t virtual_memory_free;
    double   virtual_memory_percent;
    uint64_t physical_memory_total;
    uint64_t physical_memory_used;
    uint64_t physical_memory_free;
    double   physical_memory_percent;
    uint64_t all_usage;
    uint64_t user_usage;
    double   cpu_percent;
    double   user_cpu_percent;
    double   system_cpu_percent;
    double   idle_cpu_percent;
    double   up_time;
    Limits::Contents  sys_limits;  
    double   last_update_time;
    loadavg  load;
  };
  
public:
  SystemMonitor( const CpuList &cpu_list, const int nhistory=5 );


  SystemMonitor( const CpuStatsPtr & cpu_usage_stats,
                 const MemInfoPtr  & mem_usage_state,
                 const SysLimitsPtr& sys_limit );
 
  double   get_idle_percent() const;
  double   get_idle_average() const;
  uint64_t get_mem_free() const;
  uint64_t get_phys_free() const;
  uint64_t get_all_usage() const;
  uint64_t get_user_usage() const;
  double   get_loadavg() const;
  const Report &getReport() const;
  void report();
  const CpuStatsPtr getCpuStats() const;
    
private:
    CpuStatsPtr     cpu_usage_stats_;
    MemInfoPtr      mem_usage_state_;
    SysLimitsPtr    sys_limit_state_;
    Report          report_;
};


#endif
