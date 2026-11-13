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
#ifndef LOG_HELPERS_H
#define LOG_HELPERS_H

#include <string>
#include <vector>
#include <map>
#include <exception>
#include <boost/shared_ptr.hpp>
#include <ossie/CF/LogInterfaces.h>
#include <ossie/logging/rh_logger.h>
#include <ossie/logging/LogConfigUriResolver.h>

/*
The logging namespace contain common routines for configuration
and manipulation of the logging interface.
*/

namespace ossie
{

  namespace logging {

    // resolve logging config uri from command line
    std::string  ResolveLocalUri( const std::string &logfile_uri,
                                  const std::string &rootPath,
                                  std::string &validated_uri );

    std::string GetDeviceMgrPath( const std::string &dm,
                                  const std::string &node );

    std::string GetComponentPath( const std::string &dm,
				  const std::string &wf,
				  const std::string &cid );

    std::string GetDevicePath( const std::string &dm,
				  const std::string &node,
			       const std::string &dev_id);

    std::string GetServicePath( const std::string &dm,
				  const std::string &node,
				const std::string &sname);

    // Returns object to assist with resolving LOGGING_CONFIG_URI value for all Domain resources
    LogConfigUriResolverPtr   GetLogConfigUriResolver();
    

    /* 
      Default Logging Macro Table tokens available for substitution in a log4j properties or xml configuration file

      -- tokens that are resolved for all resource object types
      ctx["@@@HOST.NAME@@@"]               -- return from gethostname
      ctx["@@@HOST.IP@@@"]                 -- return from getaddrinfo
      ctx["@@@NAME@@@"]                    -- resource name 
      ctx["@@@INSTANCE@@@"]                -- resource instance identifier
      ctx["@@@PID@@@"]                     -- return from getpid
      ctx["@@@DOMAIN.NAME@@@"]             -- Domain Name the resource is running under
      ctx["@@@DOMAIN.PATH@@@"]             -- Path to Domain's root in the NamingService  (DOM_PATH) from resource launching

      -- DeviceManager's specific tokens
      ctx["@@@DEVICE_MANAGER.NAME@@@"]     -- Name of DeviceManager, available for DeviceManager, Device, Service 
      ctx["@@@DEVICE_MANAGER.INSTANCE@@@"] -- Instance ID of DeviceManager, available for DeviceManager, Device, Service 

      -- Service's specific tokens
      ctx["@@@SERVICE.NAME@@@"]            -- Service Name, taken from command line
      ctx["@@@SERVICE.INSTANCE@@@"]        -- Instance ID of the Service, taken from command line
      ctx["@@@SERVICE.PID@@@"]             -- process ID of the resource, result of getpid for the Service process

      -- Device's specific tokens
      ctx["@@@DEVICE.NAME@@@"]             -- Device Name, taken from command line
      ctx["@@@DEVICE.INSTANCE@@@"]         -- Instance ID of the Device, taken from command line
      ctx["@@@DEVICE.PID@@@"]              -- process ID of the resource,result of getpid for the Device process

      -- Component's specific tokens
      ctx["@@@WAVEFORM.NAME@@@"]           -- Waveform name associated the Component, taken from DOM_PATH command line param
      ctx["@@@WAVEFORM.INSTANCE@@@"]       -- Waveform instance ID for a the Component, taken from DOM_PATH command line param
      ctx["@@@COMPONENT.NAME@@@"]          -- Component Name, taken from DOM_PATH command line param
      ctx["@@@COMPONENT.INSTANCE@@@"]      -- Component Instance ID, taken from the command line
      ctx["@@@COMPONENT.PID@@@"]           -- process ID of the resource, result of getpid for the Component process
    */

    //
    // This keys of the table are tokens to perform a match search against a stream of data. The values for the
    // associative array are used to replace the contents of the matched tokens.
    //
    typedef std::map<  std::string, std::string >       MacroTable;
  
    typedef enum { JAVA_PROPS=0, XML_PROPS }            LogConfigFormatType;


    struct ResourceCtx {
      std::string name;
      std::string instance_id;
      std::string domain_name;
      std::string dom_path;
      ResourceCtx( const std::string &name,
		    const std::string &id,
		    const std::string &dpath );
      
      virtual ~ResourceCtx() {};
      virtual void apply( MacroTable &tbl );
      virtual std::string getLogCfgUri( const std::string &logcfg_uri ) { return logcfg_uri; };
      virtual void configure( const std::string &logcfg_uri, int debugLevel );
      std::string  get_path( ) { return dom_path; };

    };


    struct DomainCtx : public ResourceCtx {
      std::string   rootPath;
      DomainCtx( const std::string &appName,
                 const std::string &domName,
                 const std::string &domPath );
      ~DomainCtx() {};
      virtual void apply( MacroTable &tbl );
      virtual std::string getLogCfgUri( const std::string &logcfg_uri );
      void configure( const std::string &logcfg_uri,
                      int debugLevel,
                      std::string &validated_uri);
    };

    struct ComponentCtx : public ResourceCtx {
      std::string waveform;
      std::string waveform_id;

      ComponentCtx( const std::string &name,
		    const std::string &id,
		    const std::string &dpath );
      ~ComponentCtx() {};
      virtual void apply( MacroTable &tbl );
    };

    struct ServiceCtx : public ResourceCtx {
      std::string device_mgr;
      std::string device_mgr_id;
      ServiceCtx( const std::string &name,
		  const std::string &dpath );
      ~ServiceCtx() {};
      virtual void apply( MacroTable &tbl );
    };

    struct DeviceCtx : public ResourceCtx {
      std::string device_mgr;
      std::string device_mgr_id;
      DeviceCtx( const std::string &name,
		 const std::string &id,
		  const std::string &dpath );
      ~DeviceCtx() {};
      virtual void apply( MacroTable &tbl );
    };

    struct DeviceMgrCtx : public ResourceCtx {
      std::string  rootPath;
      DeviceMgrCtx( const std::string &nodeName,
                    const std::string &domName,
                    const std::string &devPath );
      ~DeviceMgrCtx() {};
      virtual void apply( MacroTable &tbl );
      virtual std::string getLogCfgUri( const std::string &logcfg_uri );
      void configure( const std::string &logcfg_uri,
                      int debugLevel,
                      std::string &validated_uri);
    };


    //
    // Resource Context information used to resolve macros in configuration files
    //
    typedef boost::shared_ptr< ossie::logging::ResourceCtx >    ResourceCtxPtr;
      
    //
    // GetDefaultMacros
    //
    // returns a default set of search tokens and replacments strings...
    //
    MacroTable GetDefaultMacros();

    //
    // ExpandMacros
    //
    // Process contents of src against of set of macro definitions contained in ctx.
    // The contents of ctx will be used to generate a set of regular expressions that can
    // search src for tokens and substitute their contents.
    //
    // @return string object containing any subsitutions
    //
    std::string  ExpandMacros ( const std::string &src, const MacroTable &ctx );

    //
    // ResolveHostInfo
    //
    // Resolve host and ip address where this resource is running
    //
    void ResolveHostInfo( MacroTable &tbl );

    //
    // SetResourceInfo
    //
    // Set the resource context
    //
    void SetResourceInfo( MacroTable &tbl, const ResourceCtx &ctx );

    //
    // SetComponentInfo
    //
    // Apply ComponentCtx settings to the component's macro set
    //
    void SetComponentInfo( MacroTable &tbl, const ComponentCtx &ctx );

    //
    // SetServiceInfo
    //
    // Apply ServiceCtx settings to the service's macro set
    //
    void SetServiceInfo( MacroTable &tbl, const ServiceCtx &ctx );

    //
    // SetDeviceInfo
    //
    // Apply DeviceCtx settings to the device's macro set
    //
    void SetDeviceInfo( MacroTable &tbl, const DeviceCtx &ctx );

    //
    // SetDeviceMgrInfo
    //
    // Apply DeviceMgrCtx settings to the device manager's macro set
    //
    void SetDeviceMgrInfo( MacroTable &tbl, const DeviceMgrCtx &ctx );


    //
    // ConvertDebugToCFLevel
    //
    // Convert from command line argument debug level to CF::Logging
    //
    CF::LogLevel ConvertDebugToCFLevel( int oldstyle_level );


    //
    // ConvertDebugToRHLevel
    //
    // Convert from command line argument debug level to CF::Logging
    //
    rh_logger::LevelPtr ConvertDebugToRHLevel( int oldstyle_level );


    //
    // ConvertRHLevelToDebug
    //
    // Convert from rh logger level to command line debug level
    //
    int ConvertRHLevelToDebug ( rh_logger::LevelPtr rh_level );


    //
    // ConvertRHLevelToCFLevel
    //
    // Convert from rh logger level to CF::Logging
    //
    int ConvertRHLevelToCFLevel ( rh_logger::LevelPtr l4_level );


    //
    // ConvertCanonicalLevelToRHLevel
    //
    // Convert from string to rh logger Level
    //
    rh_logger::LevelPtr ConvertCanonicalLevelToRHLevel( const std::string &txt_level );

    //
    // ConvertCFLevelToRHLevel
    //
    // Convert from CF::LogLevel to rh logger Level
    //
    rh_logger::LevelPtr ConvertCFLevelToRHLevel( int newlevel );


    //
    // Set the name logger's level using command line debug values
    //
    void SetLevel( const std::string  &logid, int debugLevel );


    //
    // Set the name logger to the specified rh_logger::Level 
    //
    void SetLogLevel( const std::string  &logid, const rh_logger::LevelPtr &newLevel );


    //
    // Set the name logger to the specified CF::LogLevel value
    //
    void SetLogLevel( const std::string  &logid, CF::LogLevel newLevel );


    //
    // GetDefaultConfig
    //
    // return default logging configuration information
    //
    std::string  GetDefaultConfig();


    // 
    // Get Logging Configuration file from an SCA file system and
    // copy to local file system
    //
    std::string  CacheSCAFile( const std::string &sca_url );


    // 
    // Get Logging Configuration file from an SCA file system and
    // return the contents as a string
    //
    // @param  sca_url : format as follows sca://?fs=<file system IOR>/path/to/file
    //
    std::string  GetSCAFileContents( const std::string &sca_url);

    //
    // Resolve the location of the URL and return its contents as a string.
    //
    // url format as follows:
    //   file:///path/to/file           -- looks on local file system for the file
    //   sca://path/to/file?fs=IOR      -- use fs=IOR to resolve to CF::FileSystem object and then open /path/to/file
    //   http://host/path/to/file       -- grab contents of a the URL and read data,  (currently not supported)
    //   log://lookup_key               -- use logging configuration service interface to contents of configuration file. (currently not supported)
    //                                     lookup_key is resolved by configuration service, e.g /domain/waveform/componentname or /waveform_class
    //   str://<log4 file contents>     -- interprets the characters past the str:// as the configuration file contents (currently not supported)
    //
    //
    // @url   url formatted location to the file
    //
    std::string  GetConfigFileContents( const std::string &url );

    // 
    //
    // Set the default logging configuration to be standard out and msg level == Info
    //
    void ConfigureDefault();

    //
    // 1.9 and prior logging configuration method,
    //
    // Set the default logging configuration to be standard out and msg level == Info
    //
    void Configure(const char* logcfgUri="", int logLevel=3 );

    //
    // Used by a resource to configure the logging provider 
    //
    // @param configure_data contents of log4j properties or xml file  
    // @param logLevel  -1 do not set level, > 0 set level of root logger
    // @param ctx execution context for the resource
    //
    void Configure(const std::string &logcfgUri, int logLevel, ResourceCtxPtr ctx);
    void Configure(const std::string &logcfgUri, int logLevel, ResourceCtx *ctx);


    //
    // Configure
    //
    // Apply the macro defintion table against the contents of log4j 
    // configuration file (xml or java props) and set the appropriate 
    // log4cxx configuration.
    //
    // This method allows for custom configuration files and MacroTables to be used an resolved.
    //
    // @param configure_data contents of log4j properties or xml file
    // @param MacroTable set of tokens to match and substitution values
    // @param cfgContents  returns converted form of configure_data if macro definitions were applied
    //
    void Configure( const std::string  &configure_data, const MacroTable &ctx, std::string &cfgContents );   

    //
    // Terminate
    //   Teriminate the logging context because we are shutting down
    //   
    void Terminate();


  };  // end logging interface

};  // end ossie namespace
#endif
