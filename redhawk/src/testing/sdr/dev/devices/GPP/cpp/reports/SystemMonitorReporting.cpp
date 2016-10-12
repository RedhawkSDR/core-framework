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

SystemMonitor::SystemMonitor( const CpuStatsPtr & cpu_usage_stats,
                              const MemInfoPtr &mem_usage_state ) :
  cpu_usage_stats_(cpu_usage_stats),
  mem_usage_state_(mem_usage_state)
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
  return report_.physical_memory_free;
}

const SystemMonitor::Report &SystemMonitor::getReport() const {
  return report_;
}

void
SystemMonitor::report()
{
  cpu_usage_stats_->update();
  mem_usage_state_->update();
  try {
    ProcMeminfo::Counter total_memory = mem_usage_state_->getMetric("CommitLimit");
    ProcMeminfo::Counter committed_memory = mem_usage_state_->getMetric("Committed_AS");
    report_.physical_memory_free = (double)(total_memory - committed_memory);
  }
  catch (...) {
    struct sysinfo info;
    sysinfo(&info);
    //report_.physical_memory_free = info.freeram / BYTES_PER_MEGABYTE * info.mem_unit;
    report_.physical_memory_free = info.freeram / BYTES_PER_MEGABYTE * info.mem_unit;
  }

	//reporting_data_.virtual_memory_total = (info.totalram+info.totalswap) / BYTES_PER_MEGABYTE * info.mem_unit;
	//reporting_data_.virtual_memory_free = (info.freeram+info.freeswap) / BYTES_PER_MEGABYTE * info.mem_unit;
	//reporting_data_.virtual_memory_used = reporting_data_.virtual_memory_total-reporting_data_.virtual_memory_free;
	//reporting_data_.virtual_memory_percent = (double)reporting_data_.virtual_memory_used / (double)reporting_data_.virtual_memory_total * 100.;
	//reporting_data_.physical_memory_total = info.totalram / BYTES_PER_MEGABYTE * info.mem_unit;
	//reporting_data_.physical_memory_free = info.freeram / BYTES_PER_MEGABYTE * info.mem_unit;
	//reporting_data_.physical_memory_used = reporting_data_.physical_memory_total-reporting_data_.physical_memory_free;
	//reporting_data_.physical_memory_percent = (double)reporting_data_.physical_memory_used / (double)reporting_data_.physical_memory_total * 100.;
	//reporting_data_.user_cpu_percent = cpu_usage_accumulator_->get_user_percent();
	//reporting_data_.system_cpu_percent = cpu_usage_accumulator_->get_system_percent();
	report_.idle_cpu_percent = cpu_usage_stats_->get_idle_percent();
	//reporting_data_.cpu_percent = 100.0 - reporting_data_.idle_cpu_percent;
	//reporting_data_.up_time = info.uptime;
	//reporting_data_.up_time_string = format_up_time(reporting_data_.up_time);
	//reporting_data_.last_update_time = time(NULL);
}

std::string
SystemMonitor::format_up_time(unsigned long secondsUp) const
{
	std::stringstream formattedUptime;
	int days;
	int hours;
	int minutes;
	int seconds;

	int leftover;

	days = (int) secondsUp / (60 * 60 * 24);
	leftover = (int) secondsUp - (days * (60 * 60 * 24) );
	hours = (int) leftover / (60 * 60);
	leftover = leftover - (hours * (60 * 60) );
	minutes = (int) leftover / 60;
	seconds = leftover - (minutes * 60);

	formattedUptime << days << "d " << hours << "h " << minutes << "m " << seconds << "s";

	return formattedUptime.str();
}
