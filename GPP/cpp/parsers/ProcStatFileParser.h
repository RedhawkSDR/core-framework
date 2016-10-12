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
#ifndef PROC_STAT_FILE_PARSER_H_
#define PROC_STAT_FILE_PARSER_H_

#include "Parser.h"

#include <iosfwd>
#include <string>
#include <vector>

struct ProcStatFileData
{
    typedef std::vector<unsigned long> CpuJiffies;
    
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
        CPU_JIFFIES_MAX_SIZE
    };
    
    ProcStatFileData():
    os_start_time(0),
    cpu_jiffies(CPU_JIFFIES_MAX_SIZE,0)
    {
    }
    
    unsigned long os_start_time;
    CpuJiffies cpu_jiffies;
};

class ProcStatFileParser : public OverridableSingleton<ProcStatFileParser>
{
public:
    typedef ProcStatFileData DataType;
    
public:
    virtual ~ProcStatFileParser(){}
    static void Parse( ProcStatFileData& data );

protected:
    virtual void parse( std::istream& istr, ProcStatFileData& data );
    
private:
    void reset_fields(ProcStatFileData& data);

    void parse_fields(std::istream& istr, ProcStatFileData& data);
    void parse_line(const std::vector<std::string>& values, ProcStatFileData& data);

    void validate_fields(const ProcStatFileData& data) const;
    void validate_field( bool success, const std::string& message ) const;
};

#endif
