# Java and JacORB

Starting with version 3, Redhawk requires Java 11 to run Java components.  However, you don't need to install or configure Java if you configure Redhawk with:
```sh
./configure --disable-java
```

The Redhawk IDE continues to use Java 1.8, with its integrated Sun implementation of CORBA ORB.

## Install and Enable Java 11
```sh
$ sudo yum install java-11-openjdk-devel
$ sudo alternatives --config java  # select the Java 11 option
$ sudo alternatives --config javac  # select the Java 11 option
```

Check that version 11 is enabled with:
```sh
$ java -version
java 11.[...]
$ javac -version
javac 11.[...]
```

## Configure Java to Use Jacorb
With the switch to `java-11-openjdk`, the jdk no longer supports CORBA.  We get CORBA support via JacORB.

It is fine to do this step before JacORB is installed; however, the Java 11 jdk must be installed, and set active via `alternatives`.

The Java Runtime Environment (JRE) uses an optional property file to set CORBA ORB configuration options, including which ORB implementation to use.  Create that property file with this script:
```sh
#!/bin/bash

tmpfpath=$(mktemp /tmp/orb.properties.XXXXXX)
printf "%s\n" "org.omg.CORBA.ORBClass=org.jacorb.orb.ORB" >>$tmpfpath
printf "%s\n" "org.omg.CORBA.ORBSingletonClass=org.jacorb.orb.ORBSingleton" >>$tmpfpath
printf "%s\n" "jacorb.config.dir=/etc" >>$tmpfpath

dstdir=$(readlink -e `which java`)
if [[ ! "$dstdir" =~ "java-11-openjdk" ]]; then
    echo "Error:  ``java`` must point to Java 11 before running this script.  quitting.
    rm $tmpfpath
    exit 1
fi
dstdir=`dirname $dstdir`
dstdir=`dirname $dstdir`
dstdir=$dstdir/lib
fpath=$dstdir/orb.properties
echo "Create $fpath"
sudo mv $tmpfpath $fpath
sudo chmod a+r $fpath
sudo chown root:root $fpath
```

## Install JacORB
JacORB comes bundled as a dependency of Redhawk.  
If you install `redhawk` via rpm, JacORB will also be installed as a requirement.  
If you build Redhawk from source, you will install JacORB with the other packaged dependencies in the `redhawk-dependencies` yum repository.  Instructions for that are in the Installation section of the manual.

## Configure JacORB
To ensure consistency between C++, Python, and Java, Redhawk reads the omniORB configuration file (by default, `/etc/omniORB.cfg`) to get the initial references for:

* NameService
* EventService

JacORB supports its own configuration file for tuning ORB properties.
Refer to the [JacORB documentation](https://www.jacorb.org/documentation.html) for details about the supported properties.
When JacORB is installed, it installs this file at `/etc/jacorb.properties`.
The file is installed with suitable default values for running Redhawk.

> **Note**:  In its inline comments, `jacorb.properties` claims to affect JacORB logging, but in this context it does not.
See below for how to set the JacORB logging level.

## Set `JACORB_HOME`
```sh
export JACORB_HOME=/usr/share/java/jacorb
```
Make that setting persist any time you compile or run Redhawk.

## Set JacORB Logging Level
In Redhawk, we set JacORB's log level to `WARN`.
Users can adjust that level.
See the instructions in Redhawk Manual under Logging, Configuring Logger Settings.

In addition, if you are running in the Python sandbox, you can pass a logging configuration file to your component.  This file is conventionally named `log4j.properties`.  See sample configuration settings in the referenced Logging page in the manual.  After the line that creates `rootLogger`, add a line like:
```properties
log4j.logger.org.jacorb=WARN,CONSOLE
```
In the place of `WARN`, put the level you wish.  
In the place of `CONSOLE`, match the name of the appender you wish to use.  
Pass the file to your component:
```py
from ossie.utils import sb

props = {'LOGGING_CONFIG_URI': 'file:///path/to/log4j.properties'}
sb.launch(my_component, properties=props)
```

## Set Java Environment for the Redhawk IDE
The IDE uses Java 1.8:
```sh
$ yum install java-1.8.0-openjdk-devel
```
Note that Java 1.8 has an internal CORBA ORB implementation.  Things compiled and run with Java 1.8 will use that, and not JacORB.

The IDE does not operate properly if it uses Java 11; however, it does not need to use the Java that is set in `alternatives`. While REDHAWK does not use the environment variable `JAVA_HOME`, the REDHAWK IDE will use the `JAVA_HOME` value in certain cases such as the [Export to SDR] feature.

To tell the IDE to use Java 1.8:
```sh
export JAVA_HOME=/usr/lib/jvm/java-1.8.0-openjdk
export PATH=$JAVA_HOME/bin:$PATH
```
Make those settings persist any time you run the IDE.

### Select JRE for the Redhawk IDE

In some cases, the IDE should use the Java 11 JRE, while in other cases, the Java 1.8 JRE.  Here is an example of setting it to use the Java 11 JRE.

1. At the top of the tool bar click "Window" then "Preferences"
2. In the Preferences Window expand the Java tab
3. With the Java tab expanded, select "Installed JREs"
4. Within the Installed JREs window:
    * Select Add...
    * Select Standard VM
    * For Directory select `/usr/lib/jvm/java-11-openjdk`
    * Check the box next to java-11-openjdk
    * Click Apply
5. With the Java tab expanded, select "Compiler"
6. Set "Compiler compliance level:" to 10
7. Apply and Close
