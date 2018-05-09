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
#ifdef   HAVE_LOG4CXX
#include <iostream>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/outputstreamwriter.h>
#include <log4cxx/helpers/fileoutputstream.h>
#include <log4cxx/rolling/rolloverdescription.h>
#include <log4cxx/rolling/fixedwindowrollingpolicy.h>
#include <log4cxx/rolling/sizebasedtriggeringpolicy.h>

#include <boost/algorithm/string.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "RH_SyncRollingAppender.h"

typedef   ipc::scoped_lock< _IPC_Mutex >       _IPC_ScopedMLock;      
typedef   ipc::scoped_lock< ipc::file_lock >   _IPC_ScopedFLock;      

using namespace log4cxx;
using namespace log4cxx::helpers;

#define _LL_DEBUG( msg ) \
  { std::ostringstream __os; __os << msg; LogLog::debug(__os.str()); __os.str(""); }

#define _LL_WARN( msg ) \
  { std::ostringstream __os; __os << msg; LogLog::warn(__os.str()); __os.str(""); }

#define _LL_ERROR( msg ) \
  { std::ostringstream __os; __os << msg; LogLog::error(__os.str()); __os.str(""); }

#define _LLS_DEBUG( os, msg ) \
  os << msg; LogLog::debug(os.str()); os.str("");


namespace log4cxx {

  // 
  // Allows for same naming reference to LogEvent Appender class
  //
    class ClassRH_SyncRollingAppender : public Class 
    {
    public:
        ClassRH_SyncRollingAppender() : helpers::Class() {}
        virtual LogString getName() const {
            return LOG4CXX_STR("org.ossie.logging.RH_SyncRollingAppender");
        }
        virtual ObjectPtr newInstance() const {
          return new RH_SyncRollingAppender();
        }
    };

};

// Register factory class with log4cxx for the appender
IMPLEMENT_LOG4CXX_OBJECT_WITH_CUSTOM_CLASS(RH_SyncRollingAppender, ClassRH_SyncRollingAppender)

RH_SyncRollingAppender::RH_SyncRollingAppender():
rolling::RollingFileAppenderSkeleton(),
  wait_on_lock(50),
  retries(0),
  max_file_size(10*1024*1024),
  max_bkup_index(1),
  roll_count(0),
  cleanup(false),
  created(false),
  sync_ctx(NULL)
{

}


RH_SyncRollingAppender::~RH_SyncRollingAppender() {

  int pid=getpid();
  _LL_DEBUG( "RH_SyncRollingAppender::DTOR START " << pid );  

  try {
    _LL_DEBUG( "UNLOCK FILE LOCK " << pid );  
    flock.unlock();
  }
  catch(...){}

  if ( sync_ctx ) {
    _LL_DEBUG( "RH_SyncRollingAppender::DTOR unlock shared memory  <" << sync_ctx->fname << "> " << pid  << "\n" );  
    try {
      sync_ctx->mutex.unlock();
    }
    catch(...){}

    if ( cleanup ) {
      _LL_DEBUG( "RH_SyncRollingAppender::DTPR clean up sharedg memory for: <" << sync_ctx->fname << "> " << pid << "\n" );  
      // clean up shared memory opbject
      std::string fname=_clean_fname(sync_ctx->fname);
      ipc::shared_memory_object::remove(fname.c_str());
    }
    else {
      _LL_WARN( "RH_SyncRollingAppender: LEAVING SHARED MEMORY KEY <" << sync_ctx->fname << "> " << pid << "\n" );  
    }
  }

  _LL_DEBUG( "RH_SyncRollingAppender::DTOR END " << pid );  
}

std::string RH_SyncRollingAppender::_clean_fname( const std::string &fname ) {
  return boost::replace_all_copy( fname, "/", "-" );
}

int  RH_SyncRollingAppender::_get_mem( const std::string &fname) {

  int retval=0;
  created = false;
  std::string clean_fname =_clean_fname(fname);

  // use the file name as the key for the shared memory segment...
  try {
    ipc::shared_memory_object shm_obj
      (ipc::create_only,                   //only create
       clean_fname.c_str(),                      //name
       ipc::read_write                     //read-write mode
       );
    
    shm_obj.truncate(sizeof(sync_log_file));
    shm.swap(shm_obj);
    _LL_DEBUG( "RH_SyncRollingAppender::get_mem  Creating Named memory space <" << fname << ">" );  
    created = true;
  }
  catch(...) {
  }
  
  if ( !created ) {
    _LL_DEBUG( "RH_SyncRollingAppender::get_mem  Attach to Existing <" << fname << ">"  );  
    ipc::shared_memory_object shm_obj
      (ipc::open_only,                  //only create
       clean_fname.c_str(),                   //name
       ipc::read_write                  //read-write mode
       );

    shm.swap(shm_obj);
  }

  //Map the whole shared memory in this process
  ipc::mapped_region tregion(shm, ipc::read_write);
  
  void *addr = tregion.get_address();
  sync_ctx = new(addr) sync_log_file;

  if ( created ) { 
    _IPC_ScopedMLock lock(sync_ctx->mutex);
    strcpy( sync_ctx->fname, fname.c_str() ); 
    sync_ctx->n_msgs = 0;
    sync_ctx->max_size = 10*1024*1024;
    sync_ctx->max_index = 10;
    sync_ctx->roll_count = 0;
  }

  _LL_DEBUG( "RH_SyncRollingAppender::get_mem key: <" << sync_ctx->fname << ">" );  
  _LL_DEBUG( " n_msgs : " << sync_ctx->n_msgs  );  
  _LL_DEBUG( " max_size : " << sync_ctx->max_size  );  
  _LL_DEBUG( " max_index : " << sync_ctx->max_index  );  
  _LL_DEBUG( " roll_count : " << sync_ctx->roll_count );  
  _LL_DEBUG( " fname : " << sync_ctx->fname << std::endl );  
  
  region.swap(tregion);

  return retval;
}


void RH_SyncRollingAppender::resync_rollover(log4cxx::helpers::Pool &p){

  try {
    _LL_DEBUG( "RH_SyncRollingAppender::resync_rollover... RESYNC START ");  
    synchronized sync(mutex);
    setImmediateFlush(true);
    closeWriter();
    helpers::OutputStreamPtr os(new helpers::FileOutputStream(getFile(), getAppend() ));
    helpers::WriterPtr newWriter(createWriter(os));
    setFile(getFile());
    setWriter(newWriter);
    if (getAppend()) {
      fileLength = File().setPath(getFile()).length(p);
    } else {
      fileLength = 0;
    }
    _LL_DEBUG( "RH_SyncRollingAppender::resync_rollover... RESYNC COMPLETED ");  
  } catch (std::exception& ex) {
    LogLog::warn(LOG4CXX_STR("Exception during resync-rollover"));
  }     
}

void RH_SyncRollingAppender::subAppend(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p){

  int cretries = retries;
  do {
    _LL_DEBUG( "RH_SyncRollingAppender::subAppend Waiting to lock mutex.");  
    boost::posix_time::ptime abs_time = boost::posix_time::microsec_clock::universal_time()+boost::posix_time::millisec(wait_on_lock);
    try {
      _IPC_ScopedFLock  lock(flock,abs_time);  
    
      if ( lock ) {
      
        /// we are behind... need to reopen the base file...
        if ( roll_count < sync_ctx->roll_count ) {
          resync_rollover(p);
          roll_count = sync_ctx->roll_count;
        }
        else {

          // get the current file status to test the trigger with
          {
            synchronized sync(mutex);        
            fileLength = File().setPath(getFile()).length(p);
          }

          _LL_DEBUG( "RH_SyncRollingAppender::subAppend  roll_count: " << roll_count << " length:" << fileLength);  
          // The rollover check must precede actual writing. This is the
          // only correct behavior for time driven triggers.
          if (
              triggeringPolicy->isTriggeringEvent(this, event, getFile(), getFileLength())) {
            //
            //   wrap rollover request in try block since
            //    rollover may fail in case read access to directory
            //    is not provided.  However appender should still be in good
            //     condition and the append should still happen.
            try {
              _LL_DEBUG( "Rolling......for: "  << getFile() << " PRE:" << sync_ctx->roll_count);
              rollover(p);
              sync_ctx->roll_count++;
              roll_count = sync_ctx->roll_count;
              _LL_DEBUG( "Rolling......for: "  << getFile() << " POST:" << sync_ctx->roll_count);
            } catch (std::exception& ex) {
              LogLog::warn(LOG4CXX_STR("Exception during rollover attempt."));
            }
          }
        }

        FileAppender::subAppend(event, p);
      }
      else if ( cretries ) {
        _LL_DEBUG( "RH_SyncRollingAppender::subAppend --- UNABLE TO LOCK...RETRY " << cretries);  
      }
    }
    catch(...){
      _LL_DEBUG( "RH_SyncRollingAppender::subAppend : exception during subAPPEND " << cretries);  
    }
  }
  while ( cretries && --cretries );
}

void RH_SyncRollingAppender::activateOptions(Pool& p) {

  if ( !sync_ctx) {
    std::string fname;
    log4cxx::helpers::Transcoder::encode( fileName, fname  );
    _LL_DEBUG( "ACTIVATE OPTIONS FOR: "  << fname );
    _get_mem( fname  );
    if ( !sync_ctx ) {
      throw MissingResourceException(LOG4CXX_STR("No Shared Memory Access"));
    }

    if ( !created ) {
      _LL_DEBUG( "IGNORING LOG4 OPTIONS.. USING OPTIONS FROM MEMORY KEY: "  << fname << " max_size:" << sync_ctx->max_size <<  " max index:" << sync_ctx->max_index << "\n");
      setMaximumFileSize(sync_ctx->max_size);
      setMaxBackupIndex(sync_ctx->max_index);
    }
  }

 log4cxx::rolling::SizeBasedTriggeringPolicyPtr trigger(
      new log4cxx::rolling::SizeBasedTriggeringPolicy());
  trigger->setMaxFileSize(max_file_size);
  trigger->activateOptions(pool);
  setTriggeringPolicy(trigger);

  log4cxx::rolling::FixedWindowRollingPolicyPtr rolling(
      new log4cxx::rolling::FixedWindowRollingPolicy());
  rolling->setMinIndex(1);
  rolling->setMaxIndex(max_bkup_index);
  rolling->setFileNamePattern(getFile() + LOG4CXX_STR(".%i"));
  rolling->activateOptions(pool);
  setRollingPolicy(rolling);

  if ( sync_ctx ) {

    // if  we created then apply settings back to shared memory object
    if ( created ) {
      _IPC_ScopedMLock  lock(sync_ctx->mutex);
      sync_ctx->max_index = rolling->getMaxIndex();
      sync_ctx->max_size = trigger->getMaxFileSize();
    }

    _LL_DEBUG( "RH_SyncRollingAppender::memory access KEY:" << sync_ctx->fname );  
    _LL_DEBUG( " created : " << created  );  
    _LL_DEBUG( " n_msgs : " << sync_ctx->n_msgs );  
    _LL_DEBUG( " max_size : " << sync_ctx->max_size  );  
    _LL_DEBUG( " max_index : " << sync_ctx->max_index  );  
    _LL_DEBUG( " roll_count : " << sync_ctx->roll_count );  
    _LL_DEBUG( " fname : " << sync_ctx->fname << std::endl );  
  }

  rolling::RollingFileAppenderSkeleton::activateOptions(p);

  // enforce no buffered IO
  setImmediateFlush(false);

  std::string fname;
  log4cxx::helpers::Transcoder::encode( fileName, fname  );
  flock = ipc::file_lock(fname.c_str());

}

int RH_SyncRollingAppender::getMaxBackupIndex() const {
  return max_bkup_index;
}

size_t RH_SyncRollingAppender::getMaximumFileSize() const {
  return max_file_size;
}

void RH_SyncRollingAppender::setMaxBackupIndex(int maxBackups) {
  max_bkup_index = maxBackups;
}

void RH_SyncRollingAppender::setMaximumFileSize(size_t maxFileSize1) {
  max_file_size = maxFileSize1;
}

void RH_SyncRollingAppender::setMaxFileSize(const LogString& value) {
  long maxFileSize=100;
  max_file_size = OptionConverter::toFileSize(value, maxFileSize + 1);
}

void RH_SyncRollingAppender::setOption(const LogString& option, const LogString& value) {

   RollingFileAppenderSkeleton::setOption( option, value );

   
  if(StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("CLEANUP"), LOG4CXX_STR("cleanup"))) {
      synchronized sync(mutex);
      cleanup=false;
      cleanup = OptionConverter::toBoolean(value,false);
      _LL_DEBUG( " RH_SyncRollingLogAppender: option: cleanup shared memory : "  << cleanup );
  }

  if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("WAITONLOCK"), LOG4CXX_STR("waitonlock")) ) {
    synchronized sync(mutex);
    wait_on_lock = StringHelper::toInt(value);
    _LL_DEBUG( "  RH_SyncRollingAppender: option: wait_on_lock : "  << wait_on_lock );
  }

  if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("RETRIES"), LOG4CXX_STR("retries")) ) {
    synchronized sync(mutex);
    retries = StringHelper::toInt(value);
    _LL_DEBUG( "  RH_SyncRollingAppender: option: retries : "  << retries );
  }

  if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("MAXFILESIZE"), LOG4CXX_STR("maxfilesize"))
      || StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("MAXIMUMFILESIZE"), LOG4CXX_STR("maximumfilesize")))  {
    synchronized sync(mutex);
    setMaxFileSize(value);
  }

  if (StringHelper::equalsIgnoreCase(option,LOG4CXX_STR("MAXBACKUPINDEX"), LOG4CXX_STR("maxbackupindex"))
      || StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("MAXIMUMBACKUPINDEX"), LOG4CXX_STR("maximumbackupindex"))) {
    synchronized sync(mutex);
    setMaxBackupIndex(StringHelper::toInt(value));
  }

}
#endif   //   HAVE_LOG4CXX
