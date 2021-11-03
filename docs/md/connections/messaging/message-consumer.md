# Message Consumer

A <abbr title="See Glossary.">message</abbr> consumer may be created when creating a new <abbr title="See Glossary.">component</abbr> or editing an existing component. After creating a message consumer, you must register your code to receive a message from the <abbr title="See Glossary.">port</abbr>. The following procedures explain how to create a message consumer and register code to process messages.

## Creating a Message Consumer

 Use the following procedure to add a message consumer port to a component or device using the REDHAWK IDE.

1.  From the <abbr title="See Glossary.">Project Explorer View</abbr>, double-click the component's Software Package Descriptor (SPD) file.

    The Component Editor is displayed.

2.  From the Component Editor, select the Properties tab.

    The Component Editor Properties tab is displayed.
    ![Component Editor Properties Tab](../images/propertiestab.png)

3.  To add a `struct` <abbr title="See Glossary.">property</abbr>, click Add Struct.

    The Struct Property section of the Properties tab is displayed.
    ![Struct Property Section of Properties Tab](../images/structprop.png)

4.  In the Struct Property section, enter the name of the message you want the component to consume. The ID defaults to the name you enter.
5.  From the Kind drop-down, select `message`.
6.  In the All Properties section, select the `struct` property you added. By default, a `simple` member  already exists. You can modify it and create additional members for the `struct` property. For more information about property structures, kinds, and types, refer to [Managing and Defining Properties](../../component-structure/managing-defining-properties.html).
After you modify and/or create them, the members of the structure and corresponding property information is displayed.
  ![Struct Property and Members](../images/message.png)

7.  Select the Ports tab, click Add, and in the Name field, enter a name.

8.  In the Port Details section, in the Direction drop-down, select `bi-dir <uses/provides>`.

9.  Next to the Interface field, click Browse.

    The Select an Interface dialog is displayed.

10. From the list of interfaces displayed, select `ExtendedEvent> MessageEvent` and click OK.

    The message consumer port information is displayed.
    ![Message Consumer Port](../images/consumerportstab.png)

11.  Regenerate the component.

A bidirectional port is required to support connections to an <abbr title="See Glossary.">event channel</abbr> and a message supplier's output (uses) port.  In point-to-point connections, the port behaves like a provides port.  In connections with an event channel, the consumer behaves like a uses port.

After creating a message consumer, you must register your code to receive a message from the message consumer port.

## Registering for Messages

The following examples explain how to register code in C++, Java, and Python to process an incoming message.

For the purposes of the following examples, assume that the structure is as follows:

  - id: `foo`
  - Contains two members:
      - name: `some_string`, type: `string`
      - name: `some_float`, type: `float`
  - The component's uses/provides port is called `message_in`
  - The component's callback function for this message is `messageReceived()`
  - The component's name is `message_consumer`

If a connection exists between this component and either a message producer or an event channel, the following code examples process an incoming message.


> **NOTE**  
> Any message that comes in with the property ID `foo` will trigger the callback function `messageReceived()`.  

### C++

Given the asynchronous nature of events, a callback pattern was selected for the consumer.

In the component header file, declare the following callback function:

```cpp
void messageReceived(const std::string &id, const foo_struct &msg);
```

In the component source file, implement the callback function:

```cpp
void message_consumer_i::messageReceived(const std::string &id, const foo_struct &msg) {
  LOG_INFO(message_consume_i, id<<" "<<msg.some_float<<" "<<msg.some_string);
}
```

In the `constructor()` method, register the callback function:

```cpp
message_in->registerMessage("foo", this, &message_consumer_i::messageReceived);
```

### Java

Java callbacks use the `org.ossie.events.MessageListener` interface, which has a single `messageReceived()` method. The recommended style for Java messaging is to define the callback as a private method on the component class, and use an anonymous subclass of `MessageListener` to dispatch the message to your callback.

Add to the list of imports:

```Java
import org.ossie.events.MessageListener;
```

Implement the callback as a method on the component class:

```Java
private void messageReceived(String id, foo_struct msg) {
  logger.info(id + " " + msg.some_float.getValue() + " " + msg.some_string.getValue());
}
```

In the `constructor()` method, register a `MessageListener` for the message to dispatch the message to your callback:

```Java
this.port_message_in.registerMessage("foo", foo_struct.class, new MessageListener<foo_struct>() {
  public void messageReceived(String messageId, foo_struct messageData) {
    message_consumer.this.messageReceived(messageId, messageData);
  }
});
```

### Python

In the `constructor()` method, register the expected message with a callback method:

```python
self.port_message_in.registerMessage("foo", message_consumer_base.Foo, self.messageReceived)
```

In the class, define the callback method. In this example, the method is called `messageReceived()`:

```python
def messageReceived(self, msgId, msgData):
  self._log.info("messageReceived *************************")
  self._log.info("messageReceived msgId " + str(msgId))
  self._log.info("messageReceived msgData " + str(msgData))
```
