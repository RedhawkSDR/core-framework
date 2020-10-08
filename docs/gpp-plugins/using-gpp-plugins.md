# Customizing Resources per GPP

The GPP supports a plugin infrastructure that allows the addition of custom resource monitoring.
Plugins enable the GPP to monitor additional system resources than those monitored by default.
The following sections describe the operation of a GPP plugin and their development.

## Theory of operation

A plugin is a separate binary from GPP and is installed into `$SDRROOT/dev/devices/GPP/plugins`.
On startup, the GPP scans the directory `$SDRROOT/dev/devices/GPP/plugins` for any plugin binaries and launches each plugin with the following command line arguments:
  - instance id: identifier provided by GPP to use when passing messages
  - MessageConsumerPort reference: an IOR for a message port that consumes status messages from the plugin to the GPP

On plugin startup, the plugin registers with the GPP using the plugin's `message_out` port.

Plugins can manage any number of metrics.
Each metric is automatically registered when the plug sends a `plugin::message` to the GPP over its `message_out` port.
The `plugin::message` passes the result of its assessment of the metric it tracks.
The `plugin::message` structure contains the identifier for the plugin, the name of the metric, the measured metric, the threshold used, the reason why the metric triggered, a busy/not busy determination, and the timestamp for the decision.
If the plugin sets its message's busy flag to true, the GPP's usageState is set to BUSY.
If the plugin sets its message's busy flag to false, the flag has no direct effect on the GPP's usageState, and if no other metrics trigger their threshold, the GPP's usageState is not BUSY (IDLE or ACTIVE).
Metric thresholds can be changed by providing updates through the GPP's `plugin::set_threshold` property.
The `plugin::set_threshold` message is used to set the threshold for a specific metric on a specific plugin.

## Plugin Development

Plugins are generated using the `createGPPplugin` tool.

To create a new plugin, use the command-line tool with the plugin name as the argument:

    $ createGPPplugin pluginName

The tool creates a C++ project with autotools support and the following generated source files:
  - `pluginName.h` - The appropriate file to put header-related code in support of `pluginName.cpp`.

  - `pluginName.cpp` - The appropriate file for developers to add their own plugin behavior.

  - `main.cpp` - Housekeeping for the deployment of the plugin.

  - `struct_props.h` - Contains support classes for messaging between the plugin and the GPP. Modification of this file is not recommended.

  - `pluginName_base.h` - Support code for the plugin. Modification of this file is not recommended.

  - `pluginName_base.cpp` - Support code for the plugin. Modification of this file is not recommended.

After compiling and installing the plugin, it automatically becomes available the next time the GPP is deployed.

The plugin registers with the GPP on startup, giving the GPP a variety of registration information.
Items in the registration like the plugin description are updated by modifying the `registerPlugin` method in the file `pluginName.h`.

The `pluginName.cpp` file contains two methods that are designed to support the plugin functionality, `serviceFunction` and `updateThreshold`.

The `serviceFunction` function is the plugin's main thread function.
The processing thread is started automatically when the plugin launches.
This method is provided as a convenient place to include plugin processing, but the processing code can be placed anywhere in the plugin.
Exiting this function terminates the thread but does not affect the lifecycle of the plugin.
The lifecycle of a plugin is linked to the lifecycle of the GPP.
If a plugin is no longer tracking a metric of interest, it can terminate its thread and remain in an idle state until the GPP terminates.
If the plugin were to self-terminate (e.g.: through an exit call or an error), the GPP would detect the plugin's terminate and mark the plugin as not alive.

The plugin's `updateThreshold` method allows for runtime changes to a metric's threshold value.
The GPP's sends a `plugin::set_threshold` message to the plug when it's pluging::set_threshold property is modified.
The `plugin::set_threshold` message contains three fields: the `plugin_id`, the `metric_name`, and the new `metric_threshold_value`, where the details of the field are described in the [Run Time Metric Control](#run-time-metric-control) section.

## Plugin Messaging

The plugin's thresholds can be modified by changing GPP properties (described in the next section).
The plugin sends the GPP messages with metric updates.
To provide an update to the GPP from the plugin, create a `plugin_message_struct` instance and send it using the provided `message_out` port included in the plugin by the code generators.

The plugin message structure elements are:

 * busy (boolean): BUSY/not BUSY
 * plugin_id (string): The plugin's id (available in the _id plugin base class)
 * metric_name (string): Name for the metric. Must be unique in the plugin but does not have to be globally unique
 * metric_timestamp (CF::UTCTime): Timestamp for the message
 * metric_reason (string): Reason why the state is set (e.g.: value above threshold, value below threshold, value not equal to threshold)
 * metric_threshold_value (string): Trigger threshold for metric
 * metric_recorded_value (string): Recorded value

The `metric_recorded_value` and `metric_threshold_value` elements in the message structure are defined as strings to simplify communications between the GPP and the plugin.
Part of the plugin development process is to convert the metric value to/from strings as appropriate.
The generated plugin code includes an example of sending a status message in the plugin's `serviceFunction` method.

## Run Time Plugin Inspection

Plugin status and their metric state/status can be inspected though the GPP properties `plugin::status` and `plugin::metric_status`, respectively.
Querying the GPP's `pluging::status` property provides the state of all the registered plugins.

The `plugin::status` is a sequence of structures whose elements are:
 * id (string): The plugin's id
 * name (string): The plugin's name
 * description (string): The plugin's description
 * busy (boolean): BUSY/not BUSY
 * alive (boolean): Plugin process is alive/not alive
 * pid (CORBA::Ulong): The plugin's process id
 * metric_names (string sequence): List of metrics the plugin is evaluating

The `plugin::metric_status` structure provides detailed information about the metrics monitored by the different plugins.
While the `plugin::status` property provides an overview of the different plugins, `plugin::metric_status` provides insight into each of the metrics monitored by any running plugins.
The `plugin::metric_status` is a sequence of structures whose elements are:

 * plugin_id (string): The plugin's id (available in the _id plugin base class)
 * metric_name (string): Name for the metric. Must be unique in the plugin but does not have to be globally unique
 * busy (boolean): BUSY/not BUSY
 * metric_timestamp (CF::UTCTime): Timestamp for the last message received by the GPP from this metric
 * metric_reason (string): Most recent reason why the state was set (e.g.: value above threshold, value below threshold, value not equal to threshold)
 * metric_threshold_value (string): Current trigger threshold for metric
 * metric_recorded_value (string): Most recent recorded value

## Run Time Metric Control
Control is performed through the GPP's `plugin::set_threshold` property.
Since each plugin can manage multiple metrics, changes to the `plugin::set_threshold` property require the `plugin-id` and `metric-name` as described below:

The `plugin::set_threshold` message structure elements are:

 * plugin_id (string): The plugin's id
 * metric_name (string): The metric's name
 * metric_threshold_value (string): The new threshold value for the metric

The plugin generated code contains the callback function `updateThreshold` to respond to `plugin::set_threshold` messages from the GPP.
The plugin receives all messages for any metrics that the plugin supports through this callback.
The developer needs to evaluate the incoming messages to apply them to the correct metric.
