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

#
# this needs to be absolute path to python unit test framework will work
#
bulkio_top=$myDir/../../../../../
bulkio_libsrc_top=$bulkio_top/libsrc

# Setup the OSSIEHOME Lib jars on the classpath
libDir=$OSSIEHOME/lib
libFiles=`ls -1 $libDir/*.jar`
for file in $libFiles
do
	if [ x"$CLASSPATH" = "x" ]
	then
		export CLASSPATH=$file
	else
		export CLASSPATH=$file:$CLASSPATH
	fi
done

# NOTE: the $@ must be quoted "$@" for arguments to be passed correctly

#Sun ORB start line
# JNI
exec $JAVA_HOME/bin/java -cp ::$myDir/Java_Ports.jar:$myDir/bin:$bulkio_libsrc_top/bulkio.jar:$bulkio_top/BULKIOInterfaces.jar:$CLASSPATH Java_Ports.java.Java_Ports "$@"
#exec $JAVA_HOME/bin/java -cp ::$myDir/Java_Ports.jar:$myDir/bin:$CLASSPATH Java_Ports.java.Java_Ports "$@"

#JacORB start lines
#$JAVA_HOME/bin/java -cp ::$myDir/jacorb.jar:$myDir/antlr.jar:$myDir/avalon-framework.jar:$myDir/backport-util-concurrent.jar:$myDir/logkit.jar:$myDir/TestJava.jar:$myDir/bin:$CLASSPATH TestJava.java.TestJava "$@"
