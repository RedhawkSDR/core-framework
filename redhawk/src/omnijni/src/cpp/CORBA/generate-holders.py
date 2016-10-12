#!/usr/bin/python
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

import sys
import os, os.path

# Copy of path-building logic from omniidl
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

from omniidl import idlast, idltype
from omnijni import cppcode
from omnijni import idljni

holderKinds = (
    idltype.tk_boolean,
    idltype.tk_octet,
    idltype.tk_char,
    idltype.tk_short,
    idltype.tk_ushort,
    idltype.tk_long,
    idltype.tk_ulong,
    idltype.tk_longlong,
    idltype.tk_ulonglong,
    idltype.tk_float,
    idltype.tk_double,
    idltype.tk_any
    )

holderTypes = [idltype.Base(k) for k in holderKinds] + [idltype.String(0)]

if __name__ == '__main__':
    header = cppcode.Header('holders.h')
    header.include('<omniORB4/CORBA.h>')
    header.include('<jni.h>')
    header.include('<omnijni/loader.h>')
    body = header.Namespace('CORBA').Namespace('jni')

    module = cppcode.Module()
    module.include('<omnijni/omnijni.h>')
    module.include('"holders.h"')

    helper = idljni.HolderHelper()
    for node in holderTypes:
        # Generate declarations
        body.append(helper.generateDecl(node))
        body.append()

        # Generate implementations
        module.append(helper.generateImpl(node))
        module.append()

    header.write(cppcode.SourceFile(open('holders.h', 'w')))
    module.write(cppcode.SourceFile(open('holders.cpp', 'w')))
