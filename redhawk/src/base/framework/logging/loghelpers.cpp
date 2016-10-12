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
#include <string>
#include <iostream>
#include <uuid/uuid.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <boost/regex.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include <ossie/CorbaUtils.h>
#include <ossie/ossieSupport.h>
#include <ossie/debug.h>
#include <ossie/logging/loghelpers.h>
#include <boost/asio.hpp>
#include <fstream>
#include <dlfcn.h>
using boost::asio::ip::tcp;
#ifdef   HAVE_LOG4CXX
#include <log4cxx/logger.h>
#include <log4cxx/level.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/logstring.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/bytearrayinputstream.h>
#include <log4cxx/stream.h>
#include "StringInputStream.h"
#else
#include "rh_logger_cfg.h"                       // this class spoofs the log4cxx configuration calls, when log4cxx is disabled
#endif


#ifdef LOCAL_DEBUG_ON
#define STDOUT_DEBUG(x)    std::cout << x << std::endl
#else
#define STDOUT_DEBUG(x)    
#endif





namespace ossie {

  namespace logging {

    // resolve logging config uri from command line
    std::string  ResolveLocalUri( const std::string &logfile_uri,
                                  const std::string &rootPath,
                                  std::string &validated_uri );

    static const std::string DomPrefix("dom");
    static const std::string DevMgrPrefix("devmgr");
    static const std::string DevicePrefix("dev");
    static const std::string ServicePrefix("svc");
    static const std::string ComponentPrefix("rsc");

    class _EmptyLogConfigUri : public LogConfigUriResolver {

    public:

      _EmptyLogConfigUri() {};

      virtual ~_EmptyLogConfigUri(){};

      std::string get_uri( const std::string &path ) { return std::string(""); };

    };

    struct  LogConfigFactoryHolder {
      void               *library;
      LogConfigFactory   factory;
      LogConfigFactoryHolder(): library(NULL), factory(NULL){};
      ~LogConfigFactoryHolder() {
        close();
      };
      void close() {
        if (library) dlclose(library);
        library=NULL;
      }
    };

    static LogConfigUriResolverPtr  _logcfg_resolver;
    static LogConfigFactoryHolder   _logcfg;

    void _LoadLogConfigUriLibrary() {

      // multiple dlopens return the same instance so we need to close each time
      if ( _logcfg.library ) _logcfg.close();
      
      try{
        void* log_library = dlopen("libossielogcfg.so", RTLD_LAZY);
        if (!log_library) {
          RH_NL_DEBUG("ossie.logging", "Cannot load library (libossielogcfg.so) : " << dlerror() );
          throw 1;
        }
        _logcfg.library=log_library;
        // reset errors
        dlerror();
    
        // load the symbols
        ossie::logging::LogConfigFactory logcfg_factory = (ossie::logging::LogConfigFactory) dlsym(log_library, "logcfg_factory");
        const char* dlsym_error = dlerror();
        if (dlsym_error) {
          RH_NL_ERROR( "ossie.logging", "Cannot file logcfg_factory symbol in libossielogcfg.so library: " << dlsym_error);
          throw 2;
        }
        _logcfg.factory=logcfg_factory;

        RH_NL_DEBUG( "ossie.logging",  "ossie.logging: Found libossielogcfg.so for LOGGING_CONFIG_URI resolution." );
      }
      catch( std::exception &e){
      }
      catch( int e){
        if ( e == 2 ) { // library symbol look up error
          if ( _logcfg.library ) dlclose(_logcfg.library); 
          _logcfg.library = 0;
        }
        
      }

    }


    LogConfigUriResolverPtr GetLogConfigUriResolver() {
      
      if ( !_logcfg_resolver ) {
        _LoadLogConfigUriLibrary();
      }
        

      if ( !_logcfg_resolver ) {
        // if the library is missing use empty resolver for backwards compatability
	if ( !_logcfg.factory ) return LogConfigUriResolverPtr( new _EmptyLogConfigUri() );
	_logcfg_resolver = _logcfg.factory();
      }
	
     return _logcfg_resolver;
    }


    std::string GetComponentPath( const std::string &dm,
				  const std::string &wf,
				  const std::string &cid ) {
      std::ostringstream os;
      os << ComponentPrefix << ":" << dm << "/" << wf << "/" << cid;
      return os.str();
    }

    std::string GetDeviceMgrPath( const std::string &dm,
				  const std::string &node  ) {
      std::ostringstream os;
      os << DevMgrPrefix << ":" << dm << "/" << node ;
      return os.str();
    }

    std::string GetDevicePath( const std::string &dm,
				  const std::string &node,
				  const std::string &dev_id) {
      std::ostringstream os;
      os << DevicePrefix << ":" << dm << "/" << node << "/" << dev_id;
      return os.str();
    }

    std::string GetServicePath( const std::string &dm,
				  const std::string &node,
				  const std::string &sname) {
      std::ostringstream os;
      os << ServicePrefix << ":" << dm << "/" << node << "/" << sname;
      return os.str();
    }

    


    MacroTable GetDefaultMacros() {
      MacroTable ctx;
      ctx["@@@HOST.NAME@@@"] = "HOST.NO_NAME";
      ctx["@@@HOST.IP@@@"] = "HOST.NO_IP";
      ctx["@@@NAME@@@"] = "NO_NAME";
      ctx["@@@INSTANCE@@@"] = "NO_INST";
      ctx["@@@PID@@@"] = "NO_PID";
      ctx["@@@DOMAIN.NAME@@@"] = "DOMAIN.NO_NAME";
      ctx["@@@DOMAIN.PATH@@@"] = "DOMAIN.PATH";
      ctx["@@@DEVICE_MANAGER.NAME@@@"] = "DEV_MGR.NO_NAME";
      ctx["@@@DEVICE_MANAGER.INSTANCE@@@"] = "DEV_MGR.NO_INST";
      ctx["@@@SERVICE.NAME@@@"]  = "SERVICE.NO_NAME";
      ctx["@@@SERVICE.INSTANCE@@@"] = "SERVICE.NO_INST";
      ctx["@@@SERVICE.PID@@@"] = "SERVICE.NO_PID";
      ctx["@@@DEVICE.NAME@@@"] = "DEVICE.NO_NAME";
      ctx["@@@DEVICE.INSTANCE@@@"] = "DEVICE.NO_INST";
      ctx["@@@DEVICE.PID@@@"] = "DEVICE.NO_PID";
      ctx["@@@WAVEFORM.NAME@@@"] = "WAVEFORM.NO_NAME";
      ctx["@@@WAVEFORM.INSTANCE@@@"]  = "WAVEFORM.NO_INST";
      ctx["@@@COMPONENT.NAME@@@"] = "COMPONENT.NO_NAME";
      ctx["@@@COMPONENT.INSTANCE@@@"] = "COMPONENT.NO_INST";
      ctx["@@@COMPONENT.PID@@@"] = "COMPONENT.NO_PID";
      return ctx;
    }


    static std::vector< std::string > split_path( const std::string &dpath ) {
      std::string tpath=dpath;
      if ( dpath[0] == '/' ) 
        tpath=dpath.substr(1);
      std::vector< std::string > t;
      // path should be   /domain/<dev_mgr or waveform>
      boost::algorithm::split( t, tpath, boost::is_any_of("/") );
      return t;
    }

    ResourceCtx::ResourceCtx( const std::string &rname,
                              const std::string &rid,
                              const std::string &dpath ) {
                                     
          name =  rname;
          instance_id =  rid;
          dom_path = dpath;
          
          // split path
          std::vector< std::string > t = split_path(dpath);
          if ( t.size() > 1  ) {
            domain_name = t[0];
          }
    }

    void ResourceCtx::apply( MacroTable &tbl ) {
      SetResourceInfo( tbl, *this );
    }

    void ResourceCtx::configure( const std::string &logcfg_uri, int debugLevel ) {
      Configure( logcfg_uri, debugLevel, this  );
    };



    DomainCtx::DomainCtx( const std::string &appName,
			  const std::string &domName,
			  const std::string &domPath ):
      ResourceCtx(appName, "DOMAIN_MANAGER_1", domName+"/"+domName)
    {
      rootPath=domPath;
    }

    void DomainCtx::apply( MacroTable &tbl ) {
      SetResourceInfo( tbl, *this );
    }

    std::string DomainCtx::getLogCfgUri( const std::string &log_uri ) {
      std::string val_uri;
      return ResolveLocalUri(log_uri, rootPath, val_uri);
    }

    void DomainCtx::configure( const std::string &log_uri, int level, std::string &validated_uri ) {
      std::string local_uri= ResolveLocalUri(log_uri, rootPath, validated_uri);
      Configure( local_uri, level, this  );
    }


    ComponentCtx::ComponentCtx( const std::string &cname,
                                     const std::string &cid,
                                  const std::string &dpath ):
      ResourceCtx(cname, cid, dpath)
    {

      // path should be /domain/waveform
      std::vector< std::string > t = split_path(dpath);
      int n=0;
      if ( t.size() > 1 ) {
        domain_name = t[n];
        n++;
      }
      if ( t.size() > 0 ) {
        waveform = t[n];
        waveform_id = t[n];
      }
#if 0    
      std::cout << "ComponentCtx: dpath " << dpath << std::endl;
      std::cout << "ComponentCtx: name/id" << name << "/" << instance_id << std::endl;
      std::cout << "ComponentCtx: domain/waveform" << domain_name << "/" << waveform << std::endl;
#endif
    }

    void ComponentCtx::apply( MacroTable &tbl ) {
      SetComponentInfo( tbl, *this );
    }

    ServiceCtx::ServiceCtx( const std::string &sname,
                              const std::string &dpath ) :
      ResourceCtx(sname,"",dpath)
    {
      // path should be   /domain/devmgr
      std::vector< std::string > t = split_path(dpath);
      int n=0;
      if ( t.size() > 1 ) {
        domain_name = t[n];
        n++;
      }
      if ( t.size() > 0 ) {
        device_mgr = t[n];
      }
    }

    void ServiceCtx::apply( MacroTable &tbl ) {
      SetServiceInfo( tbl, *this );
    }

    DeviceCtx::DeviceCtx( const std::string &dname,
                          const std::string &did,
                          const std::string &dpath ) :
      ResourceCtx(dname,did,dpath)
    {
      // path should be   /domain/devmgr
      std::vector< std::string > t = split_path(dpath);
      int n=0;
      if ( t.size() > 1 ) {
        domain_name = t[n];
        n++;
      }
      if ( t.size() > 0 ) {
        device_mgr = t[n];
      }
    }

    void DeviceCtx::apply( MacroTable &tbl ) {
      SetDeviceInfo( tbl, *this );
    }

    DeviceMgrCtx::DeviceMgrCtx( const std::string &nodeName,
                                const std::string &domName,
                                const std::string &devPath ) :
      ResourceCtx(nodeName,domName+":"+nodeName, domName+"/"+nodeName)
    {
      rootPath=devPath;
      // path should be   /domain/devmgr
      std::vector< std::string > t = split_path(dom_path);
      int n=0;
      if ( t.size() > 0 ) {
        domain_name = t[n];
        n++;
      }
    }

    void DeviceMgrCtx::apply( MacroTable &tbl ) {
      SetDeviceMgrInfo( tbl, *this );
    }

    std::string DeviceMgrCtx::getLogCfgUri( const std::string &log_uri ) {
      std::string val_uri;
      return ResolveLocalUri(log_uri, rootPath, val_uri);
    }

    void DeviceMgrCtx::configure( const std::string &log_uri, int level, std::string &validated_uri ) {
      std::string local_uri= ResolveLocalUri(log_uri, rootPath, validated_uri);
      Configure( local_uri, level, this  );
    }


    void ResolveHostInfo( MacroTable &tbl )  {
      std::string hname("unknown");
      std::string ipaddr("unknown");
      char buf[256];  
  
      if (gethostname(buf,sizeof buf) == 0 ){

        hname = buf;
        struct addrinfo hints, *res;
        struct in_addr addr;
        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_family = AF_INET;

        if ( getaddrinfo(buf, NULL, &hints, &res) == 0) {
          addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
          ipaddr = inet_ntoa(addr);
          freeaddrinfo(res);
        }
      }

      tbl["@@@HOST.NAME@@@"] = hname;
      tbl["@@@HOST.IP@@@"] = ipaddr;
    }

    void SetResourceInfo( MacroTable &tbl, const ResourceCtx &ctx ){
      tbl["@@@DOMAIN.NAME@@@"] = boost::replace_all_copy( ctx.domain_name, ":", "-" );
      tbl["@@@DOMAIN.PATH@@@"] = boost::replace_all_copy( ctx.dom_path, ":", "-" );
      tbl["@@@NAME@@@"] = boost::replace_all_copy( ctx.name, ":", "-" );
      tbl["@@@INSTANCE@@@"] = boost::replace_all_copy( ctx.instance_id, ":", "-" );
      pid_t pid = getpid();
      std::ostringstream os;
      os << pid;
      tbl["@@@PID@@@"] = os.str();
    }

    void SetComponentInfo( MacroTable &tbl, const ComponentCtx &ctx ){
      SetResourceInfo( tbl, ctx );
      tbl["@@@WAVEFORM.NAME@@@"] = boost::replace_all_copy( ctx.waveform, ":", "-" );
      tbl["@@@WAVEFORM.ID@@@"] = boost::replace_all_copy( ctx.waveform_id, ":", "-" );
      tbl["@@@COMPONENT.NAME@@@"] = boost::replace_all_copy( ctx.name, ":", "-" );
      tbl["@@@COMPONENT.INSTANCE@@@"] = boost::replace_all_copy( ctx.instance_id, ":", "-" );
      pid_t pid = getpid();
      std::ostringstream os;
      os << pid;
      tbl["@@@COMPONENT.PID@@@"] = os.str();
    }

    void SetServiceInfo( MacroTable &tbl, const ServiceCtx & ctx ) {
      SetResourceInfo( tbl, ctx );
      tbl["@@@DEVICE_MANAGER.NAME@@@"] =  boost::replace_all_copy( ctx.device_mgr, ":", "-" );
      tbl["@@@DEVICE_MANAGER.INSTANCE@@@"] =  boost::replace_all_copy( ctx.device_mgr_id, ":", "-" );
      tbl["@@@SERVICE.NAME@@@"]  =  boost::replace_all_copy( ctx.name, ":", "-" );
      tbl["@@@SERVICE.INSTANCE@@@"] =  boost::replace_all_copy( ctx.instance_id, ":", "-" );
      pid_t pid = getpid();
      std::ostringstream os;
      os << pid;
      tbl["@@@SERVICE.PID@@@"] = os.str();
    }

    void SetDeviceInfo( MacroTable &tbl, const DeviceCtx & ctx ) {
      SetResourceInfo( tbl, ctx );
      tbl["@@@DEVICE_MANAGER.NAME@@@"] =  boost::replace_all_copy( ctx.device_mgr, ":", "-" );
      tbl["@@@DEVICE_MANAGER.INSTANCE@@@"] =  boost::replace_all_copy( ctx.device_mgr_id, ":", "-" );
      tbl["@@@DEVICE.NAME@@@"] =  boost::replace_all_copy( ctx.name, ":", "-" );
      tbl["@@@DEVICE.INSTANCE@@@"] =  boost::replace_all_copy( ctx.instance_id, ":", "-" );
      pid_t pid = getpid();
      std::ostringstream os;
      os << pid;
      tbl["@@@DEVICE.PID@@@"] = os.str();
    }

    void SetDeviceMgrInfo( MacroTable &tbl, const DeviceMgrCtx & ctx ) {
      SetResourceInfo( tbl, ctx );
      tbl["@@@DEVICE_MANAGER.NAME@@@"] =  boost::replace_all_copy( ctx.name, ":", "-" );
      tbl["@@@DEVICE_MANAGER.INSTANCE@@@"] =  boost::replace_all_copy( ctx.instance_id, ":", "-" );
    }

    static std::string ReplaceString(std::string subject, const std::string& search,
                              const std::string& replace) {
      size_t pos = 0;
      while((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
      }
      return subject;
    }

    //
    // ExpandMacros
    //
    // Process contents of src against of set of macro definitions contained in ctx.
    // The contents of ctx will be searched in the src string and their values
    // replaced with the contents of the their map.
    //
    // @return string object containing any subsitutions
    //
#ifndef USE_REGEX
    std::string ExpandMacros ( const std::string &src, const MacroTable &tbl )
    {
      MacroTable::const_iterator iter=tbl.begin();
      std::string target;
      target = src;
      for(; iter != tbl.end(); iter++ ) {
        std::string res;
        res = ReplaceString( target, iter->first, iter->second );
        target=res;
      }
      
      return target;
    }
#else
    //
    // ExpandMacros
    //
    // Process contents of src against of set of macro definitions contained in ctx.
    // The contents of ctx will be used to generate a set of regular expressions that can
    // search src for tokens and substitute their contents.
    //
    // @return string object containing any subsitutions
    //
    std::string ExpandMacros ( const std::string &src, const MacroTable &ctx )
    {
      //
      // create regular expression and substitutions 
      // from context map
      //
      MacroTable::const_iterator iter=ctx.begin();
      boost::regex         e1;
      std::string          token_exp;        // create list of regex to search for
      std::string          match_exp;        // create list of substitutions when match occurrs
      {
        std::ostringstream tk;
        std::ostringstream mo;
        int cnt=1;
        for( ; iter != ctx.end(); iter++, cnt++ ) {
          tk << "(" << iter->first << ")|";
          mo << "(?" << cnt << iter->second << ")";
        }

#if 0      
        std::cout << " token_exp:" << tk.str() << std::endl;
        std::cout << " mo:" << mo.str() << std::endl;
#endif
        token_exp=tk.str();
        match_exp=mo.str();
      }

      e1.assign(token_exp);

#if 0
      std::string out_name(fileName + std::string(".htm"));
      std::ofstream os(out_name.c_str());
#endif

      // need stream...
      std::ostringstream convertedText(std::ios::out | std::ios::binary);
      std::ostream_iterator<char, char> oi(convertedText);

      // process regex against source text
      boost::regex_replace(oi, src.begin(), src.end(),e1, match_exp, boost::match_default | boost::format_all);

#if 0
      std::cout << "IN: >>" << src << std::endl;
      std::cout << "<<<EOF " << std::endl;
      std::cout << "OUT: >>" << convertedText.str() << std::endl;
      std::cout << "<<<EOF " << std::endl;
      std::cout << convertedText.str() << std::endl;
      os << convertedText.str();
#endif
      return convertedText.str();
    }
#endif


    static std::string _saveConfig ( const std::string &fc ) {
      char xx[32];
      std::strcpy(xx,"XX.rhlog.XXXXXX");
      int fd;
      if ( (fd = mkstemp(xx)) > 0  ) {
        write( fd, fc.c_str(), fc.size() );
        close(fd);
      }
      return std::string(xx);
    }

    rh_logger::LevelPtr ConvertCanonicalLevelToRHLevel ( const std::string &txt_level ) {
      if ( txt_level == "OFF" )   return rh_logger::Level::getOff();
      if ( txt_level == "FATAL" ) return rh_logger::Level::getFatal();
      if ( txt_level == "ERROR" ) return rh_logger::Level::getError();
      if ( txt_level == "WARN" )  return rh_logger::Level::getWarn();
      if ( txt_level == "INFO" )  return rh_logger::Level::getInfo();
      if ( txt_level == "DEBUG" ) return rh_logger::Level::getDebug();
      if ( txt_level == "TRACE")  return rh_logger::Level::getTrace();
      if ( txt_level ==  "ALL" )  return rh_logger::Level::getAll();
      return rh_logger::Level::getInfo();
    };

    int ConvertRHLevelToCFLevel ( rh_logger::LevelPtr l4_level ) {
      if (l4_level == rh_logger::Level::getOff() )   return CF::LogLevels::OFF;
      if (l4_level == rh_logger::Level::getFatal() ) return CF::LogLevels::FATAL;
      if (l4_level == rh_logger::Level::getError() ) return CF::LogLevels::ERROR;
      if (l4_level == rh_logger::Level::getWarn() )  return CF::LogLevels::WARN;
      if (l4_level == rh_logger::Level::getInfo() )  return CF::LogLevels::INFO;
      if (l4_level == rh_logger::Level::getDebug() ) return CF::LogLevels::DEBUG;
      if (l4_level == rh_logger::Level::getTrace() ) return CF::LogLevels::TRACE;
      if (l4_level == rh_logger::Level::getAll() )   return CF::LogLevels::ALL;
	return CF::LogLevels::INFO;
    };


    int ConvertRHLevelToDebug ( rh_logger::LevelPtr rh_level ) {
      if (rh_level == rh_logger::Level::getFatal() ) return 0;
      if (rh_level == rh_logger::Level::getError() ) return 1;
      if (rh_level == rh_logger::Level::getWarn() )  return 2;
      if (rh_level == rh_logger::Level::getInfo() )  return 3;
      if (rh_level == rh_logger::Level::getDebug() ) return 4;
      if (rh_level == rh_logger::Level::getTrace() ) return 5;
      if (rh_level == rh_logger::Level::getAll() )   return 5;
      return 3;
    };
    
    rh_logger::LevelPtr ConvertCFLevelToRHLevel ( int newlevel ) {
      if ( newlevel == CF::LogLevels::OFF )   return rh_logger::Level::getOff();
      if ( newlevel == CF::LogLevels::FATAL ) return rh_logger::Level::getFatal();
      if ( newlevel == CF::LogLevels::ERROR ) return rh_logger::Level::getError();
      if ( newlevel == CF::LogLevels::WARN )  return rh_logger::Level::getWarn();
      if ( newlevel == CF::LogLevels::INFO )  return rh_logger::Level::getInfo();
      if ( newlevel == CF::LogLevels::DEBUG ) return rh_logger::Level::getDebug();
      if ( newlevel == CF::LogLevels::TRACE)  return rh_logger::Level::getTrace();
      if ( newlevel ==  CF::LogLevels::ALL )  return rh_logger::Level::getAll();
      return rh_logger::Level::getInfo();
    }

    CF::LogLevel  ConvertDebugToCFLevel ( const int oldstyle_level ) {
      if ( oldstyle_level == 0 ) return CF::LogLevels::FATAL;
      if ( oldstyle_level == 1 ) return CF::LogLevels::ERROR;
      if ( oldstyle_level == 2 ) return CF::LogLevels::WARN;
      if ( oldstyle_level == 3 ) return CF::LogLevels::INFO;
      if ( oldstyle_level == 4 ) return CF::LogLevels::DEBUG;
      if ( oldstyle_level == 5)  return CF::LogLevels::ALL;
      return CF::LogLevels::INFO;
    }

    rh_logger::LevelPtr ConvertDebugToRHLevel ( const int oldstyle_level ) {
      if ( oldstyle_level == 0 ) return rh_logger::Level::getFatal();
      if ( oldstyle_level == 1 ) return rh_logger::Level::getError();
      if ( oldstyle_level == 2 ) return rh_logger::Level::getWarn();
      if ( oldstyle_level == 3 ) return rh_logger::Level::getInfo();
      if ( oldstyle_level == 4 ) return rh_logger::Level::getDebug();
      if ( oldstyle_level == 5)  return rh_logger::Level::getAll();
      return rh_logger::Level::getInfo();
    }



    void SetLevel( const std::string &logid, int debugLevel) {
      STDOUT_DEBUG( " Setting Logger:" << logid << " OLD STYLE Level:" << debugLevel );
      SetLogLevel( logid, ConvertDebugToCFLevel(debugLevel));
      STDOUT_DEBUG( " Setting Logger: END " << logid << " OLD STYLE Level:" << debugLevel );
    }


    void SetLogLevel( const std::string  &logid, const rh_logger::LevelPtr &newLevel ) {

      STDOUT_DEBUG(" Setting Logger: START log:" << logid << " NEW Level:" << newLevel->toString() );
      rh_logger::LoggerPtr logger;        
      if ( logid == "" ) {
        logger = rh_logger::Logger::getRootLogger();
      }
      else {
        logger = rh_logger::Logger::getLogger( logid );
      }
      if ( logger ) {
	STDOUT_DEBUG( " Setting Redhawk Logger Name/level <" << logid << ">  Level:" << newLevel->toString() );
        logger->setLevel( newLevel );
	STDOUT_DEBUG( " Get name/level <" << logger->getName() << ">/" << logger->getLevel()->toString() );
      }   
      STDOUT_DEBUG( " Setting Logger: END  log:" << logid << " NEW Level:" << newLevel->toString() );
    }

    void SetLogLevel( const std::string  &logid, CF::LogLevel newLevel ) {

      STDOUT_DEBUG(" Setting Logger: START log:" << logid << " NEW Level:" << newLevel );
      rh_logger::LoggerPtr logger;        
      if ( logid == "" ) {
        logger = rh_logger::Logger::getRootLogger();
      }
      else {
        logger = rh_logger::Logger::getLogger( logid );
      }
      if ( logger ) {
        rh_logger::LevelPtr level = ConvertCFLevelToRHLevel( newLevel);
	STDOUT_DEBUG( " Setting Log4 Params id/level <" << logid << ">/" << newLevel << " log4 id/Level4:" << 
		      logger->getName() << "/" << level->toString() );
        logger->setLevel( level );
	STDOUT_DEBUG( " GET log4 name/level <" << logger->getName() << ">/" << logger->getLevel()->toString() );
      }   
      STDOUT_DEBUG( " Setting Logger: END  log:" << logid << " NEW Level:" << newLevel );
    }


    std::string ResolveLocalUri( const std::string &in_logfile_uri, 
                                 const std::string &root_path, 
                                 std::string  &new_uri ) {

      std::string logfile_uri(in_logfile_uri);
      std::string logcfg_uri("");
      if ( !logfile_uri.empty() ) {
        // Determine the scheme, if any.  This isn't a full fledged URI parser so we can
        // get tripped up on complex URIs.  We should probably incorporate a URI parser
        // library for this sooner rather than later
        std::string scheme;
        boost::filesystem::path path;

        std::string::size_type colonIdx = logfile_uri.find(":"); // Find the scheme separator
        if (colonIdx == std::string::npos) {

          scheme = "file";
          path = logfile_uri;
          // Make the path absolute
          boost::filesystem::path logfile_path(path);
          if (! logfile_path.is_complete()) {
            // Get the root path so we can resolve relative paths
            boost::filesystem::path root = boost::filesystem::initial_path();
            logfile_path = boost::filesystem::path(root / path);
          }
          path = logfile_path;
          logfile_uri = "file://" + path.string();

        } else {

          scheme = logfile_uri.substr(0, colonIdx);
          colonIdx += 1;
          if ((logfile_uri.at(colonIdx + 1) == '/') && (logfile_uri.at(colonIdx + 2) == '/')) {
            colonIdx += 2;
          }
          path = logfile_uri.substr(colonIdx, logfile_uri.length() - colonIdx);
        }

        if (scheme == "file") {
          std::string fpath((char*)path.string().c_str());
          logcfg_uri = "file://" + fpath;         
        }
        if (scheme == "sca") {
          std::string fpath((char*)boost::filesystem::path( root_path / path).string().c_str());
          logcfg_uri = "file://" + fpath;
        }
      }

      new_uri = logfile_uri;
      return logcfg_uri;
    }


    /*
    log4j.appender.stdout.Target=System.out\n        \
    */

    std::string  GetDefaultConfig() {
      std::string cfg = "log4j.rootLogger=INFO,STDOUT\n"
"# Direct log messages to STDOUT\n"
"log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n"
"log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n"
"log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{1}:%L - %m%n\n";

      return cfg;
    }

    std::string CacheSCAFile( const std::string &url)
    {
      std::string localPath;

      if ( url.find( "sca:") != 0 )  {
        return localPath;
      }

      std::string::size_type fsPos = url.find("?fs=");
      if (std::string::npos == fsPos) {
        return localPath;
      }

      std::string IOR = url.substr(fsPos + 4);
      CORBA::Object_var obj = ossie::corba::stringToObject(IOR);
      if (CORBA::is_nil(obj)) {
        return localPath;
      }

      CF::FileSystem_var fileSystem = CF::FileSystem::_narrow(obj);
      if (CORBA::is_nil(fileSystem)) {
        return localPath;
      }

      // skip sca: take chars till ?fs=
      std::string remotePath = url.substr(4, fsPos-4);
      CF::OctetSequence_var data;
      try {
        CF::File_var remoteFile = fileSystem->open(remotePath.c_str(), true);
        CORBA::ULong size = remoteFile->sizeOf();
        remoteFile->read(data, size);
      } catch (...) {
        return localPath;
      }

      std::string tempPath = remotePath;
      std::string::size_type slashPos = remotePath.find_last_of('/');
      if (std::string::npos != slashPos) {
        tempPath.erase(0, slashPos + 1);
      }
      std::fstream localFile(tempPath.c_str(), std::ios::out|std::ios::trunc);
      if (!localFile) {
        return localPath;
      }

      if (localFile.write((const char*)data->get_buffer(), data->length())) {
        localPath = tempPath;
      }
      localFile.close();

      return localPath;
    }


    std::string GetSCAFileContents( const std::string &url) throw ( std::exception )
    {
      std::string fileContents;
      std::string::size_type pos;
      pos = url.find( "sca:");
      if ( pos != 0 ) 
        throw std::runtime_error("invalid uri");
      
      std::string::size_type fsPos = url.find("?fs=");
      if (std::string::npos == fsPos)
        throw std::runtime_error("malformed uri, no fs= param");

      std::string IOR = url.substr(fsPos + 4);
      STDOUT_DEBUG( "GetSCAFileContents IOR:" << IOR );
      CORBA::Object_var obj = ossie::corba::stringToObject(IOR);
      if (CORBA::is_nil(obj))
        throw std::runtime_error("cannot access filesystem IOR");

      CF::FileSystem_var fileSystem = CF::FileSystem::_narrow(obj);
      if (CORBA::is_nil(fileSystem))
        throw std::runtime_error("cannot access filesystem IOR");
      
      // grab from sca:<this is what we want>?fs=
      std::string remotePath = url.substr(4, fsPos-4);
      STDOUT_DEBUG("GetSCAFileContents remove path:" << remotePath );
      CF::OctetSequence_var data;
      try {
        CF::File_var remoteFile = fileSystem->open(remotePath.c_str(), true);
        CORBA::ULong size = remoteFile->sizeOf();
        remoteFile->read(data, size);
        remoteFile->close();
      } catch (...) {
        throw std::runtime_error("error reading file contents.");
      }

      fileContents.append((const char*)data->get_buffer(), data->length());
      STDOUT_DEBUG("GetSCAFileContents fileContents:" << fileContents);
      return fileContents;
    }

    std::string GetHTTPFileContents(const std::string &url) throw ( std::exception )
    {
      std::string fileContents;
      std::ostringstream ss;
      //get host name from url
      std::string host;
      std::string address;
      unsigned int pos = 0;
      std::string delimiter = "http://";
      pos = url.find(delimiter);      
      if ( pos != 0 ) 
        throw std::runtime_error("invalid uri");

      host = url.substr(pos+delimiter.length(),url.length());
      delimiter = "/";
      pos = host.find(delimiter);
      address = host.substr(pos+delimiter.length(), host.length());
      host = host.substr(0,pos);
      
      try {
        boost::asio::io_service io_service;

        // Get a list of endpoints corresponding to the server name.
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(host, "http");
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::resolver::iterator end;

        // Try each endpoint until we successfully establish a connection.
        tcp::socket socket(io_service);
        boost::system::error_code error = boost::asio::error::host_not_found;
        while (error && endpoint_iterator != end)
        {
          socket.close();
          socket.connect(*endpoint_iterator++, error);
        }
        if (error)
          throw boost::system::system_error(error);

        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "GET " << address << " HTTP/1.0\r\n";
        request_stream << "Host: " << host << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";

        // Send the request.
        boost::asio::write(socket, request);

        // Read the response status line.
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");

        // Check that response is OK.
        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);
        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
          return "Invalid response\n";
        }
        if (status_code != 200) {
          return "Response returned with status code \n";
        }

        while (boost::asio::read(socket, response,
            boost::asio::transfer_at_least(1), error)) 
          ss << &response;
          fileContents = ss.str();
        if (error != boost::asio::error::eof)
          throw boost::system::system_error(error);
        
      }
      catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
      }
      return fileContents;
    }

    std::string GetConfigFileContents( const std::string &url ) throw (std::exception)
    {
      std::string fileContents("");
      if ( url.find( "file://") == 0 ) { 
        
        std::string fileName( url.begin()+7, url.end() );
        STDOUT_DEBUG(" GetLogConfigFile File:" << fileName );
        std::ifstream fs(fileName.c_str());
        if ( fs.good() ) {
          STDOUT_DEBUG( "GetLogConfigfile: Processing File ...");
          std::ostringstream os;
          std::copy( std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>(), std::ostream_iterator<char>(os) );
          fileContents= os.str();
        }
        else  {
          std::ostringstream os;
          os << "Error reading file contents, local file:" << fileName;
          throw std::runtime_error(os.str() );
        }
      }

      if ( url.find( "sca:") == 0 ) { 
        STDOUT_DEBUG("GetLogConfigfile: Processing SCA File ..." << url );
        // use local file path to grab file
        fileContents = GetSCAFileContents( url );
      }

      if ( url.find( "http:")  == 0 ) { 
        STDOUT_DEBUG("GetLogConfigfile: Processing HTTP File ..." << url );
        fileContents = GetHTTPFileContents( url );
      }

      if ( url.find( "log:") == 0 ) { 
        //
        // RESOLVE .. use logging service to grab file
        //fileContents = getLogConfig( uri+4 );
        //validFile=true;
      }

      if ( url.find("str://") == 0 ){
        std::string fc=url.substr(6);
        if ( fc.find("/") == 0 ) fc = fc.substr(1);
        fileContents = fc;
      }

      STDOUT_DEBUG( "GetLogConfigfile: END File ... fc:" << fileContents );
      return fileContents;
    }

    //
    // Default logging, stdout with level == INFO 
    //
    void  ConfigureDefault() {
        std::string fileContents;
        fileContents = ossie::logging::GetDefaultConfig();
        STDOUT_DEBUG( "Setting Logging Configuration Properties with Default Configuration.: " << fileContents );
        log4cxx::helpers::Properties  props;
        // need to allocate heap object...  log4cxx::helpers::Properties  props takes care of deleting the memory...
        log4cxx::helpers::InputStreamPtr is( new log4cxx::helpers::StringInputStream( fileContents ) );
        props.load(is);
        log4cxx::PropertyConfigurator::configure(props);
    }

    //
    // Default logging configuration method for c++ resource, 1.9 and prior
    // 
    void Configure(const char* logcfgUri, int logLevel)
    {
      STDOUT_DEBUG( " Configure: Pre-1.10 START logging convention" );
      if (logcfgUri) {
        if (strncmp("file://", logcfgUri, 7) == 0) {
          log4cxx::PropertyConfigurator::configure(logcfgUri + 7);
          return;
        } else if (strncmp("sca:", logcfgUri, 4) == 0) {
          // SCA URI; "?fs=" must have been given, or the file will not be located.
          std::string localFile = CacheSCAFile(std::string(logcfgUri));
          if (!localFile.empty()) {
            log4cxx::PropertyConfigurator::configure(localFile.c_str() );
            return;
          }
        }
      }
      else {
        ConfigureDefault();
      }

      STDOUT_DEBUG( " Configure: Pre-1.10 END logging convention" );
      SetLevel( "", logLevel);
    }


    //
    // Current logging configuration method used by Redhawk Resources.  
    //
    //
    void Configure(const std::string &in_logcfgUri, int logLevel, ossie::logging::ResourceCtxPtr ctx )  {
      Configure(in_logcfgUri, logLevel, ctx.get() );
    }

    void Configure(const std::string &in_logcfgUri, int logLevel, ossie::logging::ResourceCtx *ctx )  {
      std::string logcfgUri(in_logcfgUri);

      STDOUT_DEBUG( "ossie::logging::Configure Rel 1.10 START url:" << logcfgUri );
      std::string fileContents("");
      try {

        if ( logcfgUri=="" ) {
          STDOUT_DEBUG(" ossie::logging::Configure Default Configuration" );
          ConfigureDefault();
        }
        else {
          // get configuration file contents
          fileContents = GetConfigFileContents(logcfgUri);

          STDOUT_DEBUG(" ossie::logging::Configure URL configuration::" << fileContents );
         
          // get default macro defintions
          MacroTable tbl=GetDefaultMacros();
          ResolveHostInfo( tbl ) ;
          if ( ctx ) {
            ctx->apply(tbl);
          }

          if ( !fileContents.empty() ) {        
            // set logging configuration
            std::string cfg;
            Configure( fileContents, tbl, cfg );
          }
        }

       }
      catch( std::exception &e){
        std::cerr <<"ERROR: Logging configure, url:" << logcfgUri << std::endl;
        std::cerr <<"ERROR: Logging configure, exception:" << e.what() << std::endl;
       }
      catch(...){
         std::cerr <<" ossie::logging::Configure ExceException during configuration url:" << logcfgUri << std::endl;
       }

      if ( logLevel > -1 ) {
	STDOUT_DEBUG( "ossie::logging::Configure Rel 1.10  LEVEL (oldstyle):" << logLevel );
	SetLevel( "", logLevel);
      }
      else {
	STDOUT_DEBUG( "ossie::logging::Configure Rel 1.10  Using logging config URI as default level." );
      }

    }


    void Configure( const std::string &cfg_data, const MacroTable &tbl, std::string &cfgContents ) 
    {
      std::string fc_raw(cfg_data);
      std::string fileContents;
      std::string fname;
      bool saveTemp=false;
      try{
        LogConfigFormatType ptype= JAVA_PROPS;
        size_t isxml = fc_raw.find("<log4j:configuration");
        if ( isxml != std::string::npos ) ptype= XML_PROPS;

        STDOUT_DEBUG( "MACRO TABLE " );
        MacroTable::const_iterator it = tbl.begin();
        for( ; it != tbl.end(); it++ ) STDOUT_DEBUG(" K/V " << it->first << "/" << it->second );

        STDOUT_DEBUG( "Configure rc:" << fc_raw );
        // process file with macro expansion....
        fileContents= ExpandMacros(fc_raw, tbl);
        STDOUT_DEBUG( "Configure conversion:" << fileContents );

        // if we expanded macros then we need to cache the expansion
        if ( ptype == XML_PROPS ) {
          saveTemp=true;
          fname = _saveConfig(fileContents );
        }

        if ( ptype == XML_PROPS ) {
          STDOUT_DEBUG("Setting Logging Configuration, XML Properties: " << fname );
          log4cxx::xml::DOMConfigurator::configure(fname);
        }
        else {
          STDOUT_DEBUG( "Setting Logging Configuration, Java Properties: " );
          log4cxx::helpers::Properties  props;
          // need to allocate heap object...   log4cxx::helpers::Properties  props takes care of deleting the memory...
          log4cxx::helpers::InputStreamPtr is( new log4cxx::helpers::StringInputStream( fileContents ) );
          props.load(is);
          STDOUT_DEBUG("Setting Logging Configuration,  Properties using StringStream: " );
          log4cxx::PropertyConfigurator::configure(props);

          if (saveTemp)  boost::filesystem::remove(fname);
        }

        cfgContents = fileContents;
      }
      catch(...){
        std::cout << "ERROR: ossie::logging::Configure - configure via stream contents, failed: " << std::endl;
      }

    }

    void Terminate() {
      log4cxx::LogManager::shutdown();
      _logcfg_resolver.reset();
   }


  }; // end of logging namespace


};  // end of ossie namespace
