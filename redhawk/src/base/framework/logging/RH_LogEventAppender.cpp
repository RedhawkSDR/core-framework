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
#include <ossie/CF/LogInterfaces.h>
#include "RH_LogEventAppender.h"
#include <ossie/CorbaUtils.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/stringhelper.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

#define _LL_DEBUG( msg ) \
  { std::ostringstream __os; __os << msg; LogLog::debug(__os.str()); __os.str(""); }

#define _LLS_DEBUG( os, msg ) \
  os << msg; LogLog::debug(os.str()); os.str("");


struct OrbContext;
typedef OrbContext*  OrbPtr;
 
//
// Orb
//
// Context for access to ORB and common CORBA services
//
struct OrbContext {

  // orb instantiation
  CORBA::ORB_ptr                          orb;

  // root POA for to handle object requests
  PortableServer::POA_ptr                 rootPOA;

  // handle to naming service
  CosNaming::NamingContext_ptr            namingService;

  // handle to naming service
  CosNaming::NamingContextExt_ptr         namingServiceCtx;

  virtual ~OrbContext() {};

  OrbContext() {
    orb = ossie::corba::Orb();
    rootPOA = ossie::corba::RootPOA();
    namingService = ossie::corba::InitialNamingContext();
    namingServiceCtx = CosNaming::NamingContextExt::_nil();
    try {
      CORBA::Object_ptr obj;
      obj=orb->resolve_initial_references("NameService");
      namingServiceCtx = CosNaming::NamingContextExt::_narrow(obj);
    }
    catch(...){
    };
  };

};




namespace log4cxx {
  // 
  // Allows for same naming reference to LogEvent Appender class
  //
    class ClassRH_LogEventAppender : public Class 
    {
    public:
        ClassRH_LogEventAppender() : helpers::Class() {}
        virtual LogString getName() const {
            return LOG4CXX_STR("org.ossie.logging.RH_LogEventAppender");
        }
        virtual ObjectPtr newInstance() const {
          return new RH_LogEventAppender();
        }
    };
}


// Register this class with log4cxx
IMPLEMENT_LOG4CXX_OBJECT_WITH_CUSTOM_CLASS(RH_LogEventAppender, ClassRH_LogEventAppender)


RH_LogEventAppender::RH_LogEventAppender():
channelName("LOG_CHANNEL"),
	nameContext(""),
	prodId("RESOURCE.ID"),
	prodName("RESOURCE.Name"),
	prodFQN("RESOURCE.FQN"),
	_channelName(""),
	_nameContext(""),
	_reconnect_retries(10),
	_reconnect_delay(10),
	_cleanup_event_channel(0)
{

}


RH_LogEventAppender::~RH_LogEventAppender() {

  if ( _event_channel &&  _cleanup_event_channel ) {
      _event_channel.reset();
      ossie::events::DeleteEventChannel( _channelName, _nameContext );
  }

}


void RH_LogEventAppender::setOption(const LogString& option, const LogString& value) {

    if(StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("EVENT_CHANNEL"), LOG4CXX_STR("event_channel"))) {
      synchronized sync(mutex);
      channelName = value;
      _LL_DEBUG("RH_LogEventAppender::setOption event_channel: " << value );
    }
    else if(StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("NAME_CONTEXT"), LOG4CXX_STR("name_context"))) {
      synchronized sync(mutex);
      nameContext = value;
      _LL_DEBUG("RH_LogEventAppender::setOption name_context: " << value );
    }
    else if(StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("PRODUCER_ID"), LOG4CXX_STR("producer_id"))) {
      synchronized sync(mutex);
      prodId = value;
      _LL_DEBUG("RH_LogEventAppender::setOption producer_id: " << value );
    }
    else if(StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("PRODUCER_NAME"), LOG4CXX_STR("producer_name"))) {
      synchronized sync(mutex);
      prodName = value;
      _LL_DEBUG("RH_LogEventAppender::setOption producer_name: " << value );
    }
    else if(StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("PRODUCER_FQN"), LOG4CXX_STR("producer_fqn"))) {
      synchronized sync(mutex);
      prodFQN = value;
      _LL_DEBUG("RH_LogEventAppender::setOption producer_fqn: " << value );
    }
    else if(StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("ARGV"), LOG4CXX_STR("argv"))) {
      synchronized sync(mutex);
      _args = value;
      _LL_DEBUG("RH_LogEventAppender::setOption argv: " << value );
    }
    else if(StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("REMOVE_ON_DESTROY"), LOG4CXX_STR("remove_on_destroy"))) {
      synchronized sync(mutex);
      int nds = StringHelper::toInt(value);
      if ( nds == 0 || nds == 1 ) {
	_cleanup_event_channel = nds;
      }
    }
    else if(StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("RETRIES"), LOG4CXX_STR("retries"))) {
      synchronized sync(mutex);
      int newRetry = StringHelper::toInt(value);
      _LL_DEBUG("RH_LogEventAppender::setOption retries: " << value );
      if ( newRetry > 0 ) {
	_reconnect_retries = newRetry;
      }
    }
    else if(StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("RETRY_DELAY"), LOG4CXX_STR("retry_delay"))) {
      synchronized sync(mutex);
      int newDelay = StringHelper::toInt(value);
      _LL_DEBUG("RH_LogEventAppender::setOption retry_delay: " << value );
      if ( newDelay > 0 ) {
	_reconnect_delay = newDelay;
      }
    }
    else {
      _LL_DEBUG("RH_LogEventAppender::setOption non-appender option: value : " << value );
      AppenderSkeleton::setOption(option, value);
    }
}


void RH_LogEventAppender::activateOptions(Pool& p) {

  synchronized sync(mutex);
  std::ostringstream os;
  _LLS_DEBUG( os, "RH_LogEventAppender: CH:" << channelName ); 
  _LLS_DEBUG( os, "RH_LogEventAppender: NameContext:" << nameContext );
  _LLS_DEBUG( os, "RH_LogEventAppender: Retries:" << _reconnect_retries);
  _LLS_DEBUG( os, "RH_LogEventAppender: RetryDelay:" << _reconnect_delay);

  if ( _channelName != channelName && channelName != "" ) {
    LOG4CXX_ENCODE_CHAR(t, channelName );
    _channelName = t;
    LOG4CXX_ENCODE_CHAR(t2, nameContext );
    _nameContext = t2;
    connect_();
  }

  AppenderSkeleton::activateOptions(p);
  
}

	 
void RH_LogEventAppender::append(const spi::LoggingEventPtr& event, Pool& p){
  if ( this->layout == NULL ) {
    LOG4CXX_ENCODE_CHAR(nameStr, name);
    std::string msg("No Layout set for the appender named [ ");
    msg.append(nameStr);
    msg.append(" ].");
	 
    LOG4CXX_DECODE_CHAR(msgL, msg);
    errorHandler->error(msgL);
    return;
  }
	 
  log4cxx::LogString fMsg;
	 
  this->layout->format(fMsg, event, p);
	 
  LOG4CXX_ENCODE_CHAR(fMsgStr, fMsg);

  // This is the message structure for a Redhawk logging event
  CF::LogEvent rh_event;
  LOG4CXX_ENCODE_CHAR(t1,prodId);
  rh_event.producerId = CORBA::string_dup(t1.c_str());
  LOG4CXX_ENCODE_CHAR(t2,prodName);
  rh_event.producerName = CORBA::string_dup(t2.c_str());
  LOG4CXX_ENCODE_CHAR(t3,prodFQN);
  rh_event.producerName_fqn = CORBA::string_dup(t3.c_str());

  CORBA::Long level=CF::LogLevels::FATAL;
  if ( event->getLevel() == log4cxx::Level::getError() )
    level=CF::LogLevels::ERROR;
  if ( event->getLevel() == log4cxx::Level::getWarn() )
    level=CF::LogLevels::WARN;
  if ( event->getLevel() == log4cxx::Level::getInfo() )
    level=CF::LogLevels::INFO;
  if ( event->getLevel() == log4cxx::Level::getDebug() )
    level=CF::LogLevels::DEBUG;
  if ( event->getLevel() == log4cxx::Level::getTrace() )
    level=CF::LogLevels::TRACE;
  if ( event->getLevel() == log4cxx::Level::getAll() )
    level=CF::LogLevels::ALL;
  rh_event.level = level;

  //timeStamp in LoggingEventPtr is in microseconds
  //need to convert to seconds for rh_event
  rh_event.timeStamp = event->getTimeStamp()/1000000;
  rh_event.msg = CORBA::string_dup(fMsg.c_str());
  
  // push log message to the event channel
  if ( _event_channel ) {
    if ( _event_channel->push(rh_event) != 0 ) {
      _LL_DEBUG( "RH_LogEventAppender::append EVENT CHANNEL, PUSH OPERATION FAILED.");
    }
 }
  
}
	 
void RH_LogEventAppender::close()
{
  _LL_DEBUG( "RH_LogEventAppender::close START");
  if ( closed ) return;
  _event_channel.reset();
  closed=true;
  _LL_DEBUG( "RH_LogEventAppender::close END");
}


int RH_LogEventAppender::connect_() {

  int retval = 0;

  _event_channel.reset();
  std::ostringstream os;
  _LLS_DEBUG( os, "RH_LogEventAppender::connect Create PushEventSupplier" << _channelName );
  ossie::events::PushEventSupplier *pes=new ossie::events::PushEventSupplier( _channelName, 
							      _nameContext, 
							      _reconnect_retries, 
							      _reconnect_delay );
  if (pes != NULL ) {
    _LLS_DEBUG( os, "RH_LogEventAppender::connect Create PushEventSupplier Created." );
    _event_channel.reset(pes);
  }

  return retval;
}

#endif   //   HAVE_LOG4CXX
