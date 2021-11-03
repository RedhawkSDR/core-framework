# AdminService Configuration

This section explains how to create a custom AdminService configuration and how to configure the `rhadmin` script  used to control the AdminService.


#### Creating a Custom AdminService Configuration

To create a new AdminService configuration file, enter the following commands.
```sh
cd /etc/redhawk
rhadmin config admin > adminserviced.cfg
vi adminserviced.cfg
```

> **NOTE**  
> The configuration files are located in a system privileged directory. Ensure that you have proper privileges to create and edit files in those directories.  

To start the AdminService as a local user, enter the following command.
```sh
adminserviced
```

To start the AdminService as a system service on CentOS 6, enter the following commands.
```sh
/sbin/service redhawk-adminservice start
```

To start the AdminService as a system service on CentOS 7, enter the following commands.
```sh
systemctl start redhawk-adminservice.service
```

The following table describes the sections of the AdminService configuration file.

##### AdminService Configuration File Sections

| **Section**                                                | **Description**                                               |
| :--------------------------------------------------------- | :------------------------------------------------------------ |
| [[`unix_http_server`](#unix_http_server)] | Defines the Unix socket interface to the AdminService.        |
| [[`rhadmin`](#rhadmin)]                   | Specifies settings for running the `rhadmin` client.          |

## `unix_http_server`

The `unix_http_server` section of the AdminService configuration file defines the local socket that the `rhadmin` can use for remote control of the AdminService.

### Configuration Parameters

This section describes the `unix_http_server` configuration parameters.

parameter: `file`  
required: No  
default value: `/var/run/redhawk/adminserviced.sock`  
description: The absolute path to a Unix domain socket used to listen for remote commands.

parameter: `username`  
required: No  
default value: `redhawk`  
description: The username for remote control of the AdminServer.

parameter: `password`  
required: No  
default value: `redhawk`  
format: Cleartext password or may be specified as a SHA-1 hash if prefixed by the string `{SHA}`. For example, `{SHA}82ab876d1387bfafe46cc1c8a2ef074eae50cb1d` is the SHA-stored version of the password `thepassword`.  
description: The password for remote control of the AdminServer.

## `rhadmin`

The `rhadmin` control script can be configured in the main AdminService configuration file.

### Configuration Parameters

This section describes the `rhadmin` configuration parameters.

parameter: `serverurl`  
required: No  
default value: `unix:///var/run/redhawk/adminserviced.sock`  
format: `unix:///path/to/file.sock`  
description: The path to the Unix domain socket used by the AdminServer.

parameter: `username`  
required: No  
default value: `redhawk`  
description: The username to use when connecting to the AdminServer.

parameter: `password`  
required: No  
default value: `redhawk`  
format: Clear text  
description: The password to use when connecting to the AdminServer.
