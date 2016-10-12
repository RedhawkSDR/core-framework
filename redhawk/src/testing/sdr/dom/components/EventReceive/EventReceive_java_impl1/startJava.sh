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

myDir=`dirname $0`

# NOTE: the $@ must be quoted "$@" for arguments to be passed correctly

#Sun ORB start line

$JAVA_HOME/bin/java -cp :../../../../../base/framework/java/ossie.jar:../../../../../base/framework/java/log4j-1.2.15.jar:../../../../../base/framework/java/CFInterfaces.jar:$myDir/EventReceive_java_impl1.jar:$myDir/bin:$CLASSPATH EventReceive_java_impl1.EventReceive_java_impl1 "$@"

#JacORB start lines
#$JAVA_HOME/bin/java -cp :$OSSIEHOME/lib/ossie.jar:$OSSIEHOME/lib/CosEventChannelAdminInterfaces.jar:$OSSIEHOME/lib/log4j.jar:$OSSIEHOME/lib/CFInterfaces.jar::$myDir/jacorb.jar:$myDir/antlr.jar:$myDir/avalon-framework.jar:$myDir/backport-util-concurrent.jar:$myDir/logkit.jar:$myDir/EventReceive_java_impl1.jar:$myDir/bin:$CLASSPATH EventReceive_java_impl1.EventReceive_java_impl1 "$@"
