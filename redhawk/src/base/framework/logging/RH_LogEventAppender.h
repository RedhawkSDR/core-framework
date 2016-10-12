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

#ifndef RH_LogEvent_APPENDER_H
#define RH_LogEvent_APPENDER_H
#include <string>
#include <ossie/EventChannelSupport.h>
#include <log4cxx/appenderskeleton.h>
#include <log4cxx/logstring.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/pool.h>
#include <boost/shared_ptr.hpp>
 
namespace log4cxx
{
	 
class RH_LogEventAppender : public AppenderSkeleton
{
 public:

  DECLARE_LOG4CXX_OBJECT_WITH_CUSTOM_CLASS(RH_LogEventAppender, ClassRH_LogEventAppender )
 
    BEGIN_LOG4CXX_CAST_MAP()
    LOG4CXX_CAST_ENTRY(RH_LogEventAppender)
    LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
    END_LOG4CXX_CAST_MAP()

  
  RH_LogEventAppender();
  virtual ~RH_LogEventAppender();

  //
  // Called by log4cxx internals to process options
  //
  void setOption(const LogString& option, const LogString& value);

  void activateOptions( log4cxx::helpers::Pool& p);

  // This method is called by the AppenderSkeleton#doAppend method
  void append(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p);

  void close();
 
  bool isClosed() const { return closed; }
	 
  bool requiresLayout() const { return true; }

 private:

  typedef boost::shared_ptr< ossie::events::PushEventSupplier >     PushEventSupplierPtr;

  std::vector< std::string >                                ArgList;

  //
  // perform connect operation to establish a corba context 
  //
  int                                      connect_();
  
  //
  // Command line arguments used to configure corba util methods
  //
  LogString                                 _args;

  //
  // channel name
  //
  LogString                                 channelName;

  //
  // naming context 
  //
  LogString                                 nameContext;

  //
  // Producer Identifier
  //
  LogString                                 prodId;

  //
  // Producer Name
  //
  LogString                                 prodName;

  //
  // Producer FQN - fully qualified domain name for resource
  //
  LogString                                 prodFQN;

  //
  // channel name, shadow variable
  //
  LogString                                 _channelName;

  //
  // naming context, shadow variable
  //
  LogString                                 _nameContext;

  //
  // Handle to requested channel, might want to make this a vector... and this class a singleton
  //
  PushEventSupplierPtr                      _event_channel;

  // number of times to retry before calling it quits.. reset after each successfull connection ( -1 try forever )
  int                                       _reconnect_retries;

  // number of milliseconds to delay before retrying to connect to CORBA resoure
  int                                       _reconnect_delay;

  // clean up event channel when appender is removed
  int                                       _cleanup_event_channel;

  //  prevent copy and assignment statements
  RH_LogEventAppender(const RH_LogEventAppender&);
  RH_LogEventAppender& operator=(const RH_LogEventAppender&);

  };
	 
}; // end of namespace
#endif


#endif     // HAVE_LOG4CXX
