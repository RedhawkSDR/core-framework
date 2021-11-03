# Connecting Producers and Consumers

Producers and consumers can be connected either point-to-point or through an <abbr title="See Glossary.">event channel</abbr> in the IDE. Connecting a producer directly to a consumer does not require an <abbr title="See Glossary.">application</abbr> and can be done in the <abbr title="See Glossary.">sandbox</abbr>:

```python
from ossie.utils import sb
sb.catalog()
#['structs_test', 'm_in', 'prop_changes', 'm_out','pass']
prod=sb.launch("m_out")
cons=sb.launch("m_in")
prod.connect(cons)
#True
sb.start()
```
Output:
```python
foo 1 hello
foo 1 hello
foo 1 hello
foo 1 hello
foo 1 hello
foo 1 hello
foo 1 hello
```

Connecting producers to consumers through an event channel requires an application. An application can also support point-to-point connections.

Below is a description of how to connect producers through point-to-point and through an event channel:

1.  Add producer and consumer <abbr title="See Glossary.">components</abbr>.
2.  For point-to-point messaging, connect the output `MessageEvent` port, `message_out` in this example, to the input `MessageEvent` <abbr title="See Glossary.">port</abbr> of the receive component.
3.  For messaging via an event channel, add an event channel to the <abbr title="See Glossary.">waveform</abbr> and connect to it.

    1.  In the waveform Diagram, under **Palette > Find By:**
          - Select **EventChannel** and drag it onto the diagram. The **New Event Channel** dialog is displayed.
            ![New Event Channel](../images/NewEventChannel.png)
          - Enter the event channel you want to find and click **OK**. The **EventChannel** is displayed in the diagram.
    2.  Connect the Uses (Output) `MessageEvent` port of the sending component, `message_out` in this example, to the event channel.
    3.  Connect the Uses (Output) `MessageEvent` port of the receiving component, `message_in`, to the event channel. This is the **black** output port that must be connected to the event channel.

In this example, connections are made point-to-point and through the event channel. Therefore, for every <abbr title="See Glossary.">message</abbr> sent, two messages are received.
