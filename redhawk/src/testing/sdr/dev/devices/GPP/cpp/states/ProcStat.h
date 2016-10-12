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
#ifndef _PROCSTAT_H_
#define _PROCSTAT_H_
#include <stdint.h>
#include <ctime>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "states/State.h"

class ProcStat;
typedef  boost::shared_ptr<ProcStat>  ProcStatPtr;


class ProcStat : public State
{

 public:
  typedef  uint64_t                 Jiffie;
  typedef  uint64_t                 Counter;
  typedef std::vector< Jiffie >     CpuJiffies;
  typedef std::vector< Counter >    CounterList;


    enum CpuJiffiesField
    {
        CPU_JIFFIES_USER = 0,
        CPU_JIFFIES_NICE,
        CPU_JIFFIES_SYSTEM,
        CPU_JIFFIES_IDLE,
        CPU_JIFFIES_IOWAIT,     // Since Linux 2.5.41
        CPU_JIFFIES_IRQ,        // Since Linux 2.6.0-test4
        CPU_JIFFIES_SOFTIRQ,    // Since Linux 2.6.0-test4
        CPU_JIFFIES_STEAL,      // Since Linux 2.6.11
        CPU_JIFFIES_GUEST,      // Since Linux 2.6.24
        CPU_JIFFIES_GUEST_NICE, // Since Linux 2.6.33
        CPU_JIFFIES_MAX
    };

    struct CpuStat {
      std::string  id;
      int          idx;             // -1 == all
      CpuJiffies   jiffies;

    };
    
    
    typedef std::vector< CpuStat >   CpuStats;

    struct Contents {
      CpuStat          all;
      CpuStats         cpus;
      CounterList      interrupts;
      Counter          context_switches;
      Counter          boot_time;
      Counter          processes_started;
      Counter          processes_running;
      Counter          processes_blocked;
      CounterList      soft_irqs;
      time_t           time_stamp;

    };

    
    // init file and read in baseline stats
    ProcStat();

    virtual ~ProcStat();

    // update content state by processing /proc/stat
    void              update_state();

    const uint32_t    get_ncpus() const { return contents.cpus.size(); };

    // return contents of file
    const Contents    &get() const;
    
 protected:

    Contents        contents;

 private:

};


#endif  // __PROCSTAT_H__
