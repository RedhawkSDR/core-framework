# REDHAWK Device Manager Service

This section explains how to manage a single REDHAWK REDHAWK <abbr title="See Glossary.">Device Manager</abbr> <abbr title="See Glossary.">service</abbr>.  For additional information on managing service configurations and life cycle management, refer to [Domain Manager Service](../../../appendices/adminservice/redhawkcoreservices/domainmanager.html),
 [Waveform Service](../../../appendices/adminservice/redhawkcoreservices/waveform.html), and [Managing Entire Domains](../../../appendices/adminservice/redhawkcoreservices/domains.html).



### Creating a Device Service Configuration Using the `rhadmin` Script

To create a <abbr title="See Glossary.">node</abbr> service configuration, enter the following command:

```sh
rhadmin config node >  <output file>.ini
```
A sample configuration is created, which requires the `DOMAIN_NAME` and `NODE_NAME` configuration properties and the section's name to be specified. The section name may be used with `rhadmin` commands. For additional configuration property settings, refer to the [Device Manager Configuration File](../../../appendices/adminservice/configuration/devicemanager.html). For the file to be recognized by the AdminService, the file must have an .ini extension and be installed into the proper service directory: `/etc/redhawk/nodes.d`.  

#### Creating a Device Service Configuration Using the REDHAWK IDE

1. In the REDHAWK IDE, to create a configuration file, click the Generate Node button in the Device Configuration Descriptor (DCD) editor.
![Generate Node Button](../../../images/GenerateNodeButton.png)

2. In the Regenerate Files dialog, check the checkbox next to the .ini file to generate it. If the .spec file is also checked, the generated .spec file will include the installation of the .ini file.
![Generate Node File Selection](../../../images/GenerateNodeSelectIni.png)


### Displaying a Configuration

To display the current configuration for a service, enter the following command:

```sh
rhadmin getconfig  <Domain Name>:<section name>
```

### Starting a Service

To start a single Device Manager service, enter the following command:

```sh
rhadmin start <Domain Name>:<section name>
```

### Stopping a Service

To stop a single Device Manager service, enter the following command:

```sh
rhadmin stop <Domain Name>:<section name>
```

### Requesting Status of a Service

To status a single Device Manager service, enter the following command:

```sh
rhadmin status <Domain Name>:<section name>
```

### Restarting a Service

To restart a single Device Manager service, enter the following command:

```sh
rhadmin restart <Domain Name>:<section name>
```

### Example Session

The following example creates, activates, starts, and statuses a Device Manager service for the <abbr title="See Glossary.">domain</abbr>, `REDHAWK_PROD`, which is identified by the section name `prodsvr1_gpp`.

```sh
# generate a node configuration file
rhadmin config node >  redhawk_prod_svr1_gpp.ini

# edit the ini file and change node1 to prodsvr1_gpp in [node:node1],
# and set the following properties: DOMAIN_NAME=REDHAWK_PROD, NODE_NAME=ProdSvr1_GPP
vi redhawk_prod_svr1_gpp.ini

cp redhawk_prod_svr1_gpp.ini /etc/redhawk/nodes.d

rhadmin update REDHAWK_PROD
rhadmin start REDHAWK_PROD:prodsvr1_gpp
rhadmin status REDHAWK_PROD:prodsvr1_gpp

# produces the following output
REDHAWK_PROD:prodsvr1_gpp            RUNNING   pid 2345, uptime 0:00:10
```
