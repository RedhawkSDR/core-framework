# Customizing Resources per GPP

The GPP supports a plugin infrastructure that allows the addition of custom resource management.
On startup, the GPP scans the directory `$SDRROOT/dev/devices/GPP/plugins` for any installed plugins.
A plugin is a separate binary that is forked by the GPP.
As part of the deployment process, the GPP provides the plugin a plugin instance id and a reference to the GPP.
As part of its startup sequence, the connects to the GPP through the `message_out` messaging port object in the plugin.
The plugin also registers with the GPP, providing it with its name and a short description of the plugin.
To populate the correct information for the plugin, modify the `registerPlugin` method in the file `pluginName.h`.

Plugins can manage an arbitrary number of metrics.
Each metric is identified by its name.
Individual metrics are registered implictly by sending out a `plugin_message` over the `message_out` port.


Plugins are generated using the `createGPPplugin` tool.

To create a new plugin, use the command-line tool with the plugin name as the argument:

    $ createGPPplugin pluginName

The tool creates a C++ project; along with the automake files, the additional generated files are:
  - `pluginName.h` - The appropriate place to put header-related code in support of `pluginName.cpp`.

  - `pluginName.cpp` - The appropriate place for developers to add their own plugin behavior.

  - `main.cpp` - Housekeeping for the deployment of the plugin.

  - `struct_props.h` - Contains support classes for messaging between the plugin and the GPP. Modification of this file is not recommended.

  - `pluginName_base.h` - Support code for the plugin. Modification of this file is not recommended.

  - `pluginName_base.cpp` - Support code for the plugin. Modification of this file is not recommended.

The `pluginName.cpp` file contains two methods that are designed to support the plugin functionality, `run` and `updateThreshold`

The `run` function is the plugin's main thread function.
Exiting this function will terminate the thread.
This thread is started as part of the plugin deployment process, where as soon as the plugin process is deployed by the GPP, the processing thread is started.

The `updateThreshold` method is a callback function that is invoked anytime the threshold for this plugin is modified on the GPP.
The message structure contains three fields, the `plugin_id`, the `metric_name`, and the new `metric_threshold_value`.
This callback function exists to provide the user with the ability to modify the behavior of the plugin at runtime.

To provide an update to the GPP from the plugin, create a `plugin_messae_struct` instance and send it using the `message_out` port included in the plugin by the code generators.
The generated plugin code includes the callback function and an example of sending a status message in the plugin's run method.

This runtime support is through the GPP's properties.
The GPP contains the `plugin::update_metric` property, where setting this property will trigger a message to the appropriate plugin.
Each plugin can control multiple metrics, so messages regarding changes in a threshold value need to be addressed with the plugin id and the metric in question.

To scan a GPP's running plugins, use the `plugin::status` property.
This property lists all the installed plugins, their unique id (given by the GPP at deployment time), an indication as to whether or not the plugin is in a BUSY or not BUSY state, and the list of metrics the plugin supports along with other logistical data.

To view a specific metric, use the `plugin::metric_status` structure.
In this property, each plugin's state is described, including its parent plugin id, its current state (BUSY or not BUSY), the current threshold value and the measure value, its last state change along with a description of why the state changed and the timestamp when the change occurred.

By compiling and installing the plugin, it automatically becomes available next time the GPP is deployed.
The GPP scans the plugin installation location ($SDRROOT/dev/devices/GPP/plugins) upon startup.
