# Adjusting Logging at Runtime

The logging level for the root logger of a <abbr title="See Glossary.">component</abbr>/<abbr title="See Glossary.">device</abbr> can be adjusted at runtime in the IDE. The following procedure explains how to adjust the logging level.

1.  Right-click the running component or device and select **Logging > Log Level**.

    The Set Debug Level dialog displays the current logging level:
    ![Set Debug Level](img/SetDebugLevel.png)

2.  Select the new logging level you want to use and click **OK**.

    The new log level is used.

After a component or device has been launched, its logging configuration can also be dynamically modified.

1.  Right-click the running component or device and select **Logging > Edit Log Config**. If this is the first time you have used the editor, a warning is displayed.

2.  If a warning is displayed, click **Yes**. The Edit Log Config editor is displayed.

    ![Edit Log Config Editor](img/logconfigeditor.png)

    The editor shows the resource's logging configuration. Saving changes to the editor performs a live update of the resource's logging configuration.

The logging API provides fine-grained access to the various loggers. The logging API can be used to retrieve a list of the resource's loggers, change the logging level or configuration for any one specific logger, or determine the state of any one logger.

This API is available directly to the resources; one way of accessing this API is through the Python package.

Assuming that there is a reference to an instance of a component called `comp_1` associated with variable `c`, the following Python examples can be exercised:

- Use `getNamedLoggers` to get a list of the named loggers in a system.

    ```python
    >>> c.getNamedLoggers()
    ['comp_1', 'comp_1.system.PortSupplier', 'comp_1.system.PropertySet', 'comp_1.system.Resource']
    ```

- Use `setLogLevel` to change a named logger's level.

    ```python
    >>> c.setLogLevel('comp_1', 'trace')
    ```

- Use `getLogLevel` to get a named logger's level.

    ```python
    >>> c.getLogLevel('comp_1')
    5000
    ```

- Use `setLogConfig/getLogConfig` to set or get a named logger's configuration.

    ```python
    >>> c.getLogConfig('comp_1')
    'log4j.rootLogger=INFO,STDOUT\n# Direct log messages to STDOUT\nlog4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\nlog4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\nlog4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{1}:%L - %m%n\n'
    ```

- Use `resetLog` to reset a resource's loggers to whatever configuration each had on startup.

    ```python
    >>> c.resetLog()
    ```

- Use `log_level` with no arguments to get the log level for the base logger.

    ```python
    >>> c.log_level()
    5000
    ```

- Use `log_level` with an argument to set the log level for the base logger.

    ```python
    >>> c.log_level(10000)
    ```

These commands are also available for the <abbr title="See Glossary.">Domain Manager</abbr>, <abbr title="See Glossary.">Device Manager</abbr>, and <abbr title="See Glossary.">application</abbr> objects. In the case of the Domain Manager and Device Manager, they affect the specific object. In the case of an application, the logging API aggregates all components' loggers through a single interface. For example, calling `getNamedLoggers` on an application object returns a list of all the named loggers in all components in the application.

