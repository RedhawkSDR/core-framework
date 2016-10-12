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
#ifndef ENVIRONMENT_PATH_PARSER_H_
#define ENVIRONMENT_PATH_PARSER_H_

#include <string>
#include <vector>

/**
 * EnvironmentPathParser provides operations to read, write, and modify
 * environment path strings (e.g. LD_LIBRARY_PATH).
 */
class EnvironmentPathParser
{
public:
    /**
     * Constructor from std::string.  Extracts paths from path.
     * @param path Path string in the format PATH1:PATH2:PATH3
     */
    EnvironmentPathParser( const std::string& path="" );
    
    /**
     * Constructor from const char*.  Supports NULL pointers, as returned from
     * getenv() on unknown environment variable.
     * @param path Path string in the format PATH1:PATH2:PATH3
     */
    EnvironmentPathParser( const char* path );

    void from_string( const std::string& path );
    std::string to_string() const;
    
    void merge_front( const std::string& path );
    
private:
    void strip_empty_paths();
    
private:
    std::vector<std::string> paths;
};

#endif
