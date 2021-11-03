# Message Producer

A <abbr title="See Glossary.">message</abbr> producer may be created when creating a new <abbr title="See Glossary.">component</abbr> or editing an existing component. After creating a message producer, you must register your code to send a message from the <abbr title="See Glossary.">port</abbr>. The following procedures explain how to create a message producer and send messages.

## Creating a Message Producer

 Use the following procedure to add a message producer port to a component or device using the REDHAWK IDE.

1.  From the <abbr title="See Glossary.">Project Explorer View</abbr>, double-click the component's Software Package Descriptor (SPD) file.

    The Component Editor is displayed.

2.  From the Component Editor, select the Properties tab.

    The Component Editor Properties tab is displayed.
    ![Component Editor Properties Tab](../images/propertiestab.png)

3.  To add a `struct` <abbr title="See Glossary.">property</abbr>, click Add Struct.

    The Struct Property section of the Properties tab is displayed.
    ![Struct Property Section of Properties Tab](../images/structprop.png)

4.  In the Struct Property section, enter the name of the message produced. The ID defaults to the name you enter.
5.  From the Kind drop-down, select `message`.
6.  In the All Properties section, select the `struct` property you added. By default, a `simple` member  already exists. You can modify it and create additional members for the `struct` property. For more information about property structures, kinds, and types, refer to [Managing and Defining Properties](../../component-structure/managing-defining-properties.html).
After you modify and/or create them, the members of the structure and corresponding property information is displayed.
  ![Struct Property and Members](../images/message.png)

7.  Select the Ports tab, click Add, and in the Name field, enter a name.

8.  In the Port Details section, in the Direction drop-down, select `out <uses>`.

9.  Next to the Interface field, click Browse.

    The Select an Interface dialog is displayed.

10. From the list of interfaces displayed, select `ExtendedEvent> MessageEvent` and click OK.

    The message producer port information is displayed.
    ![Message Producer Port](../images/producerportstab.png)

11.  Regenerate the component.

After creating a message producer, you may send a message from the message producer port.

## Sending Messages

The following code examples demonstrate how to send an outgoing message in C++, Java, and Python from a component's message output port to an <abbr title="See Glossary.">event channel</abbr> or another component's message input port.

For the purposes of the following examples, assume that the structure is as follows:

  - id: `foo`
  - Contains two members:
      - name: `some_string`, type: `string`
      - name: `some_float`, type: `float`
  - The component's uses port is called `message_out`
  - The component's name is `message_producer`

 In each example, a message is created by declaring a variable of that type. Then, its state is set and the message is sent using the message port's `sendMessage()` method with the message variable as the parameter.

### C++
To generate a message, the following code can be added in the `serviceFunction()` method of the implementation file.

```cpp
foo_struct my_msg;
my_msg.some_string = "hello";
my_msg.some_float = 1.0;
this->message_out->sendMessage(my_msg);
// Send a message to a specific connection by providing a `connectionId` parameter.
// If `connectionId` does not match any connection, an `std::illegal_argument` exception is thrown.
this->message_out->sendMessage(my_msg, "connection_1");
```

### Java
To generate a message, the following code can be added in the `serviceFunction()` method.

```Java
foo_struct my_msg = new foo_struct();
my_msg.some_string.setValue("hello");
my_msg.some_float.setValue((float)1.0);
this.port_message_out.sendMessage(my_msg);
// Send a message to a specific connection by providing a `connectionId` parameter.
// If `connectionId` does not match any connection, an `IllegalArgumentException` is thrown.
this.port_message_out.sendMessage(my_msg, "connection_1");
```

### Python
To generate a message, the following code can be added in the `process()` method of the implementation file.

```python
my_msg = message_producer_base.Foo()
my_msg.some_string = "hello"
my_msg.some_float = 1.0
self.port_message_out.sendMessage(my_msg)
# Send a message to a specific connection by providing a `connectionId` parameter.
# If `connectionId` does not match any connection, a `ValueError` is raised.
self.port_message_out.sendMessage(my_msg, "connection_1")
```
