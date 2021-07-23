#!/bin/sh
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

myDir=`dirname $0`

# this needs to be absolute path to python unit test framework will work
bulkio_top=$myDir/../../../../../
bulkio_libsrc_top=$bulkio_top/libsrc

classpath=$myDir/Oversized_framedata.jar:$myDir/bin:$bulkio_libsrc_top/bulkio.jar:$bulkio_top/BULKIOInterfaces.jar
for jar in $(readlink -e $OSSIEHOME/lib/*.jar | uniq)
do
    classpath=$classpath:$jar
done

# NOTE: the $@ must be quoted "$@" for arguments to be passed correctly
exec java -cp $classpath:$CLASSPATH Oversized_framedata.java.Oversized_framedata "$@"
