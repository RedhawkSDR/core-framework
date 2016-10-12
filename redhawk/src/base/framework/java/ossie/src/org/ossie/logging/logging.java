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


package org.ossie.logging;

import java.util.Map;
import java.util.HashMap;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.lang.management.*;  
import java.lang.Exception;
import java.util.Properties;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.Logger;
import org.apache.log4j.LogManager;
import org.apache.log4j.PropertyConfigurator;
import org.apache.log4j.LogManager;
import org.apache.log4j.PatternLayout;
import org.apache.log4j.Layout;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Appender;
import org.apache.log4j.Level;
import org.apache.log4j.xml.DOMConfigurator;
import org.omg.CORBA.ORB;
import CF.LogLevels;


public class logging {

    //
    // This keys of the table are tokens to perform a match search against a stream of data. The values for the
    // associative array are used to replace the contents of the matched tokens.
    //
    // The search tokens and replacements will be use to create a regular expression table that is feed
    // to the boost::regex_replace method.
    //
    public static class MacroTable extends HashMap<String, String > {};
  
    public enum LogConfigFormatType { JAVA_PROPS, XML_PROPS };

    public static class ResourceCtx {
        public String name="";
        public String instance_id="";
        public String domain_name="";
        public String dom_path="";

	public ResourceCtx(  String name,
			     String id,
			     String dpath ) {

	    if ( name != null ) this.name=name;
	    if ( id != null ) this.instance_id=id;
	    if ( dpath != null ) {
		dom_path=dpath;
		String [] seg=_split_path(dpath);
		if (seg.length > 1) {
		    domain_name=seg[0];
		};
	    }
	};
        
        public void apply( MacroTable tbl ) {
            SetResourceInfo( tbl, this );
        };

	public String get_path () {
	    return dom_path;
	};

        protected String [] _split_path( String dpath) {
            String path=dpath;
            if ( path.startsWith("/") ) {
                path=path.substring(1);
            }
            return path.split("/");
        };
    };

    public static class ComponentCtx extends ResourceCtx {
	public String waveform="";
	public String waveform_id="";

	public ComponentCtx(  String name,
			      String id,
			      String dpath )
        {
            super(name, id, dpath);
	    if ( dpath != null ) {
		String [] seg=_split_path(dpath);
		int n=0;
		if ( seg.length > 1 ) {
		    domain_name = seg[n];
		    n++;
		}
		if ( seg.length  > 0 ) {
		    waveform = seg[n];
		    waveform_id = seg[n];
		}
	    }
	    
        };
	public void apply( MacroTable tbl ){
	    SetComponentInfo( tbl, this );
	};
    };

    public static class ServiceCtx extends ResourceCtx {
	public String device_mgr="";
	public String device_mgr_id="";

	public ServiceCtx(  String name,
			    String dpath )
        {
            super(name, "", dpath);
	    if ( dpath != null ){
		String [] seg=_split_path(dpath);
		int n=0;
		if ( seg.length > 1 ) {
		    domain_name = seg[n];
		    n++;
		}
		if ( seg.length  > 0 ) {
		    device_mgr = seg[n];
		}
	    }
	    
        };

	public void apply( MacroTable tbl ){
	    SetServiceInfo( tbl, this );
	};
    };

    public static class DeviceCtx extends ResourceCtx {
	public String device_mgr="";
	public String device_mgr_id="";
	public DeviceCtx(  String name,
			   String id,
			   String dpath )
        {
            super(name, id, dpath);
	    if ( dpath != null ){
		String [] seg=_split_path(dpath);
		int n=0;
		if ( seg.length > 1 ) {
		    domain_name = seg[n];
		    n++;
		}
		if ( seg.length  > 0 ) {
		    device_mgr = seg[n];
		}
	    }

        };

	public void apply( MacroTable tbl ){
	    SetDeviceInfo( tbl, this );
	};
    };

    public static class DeviceMgrCtx extends ResourceCtx {
	public DeviceMgrCtx(  String name,
			      String id,
			      String dpath )
        {
            super(name, id, dpath);
	    if ( dpath != null ) {
		String [] seg=_split_path(dpath);
		int n=0;
		if ( seg.length > 0 ) {
		    domain_name = seg[n];
		    n++;
		}
	    }
        };

	public void apply( MacroTable tbl ){
	    SetDeviceMgrInfo( tbl, this );
	};
    };
      
    //
    // GetDefaultMacros
    //
    // returns a default set of search tokens and replacments strings...
    //
    //public static  HashMap<String,String> GetDefaultMacros() 
    public static  MacroTable GetDefaultMacros() 
    {
        // No ":" , this will fail the regular expression
        MacroTable ctx = new MacroTable();
        ctx.put("@@@HOST.NAME@@@", "HOST.NO_NAME");
        ctx.put("@@@HOST.IP@@@", "HOST.NO_IP");
        ctx.put("@@@NAME@@@", "NO_NAME");
        ctx.put("@@@INSTANCE@@@", "NO_INST");
        ctx.put("@@@PID@@@", "NO_PID");
        ctx.put("@@@DOMAIN.NAME@@@", "DOMAIN.NO_NAME");
        ctx.put("@@@DOMAIN.PATH@@@", "DOMAIN.NO_PATH");
        ctx.put("@@@DEVICE_MANAGER.NAME@@@", "DEV_MGR.NO_NAME");
        ctx.put("@@@DEVICE_MANAGER.INSTANCE@@@", "DEV_MGR.NO_INST");
        ctx.put("@@@SERVICE.NAME@@@", "SERVICE.NO_NAME");
        ctx.put("@@@SERVICE.INSTANCE@@@", "SERVICE.NO_INST");
        ctx.put("@@@SERVICE.PID@@@", "SERVICE.NO_PID");
        ctx.put("@@@DEVICE.NAME@@@", "DEVICE.NO_NAME");
        ctx.put("@@@DEVICE.INSTANCE@@@", "DEVICE.NO_INST");
        ctx.put("@@@DEVICE.PID@@@", "DEVICE.NO_PID");
        ctx.put("@@@WAVEFORM.NAME@@@", "WAVEFORM.NO_NAME");
        ctx.put("@@@WAVEFORM.INSTANCE@@@", "WAVEFORM.NO_INST");
        ctx.put("@@@COMPONENT.NAME@@@", "COMPONENT.NO_NAME");
        ctx.put("@@@COMPONENT.INSTANCE@@@", "COMPONENT.NO_INST");
        ctx.put("@@@COMPONENT.PID@@@", "COMPONENT.NO_PID");
        return ctx;
    };

    //
    // ExpandMacros
    //
    // Process contents of src against of set of macro definitions contained in ctx.
    // The contents of ctx will be used to generate a set of regular expressions that can
    // search src for tokens and substitute their contents.
    //
    // @return string object containing any subsitutions
    //
    public static String  ExpandMacros (  String src,  MacroTable ctx ) {
        String text;
        text = src;
        for( Map.Entry< String, String > entry: ctx.entrySet() ){
            text = text.replaceAll( entry.getKey(), entry.getValue() );
        };
        return text;
    };

   
    public static String GetPid() {
        String name = ManagementFactory.getRuntimeMXBean().getName();
        String []s=name.split("@");
        String ret="-1";
        if ( s.length > 1 ) {
            ret=s[0];
        }
        return ret;
    }

    //
    // ResolveHostInfo
    //
    // Resolve host and ip address where this resource is running
    //
    public static void ResolveHostInfo( MacroTable tbl ) 
    {
        String hname = "unknown";
        String ipaddr = "unknown";
        try {
            InetAddress addr = InetAddress.getLocalHost();
            hname = addr.getHostName();
            ipaddr = addr.getHostAddress();
        } catch (UnknownHostException e) {
	    // failed;  try alternate means.
        };
        tbl.put("@@@HOST.NAME@@@", hname );
        tbl.put("@@@HOST.IP@@@", ipaddr );
    };

    //
    // SetResourceInfo
    //
    // Set the resource context
    //
    public static void SetResourceInfo( MacroTable tbl, ResourceCtx ctx ) {
        tbl.put("@@@DOMAIN.NAME@@@", ctx.domain_name.replaceAll(":", "-" ) );
        tbl.put("@@@NAME@@@",  ctx.name.replaceAll(":", "-" ));
        tbl.put("@@@INSTANCE@@@", ctx.instance_id.replaceAll( ":", "-" ) );
	      tbl.put("@@@PID@@@", GetPid() );
    };

    //
    // SetComponentInfo
    //
    // Set the component context
    //
    public static void SetComponentInfo( MacroTable tbl, ComponentCtx ctx ) {
	SetResourceInfo( tbl, ctx );
	tbl.put("@@@WAVEFORM.NAME@@@", ctx.waveform.replaceAll(":", "-" ) );
	tbl.put("@@@WAVEFORM.ID@@@", ctx.waveform_id.replaceAll(":", "-" ) );
	tbl.put("@@@COMPONENT.NAME@@@", ctx.name.replaceAll(":", "-" ) );
	tbl.put("@@@COMPONENT.INSTANCE@@@", ctx.instance_id.replaceAll(":", "-" ) );
	tbl.put("@@@COMPONENT.PID@@@", GetPid() );
    };

    //
    // SetServiceInfo
    //
    // Set the service context
    //
    public static void SetServiceInfo( MacroTable tbl, ServiceCtx ctx ) {
	SetResourceInfo( tbl, ctx );
	tbl.put("@@@DEVICE_MANAGER.NAME@@@", ctx.device_mgr.replaceAll(":", "-" ) );
	tbl.put("@@@DEVICE_MANAGER.INSTANCE@@@", ctx.device_mgr_id.replaceAll(":", "-" ) );
	tbl.put("@@@SERVICE.NAME@@@", ctx.name.replaceAll(":", "-" ) );
	tbl.put("@@@SERVICE.INSTANCE@@@", ctx.instance_id.replaceAll(":", "-" ) );
	tbl.put("@@@SERVICE.PID@@@", GetPid() );

    };

    //
    // SetDeviceInfo
    //
    // Set the device context
    //
    public static void SetDeviceInfo( MacroTable tbl, DeviceCtx ctx ) {
	SetResourceInfo( tbl, ctx );
	tbl.put("@@@DEVICE_MANAGER.NAME@@@", ctx.device_mgr.replaceAll(":", "-" ) );
	tbl.put("@@@DEVICE_MANAGER.INSTANCE@@@", ctx.device_mgr_id.replaceAll(":", "-" ) );
	tbl.put("@@@DEVICE.NAME@@@", ctx.name.replaceAll(":", "-" ) );
	tbl.put("@@@DEVICE.INSTANCE@@@", ctx.instance_id.replaceAll(":", "-" ) );
	tbl.put("@@@DEVICE.PID@@@", GetPid() );
    };

    //
    // SetDeviceMgrInfo
    //
    // Set the device manager context
    //
    public static void SetDeviceMgrInfo( MacroTable tbl, DeviceMgrCtx ctx ) {
	SetResourceInfo( tbl, ctx );
	tbl.put("@@@DEVICE_MANAGER.NAME@@@", ctx.name.replaceAll(":", "-" ) );
	tbl.put("@@@DEVICE_MANAGER.INSTANCE@@@", ctx.instance_id.replaceAll(":", "-" ) );
    };


    public static int ConvertLog4ToCFLevel ( Level l4_level ) {
	if (l4_level == Level.OFF )   return CF.LogLevels.OFF;
	if (l4_level == Level.FATAL ) return CF.LogLevels.FATAL;
	if (l4_level == Level.ERROR ) return CF.LogLevels.ERROR;
	if (l4_level == Level.WARN )  return CF.LogLevels.WARN;
	if (l4_level == Level.INFO )  return CF.LogLevels.INFO;
	if (l4_level == Level.DEBUG ) return CF.LogLevels.DEBUG;
	if (l4_level == Level.TRACE ) return CF.LogLevels.TRACE;
	if (l4_level == Level.ALL )   return CF.LogLevels.ALL;
	return CF.LogLevels.INFO;
    };

    public static Level ConvertToLog4Level( int newlevel ) {
        if ( newlevel == CF.LogLevels.OFF ) return Level.OFF;
        if ( newlevel == CF.LogLevels.FATAL ) return Level.FATAL;
        if ( newlevel == CF.LogLevels.ERROR ) return Level.ERROR;
        if ( newlevel == CF.LogLevels.WARN )  return Level.WARN;
        if ( newlevel == CF.LogLevels.INFO )  return Level.INFO;
	if ( newlevel == CF.LogLevels.DEBUG ) return Level.DEBUG;
	if ( newlevel == CF.LogLevels.TRACE ) return Level.TRACE;
        if ( newlevel ==  CF.LogLevels.ALL )  return Level.ALL;
        return Level.INFO;
    };

    public static int ConvertLogLevel( int oldstyle_level ) {
	if ( oldstyle_level == 0 ) return CF.LogLevels.FATAL;
	if ( oldstyle_level == 1 ) return CF.LogLevels.ERROR;
	if ( oldstyle_level == 2 ) return CF.LogLevels.WARN;
	if ( oldstyle_level == 3 ) return CF.LogLevels.INFO;
	if ( oldstyle_level == 4 ) return CF.LogLevels.DEBUG;
	if ( oldstyle_level == 5)  return CF.LogLevels.ALL;
	return CF.LogLevels.INFO;
    };

    public static void SetLevel( String  logid, int oldstyle_level ) {
        SetLogLevel( logid, ConvertLogLevel(oldstyle_level) );
    };


    public static void SetLogLevel(  String  logid, int newLogLevel ) {
        Level tlevel=Level.INFO;
        tlevel = ConvertToLog4Level( newLogLevel );
	Logger logger = null;
	if ( logid != null )  {
	    logger = Logger.getLogger(logid);
	}
	else {
	    logger = Logger.getRootLogger();
	}
        if ( logger != null  ) {
            logger.setLevel(tlevel);
        };
    };

    //
    // GetDefaultConfig
    //
    // return default logging configuration information
    //
    public static String  GetDefaultConfig() {
        String cfg = "log4j.rootLogger=INFO,STDOUT\n " + 
	    "# Direct log messages to STDOUT\n" + 
	    "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + 
	    "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" +
	    "log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{1}:%L - %m%n\n";
	return cfg;
    };

    // 
    // Get Logging Configuration file from an SCA file system and
    // copy to local file system
    //
    public static String  CacheSCAFile( String sca_url ) {
        String localPath = "";
        String uri = sca_url;

        if ( sca_url.startsWith("sca:") == false ) {
            return localPath;
        }
        
        uri= sca_url.substring(4);
        int fsPos = uri.indexOf("?fs=");
        if (fsPos == -1) {
            return localPath;
        }
        
        org.omg.CORBA.ORB orb;
        orb = ORB.init((String[]) null, null);
        
        String IOR = uri.substring(fsPos + 4);
        org.omg.CORBA.Object obj = orb.string_to_object(IOR);
        if (obj == null) {
            return localPath;
        }

        CF.FileSystem fileSystem = CF.FileSystemHelper.narrow(obj);
        if (fileSystem == null) {
            return localPath;
        }

        String remotePath = uri.substring(0, fsPos);
        CF.OctetSequenceHolder data = new CF.OctetSequenceHolder ();
        try {
            CF.File remoteFile = fileSystem.open(remotePath, true);
            int size = remoteFile.sizeOf();
            remoteFile.read(data, size);

            String tempPath = remotePath;
            int slashPos = remotePath.lastIndexOf('/');
            if (slashPos != -1) {
                tempPath = tempPath.substring(slashPos + 1);
            }
            
            FileOutputStream localFile = new FileOutputStream(tempPath);
            localFile.write(data.value);
            localPath = tempPath;
            localFile.close();
            return localPath;
        } catch (Exception e){
            return localPath;
        }
    };

    // 
    // Get Logging Configuration file from an SCA file system and
    // return the contents as a string
    //
    // @param  sca_url : format as follows sca://?fs=<file system IOR>/path/to/file
    //
    public static String  GetSCAFileContents(  String sca_url) throws Exception {
        String uri = sca_url;

        if ( sca_url.startsWith("sca:") == false ) {
            throw new Exception("invalid URL for sca: file");
        }
        
        uri= sca_url.substring(4);
        int fsPos = uri.indexOf("?fs=");
        if (fsPos == -1) {
            throw new Exception("malformed url, missing fs=param");
        }
        
        org.omg.CORBA.ORB orb;
        orb = ORB.init((String[]) null, null);
        
        String IOR = uri.substring(fsPos + 4);
        org.omg.CORBA.Object obj = orb.string_to_object(IOR);
        if (obj == null) {
            throw new Exception("cannot access filesystem IOR");
        }

        CF.FileSystem fileSystem = CF.FileSystemHelper.narrow(obj);
        if (fileSystem == null) {
            throw new Exception("cannot access filesystem IOR");
        }

        String remotePath = uri.substring(0, fsPos);
        CF.OctetSequenceHolder data = new CF.OctetSequenceHolder ();
        CF.File remoteFile = null;
        try {
            remoteFile = fileSystem.open(remotePath, true);
            int size = remoteFile.sizeOf();
            remoteFile.read(data, size);
        } catch (Exception e){
            throw new Exception("error reading file contents");     
        }
        finally {
            if ( remoteFile != null ) {
                remoteFile.close();
            }
        }

        String fileContents = new String(data.value,"UTF-8");
        return fileContents;
    };


    private  static String readFile( String file ) throws IOException {
        BufferedReader reader = new BufferedReader( new FileReader (file));
        String         line = null;
        StringBuilder  stringBuilder = new StringBuilder();
        String         ls = System.getProperty("line.separator");
        try {
            while ( ( line = reader.readLine() ) != null ) {
                stringBuilder.append( line );
                stringBuilder.append( ls );
            }
            return stringBuilder.toString();
        } finally {
          reader.close();
        }
    }

    //
    // Resolve the location of the URL and return its contents as a string.
    //
    //
    //
    public static String  GetConfigFileContents(  String url ) throws Exception {

        String fileContents = "";
        if ( url.startsWith( "file://") ) { 
            String fileName = url.substring( 7 );
            fileContents = readFile( fileName );
        };
      
        if ( url.startsWith( "sca:") ) { 
            fileContents = GetSCAFileContents( url );
        };

        if ( url.startsWith( "http:")  ) { 
            // RESOLVE .. need to grab contents of remote file via http
            //fileContents = getLogConfig( uri+5 );
            //validFile=true;
        };

        if ( url.startsWith( "log:") ) { 
            //
            // RESOLVE .. use logging service to grab file
            //fileContents = getLogConfig( uri+4 );
            //validFile=true;
        };

        if ( url.startsWith("str://")  ){
            String fc=url.substring(6);
            if ( fc.startsWith("/") ) {
                fc=fc.substring(1);
            }
            fileContents = fc;
        };

        return fileContents;

    };


    //
    // Default logging, stdout with level == INFO 
    //
    public static void  ConfigureDefault() {
	//System.out.println("ConfigureDefault START" );
        String fileContents;
        fileContents = GetDefaultConfig();
        try {
            ByteArrayInputStream is = new ByteArrayInputStream(fileContents.getBytes("UTF-8"));
            Properties props = new Properties();
            props.load(is);
            PropertyConfigurator.configure(props);
        }
        catch(Exception e  ){
	    System.out.println("Log4J Exception:" + e.getMessage() );
        }
	//System.out.println("ConfigureDefault END" );
    }



    //
    // Used by a resource to configure the logging provider 
    //
    //
    public static void Configure( String logcfgUri, int logLevel ) {

        if ( logcfgUri == "" ||  logcfgUri.length() == 0 ){
            ConfigureDefault();
        }
        else{
            String fileContents="";
	    try {
		fileContents = GetConfigFileContents(logcfgUri);
	    }
	    catch( Exception e ) {
		System.out.println("org.ossie.logging.logging.Configure,  Exception:" + e.getMessage() );
	    }

            if ( fileContents.length() != 0 ) {
		try {
		    ByteArrayInputStream is = new ByteArrayInputStream(fileContents.getBytes("UTF-8"));
		    Properties props = new Properties();
		    props.load(is);
		    PropertyConfigurator.configure(props);
		}
		catch(Exception e  ){
		    System.out.println("org.ossie.logging.logging.Configure,  Exception:" + e.getMessage() );

		}
            }

        }

	SetLevel( null, logLevel );
    };


    //
    // Used by a resource to configure the logging provider 
    //
    //
    public static void Configure( String logcfgUri, int logLevel, ResourceCtx ctx) {

        if ( logcfgUri == "" ||  logcfgUri.length() == 0 ){
            ConfigureDefault();
        }
        else{
            String fileContents="";
	    try {
		fileContents = GetConfigFileContents(logcfgUri);
	    }
	    catch( Exception e ) {
		// RESOLVE
	    }
		

            MacroTable tbl = GetDefaultMacros();
            if ( ctx != null ) {
                ctx.apply(tbl);
            }

            if ( fileContents.length() != 0 ) {
                try {
                    String cfg="";
                    cfg = Configure( fileContents, tbl );
                }
                catch(Exception e  ){
                }
            }

        }

	if ( logLevel > -1 ) {
	    SetLevel( null, logLevel );
	}
    };

    //
    // Configure
    //
    // Apply the macro defintion table against the contents of log4j 
    // configuration file (xml or java props) and set the appropriate 
    // log4cxx configuration.
    //
    // @param configure_data contents of log4j properties or xml file
    // @param MacroTable set of tokens to match and substitution values
    // @param cfgContents  returns converted form of configure_data if macro definitions were applied
    //
    public static  String Configure(  String  cfg_data,  MacroTable tbl ) {
	String fc_raw= cfg_data;
	String fileContents="";
	String fname;
	boolean saveTemp=false;
	try{
	    LogConfigFormatType ptype= LogConfigFormatType.JAVA_PROPS;
	    if ( fc_raw.contains("<log4j:configuration") )  {
		ptype= LogConfigFormatType.XML_PROPS;
	    }

	    // process file with macro expansion....
	    fileContents= ExpandMacros(fc_raw, tbl);

	    // if we expanded macros then we need to cache the expansion
	    if ( ptype == LogConfigFormatType.XML_PROPS ) {
		try {
		    //RESOLVE -- need unit name
		    //create a temp file
		    File temp = File.createTempFile("tmp", ".xml"); 
 
		    //write it
		    BufferedWriter bw = new BufferedWriter(new FileWriter(temp));
		    bw.write(fileContents);
		    bw.close(); 
		    DOMConfigurator.configure(temp.getAbsolutePath());
		    temp.delete();
		}
		catch( IOException e ){
		    // RESOLVE
		}
	    }
	    else {
		ByteArrayInputStream is = new ByteArrayInputStream(fileContents.getBytes("UTF-8"));
		Properties props = new Properties();
		props.load(is);
		PropertyConfigurator.configure(props);
	    }

	}
	catch(Exception e){
	    // RESOLVE
	}
        return fileContents;
    };   


};  // namespace wrapper for logging convenience routines


