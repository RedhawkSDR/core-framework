# Logging Within A Resource

Every resource capable of hosting custom logging (<abbr title="See Glossary.">component</abbr>, <abbr title="See Glossary.">device</abbr>, <abbr title="See Glossary.">service</abbr>) includes the class member `_baseLog`. The `_baseLog` member is a logger instance that has the same logging name as the resource instance. For example, the first instance of `comp` in a <abbr title="See Glossary.">waveform</abbr> is `comp_1`. For logging within a REDHAWK resource, `_baseLog` is the resource's "root" logger. The log4j root logger still exists and is the parent for the resource's "root" logger. The log4j root logger is referred to as the empty string.

Each logger object has a member function, `getChildLogger()`, that takes 1 required argument and a second optional argument. The first argument is the name for the child logger and the second argument is an optional namespace for this logger. If `_baseLog` belongs to component `comp_1`, calling `getChildLogger()` with the first argument set to `mylog` and no second argument, the logger name `comp_1.user.mylog` is created. Calling `getChildLogger()` with the first argument set to `mylog` and the second argument set to `some.namespace` creates the logger name `comp_1.some.namespace.mylog`.

### C++ Use

All the following logging statements work with `_baseLog`. However, to declare a new logger, use the following code in the header:
```cpp
rh_logger::LoggerPtr my_logger;
```

To instantiate the new logger, use the following code:
```cpp
my_logger = this->_baseLog->getChildLogger("my_logger");
```


> **NOTE**  
> The logger name and the logger variable name do not need to match.  

To add logging messages within your resource's code, the following macros are available. These macros use the predefined class logger as the input parameter.

  - `RH_FATAL(<logger>, message text )`
  - `RH_ERROR(<logger>, message text )`
  - `RH_WARN(<logger>, message text )`
  - `RH_INFO(<logger>, message text )`
  - `RH_DEBUG(<logger>, message text )`
  - `RH_TRACE(<logger>, message text )`

where `<logger>` is the logger instance that should publish the message.

The following example adds `DEBUG`-level logging messages to the logger `my_logger`.

```cpp
RH_DEBUG(this->my_logger, "example log message");
```

The message text can be combined with stream operations, so the variable `my_variable` can be added to the logging message:

```cpp
RH_DEBUG(this->my_logger, "The variable my_variable has the value: "<<my_variable);
```

### Java Use

All of the following logging statements work with `_baseLog`. However, to declare a new logger, use the following code:

```java
private RHLogger my_logger;
```

To instantiate the new logger, use the following code:
```java
my_logger = this._baseLog.getChildLogger("my_logger");
```


> **NOTE**  
> The logger name and the logger variable name do not need to match.  

The following example adds `DEBUG`-level logging messages to the logger `my_logger`.

```java
void someMethod() {
  this.my_logger.debug("example log message");
}
```

Log4j supports the following severity levels for logging.

```java
_baseLog.fatal(...)
_baseLog.warn(...)
_baseLog.error(...)
_baseLog.info(...)
_baseLog.debug(...)
_baseLog.trace(...)
```

It also supports programmatically changing the severity level of the logger object.

```java

_baseLog.setLevel(Level.WARN)
```

### Python Use

All the following logging statements work with `_baseLog`. However, to create a new logger, use the following code:

```python
self.my_logger = self._baseLog.getChildLogger("my_logger")
```

The following example adds `DEBUG`-level logging messages to the logger `my_logger`.

```python
self.my_logger.debug("example log message")
```

REDHAWK has extended the Python logging support to include the trace method functionality.

```python
self._baseLog.fatal(...)
self._baseLog.warn(...)
self._baseLog.error(...)
self._baseLog.info(...)
self._baseLog.debug(...)
self._baseLog.trace(...)
```
As with the other logging capabilities, you can programatically change the logging level.

```python
self._baseLog.setLevel(logging.WARN)
```
