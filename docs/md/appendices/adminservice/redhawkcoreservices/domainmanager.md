# REDHAWK Domain Manager Service

This section explains how to manage a single REDHAWK <abbr title="See Glossary.">Domain Manager</abbr> <abbr title="See Glossary.">service</abbr>.  For additional information on managing service configurations and life cycle management, refer to [Device Manager Service](../../../appendices/adminservice/redhawkcoreservices/devicemanager.html), [Waveform Service](../../../appendices/adminservice/redhawkcoreservices/waveform.html), and [Managing Entire Domains](../../../appendices/adminservice/redhawkcoreservices/domains.html).


### Creating a Domain Service Configuration

To create a <abbr title="See Glossary.">domain</abbr> service configuration, enter the following command:

```sh
rhadmin config domain >  <output file>.ini
```
 A sample configuration is created, which requires the `DOMAIN_NAME` configuration property and the section's name to be specified. The section name may be used with `rhadmin` commands. For additional configuration property settings, refer to the [Domain Manager Configuration File](../../../appendices/adminservice/configuration/domainmanager.html). For the  file to be recognized by the AdminService, the file must have an .ini extension and be installed into the proper service directory: `/etc/redhawk/domains.d`.  

### Displaying a Configuration

To display the current configuration for a service, enter the following command:

```sh
rhadmin getconfig  <Domain Name>:<section name>
```

### Starting a Service

To start a single Domain Manager service, enter the following command:

```sh
rhadmin start <Domain Name>:<section name>
```

### Stopping a Service

To stop a single Domain Manager service, enter the following command:

```sh
rhadmin stop <Domain Name>:<section name>
```

### Requesting Status of a Service

To status a single Domain Manager service, enter the following command:

```sh
rhadmin status <Domain Name>:<section name>
```

### Restarting a Service

To restart a single Domain Manager service, enter the following command:

```sh
rhadmin restart <Domain Name>:<section name>
```

### Example Session

The following example creates, activates, starts, and statuses a Domain Manager service for the domain `REDHAWK_PROD`, which is identified by the section name `redhawk_prod`.

```sh
# generate a domain configuration file
rhadmin config domain >  redhawk-prod.ini

# edit the ini file and change redhawk_prod in [domain:domain1],
# and provide a value for DOMAIN_NAME=REDHAWK_PROD
vi redhawk-prod.ini

cp redhawk-prod.ini /etc/redhawk/domains.d

rhadmin update REDHAWK_PROD
rhadmin start REDHAWK_PROD:redhawk_prod
rhadmin status REDHAWK_PROD:redhawk_prod

# produces the following output
REDHAWK_PROD:redhawk_prod            RUNNING   pid 1234, uptime 0:00:10
```
