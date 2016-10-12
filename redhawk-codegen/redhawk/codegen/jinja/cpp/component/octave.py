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

from redhawk.codegen.jinja.cpp.component.mFunction.generator import OctaveComponentGenerator, loader

def factory(**opts):
    return OctaveComponentGenerator(**opts)

def check():
    # Attempt to determine if octave-devel v3.4 or greater is installed.
    findCommand = 'find /usr -regextype posix-extended -regex ".*include\/octave\-[3-9]+\.[4-9]+\.[0-9]+$" -print -quit 2>/dev/null'
    (status,output) = commands.getstatusoutput(findCommand)
    if output == "":
        # suitable octave header files were not found
        print "Could not find suitable Octave installation.  Octave-devel v3.4 or greater is required."
        return False
    else:
        # suitable octave header files were found
        return True
