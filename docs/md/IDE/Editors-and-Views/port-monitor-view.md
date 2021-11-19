# Port Monitor View

The Port Monitor view enables you to monitor the amount of data flowing out of or into a particular <abbr title="See Glossary.">port</abbr>. These statistics are helpful when debugging and can help identify which <abbr title="See Glossary.">component</abbr> is slowing down or dropping information during data processing.

To open the Port Monitor view, right-click the port of a started component and select **Monitor Ports** from the context menu:
![Port Context Menu](img/MonitorPortMenu.png)

The Port Monitor view is opened:
![Port Monitor View](img/portMonitorView.png)

The view displays the following information:

  - **Name**: The name of the port or port connection.
  - **Elements/sec**: The rate of elements transferred in the pushPacket data call.
  - **Bytes/sec**: Bytes transferred per second.
  - **Calls/sec**: Number of push calls per second to the port.
  - **Stream ID(s)**: List of all active Stream IDs.
  - **Avg. Queue Depth**: For components that queue data before processing/sending, the average queue depth measured as a percentage. If a port does not queue data, this value is set to zero.
  - **Time**: The elapsed time, in seconds, since the last packet was transferred via a push packet call.

The following actions are available in the Port Monitor view.

1.  To Configure the Port Monitor view:

      - From the View menu, select Configure.

    The Port Monitor Configuration dialog box is displayed.
![Port Monitor Configuration Dialog](img/configuration.png)

    The following options can be configured:

      - **Refresh Interval (sec)**: The time interval between fetching the port statistics.
      - **Column Configuration**: Specifies which columns are visible.

2.  To force a refresh of any connection or port:

      - Right-click the item.
      - Select **Refresh**.

3.  To stop monitoring:

      - Right-click the item.
      - Select **Stop Monitoring**.

### Port Monitoring in a Diagram
If a diagram is open while monitoring ports, the diagram display changes the colors of connections and provides (in) ports to reflect the statistics. A green connection indicates that data is flowing. A yellow connection indicates it has been more than 1 second since data was pushed over the connection, which may indicate a data flow issue.

For provides (in) ports, a green port indicates the port's queue has plenty of space left. After the queue depth reaches 60 percent, the port color changes to yellow, and the port color slowly changes to red as the queue depth approaches 100 percent. Additionally, if there is a queue flush, the port remains red for 30 seconds after that queue flush.

To configure the threshold values that trigger color changes for port statistics:

   1. Select **Window > Preferences**.
   2. Select the **REDHAWK > Diagrams > Port Statistics page**.
   3. Change the values.
   4. Click **Apply and Close**.
