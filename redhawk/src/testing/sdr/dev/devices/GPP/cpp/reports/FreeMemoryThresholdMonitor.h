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
#ifndef FREE_MEMORY_THRESHOLD_MONITOR_H_
#define FREE_MEMORY_THRESHOLD_MONITOR_H_
#include "ThresholdMonitor.h"

class FreeMemoryThresholdMonitor : public GenericThresholdMonitor<int64_t>
{
public:

    FreeMemoryThresholdMonitor( const std::string& source_id, QueryFunction threshold, QueryFunction measured ) ;

    static std::string GetResourceId(){ return "physical_ram"; }
    static std::string GetMessageClass(){ return "MEMORY_FREE"; }

};

#endif
