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
#include <numeric>
#include <iostream>
#include "CpuUsageStats.h"


#ifdef DEBUG_ON
#define DEBUG(x)            x
#else
#define DEBUG(x)            
#endif



////////////////////////////////////////////////////////////////////////////////
//
//          CpuUsageStats
//
////////////////////////////////////////////////////////////////////////////////

CpuUsageStats::CpuUsageStats( const int nhistory ):
  proc_stat_(),
  cpus_(0),
  metrics_(ProcStat::CPU_JIFFIES_MAX, 0.0 ),
  average_(ProcStat::CPU_JIFFIES_MAX , 0.0 ),
  history_db_(nhistory)
{
  _update_stats();
}

CpuUsageStats::CpuUsageStats(const CpuList &cpus, const int nhistory  ):
  proc_stat_(),
  cpus_(cpus),
  metrics_(ProcStat::CPU_JIFFIES_MAX , 0.0 ),
  average_(ProcStat::CPU_JIFFIES_MAX , 0.0 ),
  history_db_(nhistory)
{
  _update_stats();
}

double CpuUsageStats::get_user_percent() const
{
  return metrics_[ ProcStat::CPU_JIFFIES_USER ];
}

double CpuUsageStats::get_system_percent() const
{
  return metrics_[ ProcStat::CPU_JIFFIES_SYSTEM ];
}

double CpuUsageStats::get_idle_percent() const
{
  return metrics_[ ProcStat::CPU_JIFFIES_IDLE ];
}


double CpuUsageStats::get_user_average() const
{
  return average_[ ProcStat::CPU_JIFFIES_USER ];
}

double CpuUsageStats::get_system_average() const
{
  return average_[ ProcStat::CPU_JIFFIES_SYSTEM ];
}

double CpuUsageStats::get_idle_average() const
{
  return average_[ ProcStat::CPU_JIFFIES_IDLE ];
}



void CpuUsageStats::compute_statistics()
{
  prev_cpus_stat_ = current_cpus_stat_;
  _update_stats();

  // sum up all jiffies for all required cpus or all
  Accumulator cpus_itv =  _get_interval_total();

  std::fill( metrics_.begin(),metrics_.end(), 0 );
  std::fill( average_.begin(),average_.end(), 0 );
  // calculate percentage for user, system, idle
  metrics_[ ProcStat::CPU_JIFFIES_USER ] = _calc_metric( ProcStat::CPU_JIFFIES_USER, cpus_itv );
  metrics_[ ProcStat::CPU_JIFFIES_SYSTEM ] = _calc_metric( ProcStat::CPU_JIFFIES_SYSTEM, cpus_itv );
  metrics_[ ProcStat::CPU_JIFFIES_IDLE ] = _calc_metric( ProcStat::CPU_JIFFIES_IDLE, cpus_itv );
  
  history_db_.push_back(metrics_);

  average_[ ProcStat::CPU_JIFFIES_USER ] = _calc_average( ProcStat::CPU_JIFFIES_USER );
  average_[ ProcStat::CPU_JIFFIES_SYSTEM ] = _calc_average( ProcStat::CPU_JIFFIES_SYSTEM );
  average_[ ProcStat::CPU_JIFFIES_IDLE ] = _calc_average( ProcStat::CPU_JIFFIES_IDLE );

}

uint32_t CpuUsageStats::get_ncpus() const 
{
  uint32_t ncpus = cpus_.size();
  if ( !ncpus ) ncpus= proc_stat_.get_ncpus();
  return ncpus;
}


CpuUsageStats::Accumulator CpuUsageStats::_get_interval_total() const
{
  if( !prev_cpus_stat_.empty() ) {
    uint64_t  scc; 
    uint64_t  scp;
    scc = _sum_jiffies( current_cpus_stat_ );
    scp = _sum_jiffies( prev_cpus_stat_ );
    DEBUG(std::cout << " _get_interval_total  scc/scp/diff " << scc << "/" << scp << "/" << scc - scp << std::endl);
    return scc - scp;
  }
  else
    return 0;
}


void CpuUsageStats::_update_stats() {
  proc_stat_.update_state();
  if ( cpus_.size() == 0 ) {
    current_cpus_stat_.clear();      // we just want all cpu jiffies..
    current_cpus_stat_.push_back(proc_stat_.get().all);
  }
  else {
    current_cpus_stat_  = proc_stat_.get().cpus;
  }
}


bool CpuUsageStats::_accum_cpu( const uint32_t cpu_id ) const
{
  bool retval=false;
  if ( cpus_.size() == 0 || std::count( cpus_.begin(), cpus_.end(), cpu_id ) > 0 )  {
    retval = true;
  }

  return retval;
}



CpuUsageStats::Accumulator  CpuUsageStats::_sum_jiffies(const ProcStat::CpuStats& cpu_stats ) const
{

  uint64_t  accum=0;
  // filter out cpus that were identified... if list == 0 then do not filer any
  if ( cpus_.size() == 0 ) { 
    ProcStat::CpuStats::const_iterator iter = cpu_stats.begin();
    for ( int i=0; iter != cpu_stats.end(); i++, iter++ ) {
      accum += std::accumulate( (*iter).jiffies.begin(), (*iter).jiffies.end(), 0 );              // skip guest counters...
      DEBUG(std::cout << " _sum_jiffies cpu/accum:  " << i << "/" << accum << std::endl);
    }
  }
  else {
    // filter out cpus that were identified... 
    for( uint32_t i=0; i < cpus_.size(); i++ ) {
      if ( cpus_[i] < cpu_stats.size() ) {
        int cpu_idx = cpus_[i];
        accum += std::accumulate( cpu_stats[cpu_idx].jiffies.begin(), cpu_stats[cpu_idx].jiffies.end(), 0);       // skip guest counters...
      }
    }
  }

  DEBUG(std::cout << " _sum_jiffies accum:  " << accum << std::endl);
  return accum;
}


CpuUsageStats::Accumulator  CpuUsageStats::_sum_jiffie_field( const ProcStat::CpuStats& cpu_stats,
                                               const ProcStat::CpuJiffiesField &jiffie ) const
{

  uint64_t  accum=0;
  // filter out cpus that were identified... if list == 0 then do not filer any
  ProcStat::CpuStats::const_iterator iter = cpu_stats.begin();
  for ( ; iter != cpu_stats.end(); iter++ ) {
    if ( _accum_cpu( iter->idx ) )  {
      accum += iter->jiffies[ jiffie ];  
    }
  }

  return accum;
}




double CpuUsageStats::_calc_metric( const ProcStat::CpuJiffiesField &jiffie, const Accumulator diff_total ) const
{
  // calculate percentage for user, system, idle
  Accumulator cur_jiffie;
  Accumulator prev_jiffie;
  cur_jiffie = _sum_jiffie_field ( current_cpus_stat_, jiffie );
  prev_jiffie = _sum_jiffie_field ( prev_cpus_stat_, jiffie );
  
  double diff_jiffie = (double) (cur_jiffie - prev_jiffie);
  
  DEBUG(std::cout << " _calc_metric jiffie:" << jiffie << " cur/prev/diff " << cur_jiffie << "/" << prev_jiffie << "/" << diff_jiffie << std::endl);

  if( 0 != diff_total )
    return  diff_jiffie / (double)diff_total * 100.0;
  else
    return 0.0;
}


double CpuUsageStats::_calc_average( const ProcStat::CpuJiffiesField &jiffie ) const
{
  double accum=0.0;
  MetricsHistory::const_iterator iter = history_db_.begin();
  int n=0;
  for ( ; iter != history_db_.end(); iter++, n++ ) {
    const MetricsList ml = history_db_.at(n);
    DEBUG(std::cout << " history v:"<< n << " jiffies:" << jiffie << " accum:  " << ml[jiffie] << std::endl);
    accum += ml[ jiffie ];
  }
  if ( n>0 ) {
    return accum=accum/(double)n;
    DEBUG(std::cout << " _sum_jiffies accum:  " << accum << std::endl);
  }
  return  0.0;
}
