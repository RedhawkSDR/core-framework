# Service Configurations

The INI service configuration files define the execution environment for each REDHAWK core <abbr title="See Glossary.">service</abbr>. The AdminService reads all the service configuration files at startup and executes any enabled configuration.  This section provides an overview of the service configuration files and how to manage configurations using `rhadmin`.

## Configuration Contents

The INI files contain configuration properties in the form `Name=Value` and are grouped by section headers with the syntax `[<type>:<section name>]`; where `<type>` is either `domain`, `node`, or `waveform`. All service configuration files contain a required property `DOMAIN_NAME`, which is used by the AdminService to group services by <abbr title="See Glossary.">domain</abbr>, and used to define the service's name. The complete definition for each type of service configuration file is explained in the following sections.

- [Domain Manager Configuration File](../../../appendices/adminservice/configuration/domainmanager.html)  
- [Device Manager Configuration File](../../../appendices/adminservice/configuration/devicemanager.html)  
- [Waveform Configuration File](../../../appendices/adminservice/configuration/waveform.html)  

### Configuration Concepts

This section provides general concepts about the implementation of the AdminService.

#### Service Name

The service's name takes the form of `<domain name>:<section name>`, and is derived from the section header, `[<type>:<section name>]`, in the INI file and the value of the `DOMAIN_NAME` property.  The service's name may be used as the target for `rhadmin` commands that accept this argument. The following example describes a <abbr title="See Glossary.">Device Manager</abbr> service with service name `REDHAWK_DEV:MyNode`:

```
[node:MyNode]
DOMAIN_NAME=REDHAWK_DEV
NODE_NAME=NODE
```
#### Domain Name

All REDHAWK core services are required to have a `DOMAIN_NAME` configuration property. This property defines the REDHAWK domain context and allows the AdminService to group services into a domain group.  This domain group is used to determine the execution priority and arguments for `rhadmin` commands.

#### Priority

The `priority` setting is the main contributing factor in how the AdminService starts and stops the REDHAWK core services. Every core service has a default priority that the AdminService uses to organize the startup of domain groups, and then services within a domain group. The default priority for each service type within a domain group is (<abbr title="See Glossary.">Domain Manager</abbr>, Device Managers, and then <abbr title="See Glossary.">waveforms</abbr>).  If there are multiple domains defined, all services in the domain with the lowest numerical priority Domain Manager are started first.  The priority setting can be used to customize the startup of domains and service types within the domain.

When shutting down a domain or during system shutdown, the AdminService stops the domain with the highest numerical priority first.

#### Daemon Process

By default, the `run_detached` property for all services is set to `true`.  This property controls if the service is started as a daemon and detached from the AdminService. If set to true, the service's process running state is *not* affected by restarts of the AdminService.  In essence, the service's process life cycle is independent of the AdminService's process life cycle.  If `run_detached` is set to false, then the service's life cycle follows the AdminService's life cycle.

#### Environment Variables

Environment variables may also be referenced and defined in the service configuration files.  To reference an environment variable, use the following expression syntax: `%(ENV_X)s`, where `X` is the name of the environment variable.  All AdminService's environment variables are available for use when the service configuration is processed. In the following example, the environment variable `LOGLEVEL` is used to the set the configuration property `loglevel`.

```
loglevel=%(ENV_LOGLEVEL)s
```

Environment variables may be overridden by using the `environment` configuration property. However, only uppercase configuration parameter names can use the values of these overridden environment variables. In the following example of a node configuration, the `PYTHONPATH` configuration parameter uses the overridden environment variable.
```
environment=PYTHONPATH=/usr/local/redhawk/core/lib/python:/usr/local/mylibrary/lib/python
PYTHONPATH=%(ENV_PYTHONPATH)s
```

#### Default Configurations
Each service type (`domain`, `node`, `waveform`) has an associated defaults file in the directory `/etc/redhawk/init.d/`. Each file provides the default settings for all optional configuration properties in a service configuration file.  For example, a Domain Manager service configuration file only requires setting the `DOMAIN_NAME` property; all other default property settings are resolved from the file `/etc/redhawk/init.d/domain.defaults`.

## Service File Locations
The AdminService reads service configuration files from several directories on startup. The following table lists the locations and the files read:

##### Service File Locations and File Types
| **Service**    | **Configuration Directory**  | **File**        |
| :------------- | :--------------------------- | :-------------- |
| Defaults       | `/etc/redhawk/init.d`        | `*.defaults`    |
| Domain Manager | `/etc/redhawk/domains.d`     | `*.ini`         |
| Device Manager | `/etc/redhawk/nodes.d`       | `*.ini`         |
| Waveform       | `/etc/redhawk/waveforms.d`   | `*.ini`         |


## Creating Configuration Files

To create a new configuration file, enter the following command, replacing `<type>` with `domain`, `node`, or `waveform` as appropriate.
```sh
# This will generate a generic configuration file
rhadmin config <type> > <file name>.ini

```
For the file to be recognized by the AdminService, the file is required to have an `.ini` file extension and be installed into the proper service directory under `/etc/redhawk`.  The following example describes how to create, install, and activate a Domain Manager service configuration.

```sh
# This will generate a generic configuration file
rhadmin config domain > rhdom.ini

# edit file and change DOMAIN_NAME=REDHAWK_DEV
vi rhdom.ini

cp rhdom.ini /etc/redhawk/domains.d

rhadmin reload
```


> **NOTE**  
> The configuration files are located in system privileged directories. Ensure that you have proper privileges to create and edit files in those directories.  

For Device Manager and waveform manager services, the REDHAWK IDE can also be used to generate an example configuration file and an RPM .spec file to install it. For more information, refer to [Device Manager Service](../../../appendices/adminservice/redhawkcoreservices/devicemanager.html) and
[Waveform Service](../../../appendices/adminservice/redhawkcoreservices/waveform.html).

## Viewing Configured Services in the AdminService

To view what services are currently configured in the AdminService, enter the following command:
```sh
rhadmin list
```

The configured Domain Manager, Device Manager, and waveform services are displayed.
```
Name                             In Use    Autostart Enabled   Priority
REDHAWK_DEV:GppNode              in use    auto      Enabled   100:400
REDHAWK_DEV:REDHAWK_DEV_mgr      in use    auto      Enabled   100:100
REDHAWK_DEV:Wave                 in use    auto      Enabled   100:900
```

The following columns are displayed in the configured services output:

- Name - The `service name` that may be used for other commands. The format is `<domain name>:<section name>`.
- In Use - The status of the service configuration, which indicates whether the configuration has been activated and is operable.
- Autostart - Specifies whether the service will automatically start when the AdminService starts.
- Enabled - Specifies whether the service configuration can be started. This setting may be overridden on the start command with the `-f` flag.
- Priority - The priority of the service's configuration. The format is `<domain priority>:<service priority>`. In a multiple domain scenario, the lowest value for the `domain priority` is started first.  When starting services in a domain, the lowest value for `service priority` is started first.

To view the contents of a service's configuration file use the `getconfig` command.
```sh
rhadmin getconfig <service name>
```


## Managing Configurations

There is no direct command to add or remove service configurations from the AdminService.  This is accomplished by adding or deleting files from the appropriate service directory under `/etc/redhawk` and then running the `rhadmin update` command. This command rereads the configuration files, starts any new services that were defined, and stops any currently running services that are not enabled or defined (for example, a deleted INI file). Service's with their configuration property `run_detached=True` will have their configuration processed but the underlying service process will *not* be restarted.

The following example adds a new waveform service, `REDHAWK_DEV:Wave2`, to the AdminService.

```sh
rhadmin config waveform > wave2.ini

# change DOMAIN_NAME=REDHAWK_DEV and WAVEFORM=wave2 properties, set the section header to [waveform:Wave2]
vi wave2.ini

cp wave2.ini /etc/redhawk/waveforms.d

rhadmin update REDHAWK_DEV
```

The following output is displayed:
```
REDHAWK_DEV: updated process group
```

To verify the `REDHAWK_DEV:Wave2` configuration was added and started, enter the following command:

```sh
rhadmin status
```

The following output is displayed:
```
REDHAWK_DEV:GppNode              RUNNING   pid 31316, uptime 0:00:46
REDHAWK_DEV:REDHAWK_DEV_mgr      RUNNING   pid 31185, uptime 0:00:52
REDHAWK_DEV:Wave                 RUNNING   pid 31745, uptime 0:00:30
REDHAWK_DEV:Wave2                RUNNING   pid 31408, uptime 0:00:41
```
