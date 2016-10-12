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
#include "statistics/Statistics.h"
#include "states/ProcMeminfo.h"


class SystemMonitor : public Reporting
{
 public: 
  typedef boost::shared_ptr< CpuStatistics >  CpuStatsPtr;
  typedef boost::shared_ptr< ProcMeminfo >    MemInfoPtr;

  struct Report {
    uint64_t    physical_memory_free;
    double     idle_cpu_percent;
  };


public:
  SystemMonitor( const CpuStatsPtr & cpu_usage_stats,
                 const MemInfoPtr  & mem_usage_state );
 
  double   get_idle_percent() const;
  double   get_idle_average() const;
  uint64_t get_mem_free() const;
  const Report &getReport() const;
  void report();
    
private:
    std::string format_up_time(unsigned long secondsUp) const;
    
private:
    CpuStatsPtr   cpu_usage_stats_;
    MemInfoPtr    mem_usage_state_;
    Report report_;
};


#endif
