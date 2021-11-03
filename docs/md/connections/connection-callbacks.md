# Connection Callbacks

Within a <abbr title="See Glossary.">component</abbr>, many of the Uses <abbr title="See Glossary.">port</abbr> classes in REDHAWK support notification when a connection is made or broken. The supported port types and syntax vary by language.

### C++

In C++, the connection notification mechanism is standardized for Bulk Input/Output
 (BulkIO), Burst Input/Output (BurstIO), and Messaging Uses ports in C++. Connect and disconnect callbacks are registered with the port.

The following examples assume a C++ component with a BulkIO float output port, `dataFloat_out`; however, the syntax is the same for BurstIO and Message ports.

In the component header file, declare the callbacks as private member functions. Both connect and disconnect callbacks receive a single argument, the Connection ID (a `std::string` by reference):

```cpp
void dataFloatConnected(const std::string& connectionId);
void dataFloatDisconnected(const std::string& connectionId);
```

In the component source file, implement the callback functions:

```cpp
void MyComponent_i::dataFloatConnected(const std::string& connectionId)
{
  LOG_INFO(MyComponent_i, "New connection " << connectionId << " on dataFloat_out");
}

void MyComponent_i::dataFloatDisconnected(const std::string& connectionId)
{
  LOG_INFO(MyComponent_i, "Disconnected " << connectionId << " on dataFloat_out");
}
```

Then, in the component `constructor()`, register the callback functions:

```cpp
dataFloat_out->addConnectListener(this, &MyComponent_i::dataFloatConnected);
dataFloat_out->addDisconnectListener(this, &MyComponent_i::dataFloatDisconnected);
```

`addConnectListener()` and `addDisconnectListener()` take two arguments: the target object (typically `this`) and a pointer to a member function.


> **NOTE**  
> It is not necessary to register both a connect and disconnect callback.  

### Java

In Java, BulkIO and BurstIO Uses ports support connection notification using the listener pattern. The recommended style for Java connection notification is to define the callbacks as private methods on the component class, and use an anonymous subclass of the listener interface to dispatch the notifications to your callbacks.

Message ports do not support connection notification in Java.

#### BulkIO

BulkIO connection notification uses the `bulkio.ConnectionEventListener` interface, which has `connect()` and `disconnect()` methods that are called when the port makes or breaks a connection, respectively. Both methods receive a single argument, the Connection ID (a `string`).

Add to the list of imports:

```java
import bulkio.ConnectionEventListener;
```

Implement the callbacks as methods on the component class:

```java
private void dataFloatConnected(String connectionId) {
  this._logger.info("New connection " + connectionId + " on dataFloat_out");
}
```

```java
private void dataFloatDisconnected(String connectionId) {
  this._logger.info("Disconnected " + connectionId + " on dataFloat_out");
}
```

In the `constructor()` method, register a `ConnectionEventListener` to dispatch connect and disconnect notifications to your callbacks:

```java
port_dataFloat_out.setConnectionEventListener(new ConnectionEventListener() {
    public void connect(String connectionId) {
        MyComponent.this.dataFloatConnected(connectionId);
    }
    public void disconnect(String connectionId) {
        MyComponent.this.dataFloatDisconnected(connectionId);
    }
});
```

#### BurstIO

BurstIO connection notification is similar to BulkIO but with a different listener interface, `burstio.ConnectionListener`. The connect and disconnect methods are named `portConnected()` and `portDisconnected()`, respectively.

Add to the list of imports:

```java
import burstio.ConnectionListener;
```

Implement the callbacks as methods on the component class:

```java
private void burstFloatConnected(String connectionId) {
  this._logger.info("New connection " + connectionId + " on burstFloat_out");
}
```

```java
private void burstFloatDisconnected(String connectionId) {
  this._logger.info("Disconnected " + connectionId + " on burstFloat_out");
}
```

In the `constructor()` method, register a `ConnectionListener` to dispatch connect and disconnect notifications to your callbacks:

```java
port_burstFloat_out.addConnectionListener(new ConnectionListener() {
    public void portConnected(String connectionId) {
        MyComponent.this.burstFloatConnected(connectionId);
    }
    public void portDisconnected(String connectionId) {
        MyComponent.this.burstFloatDisconnected(connectionId);
    }
});
```

### Python

In Python, only BurstIO Uses ports support connection notification.

#### BurstIO

Implement the callbacks as methods on the component class. Both methods receive a single argument, the Connection ID (a string):

```python
def burstFloatConnected(self, connectionId):
    self._log.info('New connection %s on burstFloat_out', connectionId)

def burstFloatDisconnected(self, connectionId):
    self._log.info('Disconnected %s on burstFloat_out', connectionId)
```

In the `constructor()` method, register the callbacks:

```python
self.port_burstFloat_out.addConnectListener(self.burstFloatConnected)
self.port_burstFloat_out.addDisconnectListener(self.burstFloatDisconnected)
```


> **NOTE**  
> It is not necessary to register both a connect and disconnect listener.  
