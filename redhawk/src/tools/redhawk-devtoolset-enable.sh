#!/bin/bash
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

# This file started from the `enable` file of `scl enable devtoolset-9`.
# It exports the env vars to the calling shell,
# instead of as a prefix to a command.

# Guard against executing, which fails to set the env vars in the calling shell.
if [[ "${BASH_SOURCE[0]}" -ef "$0" ]]; then
    echo "This script should be sourced, not executed."
    exit 1
fi

# General environment variables
dpath=/opt/rh/devtoolset-9
if [[ ! "$PATH" =~ "$dpath" ]]; then
    export PATH=${dpath}/root/usr/bin${PATH:+:${PATH}}
fi
if [[ ! "$MANPATH" =~ "$dpath" ]]; then
    export MANPATH=${dpath}/root/usr/share/man:${MANPATH}
fi
if [[ ! "$INFOPATH" =~ "$dpath" ]]; then
    export INFOPATH=${dpath}/root/usr/share/info${INFOPATH:+:${INFOPATH}}
fi
if [[ ! "$PCP_DIR" =~ "$dpath" ]]; then
    export PCP_DIR=${dpath}/root
fi
# bz847911 workaround:
# we need to evaluate rpm's installed run-time % { _libdir }, not rpmbuild time
# or else /etc/ld.so.conf.d files?
rpmlibdir=$(rpm --eval "%{_libdir}")
# bz1017604: On 64-bit hosts, we should include also the 32-bit library path.
if [ "$rpmlibdir" != "${rpmlibdir/lib64/}" ]; then
  rpmlibdir32=":${dpath}/root${rpmlibdir/lib64/lib}"
fi
if [[ ! "$LD_LIBRARY_PATH" =~ "$dpath" ]]; then
    export LD_LIBRARY_PATH=${dpath}/root$rpmlibdir$rpmlibdir32${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
    export LD_LIBRARY_PATH=${dpath}/root$rpmlibdir$rpmlibdir32:${dpath}/root$rpmlibdir/dyninst$rpmlibdir32/dyninst${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
fi
if [[ ! "$PKG_CONFIG_PATH" =~ "$dpath" ]]; then
    export PKG_CONFIG_PATH=${dpath}/root/usr/lib64/pkgconfig${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}
fi

