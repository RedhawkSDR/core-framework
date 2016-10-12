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
#ifndef _CPU_USAGE_STATS_H_
#define _CPU_USAGE_STATS_H_
#include <vector>
#include <iosfwd>
#include <boost/shared_ptr.hpp>
#include <boost/circular_buffer.hpp>
#include "Statistics.h"
#include "states/ProcStat.h"

////////////////////////////////////////////////////////
//
//  CpuUsageStats
//
//////////////////////////////////////////////////////// 
class CpuUsageStats;
typedef boost::shared_ptr< CpuUsageStats > CpuUsageStatsPtr;

class CpuUsageStats : public CpuStatistics
{
 public:
  typedef std::vector< uint32_t >  CpuList;
  typedef std::vector< double >    MetricsList;
  typedef boost::circular_buffer< MetricsList > MetricsHistory;

public:

  // Perform CPU usage based on specified list of cpus...
  // if  cpus.size() == 0 then perform utilization against all cpu ids
  //CpuUsageStats( );
  CpuUsageStats( const int nhistory=5 );
  CpuUsageStats( const CpuList &cpus, const int nhistory=5 );

  virtual ~CpuUsageStats() {}

  virtual void compute_statistics();

  virtual uint32_t    get_ncpus() const;
  virtual double get_user_percent() const;
  virtual double get_system_percent() const;
  virtual double get_idle_percent() const;
  virtual double get_user_average() const;
  virtual double get_system_average() const;
  virtual double get_idle_average() const;

protected:

    typedef ProcStat::Jiffie        Accumulator;

    virtual Accumulator _get_interval_total() const;
    virtual double      _calc_metric( const ProcStat::CpuJiffiesField & jiffie, const Accumulator itv ) const;
    virtual double     _calc_average(  const ProcStat::CpuJiffiesField & jiffie ) const;
    virtual Accumulator _sum_jiffies( const ProcStat::CpuStats & cpu_stats ) const;
    virtual Accumulator _sum_jiffie_field( const ProcStat::CpuStats& cpu_stats,
                                           const ProcStat::CpuJiffiesField & jiffie ) const;
    bool                _accum_cpu( const uint32_t cpu_id ) const;
    virtual void        _update_stats();

private:
    ProcStat::CpuStats      prev_cpus_stat_;  
    ProcStat::CpuStats      current_cpus_stat_;  
    ProcStat                proc_stat_;   
    CpuList                 cpus_;
    MetricsList             metrics_;
    MetricsList             average_;
    MetricsHistory          history_db_;
};


#endif
