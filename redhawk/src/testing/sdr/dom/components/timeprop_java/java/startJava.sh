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
JAVA_LIBDIR=${myDir}/../../../../../base/framework/java
JAVA_CLASSPATH=${JAVA_LIBDIR}/apache-commons-lang-2.4.jar:${JAVA_LIBDIR}/log4j-1.2.15.jar:${JAVA_LIBDIR}/CFInterfaces.jar:${JAVA_LIBDIR}/ossie.jar:${myDir}/timeprop_java.jar:${myDir}:${myDir}/bin:${CLASSPATH}

# Path for Java
if test -x $JAVA_HOME/bin/java; then
  JAVA=$JAVA_HOME/bin/java
else
  JAVA=java
fi

# NOTE: the $@ must be quoted "$@" for arguments to be passed correctly

#Sun ORB start line
exec $JAVA -cp ${JAVA_CLASSPATH} timeprop_java.java.timeprop_java "$@"
