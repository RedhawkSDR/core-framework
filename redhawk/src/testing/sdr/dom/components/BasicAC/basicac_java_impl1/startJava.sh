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
myDir=`dirname $0`
# Setup the OSSIEHOME Lib jars on the classpath
libDir=${SDRROOT}/../../base/framework/java
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

# Path for Java
if test -x $JAVA_HOME/bin/java; then
  JAVA=$JAVA_HOME/bin/java
else
  JAVA=java
fi

# NOTE: the $@ must be quoted "$@" for arguments to be passed correctly

#Sun ORB start line
exec $JAVA -cp :$myDir/BasicAC_java_impl1.jar:$myDir/bin:$CLASSPATH basicac_java_impl1.basicac_java_impl1 "$@"

#JacORB start lines
#$JAVA_HOME/bin/java -cp $OSSIEHOME/lib/log4j.jar:$OSSIEHOME/lib/CFInterfaces.jar:$OSSIEHOME/lib/ossie.jar:$myDir/jacorb.jar:$myDir/antlr.jar:$myDir/avalon-framework.jar:$myDir/backport-util-concurrent.jar:$myDir/logkit.jar:$OSSIEHOME/lib/CFInterfaces.jar:$myDir:$myDir/BasicAC_java_impl1.jar:$myDir/bin:$CLASSPATH basicac_java_impl1.basicac_java_impl1 "$@"
