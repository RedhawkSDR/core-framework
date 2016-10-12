#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK code-generator.
#
# REDHAWK code-generator is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK code-generator is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
from redhawk.codegen.lang import python

import versions

redhawkLibs = {
    'BULKIO': {
        'libname':    'bulkio',
        'interfaces': 'bulkioInterfaces',
        'rpmname':    'bulkioInterfaces',
        'version':    versions.bulkio,
        'pymodule':   'bulkio',
        'jarfiles':   ('bulkio.jar', 'BULKIOInterfaces.jar')
    },
    'BURSTIO': {
        'libname':    'burstio',
        'interfaces': 'burstioInterfaces',
        'rpmname':    'burstioInterfaces',
        'version':    versions.burstio,
        'pymodule':   'redhawk.burstio',
        'jarfiles':   ('burstio.jar', 'BURSTIOInterfaces.jar')
    },
    'FRONTEND': {
        'libname':    'frontend',
        'interfaces': 'frontendInterfaces',
        'rpmname':    'frontendInterfaces',
        'version':    versions.frontend,
        'pymodule':   'frontend',
        'jarfiles':   ('frontend.jar', 'FRONTENDInterfaces.jar',)
    }
}

def getInterfaceLibrary(namespace):
    if namespace in redhawkLibs:
        return redhawkLibs[namespace]
    else:
        name = namespace.lower() + 'Interfaces'
        library = {
            'libname': name,
            'interfaces': name,
            'rpmname': name,
            'version': None,
            'pymodule': python.idlModule(namespace),
            'jarfiles': (namespace + 'Interfaces.jar',)
        }
        return library

def getPackageRequires(namespace):
    library = getInterfaceLibrary(namespace)
    if library['version']:
        return '%s >= %s' % (library['libname'], library['version'])
    else:
        return library['libname']

def getRPMDependency(name):
    library = getInterfaceLibrary(name)
    if library['version']:
        return '%s >= %s' % (library['rpmname'], library['version'])
    else:
        return library['interfaces']
