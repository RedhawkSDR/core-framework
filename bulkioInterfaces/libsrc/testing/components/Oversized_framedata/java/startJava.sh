#!/bin/sh
myDir=`dirname $0`

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

# Path for Java
if test -x $JAVA_HOME/bin/java; then
  JAVA=$JAVA_HOME/bin/java
else
  JAVA=java
fi

# NOTE: the $@ must be quoted "$@" for arguments to be passed correctly

#Sun ORB start line
exec $JAVA -cp :$myDir/Oversized_framedata.jar:$myDir/bin:$CLASSPATH Oversized_framedata.java.Oversized_framedata "$@"

#JacORB start lines
#exec $JAVA -cp :$myDir/jacorb.jar:$myDir/antlr.jar:$myDir/avalon-framework.jar:$myDir/backport-util-concurrent.jar:$myDir/logkit.jar:$myDir/Oversized_framedata.jar:$myDir/bin:$CLASSPATH Oversized_framedata.java.Oversized_framedata "$@"
