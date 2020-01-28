#!/bin/sh
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

if [ "$1" = "rpm" ]; then
    # A very simplistic RPM build scenario
    if [ -e GPP.spec ]; then
        mydir=`dirname $0`
        tmpdir=`mktemp -d`
        cp -r ${mydir} ${tmpdir}/GPP-2.2.6
        tar czf ${tmpdir}/GPP-2.2.6.tar.gz --exclude=".svn" -C ${tmpdir} GPP-2.2.6
        rpmbuild -ta ${tmpdir}/GPP-2.2.6.tar.gz
        rm -rf $tmpdir
    else
        echo "Missing RPM spec file in" `pwd`
        exit 1
    fi
else
    for impl in cpp ; do
        cd $impl
        if [ -e build.sh ]; then
            ./build.sh $*
        elif [ -e reconf ]; then
            ./reconf && ./configure && make $*
        else
            echo "No build.sh found for $impl"
        fi
        cp GPP ../tests/sdr/dev/devices/GPP/cpp/
        cd -
    done
fi
