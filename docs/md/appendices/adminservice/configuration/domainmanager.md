# Domain Manager Service Configuration File

Each REDHAWK <abbr title="See Glossary.">Domain Manager</abbr> <abbr title="See Glossary.">service</abbr> is controlled by a file in the `/etc/redhawk/domains.d` directory. The AdminService provides the initial values for the configuration parameters of a service. Any values in the `/etc/redhawk/init.d/domain.defaults` file override the initial configuration. Finally, the values in the INI file override any configuration (defined internally or specified in the `/etc/redhawk/init.d/domain.defaults` file).

[rhadmin](../../../appendices/adminservice/rhadmin.html) can generate an example Domain Manager configuration file with the complete set of parameters that can be used to the control the setup and execution of a REDHAWK Domain Manager service. To generate a generic Domain Manager configuration, enter the following command.
```sh
cd /etc/redhawk/domains.d
rhadmin config domain > domain.ini
```
To generate a <abbr title="See Glossary.">domain</abbr> configuration from an existing Domain Manager project, enter the following command.
```sh
cd /etc/redhawk/domains.d
rhadmin config domain <path/to/domain>/DomainManager.dmd.xml <optional DomainName> > domain.ini
```

## Domain Manager Service Configuration Parameters

This section describes all available configuration parameters for the Domain Manager service.

> **NOTE**  
> Parameter names are case sensitive.  
> The following are the valid values for boolean configuration parameters. If no value is present, the feature is disabled.  
> True: 1, true, True  
> False: 0, false, False


parameter: `DOMAIN_NAME`  
required: Yes  
default value: None  
format: Name with no spaces or periods (for example, `REDHAWK_DEV`)  
description: The domain name to be assigned to the Domain Manager process.

parameter: `FORCE_REBIND`  
required: No  
default value: `False` (no rebind)  
format: `False` : no rebind, `True` : rebind  
description: If the naming context already exists for the `DOMAIN_NAME`, rebinds the Domain Manager to an existing naming context in the CORBA  <abbr title="See Glossary.">Naming Service</abbr>

parameter: `PERSISTENCE`  
required: No  
default value: False (no persistence)  
format: `True` : domain persistence enabled, `False` : disabled  
description: Enables persistence for the domain. Requires REDHAWK to be compiled with persistence.

parameter: `DB_FILE`  
required: No  
default value: None  
description: The absolute path to the file to use for domain persistence (for example, `/data/mysqlite.db`). Requires REDHAWK to be compiled with persistence enabled.

parameter: `BINDAPPS`  
required: No  
default value: blank  
format: `True` : enables option, blank: disables option  
description: Specifies that all running <abbr title="See Glossary.">applications</abbr> and <abbr title="See Glossary.">components</abbr> bind to the Domain Manager process instead of the Naming Service. This assists with high frequency deployments of <abbr title="See Glossary.">waveforms</abbr> and components.

parameter: `USELOGCFG`  
required: No  
default value: None  
format: `True` : enables option, blank : disables option  
description: Enables the use of `$OSSIEHOME/lib/libsossielogcfg.so` to resolve the `LOGGING_CONFIG_URI` command line argument.

parameter: `LOGGING_CONFIG_URI`  
required: No  
default value: `defaults.logging.properties`  
format: Absolute path to a file, `file://<path>` URI or `sca://<path>` URI  
description: The logging configuration file to be used by the Domain Manager. Simple file names will be resolved to files in `/etc/redhawk/logging` directory. All others will be resolved as an absolute path or URI to a logging properties file.

parameter: `DEBUG_LEVEL`  
required: No  
default value: `INFO`  
values: `FATAL`, `ERROR`, `WARN`, `INFO`, `DEBUG`, `TRACE`  
description: The Domain Manager's logging level at startup.

parameter: `SDRROOT`  
required: No  
default value: `$SDRROOT`  
format: Standard shell path environment variable
description: The absolute path to use as the `SDRROOT` for this Domain Manager.

parameter: `OSSIEHOME`  
required: No  
default: `$OSSIEHOME`  
format: Standard shell path environment variable  
description: The absolute path to use as the `OSSIEHOME` for this Domain Manager.

parameter: `LD_LIBRARY_PATH`  
required: No  
default value: `$LD_LIBRARY_PATH`  
format: Standard shell path environment variable  
description: The path for link loader to resolve shared object files; overrides the `LD_LIBRARY_PATH` environment variable.

parameter: `PYTHONPATH`  
required: No  
default value: `$PYTHONPATH`  
format: Standard shell path environment variable  
description: The path used by Python interpreter to load modules; overrides the `PYTHONPATH` environment variable.

parameter: `ORB_CFG`  
required: No  
default value: None  
format: Standard shell environment variable  
description: Sets the `OMNIORB_CONFIG` variable before running the process. For more information, refer to the omniORB documentation.

parameter: `ORB_INITREF`  
required: No  
default value: None  
description: Used as omniORB `ORBInitRef` command line argument when starting the process. For more information, refer to the omniORB documentation.

parameter: `ORB_ENDPOINT`  
required: No  
default value: None  
description: Used as omniORB `ORBendPoint` command line argument when starting the process. For more information, refer to the omniORB documentation.

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
default value: `100`  
description: The priority of this domain relative to other configured domains. Controls which domain gets started first on the system. Lower values will be started earlier. For example, priority 10 will be started before priority 100.

parameter: `autostart`  
required: No  
default value: `True`  
description: Specifies whether to automatically start this process when the AdminService starts, if `enable` is True.

parameter: `started_status_script`  
required: No  
default value: None  
format: Absolute path of a file  
description: Specifies an optional bash script used to determine if the Domain Manager started properly. A script exit value of `0` indicates the Domain Manager started successfully.

parameter: `status_script`  
required: No  
default value: None  
format: Absolute path of a file  
description: Specifies an optional bash script to check the status for the Domain Manager. A script exit value of `0` indicates the Domain Manager is alive.

parameter: `query_script`  
required: No  
default value: None  
format: Absolute path of a file  
description: Specifies an optional bash script used to get a detailed status output for the Domain Manager.

parameter: `environment`  
required: No  
default value: None  
format: A list of key/value pairs in the form `key="value",key2="value2"`  
description: Specifies whether to override existing environment variables or set new ones to be used when starting the Domain Manager.

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
description: Enables `numactl` processing. Any valid <abbr title="See Glossary.">NUMA</abbr> control directives will be passed on command line when starting the process. For more information, refer to the `numactl` documentation.

parameter: `corefiles`  
required: No  
default value: None  
description: The maximum size of core files created. This value is passed to the `ulimit` command using the `-c` flag when starting the process.

parameter: `ulimit`  
required: No  
default value: User's environment  
description: This value is passed directly to the `ulimit` command when starting the process. For more information, refer to the `ulimit` documentation.

parameter: `directory`  
required: No  
default value: `$SDRROOT`  
description: Specifies to change the directory to `directory` before running the process.

parameter: `run_detached`  
required: No  
default value: `True`  
description: Specifies to run the Domain Manager as a daemon, not a child of the AdminService process.

parameter: `logfile_directory`  
required: No  
default value: `/var/log/redhawk/domain-mgr`  
description: The absolute path to the logging directory.

parameter: `stdout_logfile`  
required: No  
default value: `<domain name>.stdout.log`  
description: The name of a file that captures the stdout from the process. If not specified, the default value list above is used.

parameter: `stderr_logfile`  
required: No  
default value: `<domain name>.stderr.log`  
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
