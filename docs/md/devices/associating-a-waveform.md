# Associating a Waveform with an FEI Device

Two processes can be used to associate a <abbr title="See Glossary.">waveform</abbr> with an FrontEnd Interfaces (FEI) <abbr title="See Glossary.">device</abbr>:

  - Write code that creates an allocation and creates the connection between the allocated device and the waveform or <abbr title="See Glossary.">component</abbr> that is to receive or produce the data.
  - Create a <abbr title="See Glossary.">usesdevice relationship</abbr> artifact in the waveform.

The benefit of creating a usesdevice relationship artifact is that the deployment of the waveform can be tied to the availability of hardware resources. If the usesdevice relationship cannot be established at deployment time, then the waveform deployment will fail. Also, when the waveform is torn down, the allocation created for this deployment is undone automatically.

To create a usesdevice relationship artifact on the waveform:

1.  Create a [waveform](../waveforms/_index.html)

2.  Find and place the usesdevice relationship artifact, which is located under the Advanced tab:

    ![FrontEnd Tuner Device Artifact](images/Uses_Step_1.png)

    When the artifact is dragged onto the canvas, a menu of target devices is shown. The devices shown are the result of a scan of `$SDRROOT`.

3.  Either select the desired device or select the **Generic FrontEnd device**, which does not have any <abbr title="See Glossary.">ports</abbr> (so a connection cannot be defined), but it allows an allocation to be associated with the deployment.

    ![Select Target Device](images/Uses_Step_2.png)

    After selecting a device, a series of wizards are displayed. In the wizards, define the usesdevice relationship ID to use and the parameters for the allocation itself.

> **NOTE**  
> The usesdevice relationship ID is the identifier used in the waveform description to reference the usesdevice relationship artifact. It is an architectural detail that is not relevant to the developer, so use the default value given.

4.  If the device has ports, connect them to the appropriate component:

    ![Create a Connection Between the FEI Device and a Component](images/Uses_Step_3.png)

