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

import os as _os
import sys as _sys
from omniORB import CORBA as _CORBA
import CosNaming as _CosNaming
from xml.dom import minidom as _minidom
import core as _core
from ossie.cf import CF as _CF
import ossie.utils as _utils
from ossie.utils.sca import importIDL as _importIDL
from ossie.logger import ConvertLevelNameToDebugLevel
import atexit
import signal as _signal
import time  as _time
import traceback

class _envContainer(object):
    def __init__(self, process, stdout):
        self.process = process
        self.stdout = stdout

def __waitTermination( process, timeout=5.0, pause=0.1):
        while process and process.poll() is None and timeout > 0.0:
            timeout -= pause
            _time.sleep(pause)
        return process.poll() != None

def __terminate_process( process, signals=(_signal.SIGINT, _signal.SIGTERM, _signal.SIGKILL) ):
        if process and process.poll() != None:
           return
        try:
            for sig in signals:
                _os.kill(process.pid, sig)
                if __waitTermination(process):
                    break
            process.wait()
        except OSError, e:
            pass
        finally:
            pass

def _cleanup_domain():
    try:
        if globals().has_key('currentdomain'):
            __terminate_process( globals()['currentdomain'].process)
            x = globals().pop('currentdomain')
            if x : del x
    except:
        traceback.print_exc()
        pass
    if globals().has_key('currentdevmgrs'):
        for x in globals()['currentdevmgrs']:
            try:
                __terminate_process(x.process)
            except:
                traceback.print_exc()
                pass
        x = globals().pop('currentdevmgrs')
        if x : del x

def _shutdown_session():
    if globals().has_key('orb_to_shutdown'):
        orb = globals()['orb_to_shutdown']
        if orb:
            orb.shutdown(True)
        globals().pop('orb_to_shutdown')
    _cleanup_domain()

atexit.register(_shutdown_session)

def _getDCDFile(sdrroot, dcdFile):
    """
    Try to find the DCD file, either as an absolute path or relative to SDRROOT
    """
    # The DCD file may just be a directory name under $SDRROOT/dev/nodes
    if not dcdFile.endswith('.dcd.xml'):
        # Path did not include a final DCD file
        node_path = _os.path.join(sdrroot, 'dev/nodes/') + dcdFile
        node_path += '/DeviceManager.dcd.xml'
        if _os.path.exists(node_path):
            return node_path

    # If the file exists as-is, return it
    if _os.path.exists(dcdFile):
        return dcdFile

    # Try an SDR-relative path
    sdr_path = _os.path.join(sdrroot, 'dev/') + dcdFile
    if _os.path.exists(sdr_path):
        return sdr_path

    # Could not find any matching path
    return None

def kickDomain(domain_name=None, kick_device_managers=True, device_managers=[], detached=False, sdrroot=None, stdout=None, logfile=None, debug_level=None, device_managers_debug_levels=[]):
    """Kick-start a REDHAWK domain.
         domain_name: the name that should be used
         kick_device_managers: one or more Device Managers should be automatically started
         device_managers: if kick_device_managers set to True, list of Device Managers to start. If the list is empty, then start all Device Managers in
                          $SDRROOT/dev/nodes.  List can be node names i.e. GPP_node or absolute path to DCD files
         detached: determine whether the life cycle of the started Domain and Device Managers should follow the lifecycle of the current Python session
         sdrroot: use this sdr root. If set to None, then use $SDRROOT
         stdout: filename where stdout should be redirected. None sends stdout to /dev/null
         debug_level: debug level to pass on command line: FATAL, ERROR, WARN, INFO, DEBUG, TRACE
         device_managers_debug_levels = list of debug levels to pass on command line with corresponding device_managers

    """
        
    if sdrroot == None:
        try:
            sdrroot = _os.getenv('SDRROOT')
        except:
            print "The environment variable SDRROOT must be set or an sdrroot value must be passed as an argument"
    
    args = ['nodeBooter']
    args.append('-D')
    args.append('-sdrroot')
    args.append(sdrroot)
    if logfile:
        args.append('-logcfgfile')
        args.append(logfile)

    if debug_level:
        args.append('-debug')
        args.append(str(ConvertLevelNameToDebugLevel(debug_level)))

    if domain_name != None:
        args.append('--domainname')
        args.append(domain_name)
    else:
        dmd_file = sdrroot+'/dom/domain/DomainManager.dmd.xml'
        try:
            fp = open(dmd_file,'r')
            dmd_contents = fp.read()
            fp.close
        except:
            print "Unable to read domain profile "+dmd_file
            return None
        try:
            dmd = _minidom.parseString(dmd_contents)
            domain_name = str(dmd.getElementsByTagName('domainmanagerconfiguration')[0].getAttribute('name'))
        except:
            print "Invalid domain profile "+dmd_file
            return None
    
    _devnull = open('/dev/null')
    if stdout == None:
        stdout_fp = None
    elif stdout == 0:
        stdout_fp = open('/dev/null','w')
    else:
        stdout_fp = open(stdout,'w')
    
    if stdout_fp == None:
        sp = _utils.Popen(args, executable=None, cwd=_os.getcwd(), close_fds=True, stdin=_devnull, preexec_fn=_os.setpgrp)
    else:
        sp = _utils.Popen(args, executable=None, cwd=_os.getcwd(), close_fds=True, stdin=_devnull, stdout=stdout_fp, preexec_fn=_os.setpgrp)
    
    if globals().has_key('currentdomain'):
        globals()['currentdomain'] = None
        
    globals()['currentdomain'] = _envContainer(sp, stdout_fp)
    
    if kick_device_managers:
        dm_procs=[]
        if len(device_managers) == 0:
            base = sdrroot + '/dev/nodes'
            for (directory,sub,files) in _os.walk(base):
                foundDCD = False
                filename = ''
                for file in files:
                    if file[-8:] == '.dcd.xml':
                        filename = file
                        foundDCD = True
                        break
                if foundDCD:
                    device_managers.append(directory[len(sdrroot)+4:]+'/'+filename)

        for idx, device_manager in enumerate(device_managers):
            dcd_file = _getDCDFile(sdrroot, device_manager)
            if not dcd_file:
                print "Unable to locate DCD file for '%s'" % device_manager
                continue

            args = ['nodeBooter']
            args.append('-d')
            args.append(dcd_file)
            args.append('-sdrroot')
            args.append(sdrroot)
            args.append('--domainname')
            args.append(domain_name)
            if logfile:
                args.append('-logcfgfile')
                args.append(logfile)
            if device_managers_debug_levels and len(device_managers_debug_levels) > 0 :
                dlevel = None
                if idx < len(device_managers_debug_levels):
                    dlevel = device_managers_debug_levels[idx]
                if dlevel:
                     args.append('-debug')
                     args.append(str(ConvertLevelNameToDebugLevel(dlevel)))
            sp = _utils.Popen(args, executable=None, cwd=_os.getcwd(), close_fds=True, stdin=_devnull, stdout=stdout_fp, preexec_fn=_os.setpgrp)
            dm_procs.append( _envContainer(sp, stdout_fp) )

        if globals().has_key('currentdevmgrs'):
            globals()['currentdevmgrs'] += dm_procs
        else:
            globals()['currentdevmgrs'] = dm_procs

    dom = attach(domain_name)

    return dom

def scan(location=None):
    orb = _CORBA.ORB_init(_sys.argv, _CORBA.ORB_ID)

    if location:
        try:
            obj = orb.string_to_object('corbaname::'+location)
        except _CORBA.BAD_INV_ORDER:
            orb.destroy()
            orb = _CORBA.ORB_init(_sys.argv, _CORBA.ORB_ID)
            obj = orb.string_to_object('corbaname::'+location)
    else:
        try:
            obj = orb.resolve_initial_references("NameService")
        except _CORBA.BAD_INV_ORDER:
            orb.destroy()
            orb = _CORBA.ORB_init(_sys.argv, _CORBA.ORB_ID)
            obj = orb.resolve_initial_references("NameService")
    try:
        rootContext = obj._narrow(_CosNaming.NamingContext)
    except:
        raise RuntimeError('NameService not found')

    base_list = rootContext.list(100)
    domainsFound = []
    for entry in base_list[0]:
        if entry.binding_type != _CosNaming.ncontext:
            continue
        ctx = rootContext.resolve(entry.binding_name)
        objs = ctx.list(100)[0]
        foundDomain = False
        for obj in objs:
            if obj.binding_type == _CosNaming.nobject:
                if obj.binding_name[0].id == entry.binding_name[0].id:
                    tmpobj = ctx.resolve(obj.binding_name)
                    tmpDomainManager = tmpobj._narrow(_CF.DomainManager)
                    # Make sure Domain is alive
                    try:
                        fileManager = tmpDomainManager._get_fileMgr()
                    except:
                        continue
                    if tmpDomainManager:
                        foundDomain = True
                        break
        if foundDomain:
            domainsFound.append(entry.binding_name[0].id)

    return domainsFound

def attach(domain=None, location=None, connectDomainEvents=True):
    """
    Attach to a Domain and return a reference to the Domain.

    Arguments
      domain   - Name of domain. If there is only one domain, passing None
                 will connect to that domain.
      location - Location of naming service to look up domain. If None, use
                 the default NameService initial reference.
      connectDomainEvents - If True, connect to the IDM and ODM channels for
                            domain state updates (default: True).
    """
    if domain == None:
        domains = scan(location)
        if len(domains) == 1:
            domain = domains[0]
        else:
            if len(domains) == 0 :
                print "No domains found."
            else:
                print "Multiple domains found: "+str(domains)+". Please specify one."
            return None

    dom_entry = _core.Domain(name=str(domain), location=location, connectDomainEvents=connectDomainEvents)
    globals()['orb_to_shutdown'] = dom_entry.orb

    return dom_entry
