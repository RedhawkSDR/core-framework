# Managing Services By Domains and Types

This section explains how to manage REDHAWK core <abbr title="See Glossary.">services</abbr> either by <abbr title="See Glossary.">domain</abbr> or by type (`domain`, `nodes`, or `waveforms`).  For additional information on managing service configurations and life cycle management, refer to [Domain Manager Service](../../../appendices/adminservice/redhawkcoreservices/domainmanager.html), [Device Manager Service](../../../appendices/adminservice/redhawkcoreservices/devicemanager.html), and [Waveform Service](../../../appendices/adminservice/redhawkcoreservices/waveform.html).

### Managing Services for a Domain

Services for a domain can be managed using the following commands.

* Reloading and restarting all services for a domain from current configuration files:

```sh
rhadmin update <Domain Name>
```

* Starting all services in a domain:

```sh
rhadmin start <Domain Name>
```

* Stopping all the services in a domain:

```sh
rhadmin stop <Domain Name>

```

* Inspecting the status of all the services in a domain:

```sh
rhadmin status <Domain Name>
```

* Running a custom status script for all the services in a domain and displaying the script output:

```sh
rhadmin query <Domain Name>
```

* Restarting all the services for a domain:

```sh
rhadmin restart <Domain Name>
```

### Managing Services by Type

Each life cycle management command (`start`, `stop`, `status`, `query`, and `restart`)  has an optional `type` parameter (`domain`, `nodes`, and `waveforms`), which restricts the command to execute against the specific type of service for a domain.  In addition, the value `all` can be substituted for the `<Domain Name>` argument, which executes the command for a specific service type, regardless of the service type's domain. The same command syntax is supported for all life cycle commands (`start`, `stop`, `status`, `query`, `restart`):

```sh
rhadmin command type <Domain Name>|all
```

The following commands demonstrate how to execute the `start` command  using the `type` option.

* Starting a specific <abbr title="See Glossary.">Domain Manager</abbr> service:

```sh
rhadmin start domain <Domain Name>
```

* Starting all defined Domain Manager services:

```sh
rhadmin start domain all
```

* Starting all Device Manager services for a specific domain:

```sh
rhadmin start nodes <Domain Name>
```

* Starting all Device Manager services:

```sh
rhadmin start nodes all
```

* Starting all <abbr title="See Glossary.">waveform</abbr> services for a specific domain:

```sh
rhadmin start waveforms <Domain Name>
```

* Starting all waveform services:

```sh
rhadmin start waveforms all
```
