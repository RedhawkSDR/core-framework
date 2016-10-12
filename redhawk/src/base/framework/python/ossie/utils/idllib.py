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
import commands
import os
import glob
import threading

from ossie.utils.log4py import logging
from ossie.utils.sca import importIDL

__all__ = ('InvalidRepoIDError', 'UnknownInterfaceError', 'IDLLibrary')

class IDLError(Exception):
    pass

class InvalidRepoIDError(IDLError):
    def __init__(self, repoid):
        super(InvalidRepoIDError, self).__init__("Invalid IDL repository ID '"+repoid+"'")

class UnknownInterfaceError(IDLError):
    def __init__(self, repoid):
        super(UnknownInterfaceError, self).__init__("Unknown IDL interface '"+repoid+"'")

def _pkgconfigVar(name, variable):
    status, path = commands.getstatusoutput('pkg-config --variable=%s %s' % (variable, name))
    if status == 0:
        return path
    else:
        return None

def _getIDLDir(package, dirname):
    path = _pkgconfigVar(package, 'idldir')
    if path:
        return path
    for prefix in ('/usr/share/idl', '/usr/local/share/idl'):
        path = os.path.join(prefix, dirname)
        if os.path.exists(path):
            return path
    return None

class IDLLibrary(object):
    __log = logging.getLogger(__name__).getChild('IDLLibrary')

    def __init__(self):
        self._interfaces = {}
        self._searchPaths = []
        self._includePaths = []
        self._lock = threading.Lock()

        # Track parsed files to avoid repeatedly parsing the same files
        self._parsed = set()

        self._omniPath = _getIDLDir('omniORB4', 'omniORB')
        self.addIncludePath(self._omniPath)
        self._cosPath = _getIDLDir('omniCOS4', 'omniORB/COS')
        self.addIncludePath(self._cosPath)

    def addIncludePath(self, path):
        if path not in self._includePaths:
            self._includePaths.append(path)

    def addSearchPath(self, path):
        if not path in self._searchPaths:
            self._searchPaths.append(path)
            self.addIncludePath(path)

    def _importCosModule(self, name):
        # Cos interfaces follow a predictable file naming scheme
        filename = name.replace('omg.org/', '') + '.idl'
        fullpath = os.path.join(self._cosPath, filename)

        self._importFile(fullpath)

    def _importFile(self, filename):
        if filename in self._parsed:
            return
        elif not os.path.exists(filename):
            return

        # Parse file and save the interfaces; this may return interfaces
        # from other files as well.
        for interface in importIDL.getInterfacesFromFile(filename, self._includePaths):
            # Only add new, unseen interfaces.
            if interface.repoId not in self._interfaces:
                self._interfaces[interface.repoId] = interface

            # Mark the file that provided this interface as parsed
            self._parsed.add(interface.fullpath)

        # Mark the file as parsed in case it didn't contain any interfaces
        self._parsed.add(filename)

    def _importAllFromPath(self, path):
        if not path:
            return
        for filename in glob.glob(os.path.join(path, '*.idl')):
            self._importFile(filename)

    def _importAllRecursive(self, path):
        self._importAllFromPath(path)
        for root, dirs, files in os.walk(path):
            for dirname in dirs:
                self._importAllFromPath(os.path.join(root, dirname))

    def _importBulkioModule(self, name):
        bulkio_path = self._findPath('ossie/BULKIO')
        if not bulkio_path:
            # BULKIO is not installed
            return

        if name.startswith('data'):
            filename = os.path.join(bulkio_path, 'bio_'+name+'.idl')
            if os.path.exists(filename):
                self.__log.trace("Importing file '%s'", filename)
                self._importFile(filename)
                return

        self.__log.trace('Importing all BULKIO IDL')
        self._importAllFromPath(bulkio_path)
    
    def _importOmniPath(self):
        # Import the omniORB IDL path non-recursively, as COS is typically
        # installed as a subdirectory, and many of its modules dump errors to
        # the console.
        self.__log.trace("Importing all from '%s'", self._omniPath)
        for filename in glob.glob(os.path.join(self._omniPath, '*.idl')):
            if os.path.basename(filename) in ('poa.idl', 'poa_include.idl'):
                # Exclude the POA IDL, which is unlikely to appear in real use
                # but will cause parser warnings
                continue
            self._importFile(filename)

    def _parseInterface(self, repoid):
        try:
            idl, name, version = repoid.split(':')
        except ValueError:
            raise InvalidRepoIDError(repoid)
        if '/' in name:
            namespace, name = name.rsplit('/', 1)
        else:
            namespace = ''
        
        if namespace.startswith('omg.org/'):
            self._importCosModule(namespace)
        elif namespace == 'BULKIO':
            self._importBulkioModule(name)
        elif namespace:
            if namespace in ('CF', 'ExtendedCF', 'StandardEvent', 'ExtendedEvent', 'PortTypes'):
                search_path = self._findPath('ossie/CF')
            else:
                search_path = self._findPath(os.path.join('redhawk', namespace))
            self.__log.trace("Importing all from '%s'", search_path)
            self._importAllFromPath(search_path)

    def _findInterface(self, repoid):
        self.__log.trace("Searching for interface '%s'", repoid)
        self._parseInterface(repoid)
        if repoid in self._interfaces:
            self.__log.trace("Found '%s' in standard location", repoid)
            return

        self.__log.trace("Doing exhaustive search")
        for path in self._searchPaths:
            self.__log.trace("Importing all recursively from '%s'", path)
            self._importAllRecursive(path)
            if repoid in self._interfaces:
                self.__log.trace("Found '%s' in search path '%s'", repoid, path)
                return

        # If all else fails, search the omniORB IDL
        self._importOmniPath()
        if repoid in self._interfaces:
            self.__log.trace("Found '%s' in search path '%s'", repoid, self._omniPath)
            return

        self.__log.trace("No definition found for '%s'", repoid)

    def _findPath(self, dirname):
        for path in self._searchPaths:
            fullpath = os.path.join(path, dirname)
            if os.path.exists(fullpath):
                return fullpath
        return None

    def getInterface(self, repoid):
        self._lock.acquire()
        try:
            if not repoid in self._interfaces:
                self._findInterface(repoid)

            interface = self._interfaces.get(repoid, None)
            if interface is None:
                raise UnknownInterfaceError(repoid)
            else:
                return interface
        finally:
            self._lock.release()
