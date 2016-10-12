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
#ifndef _PROCSTATPARSER_H_
#define _PROCSTATPARSER_H_

#include <iosfwd>
#include <string>
#include <vector>
#include "states/ProcStat.h"


class ProcStatParser {

  friend class ProcStat;
    
public:

  ProcStatParser();

  ProcStatParser( const std::string &fname );

  virtual ~ProcStatParser();

  void parse( ProcStat::Contents &data );
    
private:

  std::string  fname;
};



#endif
