#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK core.
#
# REDHAWK core is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import logging
import socket
import os
from   ossie.cf import CF
import urlparse
import urllib
import ossie.utils.log4py.config
from   ossie.utils.log4py import RedhawkLogger
from  omniORB import CORBA

def GetDefaultMacros():
    ctx={}
    ## No ":" , this will fail the regular expression
    ctx["@@@HOST.NAME@@@"] = "HOST.NO_NAME";
    ctx["@@@HOST.IP@@@"] = "HOST.NO_IP";
    ctx["@@@NAME@@@"] = "NO_NAME";
    ctx["@@@INSTANCE@@@"] = "NO_INST";
    ctx["@@@PID@@@"] = "NO_PID";
    ctx["@@@DOMAIN.NAME@@@"] = "DOMAIN.NO_NAME";
    ctx["@@@DOMAIN.PATH@@@"] = "DOMAIN.NO_PATH";
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
    return ctx

def ResolveHostInfo( tbl ):
    h=socket.gethostname()
    addr="unknown"
    try:
       a=socket.getaddrinfo(h,0)
       addr=a[0][4][0]
    except:
        pass
    tbl["@@@HOST.NAME@@@"] = h
    tbl["@@@HOST.IP@@@"] = addr

class ResourceCtx:
    def __init__(self, name, id, dpath):
        self.name=name
        self.instance_id=id
        self.dom_path = dpath
        seg=self._split_path(dpath)
        if len(seg) > 0 :
            self.domain_name=seg[0]

    def apply(self, tbl):
        SetResourceInfo(tbl,self)

    def get_path(self):
        return self.dom_path

    def _split_path(self,dpath=''):
        path=dpath
        if path.startswith('/'):
            path=path[1:]
        return path.split('/')

class DeviceMgrCtx(ResourceCtx):
    def __init__(self, name, id, dpath):
       ResourceCtx.__init__(self, name, id, dpath)
       seg=self._split_path(dpath)
       if len(seg) > 0 :
            self.domain_name=seg[0]

    def apply(self, tbl):
        SetDeviceMgrInfo(tbl,self)

class ComponentCtx(ResourceCtx):
    def __init__(self, name, id, dpath):
        ResourceCtx.__init__(self, name, id, dpath)
        self.waveform=""
        self.waveform_id=""
        n=0
        seg=self._split_path(dpath)
        if len(seg) > 1 :
            self.domain_name=seg[n]
            n = n + 1
        if len(seg) > 0 :
            self.waveform=seg[n]
            self.waveform_id=seg[n]

    def apply(self, tbl):
        SetComponentInfo(tbl=tbl,ctx=self)

class ServiceCtx(ResourceCtx):
    def __init__(self, name, dpath):
       ResourceCtx.__init__(self, name, "", dpath)
       self.device_mgr=""
       self.device_mgr_id=""
       n=0
       seg=self._split_path(dpath)
       if len(seg) > 1 :
            self.domain_name=seg[n]
            n = n + 1
       if len(seg) > 0 :
            self.device_mgr=seg[n]

    def apply(self, tbl):
        SetServiceInfo(tbl,self)

class DeviceCtx(ResourceCtx):
    def __init__(self, name, id, dpath):
       ResourceCtx.__init__(self, name, id, dpath)
       self.device_mgr=""
       self.device_mgr_id=""
       n=0
       seg=self._split_path(dpath)
       if len(seg) > 1 :
            self.domain_name=seg[n]
            n = n + 1
       if len(seg) > 0 :
            self.device_mgr=seg[n]

    def apply(self, tbl):
        SetDeviceInfo(tbl,self)


def SetResourceInfo( tbl, ctx ):
    tbl["@@@DOMAIN.NAME@@@"] = ctx.domain_name.replace(":", "-" )
    tbl["@@@DOMAIN.PATH@@@"] = ctx.dom_path.replace(":", "-" )
    tbl["@@@NAME@@@"] =  ctx.name.replace( ":", "-" )
    tbl["@@@INSTANCE@@@"] = ctx.instance_id.replace( ":", "-" )
    tbl["@@@PID@@@"] = str(os.getpid())


def SetComponentInfo( tbl, ctx ):
    """
    Adding entries in tbl based on ctx.
    """
    SetResourceInfo( tbl, ctx )
    tbl["@@@WAVEFORM.NAME@@@"] = ctx.waveform.replace( ":", "-" )
    tbl["@@@WAVEFORM.ID@@@"] = ctx.waveform_id.replace( ":", "-" )
    tbl["@@@COMPONENT.NAME@@@"] = ctx.name.replace( ":", "-" )
    tbl["@@@COMPONENT.INSTANCE@@@"] = ctx.instance_id.replace( ":", "-" )
    tbl["@@@COMPONENT.PID@@@"] = str(os.getpid())

def SetServiceInfo( tbl, ctx ):
    SetResourceInfo( tbl, ctx )
    tbl["@@@DEVICE_MANAGER.NAME@@@"] =  ctx.device_mgr.replace( ":", "-" )
    tbl["@@@DEVICE_MANAGER.INSTANCE@@@"] =  ctx.device_mgr_id.replace( ":", "-" )
    tbl["@@@SERVICE.NAME@@@"]  =  ctx.name.replace( ":", "-" )
    tbl["@@@SERVICE.INSTANCE@@@"] =  ctx.instance_id.replace( ":", "-" )
    tbl["@@@SERVICE.PID@@@"] = str(os.getpid())

def SetDeviceInfo( tbl, ctx ):
    SetResourceInfo( tbl, ctx )
    tbl["@@@DEVICE_MANAGER.NAME@@@"] =  ctx.device_mgr.replace( ":", "-" )
    tbl["@@@DEVICE_MANAGER.INSTANCE@@@"] =  ctx.device_mgr_id.replace( ":", "-" )
    tbl["@@@DEVICE.NAME@@@"]  =  ctx.name.replace( ":", "-" )
    tbl["@@@DEVICE.INSTANCE@@@"] =  ctx.instance_id.replace( ":", "-" )
    tbl["@@@DEVICE.PID@@@"] = str(os.getpid())

def SetDeviceMgrInfo( tbl, ctx ):
    SetResourceInfo( tbl, ctx )
    tbl["@@@DEVICE_MANAGER.NAME@@@"]  =  ctx.name.replace( ":", "-" )
    tbl["@@@DEVICE_MANAGER.INSTANCE@@@"] =  ctx.instance_id.replace( ":", "-" )


def ExpandMacros( source,  macrotable ):
    """
    Perform a find-and-replace on the source based on the find/replace
    values storred in macrotable.
    """
    text=source
    if source and macrotable:
        for i, j in macrotable.iteritems():
            text = text.replace(i, j)
    return text

def ConvertLogLevel( oldstyle_level ):
    if  oldstyle_level == 0 :
        return CF.LogLevels.FATAL
    if  oldstyle_level == 1 :
        return CF.LogLevels.ERROR
    if  oldstyle_level == 2 :
        return CF.LogLevels.WARN
    if  oldstyle_level == 3 :
        return CF.LogLevels.INFO
    if  oldstyle_level == 4 :
        return CF.LogLevels.DEBUG
    if  oldstyle_level == 5 :
        return CF.LogLevels.ALL
    return CF.LogLevels.INFO

def ConvertLog4ToCFLevel( log4level ):
      if  log4level == logging.FATAL+1 :
          return CF.LogLevels.OFF
      if  log4level == logging.FATAL :
          return CF.LogLevels.FATAL
      if  log4level == logging.ERROR :
          return CF.LogLevels.ERROR
      if  log4level == logging.WARN :
          return CF.LogLevels.WARN
      if  log4level == logging.INFO :
          return CF.LogLevels.INFO
      if  log4level == logging.DEBUG :
          return CF.LogLevels.DEBUG
      if  log4level == logging.TRACE :
          return CF.LogLevels.TRACE
      if  log4level == logging.NOTSET:
          return CF.LogLevels.ALL
      return CF.LogLevels.INFO

def ConvertToLog4Level( newLevel ):
    level = logging.INFO
    if  newLevel == CF.LogLevels.OFF :
            level=logging.FATAL+1
    if  newLevel == CF.LogLevels.FATAL :
            level=logging.FATAL
    if  newLevel == CF.LogLevels.ERROR :
            level=logging.ERROR
    if  newLevel == CF.LogLevels.WARN :
            level=logging.WARN
    if  newLevel == CF.LogLevels.INFO:
            level=logging.INFO
    if  newLevel == CF.LogLevels.DEBUG:
            level=logging.DEBUG
    if  newLevel == CF.LogLevels.TRACE:
            level=logging.TRACE
    if  newLevel == CF.LogLevels.ALL:
            level=logging.TRACE
    return level


def SupportedCFLevel( newLevel ):
    level = True
    if  newLevel != CF.LogLevels.OFF and \
        newLevel != CF.LogLevels.FATAL and \
        newLevel != CF.LogLevels.ERROR and \
        newLevel != CF.LogLevels.WARN and \
        newLevel != CF.LogLevels.INFO and \
        newLevel != CF.LogLevels.DEBUG and \
        newLevel != CF.LogLevels.TRACE and \
        newLevel != CF.LogLevels.ALL :
            level= False
    return level


def SetLevel( logid, debugLevel):
    SetLogLevel( logid, ConvertLogLevel(debugLevel))


def SetLogLevel( logid, newLevel ):
    logger = logging.getLogger(logid )
    level = ConvertToLog4Level( newLevel )
    if logger:
        logger.setLevel( level )


def SetEventChannelManager( ECM ):
    RedhawkLogger.SetEventChannelManager(ECM)


def GetDefaultConfig():
    cfg = "log4j.rootLogger=INFO,STDOUT\n" + \
           "# Direct log messages to STDOUT\n" + \
           "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" +  \
           "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" +  \
           "log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{1}:%L - %m%n\n"
    return cfg


def GetSCAFileContents( url ):
    fileContents = None
    scheme, netloc, path, params, query, fragment = urlparse.urlparse(url)
    if scheme=="sca" :
       queryAsDict = dict([x.split("=") for x in query.split("&")])
       try:
           orb=CORBA.ORB_init()
           fileSys = orb.string_to_object(queryAsDict["fs"])
       except KeyError:
            logging.warning("sca URI missing fs query parameter")
       except:
           logging.warning("Unable to get ORB reference")
       else:
            if fileSys == None:
                logging.warning("Failed to lookup file system")
            else:
                try:
                    scaFile = fileSys.open(path, True)
                    fileSize = scaFile.sizeOf()
                    fileContents = scaFile.read(fileSize)
                    scaFile.close()
                finally:
                    pass
    return fileContents

def GetHTTPFileContents( url ):
    fileContents = None
    try:
       filehandle = urllib.urlopen( url )
       return filehandle.read()
    except:
        logging.warning("connection cannot be made to" + url)
        return

def GetConfigFileContents( url ):
    fc=None
    scheme, netloc, path, params, query, fragment = urlparse.urlparse(url)
    if scheme == "file":
        try:
            f = open(path,'r')
            fc=""
            for line in f:
                fc += line
            f.close()
        except:
            fc=None
    elif scheme == "sca":
        fc=GetSCAFileContents(url)

    elif scheme == "http":
        fc=GetHTTPFileContents(url)
    
    elif scheme == "str":
        ## RESOLVE
        if path.startswith("/"):
          fc=path[1:]
        else:
           fc=path
        pass
    else:
        # Invalid scheme
        logging.warning("Invalid logging config URI scheme")

    return fc

def ConfigureDefault():
    # use default configuration stdout and INFO level
    cfg=GetDefaultConfig()

    # apply configuration to logging module
    ossie.utils.log4py.config.strConfig(cfg,None)

    # set log level to INFO
    logging.getLogger().setLevel( logging.INFO )


def ConfigureWithContext( cfg_data, tbl, category=None ):
    cfg=None
    try:
        fileContents=""
        fc_raw=cfg_data
        if fc_raw.find("<log4j:configuration") != -1 :
            raise "XML configuration not supported by log4py"

        fileContents=ExpandMacros( fc_raw, tbl)

        ossie.utils.log4py.config.strConfig(fileContents, category)

        cfg=fileContents

    except Exception, e:
        print "Error: log4py configuration file error", e
        pass
    return cfg

def Configure( logcfgUri, logLevel=None, ctx=None, category=None ):

    # test we have a logging URI
    if logcfgUri==None or logcfgUri=="" :
        ConfigureDefault()
    else:
        try:
            fileContents = GetConfigFileContents(logcfgUri);

            ## get default macro defintions
            tbl=GetDefaultMacros()
            ResolveHostInfo( tbl )
            if ctx :
                # Apply the context to the default macros
                ctx.apply(tbl)

            if fileContents and len(fileContents) != 0:
                fc=ConfigureWithContext( fileContents, tbl, category )
        except Exception,e:
            print "Error: log4py configuration file error", e
            pass

    # If a log level was explicitly stated, set it here, potentially
    # overloading the value set by the context.
    # if a log level was not specified, set to default
    if logLevel != None and logLevel > -1 :
        SetLevel(None, logLevel)

