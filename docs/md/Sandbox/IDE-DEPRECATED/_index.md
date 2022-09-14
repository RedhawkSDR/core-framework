# IDE Sandbox (DEPRECATED)

> **WARNING**:  The IDE has been deprecated as of REDHAWK version 3.0.0.

The IDE-based sandbox provides a graphical environment for launching, inspecting, and debugging <abbr title="See Glossary.">components</abbr>, <abbr title="See Glossary.">devices</abbr>, <abbr title="See Glossary.">services</abbr>, and <abbr title="See Glossary.">waveforms</abbr>. The IDE-based sandbox can host an instance of a Python-based <abbr title="See Glossary.">sandbox</abbr>, with both interlinked, allowing artifacts from the Python environment to interact with those on the graphical UI.

### Launching Components in the IDE Sandbox

The following procedures explain how to launch a component in the IDE sandbox.

#### Default Property Values

1.  To launch an implementation of a component with default <abbr title="See Glossary.">property</abbr> values, from the <abbr title="See Glossary.">REDHAWK Explorer view</abbr>, right-click the component, select **Launch in Sandbox**, and select an implementation of the component to start within the sandbox's Chalkboard.

    The component is launched in the sandbox. The component will initially be gray in color until launching is complete. When the component is finished loading, its background color is blue.

#### Customized Property Values

1.  To launch an implementation of a component with customized property values, from the REDHAWK Explorer view, right-click the component, select **Launch in Sandbox**, and select **Advanced**.

    If the component has multiple implementations, the Select Implementation dialog of the Launch wizard is displayed. Select the implementation and click Next.

    ![Select Implementation Dialog](img/SelectImplementationlaunch.png)

    The Assign Initial Properties dialog of the Launch wizard is displayed.

    ![Assign Initial Properties Dialog](img/AssignInitialProperties.png)

2.  Enter the Properties information and click Next.

    The Launch Configuration Options dialog is displayed.

    ![Launch Configuration Options Dialog](img/LaunchConfigurationOptions.png)

3.  Specify the launch options and click Finish.

    The component is launched in the IDE sandbox. The component will initially be gray in color until launching is complete. When the component is finished loading, its background color is blue.

### Launching Devices in the IDE Sandbox

The following procedures explain how to launch a device in the IDE sandbox.

#### Default Property Values

1.  To launch an implementation of a device with default property values, from the REDHAWK Explorer view, right-click the device, select **Launch in Sandbox**, and select an implementation of the device to start within the sandbox's Chalkboard.

    The device is launched in the sandbox.

#### Customized Property Values

1.  To launch an implementation of a device with customized property values, from the REDHAWK Explorer view, right-click the device, select **Launch in Sandbox**, and select **Advanced**.

    If the device has multiple implementations, the Select Implementation dialog of the Launch wizard is displayed. Select the implementation and click Next.

    The Assign Initial Properties dialog of the Launch wizard is displayed.

2.  Enter the Properties information and click Next.

    The Launch Configuration Options dialog is displayed.

3.  Specify the launch options and click Finish.

    The device is launched in the IDE sandbox.

### Launching Services in the IDE Sandbox

The following procedures explain how to launch a service in the IDE sandbox.

#### Default Property Values

1.  To launch an implementation of a service with default property values, from the REDHAWK Explorer view, right-click the service, select **Launch in Sandbox**, and select an implementation of the service to start within the sandbox's Chalkboard.

    The service is launched in the sandbox.

#### Customized Property Values

1.  To launch an implementation of a service with customized property values, right-click the service, select **Launch in Sandbox**, and select **Advanced**.

    If the service has multiple implementations, the Select Implementation dialog of the Launch wizard is displayed. Select the implementation and click Next.

    The Assign Initial Properties dialog of the Launch wizard is displayed.

2.  Enter the Properties information and click Next.

    The Launch Configuration Options dialog is displayed.

3.  Specify the launch options and click Finish.

    The service is launched in the IDE sandbox.

### Launching Waveforms in the IDE Sandbox

The following procedures explain how to launch a waveform in the IDE sandbox.

#### Default Property Values

1.  To launch a waveform with default property values, right-click the waveform, select **Launch in Sandbox**, and select Default.

    The waveform is launched in the sandbox.

#### Customized Property Values

1.  To launch a waveform with customized property values, right-click the waveform, select **Launch in Sandbox**, and select **Advanced**.

    The Assign Initial Properties dialog of the Launch waveform wizard is displayed.

    ![Assign Initial Properties Dialog](img/WaveformInitProp.png)

2.  Enter the Properties information and click Next.

    The Launch Configuration Options dialog of the Launch waveform wizard is displayed.

    ![Launch Configuration Options Dialog](img/WaveformLaunchConfigOptions.png)

3.  Specify the launch options and click Finish.

    The waveform is launched in the IDE sandbox.
