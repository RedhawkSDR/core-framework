# Running a Node

As part of the REDHAWK install, a <abbr title="See Glossary.">domain</abbr> and <abbr title="See Glossary.">node</abbr> are setup by default. To launch the domain and node, refer to [Launching a Domain](../runtime-environment/launching-a-domain.html).

### Exploring the Running Node

When a node is running, several attributes about the node, can be viewed in the REDHAWK IDE. The following steps explain how to explore the attributes of a <abbr title="See Glossary.">device</abbr> that is part of a running node.

1.  In the <abbr title="See Glossary.">REDHAWK Explorer view</abbr>, expand **REDHAWK_DEV**.
2.  Expand **Device Managers**.
3.  Expand **DevMgr_\<localhost\>.localdomain**.
4.  Select **GPP_\<localhost\>_localdomain**.
5.  Select the <abbr title="See Glossary.">**Properties** view</abbr> tab. The **Properties** view displays all the <abbr title="See Glossary.">properties</abbr> for this device, such as the operation system name, amount of available memory, and other important information, as seen below:
![Properties View of a Running GPP](img/GPPProps.png)

### Creating a Component that Consumes Resources

All <abbr title="See Glossary.">components</abbr> consume processor resources such as memory and CPU capacity. These resources are automatically tracked by the <abbr title="See Glossary.">GPP</abbr>, and when its thresholds are exceeded, the GPP enters the BUSY state, preventing further [deployments onto the computer](../waveforms/deployment-resources.html). However, in some cases, it is desirable for the component to consume some other capacity on the computer that is not part of the standard REDHAWK deployment model. The GPP contains several `allocation` properties such as `mcastnicIngressCapacity` or `mcastnicVLANs` that are available as a deployment constraint but are not used by the default component deployments.

In a more generalized way, when a device's specialized capacity is added to the device as a property of kind `allocation`, components can be created that will allocate this attribute of the device, making this `allocation` property a deployment constraint for the component.


> **NOTE**  
> Extending the GPP with new properties requires a modification of the GPP's source code and risks compatibility with other developers' components.  

To create a component that consumes these kind of `allocation` resources:

1.  Create a Python component called `sample`.
2.  In the <abbr title="See Glossary.">Project Explorer view</abbr>, double-click the `sample.spd.xml` file.
3.  Select the **Implementations** tab.
4.  In the **Dependencies** section, next to the **Dependency** list, click **Add...**.
5.  In the **Dependency Wizard**, select **Kind=Property Reference** and **Type=allocation**.
6.  The `Refid` field is more complicated; the component needs to be given some property that the component consumes when it is running on a device. An example of such a property is memory. For this example, memory is an attribute (property) of the device that the component consumes. To describe this, click the **Browse** button on the **Refid** field and select **GPP:memCapacity** (do not worry if there are multiple instances of the same property on the list, just pick one).
7.  After selecting this property, the globally unique ID of the property populates the **Refid** field, which in this case is: **DCE:8dcef419-b440-4bcf-b893-cab79b6024fb** (this is the `id` of the corresponding property of the GPP). To complete the dependency definition, provide a value for the consumption of this property: in this case, set **Value=***1000*.
8.  Click **Finish**.
9.  Save the project, generate the code and drag the component project to <abbr title="See Glossary.">**REDHAWK Explorer**</abbr> > <abbr title="See Glossary.">Target SDR</abbr>**.

### Create a Waveform for the New Component

The following steps explain how to create a <abbr title="See Glossary.">waveform</abbr> for the new component generated in the previous section.

1.  Create a new waveform called `sample_deploy`.
2.  Drag the component `sample` from the Palette onto the Diagram.
3.  Save the project and drag the project *sample_deploy* from the **Project Explorer** to **REDHAWK Explorer > Target SDR**.

### Observe the Effect of Launching a Component

The following steps explain how to launch and release a component.

1.  On the running domain, select **GPP_\<localhost\>_localdomain** and note the value for **memCapacity** on the [**Properties** tab for the GPP device](#properties-view-of-a-running-gpp).
2.  Launch the <abbr title="See Glossary.">application</abbr> *sample_deploy* on the running domain (right-click domain, select **Launch Waveform... \> sample\_deploy > Finish**). Note **memCapacity** for **GPP_\<localhost\>_localdomain** again; it is 1000 less than before the launch of the application.
3.  Release the application (**REDHAWK_DEV > Waveforms > sample\_deploy\_\<\#\>** right-click **Release**). Note memCapacity for **GPP_\<localhost\>_localdomain** again; it is restored to the original value.
