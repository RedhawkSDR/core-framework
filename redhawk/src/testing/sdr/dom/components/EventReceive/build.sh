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


for impl in EventReceive_java_impl1 ; do
    pushd $impl &> /dev/null
    if [ "$1" == "rpm" ]; then
        if [ -e *.spec ]; then
            rpmbuild -bb *.spec
        else
            echo "Missing RPM spec file in" `pwd`
            exit 1
        fi
    elif [ -e build.sh ]; then
        ./build.sh $*
    fi
    popd &> /dev/null
done

if [ "$1" == "rpm" ]; then
    rpmbuild -bb EventReceive.spec
fi
