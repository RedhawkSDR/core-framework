# Custom IDL Interfaces

REDHAWK provides Front End Interfaces (FEI) and standard Core Framework (CF) interfaces (like CF::Resource) to control entities and promote interoperability. There are some use cases where you may find the need to use custom Interface Description Language (IDL) to control entities. For these use cases, you can create custom IDL projects in the [IDE](../IDE/_index.html).

Adding <abbr title="See Glossary.">ports</abbr> from either the FEI interface or a custom IDL interface to a <abbr title="See Glossary.">component</abbr> or <abbr title="See Glossary.">device</abbr> allows that entity to control other entities through CORBA. Because of the generic nature of these ports, it is not possible to create a language mapping like BulkIO, so interaction is through the standard CORBA API, a full description of which is outside the scope of this manual. However, the REDHAWK code generators will generate ports that simplify the interaction with the port. The following sections explain uses (output) ports because they are the most likely to be generated, for example, to control FEI devices.

### Connectivity Feedback

In all three supported languages, an FEI, standard CF, or custom IDL port will have all methods and attributes mapped to the port, and the port will then delegate the call to the remote connection. In REDHAWK, it is possible for a port to have no connections, one connection, or many connections. Each of these conditions can create issues for someone using a port for communications; for example, if a control request is sent out and there are no connections, then the user should be informed that the request did not go anywhere.

At the same time, not all methods are the same. Some methods push data in only one direction, some methods have a return value, and some methods have arguments that are pointers to be filled with information (out or inout arguments). When a port method is called and it is not possible for the port to make a call or for the call to be unambiguous (for example, if two connections exist and the function contains a return value), then a PortCallError is raised in the user code. The table below describes the method signature criteria met and its corresponding behavior.

##### Control method and error conditions based on connectivity
| **method return value**  | **argument direction** | **specified Connection ID** | **no connection** | **one connection** | **many connections** |
| :----------------------- | :--------------------------- | :---------------- | :----------------- | :------------------- | :-------------- |
| void | in only | None                         | ok                | ok                 | ok                   |
| Non void | in only | None                         | Error             | ok                 | Error                |
| All types | in only | valid ID                     | Error             | ok                 | ok                   |
| All types | inout and/or out | None                         | Error             | ok                 | Error                |
| All types | inout and/or out | valid ID                     | Error             | ok                 | ok                   |
| All types |   any direction    | invalid ID                   | Error             | Error              | Error                |

If a method has any kind of return value as part of its non-exception API (manifested as a non-void return value, or an out or inout argument), then an exception is raised if there is more than one connection out of the port. Furthermore, if a call is attempted with no connection in effect, an error is raised.

### Connection Selection

While the generated port class triggers an error when the desired connection is ambiguous, it also contains an API to allow the developer to select which connection should be exercised. Each method has an optional argument, `connection_id`, that allows the caller to disambiguate which connection should be exercised. The default value behavior will use the last connection made. If the `connection_id` specified does not exist, a `PortCallError` will be raised.


> **NOTE**  
> In the following sections, the same pattern used to disambiguate the connections is provided in all three supported languages.  

The following code example uses the default behavior when calling the read method of the CF::File interface.

```cpp
 CF::OctetSequence_var _data = new CF::OctetSequence();
 CF::OctetSequence_out data(_data);
 this->file_out->read(data, 10); // read 10 characters from the last connection made to the port
```

The following code example disambiguates the read call to a specific connection, `connection1`.

```cpp
 CF::OctetSequence_var _data = new CF::OctetSequence();
 CF::OctetSequence_out data(_data);
 this->file_out->read(data, 10, "connection1"); // read 10 characters from the connection called "connection1"
```

To view the connections that are available, use the following code:

```cpp
 std::vector<std::string> _connection_ids = this->file_out->getConnectionIds();
```

### Method Mapping

Method name mapping follows the pattern described in [Connection Selection](#connection-selection); namely that methods have the same name as described in the IDL with an additional argument (to be optionally exercised) that can specify which connection should be used. Attributes are mapped as functions to the CORBA objects. REDHAWK provides additional APIs to disambiguate these calls for multiple connections.

#### Reading Attributes

Reading attributes is performed by invoking the name of the attribute as a function. For example, if the port, `my_port`, contains the string attribute `greeting`, the value of `greeting` can be retrieved as follows:

```cpp
 std::string _greeting = this->my_port->greeting();
```

To retrieve the value from a specific connection, the `_get_` prefix is needed:

```cpp
 std::string _greeting = this->my_port->_get_greeting("some_connection_name");
```

#### Writing Attributes

Writing attributes in C++ and Java involves invoking the function with the appropriate argument:

```cpp
 this->my_port->greeting("hello"); // write "hello" to the attribute "greeting"
 this->my_port->greeting("hello", "some_connection_name"); // write "hello" to the attribute "greeting" over connection "some_connection_name"
```

Python requires the prefix `_set_` because it cannot be overloaded:

```python
 self.my_port._set_greeting("hello") # write "hello" to the attribute "greeting"
 self.my_port._set_greeting("hello", "some_connection_name") # write "hello" to the attribute "greeting" over connection "some_connection_name"
```
