# Service Life Cycle

When the AdminService is started at system startup, all enabled <abbr title="See Glossary.">services</abbr> are started.  This section explains the commands used to manage the life cycle of a REDHAWK core service process after system startup.

### Inspecting the Status of a Service
To inspect the status of a REDHAWK core service, use the `status` command.
```sh
rhadmin status service_name
```
or
```sh
rhadmin status domain_name
```

or
```sh
rhadmin status
```

or
```sh
rhadmin status [type] all
```
Where optional `[type]` is `domain`, `nodes`, or `waveforms`.

If `service_name` is provided, process the command against a specific service. If `domain_name` is provided, process the command against the specified <abbr title="See Glossary.">domain</abbr> group. If no argument is provided, process the command against all services. If the optional `[type]` is specified, then restrict the command to a specific core service type.

```sh
rhadmin status
```
The following output is displayed for all activated services:
```
REDHAWK_DEV:GppNode              STOPPED   May 11 11:31 AM
REDHAWK_DEV:REDHAWK_DEV_mgr      RUNNING   pid 19302, uptime 0:00:16
REDHAWK_DEV:Wave                 STOPPED   May 11 11:30 AM
```
The following columns are displayed in the activated services output:

- Column 1 - The `service name` that may be used for other commands. The format is `<domain name>:<section name>`.
- Column 2 - State of the service process: `RUNNING` or `STOPPED`.
- Column 3 - For `RUNNING` processes, process ID of the actual service, and the amount of time the service has been running. For `STOPPED` processes, the date and time the service was stopped.

### Starting a Service
To start a service, use the following commands:
```sh
rhadmin start service_name
```
or
```sh
rhadmin start domain_name
```

or
```sh
rhadmin start all
```

or
```sh
rhadmin start [type] all
```
Where optional `[type]` is `domain`, `nodes`, or `waveforms`.

If `service_name` is provided, the command is processed against a specific service. If `domain_name` is provided, the command is processed against the specified domain group. If 'all' is provided, the command is processed against all services. If the optional `[type]` is specified, then the command is restricted to a specific core service type.

The following example starts the <abbr title="See Glossary.">Domain Manager</abbr> service `REDHAWK_DEV:REDHAWK_DEV_mgr`:

```sh
rhadmin start REDHAWK_DEV:REDHAWK_DEV_mgr
```
The following output is displayed:
```
REDHAWK_DEV:REDHAWK_DEV_mgr: started
```
### Stopping a Service
To stop a service, use the following commands:
```sh
rhadmin stop service_name
```

or
```sh
rhadmin stop domain_name
```
or
```sh
rhadmin stop all
```
or
```sh
rhadmin stop [type] all
```
Where optional `[type]` is `domain`, `nodes`, `waveforms`.

If `service_name` is provided, the command is processed against a specific service. If `domain_name` is provided, the command is processed against the specified domain group. If 'all' is provided, the command is processed against all services. If the optional `[type]` is specified, then the command is restricted to a specific core service type.


The following example stops the <abbr title="See Glossary.">waveform</abbr> service `REDHAWK_DEV:Wave`:

```sh
rhadmin stop REDHAWK_DEV:Wave
```
The following output is displayed:
```
REDHAWK_DEV:Wave: stopped
```

The following status is displayed:
```
REDHAWK_DEV:GppNode              RUNNING   pid 17582, uptime 0:58:01
REDHAWK_DEV:REDHAWK_DEV_mgr      RUNNING   pid 17492, uptime 0:58:07
REDHAWK_DEV:Wave                 STOPPED   May 11 11:30 AM
```

### Restarting a Service
To restart a service, use the following commands:

```sh
rhadmin restart service_name
```

or
```sh
rhadmin restart domain_name
```

or
```sh
rhadmin restart all
```

or
```sh
rhadmin restart [type] all
```
Where optional `[type]` is `domain`, `nodes`, `waveforms`.

If `service_name` is provided, the command is processed against a specific service. If `domain_name` is provided, the command is processed against the specified domain group. If 'all' is provided, the command is processed against all services. If the optional `[type]` is specified, then the command is restricted to a specific core service type.


The following example restarts all the services for the domain group `REDHAWK_DEV`:

```sh
rhadmin restart REDHAWK_DEV
```
The following output is displayed:
```
REDHAWK_DEV:Wave             stopped
REDHAWK_DEV:GppNode          stopped
REDHAWK_DEV:REDHAWK_DEV_mgr  stopped
REDHAWK_DEV:REDHAWK_DEV_mgr  started
REDHAWK_DEV:GppNode          started
REDHAWK_DEV:Wave             started
```

The following status is displayed:
```
REDHAWK_DEV:GppNode              RUNNING   pid 20124, uptime 0:00:20
REDHAWK_DEV:REDHAWK_DEV_mgr      RUNNING   pid 20123, uptime 0:00:30
REDHAWK_DEV:Wave                 RUNNING   pid 20125, uptime 0:00:10
```
