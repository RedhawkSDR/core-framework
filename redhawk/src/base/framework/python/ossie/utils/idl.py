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
#
# On some systems, such as Debian-based ones, omniidl isn't directly on the
# Python path.  If you look at /usr/bin/omniidl is contains logic to find
# the modules to load.  To remain portable REDHAWK code should use:
#
#     from ossie.utils.idl import omniidl
#
# Instead of:
#
#     import omniidl
#

__all__ = ["omniidl", "omniidl_be", "_omniidl"]

import sys
import os, os.path

# Logic 'borrowed' from /usr/bin/omniidl

pylibdir   = archlibdir = None
binarchdir = os.path.abspath(os.path.dirname(sys.argv[0]))

# Try a path based on the installation prefix, customised for Debian
sppath = "/usr/lib/omniidl"

if os.path.isdir(sppath):
    sys.path.append(sppath)

# Paths in a traditional omni tree
if binarchdir != "":
    sys.path.insert(0, binarchdir)
    bindir, archname = os.path.split(binarchdir)
    treedir, bin     = os.path.split(bindir)
    if bin == "bin":
        pylibdir    = os.path.join(treedir, "lib", "python")
        vpylibdir   = pylibdir + sys.version[:3] + "/site-packages"
        vpylib64dir = (os.path.join(treedir, "lib64", "python") +
                       sys.version[:3] + "/site-packages")
        archlibdir  = os.path.join(treedir, "lib", archname)

        if os.path.isdir(pylibdir):
            sys.path.insert(0, pylibdir)

        if os.path.isdir(vpylib64dir):
            sys.path.insert(0, vpylib64dir)

        if os.path.isdir(vpylibdir):
            sys.path.insert(0, vpylibdir)

        if os.path.isdir(archlibdir):
            sys.path.insert(0, archlibdir)

    elif archname == "bin":
        pylibdir    = os.path.join(bindir, "lib", "python")
        vpylibdir   = pylibdir + sys.version[:3] + "/site-packages"
        vpylib64dir = (os.path.join(bindir, "lib64", "python") +
                       sys.version[:3] + "/site-packages")
        archlibdir  = os.path.join(bindir, "lib")

        if os.path.isdir(pylibdir):
            sys.path.insert(0, pylibdir)

        if os.path.isdir(vpylib64dir):
            sys.path.insert(0, vpylib64dir)

        if os.path.isdir(vpylibdir):
            sys.path.insert(0, vpylibdir)

        if os.path.isdir(archlibdir):
            sys.path.insert(0, archlibdir)

import omniidl
import omniidl_be
import _omniidl

omniidl = omniidl
omniidl_be = omniidl_be
_omniidl = _omniidl
