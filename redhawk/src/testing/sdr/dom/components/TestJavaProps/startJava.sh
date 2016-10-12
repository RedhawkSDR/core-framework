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

#Sun ORB start line
# Important, the $@ must be quoted "$@" for arguments to be passed correctly
myDir=`dirname $0`
JAVA_LIBDIR=../../../../../base/framework/java
JAVA_CLASSPATH="${JAVA_LIBDIR}/apache-commons-lang-2.4.jar:${JAVA_LIBDIR}/log4j-1.2.15.jar:${JAVA_LIBDIR}/CFInterfaces.jar:${JAVA_LIBDIR}/ossie.jar:${myDir}/TestJavaProps.jar:${myDir}:${CLASSPATH}"
echo "myDir = ${myDir}"

COMMAND="${JAVA_HOME}/bin/java -cp ${JAVA_CLASSPATH} TestJavaProps $@"
echo "Running [${COMMAND}]"
${COMMAND}

#JacORB start lines
#$JAVA_HOME/bin/java -cp $JAVA_LIBDIR/log4j-1.2.15.jar:$JAVA_LIBDIR/CFInterfaces.jar:$JAVA_LIBDIR/ossie.jar:$myDir/jacorb.jar:$myDir/antlr.jar:$myDir/avalon-framework.jar:$myDir/backport-util-concurrent.jar:$myDir/logkit.jar:$myDir/TestJavaProps.jar:$CLASSPATH TestJavaProps "$@"
