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
#include "CpuState.h"
#include "../parsers/ProcStatFileParser.h"


#include <sstream>

CpuState::CpuState():
  data_()
{
}

void 
CpuState::update_state()
{
  ProcStatFileParser::Parse( data_ );
}

const ProcStatFileData::CpuJiffies& 
CpuState::get_cpu_jiffies() const 
{ 
    return data_.cpu_jiffies; 
}

unsigned long
CpuState::get_os_start_time() const 
{ 
    return data_.os_start_time; 
}

