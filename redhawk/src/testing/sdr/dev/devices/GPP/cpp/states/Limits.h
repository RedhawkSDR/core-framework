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
#ifndef _LIMIT_H_
#define _LIMIT_H_
#include <stdint.h>
#include <ctime>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "states/State.h"

class Limits;
typedef  boost::shared_ptr<Limits>  LimitsPtr;

class SysLimits;
typedef  boost::shared_ptr<SysLimits>  SysLimitsPtr;

class ProcessLimits;
typedef  boost::shared_ptr<ProcessLimits>  ProcessLimitsPtr;


class Limits : public State
{

 public:
  struct Contents {
  Contents() : threads(0), threads_limit(-1), files(0), files_limit(-1) {};
    int64_t          threads;
    int64_t          threads_limit;
    int64_t          files;
    int64_t          files_limit;
  };

    
  // init file and read in baseline stats
  Limits();

  virtual ~Limits();

  //virtual void              update_state();

  // return contents of file
  const Contents    &get() const;
    
 protected:

  Contents        contents;

 private:

};



class SysLimits : public Limits
{

 public:
  // init file and read in baseline stats
  SysLimits();

  virtual ~SysLimits();

  void              update_state();

};



class ProcessLimits : public Limits
{

 public:
  // init file and read in baseline stats
  ProcessLimits(const int pid=-1);

  virtual ~ProcessLimits();

  void              update_state();

 protected:

  int pid;

};


#endif  // __SYSLIMIT_H__
