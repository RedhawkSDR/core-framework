#!/usr/bin/env python
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

import os, sys, commands, logging, platform, shutil, socket
from ossie import parsers
from ossie.utils.model import _uuidgen as uuidgen

class ConfigurationError(StandardError):
    pass

class CreateNode(object):
    def __init__(self, options, cmdlineProps):
        # Basic setup
        self._log = logging.getLogger('CreateNode')
        self.localfile_nodeprefix = '/mgr'
        self.options = options
        self.cmdlineProps = cmdlineProps
        self.hostname = socket.gethostname()
        
        # check domainname
        if options.domainname == None:
            raise ConfigurationError("A domainname is required")
        
        # Verify the base GPP profile exists
        self.gpp_templates = {"spd": os.path.join(self.options.sdrroot, "dev", self.options.gpppath[1:], "GPP.spd.xml"),
                              "prf": os.path.join(self.options.sdrroot, "dev", self.options.gpppath[1:], "GPP.prf.xml"),
                              "scd": os.path.join(self.options.sdrroot, "dev", self.options.gpppath[1:], "GPP.scd.xml")}

        for template in self.gpp_templates.values():
            if not os.path.exists(template):
                raise ConfigurationError("%s missing" % template)
                
        self.nodedir = os.path.join(self.options.sdrroot, "dev", "nodes", self.options.nodename)
        self.path_to_dcd = os.path.join(self.nodedir , "DeviceManager.dcd.xml")
            
        # Figure out where we are going to write the GPP profile
        if self.options.inplace:
            self.gpp_path = os.path.join(self.options.sdrroot, "dev", "devices", "GPP")
        else:
            self.gpp_path = os.path.join(self.nodedir, "GPP")
            
        # prep uuids
        self.uuids = {}
        self.uuids["softpkg"                ] = 'DCE:' + uuidgen()
        self.uuids["implementation"         ] = 'DCE:' + uuidgen()
        self.uuids["deviceconfiguration"    ] = 'DCE:' + uuidgen()
        self.uuids["componentfile"          ] = 'DCE:' + uuidgen()
        self.uuids["componentinstantiation" ] = 'DCE:' + uuidgen()
        self.uuids["componentimplementation"] = 'DCE:' + uuidgen()
        self.uuids["componentsoftpkg"       ] = 'DCE:' + uuidgen()
        
        self.props = {}

    def register(self):
        if not self.options.silent:
            self._log.debug("Registering...")
        self._gather_system_information()
        self._createDeviceManagerProfile()
    
    def unregister(self):
        if not self.options.silent:
            self._log.debug("Unregistering...")
        if os.path.isdir(self.nodedir):
            if not self.options.silent:
                self._log.debug("  Removing <" + self.nodedir + ">")
            shutil.rmtree(self.nodedir)
         
    def _gather_system_information(self):
        # get some platform information
        self.props['os_name']    = platform.system()
        self.props['os_version'] = platform.release()
        tmp_uname_p    = platform.processor()
        tmp_proc_map   = {'i386':'x86', 
                          'i686':'x86', 
                          'x86_64':'x86_64', 
                          'armv7l':'armv7l'}
        self.props['processor_name']  = tmp_proc_map.get(tmp_uname_p, 'x86')

        self._gather_java_information()
        self._gather_python_information()
        self._gather_xmidas_information()
        self._gather_nextmidas_information()
        
        # Output some debug information
        if not self.options.silent:
            self._log.debug("System Information for: <" + self.hostname + ">")
            self._log.debug("  %-30s %s", "Node", self.nodedir)
            k = self.props.keys()
            k.sort()
            for n in k:
                self._log.debug("  %-30s %s ", n, self.props[n])
        
    def _gather_java_information(self):
        # Get some information about the Java version, this is a bit tricky since it is not
        # uncommon for bad versions of Java to be installed. The most common issues are:
        #  1. Using a non-standard Java version            -+
        #  2. Using an ancient Java version                 +- The default version of Java on RHEL 4
        #  3. Using a 32-bit version on a 64-bit platform  -+  violates these
        #
        # For example, if the Java version is listed as 1.x.y_zz, you will get:
        #    self.java_version = '1.x.y'
        #    self.java_release =    x.y000zz   # all Java version begin with "1." so we drop that here
        self.java_version = "0.0.0"
        self.java_release = 0.0
        try:
            version_output = commands.getoutput("java -version").strip().split('\n')
            version_start  = version_output[0]
            version        = "0.0.0"
            update         = 0
          
            if (version_output[0].startswith('java version "') and version_output[0].endswith('"')):
                version = version_output[0][14:-1]  # may be in the form '1.6.0' or '1.6.0_13' (or '1.6.0u13')
                if (version.find('_') > 0):
                    update  = version[version.find('_')+1:]
                    version = version[0:version.find('_')]
                elif (version.find('u') > 0): # historically some versions have used 'u' rather than '_'
                    update  = version[version.find('u')+1:]
                    version = version[0:version.find('u')]
          
            if (version_output[2].lower().find("64-bit") > 0):
                bits = 64
            elif (version_output[2].lower().find("32-bit") > 0):
                bits = 32 # extremely uncommon
            else:
                bits = 32 # extremely common
          
            if ((bits == self.procbits) and (len(version) == 5) and version.startswith("1.")):
                self.java_version = version
                self.java_release = float(version[2:]) + float(update)*0.000001
        except:
            self.java_version = "0.0.0"
            self.java_release = 0.0
       
    def _gather_python_information(self): 
        # Get some information about the Python version. For example, if the version is
        # listed as x.y.z, you will get:
        #    self.python_version = 'x.y.z'
        #    self.python_release =  x.y0000z
        self.python_version = "0.0.0"
        self.python_release = 0.0
        try:
            version_output = sys.version.strip().split('\n')
            version_start  = version_output[0]
          
            if ((version_start[1] == '.') and (version_start[3] == '.') and (version_start[5] == ' ')):
                self.python_version = version_start[:5]  # should may be in the form '2.3.4'
                self.python_release = self.ver2rel(self.python_version)
        except:
            self.python_version = "0.0.0"
            self.python_release = 0.0
        
    def _gather_xmidas_information(self):
        # Get some information about the X-Midas version. For example, if the version is
        # listed as x.y.z, you will get:
        #    self.xmidas_version = 'x.y.z'
        #    self.xmidas_release =  x.y0000z
        self.xmidas_version = "0.0.0"
        self.xmidas_release = 0.0
        try:
            version_output = commands.getoutput("cat $XMDISK/xm/version.txt").strip().split('\n')
            version_start  = version_output[0].strip()
          
            if (version_start.startswith('X-Midas ')):
                version = version_start[8:].strip()
                if ((version[1] == '.') and (version[3] == '.') and (version[5] == ' ')):
                    self.xmidas_version = version[:5]  # should may be in the form '4.6.2'
                    self.xmidas_release = self._ver2rel(self.xmidas_version)
        except:
            self.xmidas_version = "0.0.0"
            self.xmidas_release = 0.0
        
    def _gather_nextmidas_information(self):
        # Get some information about the NeXtMidas version. For example, if the version is
        # listed as x.y.z, you will get:
        #    self.nextmidas_release =  x.y0000z
        self.nextmidas_version = "0.0.0"
        self.nextmidas_release = 0.0
        try:
            version_output = commands.getoutput("cat $NMROOT/nxm/sys/version.txt").strip().split('\n')
            version_start  = version_output[1].strip()
          
            if (version_start.startswith('** NeXtMidas Version: ')):
                version = version_start[22:].strip()
                if ((version[1] == '.') and (version[3] == '.') and (version[5] == ' ')):
                    self.nextmidas_version = version[:5]  # should may be in the form '2.8.0'
                    self.nextmidas_release = self._ver2rel(self.nextmidas_version)
        except:
            self.nextmidas_version = "0.0.0"
            self.nextmidas_release = 0.0
        
    def _ver2rel(self, ver):
        return float(ver[0:1]) + float(ver[2:3])*0.1 + float(ver[4:5])*0.000001
    
    def _get_fileSystems(self):
        """Use df to provide the current status for all file systems on this machine."""
        status, output = commands.getstatusoutput("/bin/df -P -k") # Use the POSIX definition of df for maximum portability
        output = output.split("\n")
        # Validate the first line looks as expected
        fields = output[0].split()
        if fields != ["Filesystem", "1024-blocks", "Used", "Available", "Capacity", "Mounted", "on"]:
            raise OSError, "Unexpected output from /bin/df...check for POSIX compatibility"
        
        result = []
        for line in output[1:]:
            fields = line.split()
            filesystem = fields[0]
            mounted_on = fields[-1]
            total_mbytes = int(fields[1]) / 1024
            used_mbytes = int(fields[2]) / 1024
            available_mbytes = int(fields[3]) / 1024
            result.append((filesystem, mounted_on, total_mbytes, used_mbytes, available_mbytes))
        return result
    
    def _createDeviceManagerProfile(self):
        #####################
        # Setup environment
        #####################

        # make sure node hasn't already been created
        if os.path.exists(self.path_to_dcd):
            self._log.error("Cannot 'register' new dynamicnode. A previous configuration was found. Please 'unregister' dynamicnode first.")
            sys.exit(1)

        try:
            if not os.path.isdir(self.nodedir):
                os.makedirs(self.nodedir)
            else:
                if not self.options.silent:
                    self._log.debug("Node directory already exists; skipping directory creation")
                pass
        except OSError:
            raise Exception, "Could not create device manager directory"

        GPP_componentfile = 'GPP_' + uuidgen()
        if self.options.inplace:
            compfiles = [{'id':GPP_componentfile, 'localfile':os.path.join('/devices', 'GPP', 'GPP.spd.xml')}]
        else:
            compfiles = [{'id':GPP_componentfile, 'localfile':os.path.join('/nodes', self.options.nodename, 'GPP', 'GPP.spd.xml')}]
        compplacements = [{'refid':GPP_componentfile, 'instantiations':[{'id':self.uuids["componentinstantiation"], 'usagename':'GPP_' + self.hostname.replace('.', '_')}]}]
        if self.options.noevents:
            connections = []
        else:
            connections = [{'usesport':{'id':'propEvent', 'refid':self.uuids["componentinstantiation"]}, 'findby':{'type':'eventchannel', 'name':'GPP_Channel'}}]

        #####################
        # DeviceManager files
        #####################
        if not self.options.silent:
            self._log.debug("Creating DeviceManager profile <" + self.options.nodename + ">")
        
        # set deviceconfiguration info
        _dcd = parsers.DCDParser.deviceconfiguration()
        _dcd.set_id(self.uuids["deviceconfiguration"])
        _dcd.set_name(self.options.nodename)
        _localfile = parsers.DCDParser.localfile(name=os.path.join(self.localfile_nodeprefix, 'DeviceManager.spd.xml'))
        _dcd.devicemanagersoftpkg = parsers.DCDParser.devicemanagersoftpkg(localfile=_localfile)
        
        # add componentfiles and componentfile(s)
        _dcd.componentfiles = parsers.DCDParser.componentfiles()
        for in_cf in compfiles:
            cf = parsers.DCDParser.componentfile(type_='SPD', id_=in_cf['id'], localfile=parsers.DCDParser.localfile(name=in_cf['localfile']))
            _dcd.componentfiles.add_componentfile(cf)

        # add partitioning/componentplacements
        _dcd.partitioning = parsers.DCDParser.partitioning()
        for in_cp in compplacements:
            _comp_fileref = parsers.DCDParser.componentfileref(refid=in_cp['refid'])
            _comp_placement = parsers.DCDParser.componentplacement(componentfileref=_comp_fileref)
            for ci in in_cp['instantiations']:
                comp_inst = parsers.DCDParser.componentinstantiation(id_=ci['id'], usagename=ci['usagename'])
                _comp_placement.add_componentinstantiation(comp_inst)
            _dcd.partitioning.add_componentplacement(_comp_placement)

        # add connections if we're doing events
        if len(connections) > 0:
            _dcd.connections = parsers.DCDParser.connections()
        for connection in connections:
            connectinterface = parsers.DCDParser.connectinterface()
            if 'usesport' in connection:
                usesport = parsers.DCDParser.usesport()
                usesport.usesidentifier = connection['usesport']['id']
                usesport.componentinstantiationref = parsers.DCDParser.componentinstantiationref(refid=connection['usesport']['refid'])
                connectinterface.usesport = usesport
            if 'findby' in connection:
                findby = parsers.DCDParser.findby()
                findby.domainfinder = parsers.DCDParser.domainfinder(type_=connection['findby']['type'], name=connection['findby']['name'])
                connectinterface.findby = findby;
            _dcd.connections.add_connectinterface(connectinterface)
        
        # add domainmanager lookup
        if self.options.domainname:
            _tmpdomainname = self.options.domainname + '/' + self.options.domainname
            
        _dcd.domainmanager = parsers.DCDParser.domainmanager(namingservice=parsers.DCDParser.namingservice(name=_tmpdomainname))
        dcd_out = open(self.path_to_dcd, 'w')
        dcd_out.write(parsers.parserconfig.getVersionXML())
        _dcd.export(dcd_out,0)
        dcd_out.close()

        
###########################
# Run from command line
###########################
if __name__ == "__main__":

    ##################
    # setup arg parser
    ##################
    from optparse import OptionParser
    parser = OptionParser()
    parser.usage = "%s [options] [simple_prop1 simple_value1]..."
    parser.add_option("--domainname", dest="domainname", default="REDHAWK_DEV",
                      help="domain that the Device Manager connects with")
    parser.add_option("--sdrroot", dest="sdrroot", default=os.path.expandvars("${SDRROOT}"),
                      help="path to the sdrroot; if none is given, ${SDRROOT} is used.")
    parser.add_option("--nodename", dest="nodename", default="DevMgr_%s" % socket.gethostname(),
                      help="desired nodename, if none is given DevMgr_${HOST} is used")
    parser.add_option("--inplace", dest="inplace", default=True, action="store_true",
                      help="update the GPP profile in-place in its directory (i.e.: --gpppath); setting this to false modifies the GPP configuration as a subdirectory in the node directory")
    parser.add_option("--gpppath", dest="gpppath", default="/devices/GPP",
                      help="The device manager file system absolute path to the GPP, default '/devices/GPP'")
    parser.add_option("--disableevents", dest="noevents", default=False, action="store_true",
                      help="Disable event channel registration")
    parser.add_option("--silent", dest="silent", default=False, action="store_true",
                      help="Suppress all logging except errors")
    parser.add_option("--clean", dest="clean", default=False, action="store_true",
                      help="clean up the previous configuration for this node first (delete entire node)")
    parser.add_option("-v", "--verbose", dest="verbose", default=False, action="store_true",
                      help="enable verbose logging")

    (options, args) = parser.parse_args()    

    # Configure logging
    logging.basicConfig(format='%(name)-12s:%(levelname)-8s: %(message)s', level=logging.INFO)
    if options.verbose:
        logging.getLogger().setLevel(logging.DEBUG)

    # grab tmp logger until class is created
    _log = logging.getLogger('CreateNode')

    if len(args) % 2 == 1:
        _log.error("Invalid command line arguments - properties must be specified with values")
        sys.exit(1)
    cmdlineProps = {}
    for i in range(len(args)):
        if i % 2 == 0:
            cmdlineProps[args[i]] = args[i + 1]

    # create instance of CreateNode
    try:
        dn = CreateNode(options, cmdlineProps)
        if options.clean:
            dn.unregister()
        dn.register()
        if not options.silent:
            _log.info("GPP node registration is complete")
    except ConfigurationError, e:
        _log.error("%s", e)
        sys.exit(1)
