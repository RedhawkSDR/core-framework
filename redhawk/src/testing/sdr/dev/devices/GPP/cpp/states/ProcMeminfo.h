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
#ifndef _PROCMEMINFO_H_
#define _PROCMEMINFO_H_
#include <stdint.h>
#include <ctime>
#include <string>
#include <map>
#include <boost/shared_ptr.hpp>
#include "states/State.h"

class ProcMeminfo;
typedef  boost::shared_ptr< ProcMeminfo>  ProcMeminfoPtr;


class ProcMeminfo : public State
{

 public:
  typedef  uint64_t                           Counter;
  typedef std::map< std::string, Counter >    Contents;

  // init file and read in baseline stats
  ProcMeminfo();

  virtual ~ProcMeminfo();

  // update content state by processing /proc/stat
  void              update_state();

  // return contents of file
  const Contents   &get() const;
  const Counter     getMetric( const std::string  &metric_name ) const ;
    
 protected:

    Contents        contents;

 private:

};


#endif  // __PROCMEMINFO_H__
