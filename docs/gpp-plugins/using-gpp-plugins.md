# Customizing Resources per GPP

The GPP supports a plugin infrastructure that allows the addition of custom resource monitoring.
Plugins enable the extension of the GPP to monitor additional system attributes than those monitored by the GPP by default.
Tooling is included to help develop plugins.
At runtime, the behavior of plugins can be monitored and altered as conditions change.
The following sections describe how to develop and use GPP plugins.

## Theory of operatoin

A plugin is a separate binary that is forked by the GPP.
The plugin is installed onto `$SDRROOT/dev/devices/GPP/plugins`.
On startup, the GPP scans the directory `$SDRROOT/dev/devices/GPP/plugins` for any installed plugins.
Any plugin found there is launched.
As part of the deployment process, the GPP provides the plugin an instance id and a reference to a GPP MessageConsumer port.
On startup, the plugin registers with the GPP throught the GPP's MessageConsumer port using the plugin's `message_out` messaging port.
Plugins can manage any number of metrics, where each metric is identified by its name.
Individual metrics are registered implicitly by sending out a `plugin_message` with metric status over the `message_out` port.

## Plugin Development

Plugins are generated using the `createGPPplugin` tool.

To create a new plugin, use the command-line tool with the plugin name as the argument:

    $ createGPPplugin pluginName

The tool creates a C++ project; along with the Automake files, the additional generated files are:
  - `pluginName.h` - The appropriate file to put header-related code in support of `pluginName.cpp`.

  - `pluginName.cpp` - The appropriate file for developers to add their own plugin behavior.

  - `main.cpp` - Housekeeping for the deployment of the plugin.

  - `struct_props.h` - Contains support classes for messaging between the plugin and the GPP. Modification of this file is not recommended.

  - `pluginName_base.h` - Support code for the plugin. Modification of this file is not recommended.

  - `pluginName_base.cpp` - Support code for the plugin. Modification of this file is not recommended.

After compiling and installing the plugin, it automatically becomes available the next time the GPP is deployed.
The GPP scans the plugin installation location (`$SDRROOT/dev/devices/GPP/plugins`) to launch new plugins.

The `pluginName.cpp` file contains two methods that are designed to support the plugin functionality, `serviceFunction` and `updateThreshold`.

The `serviceFunction` function is the plugin's main thread function.
Exiting this function terminates the thread.
The processing thread is started automatically as part of the plugin deployment process.
This method should be used to evaluate the metric(s) the pluging is monitoring.

The plugin's `updateThreshold` method is a callback that is invoked anytime the threshold for this plugin is modified through the GPP.
This callback method exists to provide the user with the ability to modify the behavior of the plugin at runtime.
The received message structure contains three fields: the `plugin_id`, the `metric_name`, and the new `metric_threshold_value`.

The plugin registers with the GPP on startup, giving the GPP a variety of registration information.
To populate the correct information for the plugin in the GPP, modify the `registerPlugin` method in the file `pluginName.h`.

## Plugin Messaging

The plugin's thresholds can be modified by changing GPP properties (described in the next section).
The GPP sends a message to the plugin with the new threshold value.

The plugin sends the GPP messages with metric updates.
To provide an update to the GPP from the plugin, create a `plugin_message_struct` instance and send it using the `message_out` port included in the plugin by the code generators.
The generated plugin code includes the callback function and an example of sending a status message in the plugin's `serviceFunction` method.

## Plugin Runtime Inspection

Plugins can be inspected through the GPP's properties.
The GPP contains the `plugin::update_metric` property; setting this property will trigger a message to the appropriate plugin.
Each plugin can control multiple metrics, so messages regarding changes in a threshold value need to be addressed with the plugin id and the metric in question.

To scan a GPP's running plugins, use the `plugin::status` property.
This property lists all the installed plugins, their unique id (given by the GPP at deployment time), their current state (true for `BUSY`, false otherwise), and the list of metrics the plugin manages and their corresponding logistical data.

To view a specific metric, use the `plugin::metric_status` structure.
In this property, each plugin's state is described: its parent plugin id, its current state (`BUSY` or not `BUSY`), the current threshold and measured values, and the timestamp for the last update.
