# Logging Structure

Java logging is based on log4j, and C++ logging is based on log4cxx, a C++ mapping of the log4j API. Python logging is based on log4py, a Python mapping between the Python loggers and log4j configuration files developed for REDHAWK. These three logging technologies were selected because they all follow the same structure, and they can all be configured using the same configuration files.

Loggers provide the ability to associate a particular logger identifier by its string name with one or more appenders (for example, standard out) and a log level (for example, TRACE).

Loggers are identified by their names, and the name selected for the logger determines the lineage for the logger. The root logger is the parent of all loggers. All loggers in the hierarchy are derived from this root logger; thus, all child loggers inherit the root logger settings unless the child logger is given its own configuration, in which case the settings inheritance tree is broken.

Logger names follow the Java naming pattern, where the logger `abc.def.ghi` inherits from the logger `abc.def`, which inherits from the logger `abc`, which in turn inherits from the root logger.

All REDHAWK-generated resources, such as <abbr title="See Glossary.">components</abbr> and <abbr title="See Glossary.">devices</abbr>, contain a logger, `_baseLog`, that has the same name as the resource (for example, `my_component_1`) and is removed by one level from the root logger. Under this parent logger, three categories or namespaces are available by default: "system", "ports", and "user". These three logger namespaces are created automatically, and additional namespaces may be created programmatically under the "user" namespace. The "system" namespace is the parent for the REDHAWK system services, such as property management, that log framework-level activity. The "ports" namespace is a logger that is parent to all component <abbr title="See Glossary.">ports</abbr>, where the specific port logger has the same name as the port (for example, `my_component_1.ports.data_in`). Finally, the "user" namespace is a root for all logging specific to the component or device implementation. This hierarchy is shown in the following image:

![Logger Hierarchy](img/LoggerHierarchy.png)

Loggers inherit their parent's settings. In other words, root logger settings are passed to all child loggers. Any change to the parent logger's settings (such as the log level) are automatically passed down to all its child loggers. This relationship is broken if a logger's settings are specifically changed. After a named logger's settings are changed, if the parent logger settings change, the changes are not propagated to the child with the altered settings.

Log messages are processed through appenders. An appender is a mechanism that provides a mapping between a message and some delivery mechanism for the message. For example, there may be file appenders and standard out appenders that write messages to a file and to a console, respectively. Appenders are managed through the log configuration file. REDHAWK systems can use a wide variety of general-purpose appenders, such as file and standard out, as well as some custom appenders, such as <abbr title="See Glossary.">event channel</abbr> appenders.

There are four main aspects to managing loggers in REDHAWK:

 - [configuring a logger's start settings](configuring-logging-capabilities.html),
 - [creating log messages](logging-within-a-resource.html),
 - [adjusting a logger during runtime](adjusting-logging-at-runtime.html), and
 - [viewing logging events](viewing-logging-events.html).
