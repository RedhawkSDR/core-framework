# Configuring Logger Settings

A logger is configured through two mechanisms: a log configuration file and a global log level.

The log configuration file enables users to manage appenders and configure the settings for individual loggers. The global log level is a shortcut that allows users to set the log level for an entire resource with a single call or command-line switch. When a configuraton file and log level are used together, the configuration file describes the settings for appenders and named loggers, while the log level is applied to the resource's main logger.

A log configuration file can be:

- Passed as a command-line argument when the <abbr title="See Glossary.">Domain Manager</abbr> is started
```
nodeBooter -D -logcfgfile logconfiguration.cfg
```

> **NOTE**  
> When passed through the Domain Manager, every <abbr title="See Glossary.">component</abbr> that does not have a logging configuration set will use the <abbr title="See Glossary.">domain</abbr>'s logging configuration.  

- Passed as an initialization <abbr title="See Glossary.">property</abbr> when an <abbr title="See Glossary.">application</abbr> is created
```
>>> app = dom.createApplication("/waveforms/example/example.sad.xml", initConfiguration={'LOGGING_CONFIG_URI':'file:///home/user/logconfiguration.cfg'})
```

> **NOTE**  
> When passed through the `createApplication` function, the `LOGGING_CONFIG_URI` is passed to all components in the application.  

- Added to the component instance in a Software Assembly Descriptor (SAD) file

    ![Add Logging Configuration to a Component](images/LoggingApp.png)

- Passed at runtime through the [logging API](adjusting-logging-at-runtime.html)

For <abbr title="See Glossary.">devices</abbr> and <abbr title="See Glossary.">services</abbr>, the log configuration URI is resolved using a slightly different set of rules than REDHAWK components.

- Passed as a command-line argument when the <abbr title="See Glossary.">Device Manager</abbr> is started
```
nodeBooter -d $SDRROOT/dev/nodes/DevMgr_hostname/DeviceManager.dcd.xml -logcfgfile logconfiguration.cfg
```

- Added to the device `componentinstantiation` element as element `loggingconfig` in the Device Configuration Descriptor (DCD) file
- Added to a device instance as property `LOGGING_CONFIG_URI` in the DCD file

The URI is resolved through 2 different methods:

- Logger found through the SCA file system. For the Domain Manager, this is `$SDRROOT/dom`; for the Device Manager, this is `$SDRROOT/dev`. For example, with the Domain Manager, the URI `sca:///myfile.cfg` is equivalent to `$SDRROOT/dom/myfile.cfg`.

- Logger found through the local file system: `file:///tmp/myfile.cfg` is equivalent to `/tmp/myfile.cfg`.

If the relative path to the file is provided as a command-line argument to `nodeBooter`, the file location is converted to an absolute path directory and then passed as a file URI to subsequent processes.

If no log configuration file is provided, the following log configuration is used by default:

```
log4j.rootLogger=INFO,STDOUT
log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender
log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout
log4j.appender.STDOUT.layout.ConversionPattern="%d{yyyy-MM-dd HH:mm:ss} %-5p %c{3}:%L - %m%n \n"
```

A resource has the ability to provide a severity level (logging level) with each logging message. The following severity levels are listed in order of decreasing severity. `FATAL` messages describe events that are unrecoverable to the resource through a decreasing level to `TRACE`, which is used to log fine-grained behavior. Setting a lower severity threshold increases verbosity.

  - `FATAL`
  - `ERROR`
  - `WARN`
  - `INFO`
  - `DEBUG`
  - `TRACE`

Each different logging implementation library uses a log4j configuration file format to define the configuration context for that library. This file defines how and where the logging messages are recorded. For example, if the configuration logging level is set to `INFO`, then messages published at the `INFO`, `WARN`, `ERROR`, and `FATAL` severity levels are displayed in the log. All other message levels are suppressed. The following two log-level configuration options are also available.

  - `OFF`: Suppress all logging messages from the log
  - `ALL`: Allow all logging messages

The previous configuration suppresses logging levels above `INFO` and writes those messages to the standard out console.

### Configuration Context Tokens

For REDHAWK release 1.10 and above, the log4j configuration file can contain special tokens that expand to provide the runtime execution context of the resource. These tokens can be located anywhere in the configuration file and are resolved during resource startup. The following table describes the tokens.

##### log4j Configuration File Special Tokens
| **Token**                       | **Definition**                                                                        |
| :------------------------------ | :------------------------------------------------------------------------------------ |
| `@@@HOST.NAME@@@`               | Host name returned from gethostname call (multi-homed systems may have issues).       |
| `@@@HOST.IP@@@`                 | Dot notation IP Address of the machine on which the processing is running.            |
| `@@@NAME@@@`                    | Name of the resource given on the command line.                                       |
| `@@@INSTANCE@@@`                | Instance name given to the resource on the command line.                              |
| `@@@PID@@@`                     | pid of the running resource.                                                          |
| `@@@DOMAIN.NAME@@@`             | Name of the domain under which your resource is running.                              |
| `@@@DOMAIN.PATH@@@`             | The contents of the DOM_PATH parameter on the command line.                          |
| `@@@DEVICE_MANAGER.NAME@@@`     | Name of the Device Manager that started the device or service.                         |
| `@@@DEVICE_MANAGER.INSTANCE@@@` | Instance name of the Device Manager from the command line.                             |
| `@@@SERVICE.NAME@@@`            | Name of the service as specified on the command line.                                 |
| `@@@SERVICE.INSTANCE@@@`        | Instance name of the service as specified on the command line.                        |
| `@@@SERVICE.PID@@@`             | pid of the running service.                                                           |
| `@@@DEVICE.NAME@@@`             | Name of the device as specified on the command line.                                  |
| `@@@DEVICE.INSTANCE@@@`         | Instance name of the device as specified on the command line.                         |
| `@@@DEVICE.PID@@@`              | pid of the device.                                                                    |
| `@@@WAVEFORM.NAME@@@`           | Name of the <abbr title="See Glossary.">waveform</abbr> from the DOM_PATH command line variable.             |
| `@@@WAVEFORM.INSTANCE@@@`       | Instance name of the waveform from the DOM_PATH command line variable.    |
| `@@@COMPONENT.NAME@@@`          | Name of the component binding parameter as specified on the command line.             |
| `@@@COMPONENT.INSTANCE@@@`      | Instance name of the component identifier parameter as specified on the command line. |
| `@@@COMPONENT.PID@@@`           | pid of the component. |


This table lists the availability of token definitions for each REDHAWK resource type.

##### Availability of Tokens
| **Token**                            | **Domain Manager**        | **Device Manager**       | **Device** | **Service** | **Component** |
| :----------------------------------- | :------------------------ | :----------------------- | :--------- | :---------- | :------------ |
| `@@@HOST.IP@@@`                      | Y                         | Y                        | Y          | Y           | Y             |
| `@@@HOST.NAME@@@`                    | Y                         | Y                        | Y          | Y           | Y             |
| `@@@NAME@@@`                         | Domain Manager            | Y                        | Y          | Y           | Y             |
| `@@@INSTANCE@@@`                     | `DOMAIN_MANAGER_1`          | Y                        | Y          | Y           | Y             |
| `@@@PID@@@`                          | Y                         | Y                        | Y          | Y           | Y             |
| `@@@DOMAIN.NAME@@@`                  | Y                         | Y                        | Y          | Y           | Y             |
| `@@@DOMAIN.PATH@@@`                  | Y                         | Y                        | Y          | Y           | Y             |
| `@@@DEVICE_MANAGER.NAME@@@`          | N                         | `DEVICE_MANAGER`          | Y          | Y           | N             |
| `@@@DEVICE_MANAGER.INSTANCE@@@`      | N                         | Y                        | Y          | Y           | N             |
| `@@@SERVICE.NAME@@@`                 | N                         | Y                        | N          | Y           | N             |
| `@@@SERVICE.INSTANCE@@@`             | N                         | Y                        | N          | Y           | N             |
| `@@@SERVICE.PID@@@`                  | N                         | Y                        | N          | Y           | N             |
| `@@@DEVICE.NAME@@@`                  | N                         | Y                        | Y          | N           | N             |
| `@@@DEVICE.INSTANCE@@@`              | N                         | Y                        | Y          | N           | N             |
| `@@@DEVICE.PID@@@`                   | N                         | Y                        | Y          | N           | N             |
| `@@@WAVEFORM.NAME@@@`                | N                         | N                        | N          | N           | Y             |
| `@@@WAVEFORM.INSTANCE@@@`            | N                         | N                        | N          | N           | Y             |
| `@@@COMPONENT.NAME@@@`               | N                         | N                        | N          | N           | Y             |
| `@@@COMPONENT.INSTANCE@@@`           | N                         | N                        | N          | N           | Y             |
| `@@@COMPONENT.PID@@@`                | N                         | N                        | N          | N           | Y             |


### Log Configuration Example - Simple Appender with a Named Logger

In the following example, the root most logger passes logging messages with a severity level `INFO` or higher. Those messages are sent to the appenders called: `CONSOLE` and `FILE`. The `CONSOLE` appender messages are displayed in the console of the running application. The `FILE` appender writes log messages to a file called `allmsgs.out`.

If the resource uses a named logger, `EDET_1.user.detections`, then log messages to this logger with a severity of `DEBUG` or higher are diverted to a file called `edet_log.out`.

```
# Set root logger default levels and appender
log4j.rootLogger=INFO, CONSOLE, FILE

# Console Appender
log4j.appender.CONSOLE=org.apache.log4j.ConsoleAppender
log4j.appender.STDERR=org.apache.log4j.ConsoleAppender
log4j.appender.STDERR.Threshold=ERROR
log4j.appender.STDERR.Target=System.err

# Default Log Appender
log4j.appender.FILE=org.apache.log4j.FileAppender
log4j.appender.FILE.Append=true
log4j.appender.FILE.File=allmsgs.out

# Edet Appender
log4j.appender.edetLog=org.apache.log4j.FileAppender
log4j.appender.edetLog.Append=true
log4j.appender.edetLog.File=edet_log.out

# Appender layout
log4j.appender.CONSOLE.layout=org.apache.log4j.PatternLayout
log4j.appender.CONSOLE.layout.ConversionPattern=%d{ISO8601}:
    %p:%c - %m [%F:%L]%n
log4j.appender.STDERR.layout=org.apache.log4j.PatternLayout
log4j.appender.STDERR.layout.ConversionPattern=%d{ISO8601}:
    %p:%c - %m [%F:%L]%n
log4j.appender.FILE.layout=org.apache.log4j.PatternLayout
log4j.appender.FILE.layout.ConversionPattern=%d{ISO8601}:
    %p:%c - %m [%F:%L]%n
log4j.appender.edetLog.layout=org.apache.log4j.PatternLayout
log4j.appender.edetLog.layout.ConversionPattern=%d{ISO8601}:
    %p:%c - %m [%F:%L]%n
log4j.appender.NULL.layout=org.apache.log4j.PatternLayout
log4j.appender.NULL.layout.ConversionPattern=%n

log4j.category.EDET_1.user.detections=DEBUG, edetLog
log4j.additivity.EDET_1.user.detections=false
```

### Log Configuration Example - Configuring a Component with Token Macros

The logging configuration information for component `MEGA_WORKER` is configured from `$SDRROOT/dom/logcfg/component.log4j.cfg`. Prior to configuring the underlying logging library, the configuration information is processed for the context macros (in this example, `@@@WAVEFORM.NAME@@@`, `@@@COMPONENT.NAME@@@` and `@@@COMPONENT.PID@@@`). The root most logger passes logging messages with a severity level `INFO` or higher, to the appenders called: `CONSOLE` and `FILE`. The `CONSOLE` appender messages are displayed in the console of the running application. For the `FILE` appender, the destination file is: `/data/logdir/MY_EXAMPLE_1/MEGA_WORKER_1.212.log`.

Waveform: MY_EXAMPLE  
Component: MEGA_WORKER  
 property: LOGGING_CONFIG_URI = "sca:///logcfg/component.log4j.cfg"


 $SDRROOT/  
 &nbsp;&nbsp;&nbsp;&nbsp; dev/  
 &nbsp;&nbsp;&nbsp;&nbsp; deps/  
 &nbsp;&nbsp;&nbsp;&nbsp; dom/
 &nbsp;&nbsp;&nbsp;&nbsp;  
 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; logcfg/
 &nbsp;&nbsp;&nbsp;&nbsp;  
 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; component.log4j.cfg
 &nbsp;&nbsp;&nbsp;&nbsp;  
 &nbsp;&nbsp;&nbsp;&nbsp;  
 /data/logdir/

```
# Set root logger default levels and appender
log4j.rootLogger=INFO, CONSOLE, FILE

# Console Appender
log4j.appender.CONSOLE=org.apache.log4j.ConsoleAppender

# File Appender
log4j.appender.FILE=org.apache.log4j.FileAppender
log4j.appender.FILE.File=/data/logdir/@@@WAVEFORM.NAME@@@/@@@COMPONENT.NAME@@@.@@@COMPONENT.PID@@@.log
log4j.appender.FILE.layout=org.apache.log4j.PatternLayout
log4j.appender.FILE.layout.ConversionPattern=%d{ISO8601}: %p:%c - %m [%F:%L]%n
```

When the waveform `MY_EXAMPLE` is deployed on the domain, the component is launched with the following logging configuration:

`MEGA_WORKER "....." LOGGING_CONFIG_URI sca:///logcfg/component.log4j.cfg?fs=IOR:010...`

### Logging with Event Channels for Components, Devices, and Services

For REDHAWK resources, the underlying logging functionality has been extended to include support for publishing log messages to a specified <abbr title="See Glossary.">event channel</abbr>. To include this capability, add the `org.ossie.logging.RH_LogEventAppender` in your log4j configuration file. This appender responds to the following configuration options (all options are string values unless otherwise noted):

##### RH_LogEventAppender Configuration Options
| **Appender Option** | **Description**                                                                            |
| :------------------ | :----------------------------------------------------------------------------------------- |
| `EVENT_CHANNEL`     | Event channel name where logging messages are published.             |
| `PRODUCER_ID`       | Identifier of the resource producing the log message (resource name).                      |
| `PRODUCER_NAME`     | Name of the resource producing the log message.                                            |
| `PRODUCER_FQN`      | Fully qualified name of the resource (domain-name/waveform-name/resource-name). |
| `RETRIES`           | Number of times to retry connecting to the event channel. (Integer)                         |
| `THRESHOLD`         | log4cxx log level; `FATAL`, `WARN`, `ERROR`, `INFO`, `DEBUG`, `TRACE`.                     |

In the following example, a component configured with this log4j properties file publishes log messages with a severity of `ERROR` or higher to the event channel `ERROR_LOG_CHANNEL` in the domain `REDHAWK_DEV`. The threshold level for the appender supersedes the `rootLogger`'s logging level.

```
log4j.rootLogger=INFO,stdout,pse

# Direct log messages to stdout
log4j.appender.stdout=org.apache.log4j.ConsoleAppender
log4j.appender.stdout.Target=System.out
log4j.appender.stdout.layout=org.apache.log4j.PatternLayout
log4j.appender.stdout.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{3}:%L - %m%n

# Direct error messages to an event channel
log4j.appender.pse=org.ossie.logging.RH_LogEventAppender
log4j.appender.pse.Threshold=ERROR
log4j.appender.pse.event_channel=ERROR_LOG_CHANNEL
log4j.appender.pse.name_context=@@@DOMAIN.NAME@@@
log4j.appender.pse.producer_id=@@@COMPONENT.INSTANCE@@@
log4j.appender.pse.producer_name=@@@COMPONENT.NAME@@
log4j.appender.pse.producer_FQN=@@@DOMAIN.NAME@@@.@@@WAVEFORM.NAME@@@.@@@COMPONENT.NAME@@@
log4j.appender.pse.layout=org.apache.log4j.PatternLayout
log4j.appender.pse.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n
```

### Synchronous Logging for C++ Devices and Components

In versions of log4cxx older than 0.10.0, logging messages from different sources could be interleaved in the same line when using the default file appender. To deal with this issue, the REDHAWK Core Framework (CF) libraries provide a Synchronous Rolling File Appender that will allow atomic write operations to a common file. To include this capability, add the `org.ossie.logging.RH_SyncRollingAppender` in your log4j configuration file. This appender responds to the following configuration options (all options are string values unless otherwise noted):

##### RH_SyncRollingAppender Configuration Options
| **Appender Option** | **Description**                                                                       |
| :------------------ | :------------------------------------------------------------------------------------ |
| `Retries`           | Number of retries when waiting for the lock fails. (Integer)                          |
| `WaitOnLock`        | Time in milliseconds to wait when attempting to take ownership of the lock. (Integer) |
| `MaxFileSize`       | Maximum file size before rolling to the next file.                                    |
| `MaxBackupIndex`    | Maximum number of files to keep. (Integer)                                            |
| `File`              | Full local file system path.                                                          |
| `Cleanup`           | Clean up synchronization resources when the process ends. (Value is True or False.)   |


In the following example, a component configured with this log4j properties file publishes a log message to the file `MP_RedhawkTest`.

```
log4j.rootLogger=INFO,stdout,mp

# Direct log messages to stdout
log4j.appender.stdout=org.apache.log4j.ConsoleAppender
log4j.appender.stdout.Target=System.out
log4j.appender.stdout.layout=org.apache.log4j.PatternLayout
log4j.appender.stdout.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{3}:%L - %m%n

log4j.appender.mp=org.ossie.logging.RH_SyncRollingAppender
log4j.appender.mp.Retries=2
log4j.appender.mp.WaitOnLock=30
log4j.appender.mp.MaxFileSize=5MB
log4j.appender.mp.MaxBackupIndex=10
log4j.appender.mp.File=MP_RedhawkTest
log4j.appender.mp.Cleanup=False
log4j.appender.mp.layout=org.apache.log4j.PatternLayout
log4j.appender.mp.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n
```

> **NOTE**  
> The synchronous appender only works for single <abbr title="See Glossary.">GPP</abbr> systems. The synchronization resources used require all the processes to reside on the same host.  

