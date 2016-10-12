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
#include <stdio.h>
#include "ProcStat.h"
#include "parsers/ProcStatParser.h"

ProcStat::ProcStat()
{
}

ProcStat::~ProcStat()
{
}

void ProcStat::update_state()
{
  ProcStatParser stat_parser;
  Contents tmp;
  stat_parser.parse( tmp );
  contents = tmp;
}


const ProcStat::Contents& 
ProcStat::get() const 
{ 
  return contents;
}


static void __readone(FILE *input, int64_t *x) { fscanf(input, "%lld ",(long long *) x); }
static void __readstr(FILE *input, char *x) {  fscanf(input, "%s ", x);}

int ProcStat::GetTicks( int64_t &r_sys, int64_t &r_user ) {

  char cpu[512];
  int retval=-1;
  int64_t user, nice, sys, idle, iowait, irq, softirq;
  FILE *input=fopen("/proc/stat", "r");
  if( !input ) return retval;
  __readstr(input,cpu);
  __readone(input,&user);
  __readone(input,&nice);
  __readone(input,&sys);
  __readone(input,&idle);
  __readone(input,&iowait);
  __readone(input,&irq);
  __readone(input,&softirq);
  fclose(input);
  r_sys = user+nice+sys+idle;
  r_user = user+nice+sys;
  return 0;
} 
