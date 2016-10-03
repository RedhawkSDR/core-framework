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
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/sysinfo.h>
#include "SystemMonitorReporting.h"
#include "statistics/Statistics.h"


static const size_t BYTES_PER_MEGABYTE = 1024*1024;

SystemMonitor::SystemMonitor( const CpuList & cpu_list, const int nhistory ):
    cpu_usage_stats_( new CpuUsageStats(cpu_list,nhistory) ),
    mem_usage_state_(new ProcMeminfo()),
    sys_limit_state_(new SysLimits())
{
  report();
}

SystemMonitor::SystemMonitor( const CpuStatsPtr & cpu_usage_stats,
                              const MemInfoPtr &mem_usage_state,
                              const SysLimitsPtr &sys_limit ) :
  cpu_usage_stats_(cpu_usage_stats),
  mem_usage_state_(mem_usage_state),
  sys_limit_state_(sys_limit)
{
  report();
}

double SystemMonitor::get_idle_percent() const {
  return report_.idle_cpu_percent;
}

double SystemMonitor::get_idle_average() const {
  return cpu_usage_stats_->get_idle_average();
}

uint64_t SystemMonitor::get_mem_free() const {
 return report_.virtual_memory_free;
}

uint64_t SystemMonitor::get_phys_free() const {
 return report_.physical_memory_free;
}

uint64_t SystemMonitor::get_all_usage() const {
 return report_.all_usage;
}

uint64_t SystemMonitor::get_user_usage() const {
 return report_.user_usage;
}

double SystemMonitor::get_loadavg() const {
  return report_.load.one_min;
}

const SystemMonitor::Report &SystemMonitor::getReport() const {
  return report_;
}

const SystemMonitor::CpuStatsPtr SystemMonitor::getCpuStats() const {
  return cpu_usage_stats_;
}

void
SystemMonitor::report()
{
  struct sysinfo info;
  sysinfo(&info);

  try {
    cpu_usage_stats_->update();
    mem_usage_state_->update();
    sys_limit_state_->update();
    const ProcMeminfo::Contents &mem_stats = mem_usage_state_->get();
    report_.virtual_memory_total = mem_stats.at("MemTotal")+ mem_stats.at("SwapTotal");
    report_.virtual_memory_free =  mem_stats.at("MemFree") + mem_stats.at("SwapFree");
    report_.physical_memory_total = mem_stats.at("MemTotal");
    report_.physical_memory_free = mem_stats.at("MemFree");
  }
  catch(...){
    report_.virtual_memory_total = (info.totalram+info.totalswap) * info.mem_unit;
    report_.virtual_memory_free = (info.freeram+info.freeswap) * info.mem_unit;
    report_.physical_memory_total = info.totalram * info.mem_unit;
    report_.physical_memory_free = info.freeram * info.mem_unit;
  }

  report_.virtual_memory_used = report_.virtual_memory_total-report_.virtual_memory_free;
  report_.physical_memory_used = report_.physical_memory_total-report_.physical_memory_free;
  report_.virtual_memory_percent = (double)report_.virtual_memory_used / (double)report_.virtual_memory_total * 100.;  
  report_.physical_memory_percent = (double)report_.physical_memory_used / (double)report_.physical_memory_total * 100.;
  report_.user_cpu_percent = cpu_usage_stats_->get_user_percent();
  report_.system_cpu_percent = cpu_usage_stats_->get_system_percent();
  report_.idle_cpu_percent = cpu_usage_stats_->get_idle_percent();
  report_.cpu_percent = 100.0 - report_.idle_cpu_percent;
  report_.up_time = info.uptime;
  report_.last_update_time = time(NULL);
  report_.idle_cpu_percent = cpu_usage_stats_->get_idle_percent();
  report_.all_usage = cpu_usage_stats_->get_all_usage();
  report_.user_usage = cpu_usage_stats_->get_user_usage();
  report_.sys_limits = sys_limit_state_->get();
  report_.load.one_min = info.loads[0] * 1.0/(1<<SI_LOAD_SHIFT);
  report_.load.five_min = info.loads[1] * 1.0/(1<<SI_LOAD_SHIFT);
  report_.load.fifteen_min = info.loads[2] * 1.0/(1<<SI_LOAD_SHIFT);
  
}
