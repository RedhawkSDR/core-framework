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
#ifndef CPU_USAGE_ACCUMULATOR_H_
#define CPU_USAGE_ACCUMULATOR_H_
#include <vector>
#include <iosfwd>
#include <boost/shared_ptr.hpp>
#include "Statistics.h"
#include "states/CpuState.h"

class CpuUsageAccumulator;
typedef boost::shared_ptr< CpuUsageAccumulator >  CpuUsageAccumulatorPtr;

class CpuUsageAccumulator : public CpuStatistics 
{
 public:
  
  typedef std::vector<unsigned long> CpuJiffies;

 public:
  CpuUsageAccumulator(const boost::shared_ptr<const CpuState>& cpu_state);
  virtual ~CpuUsageAccumulator(){}

  virtual void compute_statistics();

  virtual unsigned long get_delta_cpu_jiffies_total() const;

  virtual double get_user_percent() const;
  virtual double get_system_percent() const;
  virtual double get_idle_percent() const;
  virtual double get_user_average() const { return 0.0; };
  virtual double get_system_average() const { return 0.0; };
  virtual double get_idle_average() const { return 0.0; };

 private:
  double get_total_jiffies(const CpuJiffies& cpu_jiffies) const;

  double get_cpu_field_percent( size_t field ) const;

 private:
  CpuJiffies prev_cpu_jiffies_;
  CpuJiffies current_cpu_jiffies_;
    
  boost::shared_ptr<const CpuState> cpu_state_;
};

#endif
