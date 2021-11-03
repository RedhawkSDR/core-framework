# AdminService

The REDHAWK AdminService manages the life cycle of the REDHAWK core <abbr title="See Glossary.">services</abbr> (<abbr title="See Glossary.">Domain Manager</abbr>, <abbr title="See Glossary.">Device Manager</abbr>, and <abbr title="See Glossary.">waveforms</abbr>) using simple INI-style configuration files to define the execution environment of each core service. The REDHAWK core services definitions are the same regardless of whether the system uses `systemd` or `init` for service control. The AdminService itself follows the normal Linux system service life cycle and is controlled using the operating system's service control.

## Installing the AdminService

To install the AdminService, enter the following command:

```sh
yum install redhawk-adminservice
```

## Startup Process

At system startup, the AdminService performs the following tasks:

1. Processes all INI files in the service configuration directories under the `/etc/redhawk` directory.

2. Creates <abbr title="See Glossary.">domain</abbr> groups using the `DOMAIN_NAME` configuration property in each service configuration file.

3. Determines the start order of each domain group using the `priority` configuration property of each domain group's Domain Manager service.

4. Determines the start order of each core service within a domain group using the `priority` configuration property of each core service. The typical start order defines Domain Manager first, followed by Device Managers, and finally waveforms.

5. Launches each core service within a domain group using the configuration definition and performs an initial status check of the service.

6. Repeats the previous step for all remaining domain groups.


## System Shutdown

On the host system shutdown, the AdminService terminates the domain and services in the domain group in reverse priority order.

## Managing the REDHAWK Core Services

Independent of system start up and shutdown, `rhadmin` is a command line utility used to manage the REDHAWK core services' life cycle. `rhadmin` supports the following commands to manage the service life cycle: `start`, `stop`, `restart`, and `status`. These commands can be used to manage an individual service or a group of services with the same service type or logical domain. The `rhadmin` utility can also be used to generate new configuration files for all the REDHAWK core service types.


For more information about using `rhadmin` to manage the REDHAWK core services, refer to [rhadmin](../../../appendices/adminservice/rhadmin.html).

##  REDHAWK AdminService Lifecycle

The AdminService is configured to startup and shutdown using the operating system's service control. The following table lists the system service scripts used to control the AdminService life cycle.

##### System Service Scripts for the AdminService
| **Service**                   | **System Service Script**                                    |
| :---------------------------- | :----------------------------------------------------------- |
| **CentOS 6 (SysV)**           |                                                              |
| AdminService                  | `/etc/rc.d/init.d/redhawk-adminservice`                      |
| **CentOS 7 (systemd)**        |                                                              |
| AdminService Setup            | `/usr/lib/systemd/system/redhawk-adminservice-setup.service` |
| AdminService                  | `/usr/lib/systemd/system/redhawk-adminservice.service`       |
| AdminService startup wrapper  | `$OSSIEHOME/bin/adminserviced-start`                         |
| AdminService shutdown wrapper | `$OSSIEHOME/bin/adminserviced-stop`                          |
| Setup Wrapper                 | `$OSSIEHOME/bin/redhawk-adminservice-setup`                  |

As per the Fedora recommendations for service unit files, the AdminService is not enabled during RPM installation. System integrators may enable the service unit file and modify the activation to achieve desired start up and shutdown behavior for their systems.

## Enabling AdminService on System Startup

To enable the AdminService on system boot using CentOS 6, enter the following commands as `root`.
```sh
chkconfig redhawk-adminservice on
```

To enable the Adminservice on system boot using CentOS 7, enter the following commands as `root`.
```sh
systemctl enable redhawk-adminservice-setup.service
systemctl enable redhawk-adminservice.service
```

> **NOTE**  
> The `redhawk-adminservice-setup.service` systemd service is required to create the directories in `/var/run` for managing process pid files. It does not need to be restarted while the system is running.  
