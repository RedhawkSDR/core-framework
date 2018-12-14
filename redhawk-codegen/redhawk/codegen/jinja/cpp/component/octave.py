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

import commands
import os

from redhawk.codegen.jinja.cpp.component.mFunction.generator import OctaveComponentGenerator, loader
from redhawk.codegen import versions

def factory(**opts):
    return OctaveComponentGenerator(**opts)

def _version_tuple(ver):
    return tuple(int(n) for n in ver.split('.'))

def _check_octave():
    # Attempt to determine if octave-devel v3.4 or greater is installed.
    (status, output) = commands.getstatusoutput('octave-config -v')
    if status:
        return False

    # Check the version against the minimum
    version = _version_tuple(output)
    if version < _version_tuple(versions.octave):
        return False

    incdir = commands.getoutput('octave-config -p OCTINCLUDEDIR')
    return os.path.exists(incdir)

def check():
    if _check_octave():
        return True
    else:
        print "Could not find suitable Octave installation.  Octave-devel v%s or greater is required." % versions.octave
        return False
