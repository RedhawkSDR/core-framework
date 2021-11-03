# Device Manager Service Configuration File

Each REDHAWK <abbr title="See Glossary.">Device Manager</abbr> <abbr title="See Glossary.">service</abbr> is controlled by a file in the `/etc/redhawk/nodes.d` directory. The AdminService provides the initial values for the configuration parameters of a service. Any values in the `/etc/redhawk/init.d/node.defaults` file override the initial configuration. Finally, the values in the INI file override any configuration (defined internally or specified in the `/etc/redhawk/init.d/node.defaults` file).

Although there are no rules on partitioning nodes for a REDHAWK system, it is recommended that you do not define more than one <abbr title="See Glossary.">`GPP`</abbr> per computing host. To define multiple <abbr title="See Glossary.">nodes</abbr> for a computing host, create a new configuration file for each node.

The Device Manager can be configured to start after the <abbr title="See Glossary.">Domain Manager</abbr> has started up, or it can start up at the same time as the Domain Manager, and it will wait for the <abbr title="See Glossary.">domain</abbr> to be available and register its [Devices](../../../devices/_index.html) and [Services](../../../services/_index.html). If many <abbr title="See Glossary.">devices</abbr> or services need to start, it is recommended that you add a custom script to verify that the Device Manager has started all devices and services and registered them with the Domain Manager (refer to the `start_post_script` parameter).

[rhadmin](../../../appendices/adminservice/rhadmin.html) can generate an example Device Manager configuration file with the complete set of parameters that can be used to the control the setup and execution of a REDHAWK Device Manager service. To generate a generic Device Manager configuration, enter the following command.
```sh
cd /etc/redhawk/nodes.d
rhadmin config node > node.ini
```
To generate a node configuration from an existing Device Manager project, enter the following command.
```sh
cd /etc/redhawk/nodes.d
rhadmin config node <path/to/node>/DeviceManager.dcd.xml <optional DomainName> > node.ini
```

## Device Manager Configuration Parameters

This section describes all available configuration parameters for the Device Manager service.

> **NOTE**  
Parameter names are case sensitive.  
The following are the valid values for boolean configuration parameters. If no value is present, the feature is disabled.  
True: 1, true, True  
False: 0, false, False


parameter: `DOMAIN_NAME`  
required: Yes  
default value: None  
format: Name with no spaces or periods (for example, `REDHAWK_DEV`)  
description: The domain name to associate with this Device Manager.

parameter: `NODE_NAME`  
required: Yes  
default value: None  
format: Name with no spaces or periods  
description: The name of the node to launch with this Device Manager. Must be a valid directory name in `$SDRROOT/dev/nodes`.

parameter: `DCD_FILE`  
required: No  
default: `/nodes/$NODE_NAME/DeviceManager.dcd.xml`  
format: `$SDRROOT/dev` relative path to a Device Configuration Descriptor (DCD) file  
description: The path to the DCD file (`DeviceManager.dcd.xml` file) describing the Device Manager.

parameter: `SDRCACHE`  
required: No  
default value: None  
description: The absolute path to use as cache directory for Device Manager and its devices. If no value is specified, the system defaults to creating a directory in `$SDRROOT/dev`.

parameter: `CLIENT_WAIT_TIME`  
required: No  
default value: `10000` (milliseconds)  
format: number in milliseconds  
description: The wait time, in milliseconds, before the Device Manager times out waiting for a response when making remote calls.

parameter: `USELOGCFG`  
required: No  
default value: None  
format: `True` : enables option, blank : disables option  
description: Enables the use of `$OSSIEHOME/lib/libsossielogcfg.so` to resolve the `LOGGING_CONFIG_URI` command line argument.

parameter: `LOGGING_CONFIG_URI`  
required: No  
default value: `defaults.logging.properties`  
format: Absolute path to a file, `file://<path>` URI or `sca://<path>` URI  
description: The logging configuration file to be used by the Device Manager. Simple file names will be resolved to files in `/etc/redhawk/logging` directory. All others will be resolved as an absolute path or URI to a logging properties file.

parameter: `DEBUG_LEVEL`  
required: No  
default value: `INFO`  
values: `FATAL`, `ERROR`, `WARN`, `INFO`, `DEBUG`, `TRACE`  
description: The Device Manager's logging level at startup.

parameter: `SDRROOT`  
required: No  
default value: `$SDRROOT`  
format: Standard shell path environment variable  
description: The path to use as the `SDRROOT` for this Device Manager.

parameter: `OSSIEHOME`  
required: No  
default: `$OSSIEHOME`  
format: Standard shell path environment variable  
description: The absolute path to use as the `OSSIEHOME` for this Device Manager.

parameter: `LD_LIBRARY_PATH`  
required: No  
default value: `$LD_LIBRARY_PATH`  
format: Standard shell path environment variable  
description: The path for link loader to resolve shared object files; overrides the `LD_LIBRARY_PATH` environment variable.

parameter: `PYTHONPATH`  
required: No  
default value: `$PYTHONPATH`  
format: Standard shell path environment variable.  
description: The path used by Python interpreter to load modules; overrides the `PYTHONPATH` environment variable.

parameter: `JAVA_HOME`  
required: No  
default value: `$JAVA_HOME`  
format: Standard shell path environment variable  
description: The home directory used by the Java installation when launching devices and services.

parameter: `PATH`  
required: No  
default value: `$PATH`  
format: Standard shell path environment variable  
description: The search path to use when launching devices and services.

parameter: `ORB_CFG`  
required: No  
default value: None  
format: Standard shell environment variable.  
description: Sets the `OMNIORB_CONFIG` variable before running the process. For more information, refer to the omniORB documentation.

parameter: `ORB_INITREF`  
required: No  
default value: None  
description: Used as omniORB ORBInitRef command line argument when starting process. For more information, refer to the omniORB documentation.

parameter: `ORB_ENDPOINT`  
required: No  
default value: None  
description: Used as omniORB ORBendPoint command line argument when starting process. For more information, refer to the omniORB documentation.

parameter: `enable`  
required: No  
default value: `True`  
format: `True`, `False`, or a string to be matched against `conditional_config`  
description: Specifies if the process may be started. `True` or `False` will enable or disable the process. Refer to `conditional_config` for information about how a string value gets evaluated.

parameter: `conditional_config`  
required: No  
default value: `/etc/redhawk/rh.cond.cfg`  
description: Allows conditional startup of processes based on the `enable` parameter and the contents of this conditional config. If the value `enable` is a string, the process will start only if there is a line in the `conditional_config` file that has that exact content; otherwise, the process is skipped. For example, `enable="type=primary"` causes the `conditional_config` file to be examined for a line equal to `type=primary` when starting a process on the host. If there is no `type=primary` line in the file, the process is skipped.

parameter: `priority`  
required: No  
default value: `400`  
description: The relative priority of the Device Manager in the group of processes to start for this domain. Lower values will be started earlier. For example, priority 100 will be started before priority 400.

parameter: `autostart`  
required: No  
default value: `True`  
description: Specifies whether to automatically start this process when the AdminService starts, if `enable` is True.

parameter: `waitforprevious`  
required: No  
default value: `45`  
description: The number of seconds to wait for the previous higher priority process to start before trying to start this process.

parameter: `failafterwait`  
required: No  
default value: `True`  
description: Specifies whether to abort starting this domain if `waitforprevious` has expired and the previous process has not been declared started yet. This is useful to make sure the Domain Manager is started before launching the Device Manager.

parameter: `started_status_script`  
required: No  
default value: None  
format: Absolute path of a file  
description: Specifies an optional bash script used to determine if the Device Manager started properly. A script exit value of `0` indicates the Device Manager started successfully.  

parameter: `status_script`  
required: No  
default value: None  
format: Absolute path of a file  
description: Specifies an optional bash script to check the status for the Device Manager. A script exit value of `0` indicates the Device Manager is alive.  

parameter: `query_script`  
required: No  
default value: None  
format: Absolute path of a file  
description: Specifies an optional bash script used to get a detailed status output for the Device Manager. This is useful to return the status of each device or service for this Device Manager.

parameter: `environment`  
required: No  
default value: None  
format: A list of key/value pairs in the form `key="value",key2="value2"`  
description: Specifies whether to override existing environment variables or set new ones to be used when starting the Device Manager.

parameter: `user`  
required: No  
default value: `redhawk`  
description: Executes the process with User ID.

parameter: `group`  
required: No  
default value: `redhawk`  
description: Executes the process with Group ID.

parameter: `umask`  
required: No  
default value: None  
description: The umask for the process.

parameter: `nicelevel`  
required: No  
default value: None  
description: Specifies to run the process using `nice` with this niceness level.

parameter: `affinity`  
required: No  
default value: None  
description: Enables `numactl` processing. Any valid <abbr title="See Glossary.">NUMA</abbr> control directives will be passed on command line when starting the process.  For more information, refer to the `numactl` documentation.

parameter: `corefiles`  
required: No  
default value: None  
description: The maximum size of core files created. This value is passed to the `ulimit` command using the `-c` flag when starting the process.

parameter: `ulimit`  
required: No  
default value: user's environment  
description: This value is passed directly to the `ulimit` command when starting the process. For more information, refer to the `ulimit` documentation.

parameter: `directory`  
required: No  
default value: `$SDRROOT`  
description: Specifies to change the directory to `directory` before running the process.

parameter: `run_detached`  
required: No  
default value: `True`  
description: Specifies to run the Device Manager as a daemon, not a child of the AdminService process.

parameter: `logfile_directory`  
required: No  
default value: `/var/log/redhawk/device-mgr`  
description: The absolute path to the logging directory.

parameter: `stdout_logfile`  
required: No  
default value: `<domain name>.<node name>.stdout.log`  
description: The name of a file that captures the stdout from the process. If not specified, the default value list above is used.

parameter: `stderr_logfile`  
required: No  
default value: `<domain name>.<node name>.stderr.log`  
description: If `redirect_stderr` is False, the name of a file that captures the stderr from the process. If not specified, the default value list above is used.

parameter: `redirect_stderr`  
required: No  
default value: `True`  
description: Specifies to write stdout and stderr to the same file.

parameter: `start_pre_script`  
required: No  
default value: None  
format: Absolute path of a file or absolute path of a directory of files  
description: The bash script or directory of bash scripts to run before the process is started.

parameter: `start_post_script`  
required: No  
default value: None  
format: Absolute path of a file or absolute path of a directory of files  
description: The bash script or directory of bash scripts to run after the process is started.

parameter: `stop_pre_script`  
required: No  
default value: None  
format: Absolute path of a file or absolute path of a directory of files  
description: The bash script or directory of bash scripts to run before the process is stopped.

parameter: `stop_post_script`  
required: No  
default value: None  
format: Absolute path of a file or absolute path of a directory of files  
description: The bash script or directory of bash scripts to run after the process is stopped.
