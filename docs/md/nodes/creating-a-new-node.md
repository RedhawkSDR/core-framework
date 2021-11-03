# Creating a New Node

As shown in [Exploring the Running Node](running-a-node.html#exploring-the-running-node) a <abbr title="See Glossary.">node</abbr> is a <abbr title="See Glossary.">Device Manager</abbr> instance with an associated set of <abbr title="See Glossary.">devices</abbr> and <abbr title="See Glossary.">services</abbr>. A node is completely defined by a Device Configuration Descriptor (DCD) XML file. A Device Manager uses the information in this XML file to deploy, configure, and inter-connect devices and services.

The **REDHAWK Node Project** in the REDHAWK IDE provides a mechanism for generating these DCD files. By invoking the **REDHAWK Node Project**, a wizard is started where the developer selects different characteristics for the node like the project name. In the wizard, the developer must provide both a project name and a <abbr title="See Glossary.">Domain Manager</abbr> name. The Domain Manager name is the name of the <abbr title="See Glossary.">domain</abbr> that the Device Manager automatically associates with upon startup. At runtime, the Domain Manager name that the Device Manager associates with can be overridden.

The node project has multiple tabs: Overview, Devices, Diagram, and the DCD file tab (see [Node Editor](../ide/editors-and-views/node-editor.html) for additional information). The most intuitive tab is the Diagram tab, which allows a developer to drag devices available in `$SDRROOT` into the node, as shown below.  Once the set of members for a particular node is determined, save the project and drag it to <abbr title="See Glossary.">**Target SDR**</abbr> to install it.

![Node Design Diagram](images/NodeDesign.png)

To launch this new node:

1.  Right-click the node descriptor, **Target SDR  > nodes > sample_node**.
2.  Select **Launch Device Manager**. The running node is now visible under the running domain's **Device Managers** section.

### Device and Service Affinity

The REDHAWK Device Manager supports service and device deployments with affinity options. For more information about the affinity directives and how to include them in a DCD file, consult [Resource Affinity](../waveforms/deployment-resources.html#resource-affinity). The following example sets the processing affinity for the `ChannelizerSW` device to use the CPUs from the second processor socket.

```xml
<componentplacement>
  <componentfileref refid="ChannelizerSW_12345"/>
  <componentinstantiation id="Channelizer_1" startorder="1">
    <usagename>Channelizer_1</usagename>
    <affinity>
      <simpleref refid="affinity::exec_directive_class" value="socket" />
      <!-- uses numa_parse_nodestring, socket id's start at 0 -->
      <simpleref refid="affinity::exec_directive_value" value="1"/>
    </affinity>
    <findcomponent>
     <namingservice name="Channelizer_1"/>
    </findcomponent>
  </componentinstantiation>
</componentplacement>
```
