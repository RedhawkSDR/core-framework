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
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "PidProcStatParser.h"
#include "ParserExceptions.h"

#ifdef DEBUG_ON
#define DEBUG(x)            x
#else
#define DEBUG(x)            
#endif


PidProcStatParser::PidProcStatParser( const int pid) :
  _pid(pid)
{
 }

PidProcStatParser::~PidProcStatParser() 
{
}

void PidProcStatParser::readone(FILE *input, int64_t *x) { fscanf(input, "%lld ", (long long *) x); }
void PidProcStatParser::readunsigned(FILE *input, uint64_t *x) { fscanf(input, "%llu ",(unsigned long long *) x); }
void PidProcStatParser::readstr(FILE *input, char *x) {  fscanf(input, "%s ", x);}
void PidProcStatParser::readchar(FILE *input, char *x) {  fscanf(input, "%c ", x);}

const PidProcStatParser::Contents & PidProcStatParser::get() { return _data; };

int  PidProcStatParser::parse( Contents & data )
{
  char tcomm[PATH_MAX];
  char state;
  int retval=-1;
  std::stringstream ss;
  ss<<"/proc/"<<_pid<<"/stat";
  FILE *input=fopen(ss.str().c_str(), "r");
  if( !input ) return retval;
  readone(input,&data.pid);
  readstr(input,tcomm);
  data.comm = tcomm;
  readchar(input,&state);
  data.state = state;
  readone(input,&data.ppid);
  readone(input,&data.pgrp);
  readone(input,&data.session);
  readone(input,&data.tty_nr);
  readone(input,&data.tty_pgrp);
  readone(input,&data.flags);
  readone(input,&data.min_flt);
  readone(input,&data.cmin_flt);
  readone(input,&data.maj_flt);
  readone(input,&data.cmaj_flt);
  readone(input,&data.utime);
  readone(input,&data.stime);
  readone(input,&data.cutime);
  readone(input,&data.cstime);
  fclose(input);

  return 0;
}


int PidProcStatParser::parse() {
  return parse(_data);
}

