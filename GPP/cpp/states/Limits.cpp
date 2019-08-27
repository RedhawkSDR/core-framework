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
#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>
#include <linux/limits.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include "Limits.h"
#include "utils/popen.h"

#if BOOST_FILESYSTEM_VERSION < 3
#define BOOST_PATH_STRING(x) (x)
#else
#define BOOST_PATH_STRING(x) (x).string()
#endif

#ifdef DEBUG_ON
#define DEBUG(x)         std::cout << x << std::endl
#else
#define DEBUG(x)
#endif


const Limits::Contents& 
Limits::get() const 
{ 
  return contents;
}

Limits::Limits()
{
}

Limits::~Limits()
{
}



SysLimits::SysLimits()
{
}

SysLimits::~SysLimits()
{
}

void SysLimits::update_state()
{
  Contents tmp;

  //  grab current file handles 
  std::string fname;
  try{
    fname = "/proc/sys/fs/file-nr";
    std::ifstream file_nr(fname.c_str(), std::ifstream::in);
    if ( !file_nr.good()) throw std::ifstream::failure("unable to open " + fname );
    std::string line;
    while ( std::getline( file_nr, line ) ) {
      std::vector<std::string> values;
      boost::algorithm::split( values, line, boost::is_any_of(std::string(" \t")), boost::algorithm::token_compress_on );
      DEBUG(" values: " << values.size() << "  file-nr line: " << line  );
        
      if ( values.size() > 2 ) {
        try {
          tmp.files = boost::lexical_cast<int64_t>( values[0] );
          tmp.files_limit = boost::lexical_cast<int64_t>( values[2] );
        }
        catch( boost::bad_lexical_cast ){
        }
      }
    }
  }
  catch( ... ) {
  }

  try{
    fname = "/proc/sys/kernel/threads-max";
    std::ifstream sys_threads_max(fname.c_str(), std::ifstream::in);
    if ( !sys_threads_max.good()) throw std::ifstream::failure("unable to open " + fname );
    std::string line;
    while ( std::getline( sys_threads_max, line ) ) {
      std::vector<std::string> values;
      boost::algorithm::split( values, line, boost::is_any_of(std::string(" \t")), boost::algorithm::token_compress_on );
      DEBUG( " sys-kernel-threads-max line: " << line  );
        
      if ( values.size() > 0 ) {
        try {
          tmp.threads_limit = boost::lexical_cast<int64_t>( values[0] );
        }
        catch( boost::bad_lexical_cast ){
        }
      }
    }
  }
  catch( ... ) {
  }

  try {
    std::string line = utils::popen("ps -eo nlwp | tail -n +2 | awk '{ num_threads += $1 } END { print num_threads }' ", true);
    if ( line != "ERROR" ) {
      std::vector<std::string> values;
      boost::algorithm::split(values, line, boost::is_any_of(std::string(" \t")), boost::algorithm::token_compress_on );
      DEBUG(" system active threads: " << line);
      if ( values.size() > 0 ) {
        try {
          tmp.threads = boost::lexical_cast<int>( values[0] );
        }
        catch( boost::bad_lexical_cast ){
        }
      }
    }

  }
  catch( ... ) {
  }

  DEBUG( " SYSTEM: threads/max " << tmp.threads << "/" << tmp.threads_limit  );
  DEBUG( " SYSTEM: files/max " << tmp.files << "/" << tmp.files_limit  );
  contents = tmp;
}


ProcessLimits::ProcessLimits( const int in_pid) :
  pid(in_pid)
{
  if (in_pid<0) pid=getpid();
}

ProcessLimits::~ProcessLimits()
{
}

void ProcessLimits::update_state()
{
  Contents tmp;

  if ( pid < 0 ) pid = getpid();

  struct rlimit limit;
  if (getrlimit(RLIMIT_NPROC, &limit) == 0) {
      tmp.threads_limit = limit.rlim_cur;
  }
  if (getrlimit(RLIMIT_NOFILE, &limit) == 0) {
    tmp.files_limit = limit.rlim_cur;
  }

  // 
  std::ostringstream ppath;
  ppath << "/proc/"<<pid;
  boost::filesystem::path pid_dir(ppath.str());
  if (boost::filesystem::exists(pid_dir)) {
    std::stringstream filename;
    filename<< BOOST_PATH_STRING(pid_dir)<<"/status";
    // search for Threads tag in file.
    std::ifstream status_file(filename.str().c_str(), std::ifstream::in);
    if ( status_file.good() ) {
      std::string line;
      while ( std::getline( status_file, line ) ) {
        std::vector<std::string> values;
        boost::algorithm::split( values, line, boost::is_any_of(std::string(" ")), boost::algorithm::token_compress_on );
        DEBUG( " line: " << line  );
          
        if ( values.size() > 1 && boost::starts_with( values[0], "Threads:" ) ) {
          try {
            tmp.threads = boost::lexical_cast<int>( values[1] );
          }
          catch( boost::bad_lexical_cast ){
          }
        }
      }
    }
  }

  if ( !tmp.threads ) {
    std::stringstream subpath;
    subpath<< BOOST_PATH_STRING(pid_dir)<<"/task/";
    boost::filesystem::path subPath(subpath.str());
    if (boost::filesystem::exists(subPath)) {
      for (boost::filesystem::directory_iterator sub_dir_iter(subPath); 
           sub_dir_iter!= boost::filesystem::directory_iterator();++sub_dir_iter) {
        tmp.threads++;
      }
    }
  }

  std::stringstream subfilepath;
  subfilepath<< BOOST_PATH_STRING(pid_dir)<<"/fd/";
  boost::filesystem::path subFilePath(subfilepath.str());
  if (boost::filesystem::exists(subFilePath)) {
    for (boost::filesystem::directory_iterator sub_dir_iter(subFilePath); 
         sub_dir_iter!= boost::filesystem::directory_iterator();++sub_dir_iter) {
      tmp.files++;
    }
  }

  DEBUG( " Process: threads/max " << tmp.threads << "/" << tmp.threads_limit  );
  DEBUG( " Process: files/max " << tmp.files << "/" << tmp.files_limit  );

  contents = tmp;
}





