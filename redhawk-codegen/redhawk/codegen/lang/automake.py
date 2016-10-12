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

def canonicalName(name):
    """
    Returns the canonical name for the target 'name'. This is the prefix that
    should be used for all related variables (e.g. "target_SOURCES").

    From the Automake manual:
      All characters in the name except for letters, numbers, the strudel (@),
      and the underscore are turned into underscores when making macro
      references.
    """
    retval = ''
    for ch in name:
        if (ch.isalnum() or ch == '@'):
            retval += ch
        else:
            retval += '_'
    return retval

def libtoolName(name):
    """
    Returns the file name of the Libtool library given a library name.
    """
    return 'lib'+name+'.la'
