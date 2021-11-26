# SDRROOT in the IDE

The REDHAWK IDE provides a visualization of the file system contained within the SDRROOT in the <abbr title="See Glossary.">**REDHAWK Explorer** view</abbr>. The `SDRROOT` is referred to within the IDE as the <abbr title="See Glossary.">**Target SDR**</abbr>. The **Target SDR** location may be changed from within the IDE preferences page.

To change the **Target SDR**:

1.  Select **Window > Preferences**
2.  Select the **REDHAWK > SDR** preference page
3.  Change the **Local SDR Location** value
4.  Click **OK**

## Browsing Installed SDR Objects

From the **REDHAWK Explorer** view and the **Target SDR**, one can browse the installed <abbr title="See Glossary.">components</abbr>, <abbr title="See Glossary.">devices</abbr>, <abbr title="See Glossary.">nodes</abbr>, <abbr title="See Glossary.">services</abbr> and <abbr title="See Glossary.">waveforms</abbr>. Each of these REDHAWK Objects are placed in their own folder which may be expanded to show their content. Depending on the object type, different right-click operations and tree structure manipulation are permitted.

### Components

Mouse-hovering over a component within the **REDHAWK Explorer** view shows the full path to the Software Package Descriptor (SPD) file in the SDRROOT that defines it.

From the **REDHAWK Explorer** view, you can perform the following actions on components:

  - **Local Component Launch**: To launch an implementation of a component with default <abbr title="See Glossary.">property</abbr> values, right-click the component, select **Launch in Sandbox**, and select an implementation of the component to start within the <abbr title="See Glossary.">sandbox's</abbr> Chalkboard. Alternately, to launch an implementation of a component with customized property values, right-click the component, select **Launch in Sandbox**, and select **Advanced**. The Launch wizard is displayed. For more information, refer to [Launching Components in the IDE Sandbox](../Sandbox/IDE/_index.html#launching-components-in-the-ide-sandbox).

  - **View within REDHAWK Editor**: To open the component's XML files in an editor, double-click the component. This opens the [REDHAWK Component Editor](../IDE/Editors-and-Views/softpkg-editor.html) in read-only mode. You cannot edit the fields or perform any code generation from this version of the editor.

  - **Delete**: Delete a component from the SDRROOT by right-clicking and selecting **Delete**. This removes all files pertaining to this component from the SDRROOT including the Software Package Descriptor (SPD), Properties File (PRF), Software Component Descriptor (SCD), and executable files. You can highlight multiple items and delete them at the same time.

> **WARNING**  
> Deleting an item from the SDRROOT cannot be undone.  

### Devices

Mouse-hovering over a device within the **REDHAWK Explorer** shows the full path to the SPD file in the SDRROOT that defines it.

From the **REDHAWK Explorer** view, you can perform the following actions on device objects:

  - **Local Device Launch**: Right-click the device, select **Launch in Sandbox**, and select an implementation of the device to start within the sandbox's <abbr title="See Glossary.">Device Manager</abbr>. Expand **Sandbox** > **Device Manager** to view the running device. Alternately, to launch an implementation of a device with customized property values, right-click the device, select **Launch in Sandbox**, and select **Advanced**. The Launch wizard is displayed. For more information, refer to [Launching Devices in the IDE Sandbox](../Sandbox/IDE/_index.html#launching-devices-in-the-ide-sandbox).

  - **View within REDHAWK Editor**: To open the device's XML files in an editor, double-click the device. This opens the [REDHAWK Editor](../IDE/Editors-and-Views/softpkg-editor.html) in read-only mode. You cannot edit the fields or perform any code generation from this version of the editor.

  - **Delete**: Delete a device from the SDRROOT by right-clicking and selecting **Delete**. This removes all files pertaining to this device from the SDRROOT including the SPD, PRF, SCD, and executable files. You can highlight multiple items and delete them at the same time.

> **WARNING**  
> Deleting an item from the SDRROOT cannot be undone.  

### Nodes

Mouse-hovering over a node within the **REDHAWK Explorer** shows the full path to the Device Configuration Descriptor (DCD) file in the SDRROOT that defines it.

From the **REDHAWK Explorer** view, you can perform the following actions on node objects:

  - **Launch Device Manager**: You may launch a Device Manager onto a running <abbr title="See Glossary.">domain</abbr> for this node. Right-click the node and select **Launch Device Manager** to bring up a Launch Device Manager dialog. From this dialog, you may select from a list of running domains. Pressing **OK** in this dialog creates a new <abbr title="See Glossary.">Console view</abbr> and runs an instance of `nodeBooter` using this node's DCD file.

  - **View within REDHAWK Editor**: To open the node's XML files in an editor, double-click the node. This opens the [REDHAWK Node Editor](../IDE/Editors-and-Views/node-editor.html) in read-only mode. You cannot edit the fields or perform any code generation from this version of the editor.

  - **Expand node**: Using the tree structure to expand the node, the IDE decomposes the sections of the DCD file into the referenced devices, Device Manager, <abbr title="See Glossary.">Domain Manager</abbr>, and partitioning sections. By selecting these objects with the <abbr title="See Glossary.">Properties view</abbr> open, all the information from the XML is displayed in tabular format.

  - **Delete**: Delete a node from the SDRROOT by right-clicking and selecting **Delete**. This removes only the DCD file and the folder containing it from the Software-Defined Radio (SDR) and does not delete the referenced devices. You can highlight multiple items and delete them at the same time.


> **WARNING**  
> Deleting an item from the SDRROOT cannot be undone.  

### Services

Mouse-hovering over a service within the **REDHAWK Explorer** shows the full path to the SPD file in the SDRROOT that defines it.

From the **REDHAWK Explorer** view, you can perform the following actions on service objects:

  - **Local service Launch**: Right-click the service, select **Launch in Sandbox**, and select an implementation of the service to start within the sandbox's chalkboard. Alternately, to launch an implementation of a service with customized property values, right-click the service, select **Launch in Sandbox**, and select **Advanced**. The Launch wizard is displayed. For more information, refer to [Launching Services in the IDE Sandbox](../Sandbox/IDE/_index.html#launching-services-in-the-ide-sandbox).

  - **View within REDHAWK Editor**: To open the SPD file within an editor, double-click the SPD file. This opens the [REDHAWK Editor](../IDE/Editors-and-Views/softpkg-editor.html) in read-only mode. You cannot edit the fields or perform any code generation from this version of the editor.

  - **Delete**: Delete a service from the SDRROOT by right-clicking and selecting **Delete**. This removes all files pertaining to this service from the SDRROOT including any SPD, PRF, SCD, and executable files. You can highlight multiple items and delete them at the same time.

> **WARNING**  
> Deleting an item from the SDRROOT cannot be undone.  

### Waveforms

Mouse-hovering over a waveform within the **REDHAWK Explorer** shows the full path to the Software Assembly Descriptor (SAD) file in the SDRROOT that defines it.

You can perform the following actions from the **REDHAWK Explorer** view on waveform objects:

  - **Local waveform Launch**: Right-click the waveform, select **Launch in Sandbox**, and select an implementation of the waveform to start within the sandbox's chalkboard. Alternately, to launch an implementation of a waveform with customized property values, right-click the waveform, select **Launch in Sandbox**, and select **Advanced**. The Launch wizard is displayed. For more information, refer to [Launching Waveforms in the IDE Sandbox](../Sandbox/IDE/_index.html#launching-waveforms-in-the-ide-sandbox).

  - **View within REDHAWK Editor**: To open the SAD file within an editor, double-click the SAD file. This opens the [Waveform Editor](../IDE/Editors-and-Views/waveform-editor.html) in read-only mode. You cannot edit the fields or add any additional components.

  - **Expand waveform**: Using the tree structure to expand the waveform, the IDE decomposes the sections of the SAD file into the referenced <abbr title="See Glossary.">assembly controller</abbr>, component files, and partitioning sections. By selecting these objects with the **Properties** view open, all the information from the XML is displayed in a tabular format.

  - **Delete**: Delete a waveform from the SDRROOT by right-clicking and selecting **Delete**. This removes only the SAD file and the folder containing it from the SDR and does not delete the referenced components. You can highlight multiple items and delete them at the same time.

> **WARNING**  
> Deleting an item from the SDRROOT cannot be undone.  

### Browsing Installed Interface Description Language (IDL) Libraries

Even though the IDL Repository is displayed within the Target SDR tree structure, the deployed IDLs and the core IDL library are not stored within the SDRROOT directory on the file system. All IDLs exist within the `OSSIEHOME` directory. However, the IDL repository is shown here so that one may browse the content and view the installed IDLs.

Expanding the IDL Repository tree structure exposes the individual IDLs and a list of module folders which contain IDLs. To view the content of an IDL in a text editor, one may either double-click the IDL or right-click and select **Text Editor** or **Open With... > Text Editor**.

One may also expand an IDL. When an IDL is expanded, the IDE parses its content to decompose it into referenced methods and objects. By selecting these methods and objects with the **Properties** view open one can view all the information from the IDL in a tabular format.

### Getting Details About Error Conditions

If an error condition occurs within the SDRROOT that the IDE can detect, it marks the object in error with a decorator in the lower left corner. Mouse hovering over the item's icon provides a short description of the issue; however, if more than one problem has occurred the hover text reads "Multiple Problems exist with this item".

More detail about an error can be found within the **Properties** view of the item.
To view the details about an error condition:

1.  With the item selected, select or open the **Properties** view.
2.  From the **Properties** view, select the **Advanced** tab
3.  Select the status row. This causes the **Details** key to appear.
4.  Press the **Details** button to bring up a detailed dialog of the current error conditions
![Error Event Details Dialog](img/REDHAWK_Property_View_Error_Dialog.png)
