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
#ifndef _PIDPROCSTATPARSER_H_
#define _PIDPROCSTATPARSER_H_
#include <stdint.h>
#include <iosfwd>
#include <string>
#include <vector>

class PidProcStatParser {

 public:

  struct Contents {
    int64_t       pid;
    std::string   comm;
    char          state;
    int64_t       ppid;
    int64_t       pgrp;
    int64_t       session;
    int64_t       tty_nr;
    int64_t       tty_pgrp;
    int64_t       flags;
    int64_t       min_flt;
    int64_t       cmin_flt;
    int64_t       maj_flt;
    int64_t       cmaj_flt;
    int64_t       utime;
    int64_t       stime;
    int64_t       cutime;
    int64_t       cstime;
  };

public:

  PidProcStatParser();

  PidProcStatParser( const int pid );

  virtual ~PidProcStatParser();

  int parse();
  int parse( Contents &data );
  const Contents &get();
  inline int64_t         get_ticks() {
    return _data.utime + _data.stime + _data.cutime + _data.cstime;
  }

private:

  inline void readone(FILE *input, int64_t *x);
  inline void readunsigned(FILE *input, uint64_t *x);
  inline void readstr(FILE *input, char *x);
  inline void readchar(FILE *input, char *x);

  int      _pid;
  Contents _data;
};



#endif
