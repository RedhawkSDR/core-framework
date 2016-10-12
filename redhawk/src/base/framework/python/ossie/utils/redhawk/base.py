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

_ossiehome = _os.getenv('OSSIEHOME')
if _ossiehome == None:
    _ossiehome = ''
    _interface_list = []
else:
    _interface_list = _importIDL.importStandardIdl()

class _envContainer(object):
    def __init__(self, domain, stdout):
        self.domain = int(domain)
        self.stdout = stdout
    
    def __del__(self):
        import os as _os
        _os.kill(self.domain,2)
        if self.stdout != None:
            self.stdout.close()
    

def kickDomain(domain_name=None, kick_device_managers=True, device_managers=[], detached=False, sdrroot=None, stdout=None, logfile=None):
    """Kick-start a REDHAWK domain.
         domain_name: the name that should be used
         kick_device_managers: one or more Device Managers should be automatically started
         device_managers: if kick_device_managers set to True, list of Device Managers to start. If the list is empty, then start all Device Managers in
                          $SDRROOT/dev/nodes
         detached: determine whether the life cycle of the started Domain and Device Managers should follow the lifecycle of the current Python session
         sdrroot: use this sdr root. If set to None, then use $SDRROOT
         stdout: filename where stdout should be redirected. None sends stdout to /dev/null
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
        
    globals()['currentdomain'] = _envContainer(sp.pid, stdout_fp)
    
    if kick_device_managers:
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
        for device_manager in device_managers:
            args = ['nodeBooter']
            args.append('-d')
            args.append(device_manager)
            args.append('-sdrroot')
            args.append(sdrroot)
            args.append('--domainname')
            args.append(domain_name)
            if logfile:
                args.append('-logcfgfile')
                args.append(logfile)
            sp = _utils.Popen(args, executable=None, cwd=_os.getcwd(), close_fds=True, stdin=_devnull, stdout=stdout_fp, preexec_fn=_os.setpgrp)

    dom = attach(domain_name)
    
    return dom
    

def scan(location=None):
    orb = _CORBA.ORB_init(_sys.argv, _CORBA.ORB_ID)

    if location:
        obj = orb.string_to_object('corbaname::'+location)
    else:
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

def attach(domain=None, location=None):
    """
        Attach to a Domain and return a reference
        to the Domain.
    """
    if domain == None:
        return None
    
    dom_entry = _core.Domain(name=domain, int_list = _interface_list, location=location)
    
    return dom_entry
