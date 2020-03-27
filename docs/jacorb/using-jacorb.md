# Using JacORB with Java Components

JacORB is a free and open-source Java implementation of CORBA.
It provides better performance and reliability than the default Sun implementation.
The REDHAWK IDE uses JacORB internally, and the following procedures explain how a REDHAWK system can be configured to use it as the default for all components.
No changes to components are required when following these procedures.

**_Note_: Installing JacORB and configuring Java require superuser privileges.**

## Installing JacORB

1. Download JacORB from the [JacORB website](https://www.jacorb.org) or a trusted location.
Then, extract the archive in a location that is accessible to all users.

    The following examples assume that you have downloaded JacORB 3.9 to `/tmp` and are installing to `/usr/share/java`.

    ```sh
    cd /usr/share/java
    unzip /tmp/jacorb-3.9.zip
    ```

2. All `.jar` files in `$OSSIEHOME/lib` are automatically added to the Java classpath by the component's startup script. Therefore, rather than setting the `CLASSPATH` variable manually for the REDHAWK system, link or copy the required `.jar` files into `$OSSIEHOME/lib`.

    ```sh
    ln -s /usr/share/java/jacorb-3.9/lib/jacorb-*.jar $OSSIEHOME/lib/
    ln -s /usr/share/java/jacorb-3.9/lib/slf4j-*.jar $OSSIEHOME/lib/
    ```

## Configuring Java

The Java Runtime Environment (JRE) uses an optional property file to set CORBA ORB configuration options, including which ORB implementation to use.

1. Log in as root user and create a file, `$JAVA_HOME/jre/lib/orb.properties`, with the following contents:

    ```properties
    org.omg.CORBA.ORBClass=org.jacorb.orb.ORB
    org.omg.CORBA.ORBSingletonClass=org.jacorb.orb.ORBSingleton
    jacorb.config.dir=/usr/local/redhawk/core/etc
    ```

    Setting the configuration directory to `$OSSIEHOME/etc` is optional.
    However, placing JacORB configuration files in a REDHAWK-specific location makes them portable across different versions of Java.

2. Ensure that all users have read permissions on the `orb.properties` file.

    ```sh
    chmod a+r $JAVA_HOME/jre/lib/orb.properties
    ```

## Configuring JacORB

JacORB supports a properties file for tuning ORB parameters.
Refer to the [JacORB Documentation](https://www.jacorb.org/documentation.html) for details about the supported properties.

To ensure consistency between C++, Python, and Java, REDHAWK reads the omniORB configuration file (by default, `/etc/omniORB.cfg`) to get the initial references for:

* NameService
* EventService

All other settings are obtained from the JacORB properties file. To create a default configuration file for JacORB:

1. If the `$OSSIEHOME/etc` directory does not already exist, create it.

    ```sh
    mkdir -p $OSSIEHOME/etc/
    ```

2. Copy the JacORB properties template file into `$OSSIEHOME/etc`.

    ```sh
    cp /usr/share/java/jacorb-3.9/etc/jacorb_properties.template $OSSIEHOME/etc/jacorb.properties
    ```

### Logging Output

The default logging configuration for JacORB enables messages at `INFO` or higher.
This setting produces a large number of messages to the console that are not necessary in normal operation.
To reduce the logging level and eliminate `INFO` messages at component startup, edit `jacorb.properties` and change:

```properties
#jacorb.log.default.verbosity=3
```

to:

```properties
jacorb.log.default.verbosity=2
```
