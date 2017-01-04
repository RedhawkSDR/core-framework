/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifdef HAVE_LOG4CXX

#ifndef RH_SyncRollingAppender_H
#define RH_SyncRollingAppender_H
#include <string>
#include <log4cxx/rolling/rollingfileappenderskeleton.h>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace ipc=boost::interprocess;

typedef   ipc::interprocess_mutex         _IPC_Mutex;      


namespace log4cxx
{
  
  class RH_SyncRollingAppender : public rolling::RollingFileAppenderSkeleton {

    // shared memory object for synchronous file appender
    struct  sync_log_file {
      uint64_t     n_msgs;
      uint64_t     max_size;
      uint64_t     max_index;
      uint64_t     roll_count;
      char         fname[PATH_MAX];
      _IPC_Mutex   mutex;      
    };


  public:

    DECLARE_LOG4CXX_OBJECT_WITH_CUSTOM_CLASS(RH_SyncRollingAppender, ClassRH_SyncRollingAppender )
 
      BEGIN_LOG4CXX_CAST_MAP()
      LOG4CXX_CAST_ENTRY(RH_SyncRollingAppender)
      LOG4CXX_CAST_ENTRY_CHAIN(rolling::RollingFileAppenderSkeleton)
      END_LOG4CXX_CAST_MAP()

  
      RH_SyncRollingAppender();

    RH_SyncRollingAppender( const LayoutPtr &layout,
                                     const LogString &filename );

    virtual ~RH_SyncRollingAppender();

    //
    // Called by log4cxx internals to process options
    //
    void setOption(const LogString& option, const LogString& value);

    void activateOptions( log4cxx::helpers::Pool& p);

    int getMaxBackupIndex() const;
    size_t getMaximumFileSize() const;
    void setMaxBackupIndex( int maxBackupIndex );
    void setMaxFileSize( const LogString & value );
    void setMaximumFileSize(size_t value );

  protected:

    void subAppend (const spi::LoggingEventPtr &event, log4cxx::helpers::Pool &p);
    void resync_rollover (log4cxx::helpers::Pool &p);

  private:

    int _get_mem( const std::string &fname );

    // remove special characters from filename for shared memory context
    std::string _clean_fname( const std::string &fname );

    //  prevent copy and assignment statements
    RH_SyncRollingAppender(const RH_SyncRollingAppender&);
    RH_SyncRollingAppender& operator=(const RH_SyncRollingAppender&);

    int                        wait_on_lock;
    int                        retries;
    size_t                     max_file_size;
    int                        max_bkup_index;
    uint64_t                   roll_count;
    bool                       cleanup;
    bool                       created;
    sync_log_file              *sync_ctx;
    ipc::file_lock             flock;
    ipc::shared_memory_object  shm;
    ipc::mapped_region         region;

  };
	 
}; // end of namespace
#endif


#endif     // HAVE_LOG4CXX
