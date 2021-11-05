# Event Viewer View

The Event Viewer view is used to listen to any <abbr title="See Glossary.">event channel</abbr> (for example, the default **IDM_Channel** or **ODM_Channel** for a <abbr title="See Glossary.">domain</abbr>) as well as <abbr title="See Glossary.">message</abbr> events emitted by **MessageEvent** <abbr title="See Glossary.">ports</abbr>. It also provides a means of filtering, sorting, and exporting the event traffic collected.

The following figure displays the **Event Viewer view** for the Outgoing Domain Management (ODM) channel.
    ![Event Viewer View for ODM Channel](img/eventViewer.png)

To listen to a channel from the <abbr title="See Glossary.">REDHAWK Explorer view</abbr>:

1.  Expand the domain.

2.  Expand the **Event Channels** folder.

3.  Right-click the desired channel, and select **Listen to Event Channel**.

    The Event Viewer view for the selected channel is displayed and new events are added to the table.

To listen to a channel from the <abbr title="See Glossary.">CORBA Name Browser</abbr> view:

1.  Expand the <abbr title="See Glossary.">Naming Service</abbr>.

2.  Expand the domain.

3.  Right-click the desired channel, and select **Listen to Event Channel**.

    The **Event Viewer view** for the selected channel is displayed, and new events are added to the table.

To listen to message events emitted by a uses (out) port from the Diagram tab:

1.  Right-click the port and select **Listen to Message Events**.

    The **Event Viewer view** for the selected port is displayed and the message events are added to the table.

The controls in the upper right of the Event Viewer view provide the following functionality:

  - To view details for events, click the **See Details** icon. The Properties tab is displayed with the all of the details for the selected event.
  ![Properties tab with Event Details for the ODM Channel](img/eventviewer2.png)

  - To stop listening to a channel, click the **Disconnect** icon.
  - To clear the logs, click the **Clear** icon.
  - To ensure the view does not scroll automatically to the top as new events are received, click the **Scroll Lock** icon.

The controls at the bottom of the Event Viewer view enable the user to filter and search the event log.

To filter the event log:

  - In the **Filter** field, enter the filter text for the event log.
  - To remove the filter, click the **Clear** icon.
  - To enable regular expressions in the **Filter** field, check the **RE** checkbox.

To search the event log:

  - In the **Search** field, enter the text for which you want to search.
  - To clear the search highlights from the event log, click the **Clear** icon.
  - To enable regular expressions in the **Search** field, check the **RE** checkbox.
