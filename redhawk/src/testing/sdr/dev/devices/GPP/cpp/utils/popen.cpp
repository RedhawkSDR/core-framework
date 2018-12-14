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
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

#ifdef DEBUG_ON
#define DEBUG(x)   x
#else
#define DEBUG(x)
#endif

namespace utils {

  std::string popen(const std::string &cmd, const bool first_or_last) {
    DEBUG(std::cout << "CMD:"  << cmd << std::endl);
    FILE* pipe = ::popen(cmd.c_str(), "r");
    if (!pipe) return "ERROR";
    // make sure to popen and it succeeds
#if BOOST_VERSION  > 104300
    boost::iostreams::file_descriptor_source pipe_src(fileno(pipe), boost::iostreams::never_close_handle );
#else
    boost::iostreams::file_descriptor_source pipe_src(fileno(pipe) );
#endif
    boost::iostreams::stream<boost::iostreams::file_descriptor_source> stream(pipe_src);
    stream.set_auto_close(false); // https://svn.boost.org/trac/boost/ticket/3517
    std::string line;
    while(std::getline(stream,line)) {
      if ( first_or_last ) break;
      DEBUG(std::cout << "LINE-> " + line + "  length: " << line.length() << std::endl);
    }      
    pclose(pipe);
    return line;
  }


};
