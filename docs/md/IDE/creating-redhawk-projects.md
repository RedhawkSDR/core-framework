# Creating REDHAWK Projects

This section describes the different types of REDHAWK projects and how to create them using the provided Wizards. Before creating a new project, it is recommended that the IDE be in the [REDHAWK perspective](workbench.html) so that the proper menus are available.

To create a new REDHAWK project, click **File > New > Project**, and then select the project type.

The IDE displays the Select a wizard window, which prompts the user to select from multiple project types. Each project type has a custom Wizard to guide users through the initial creation process.


##### Common Fields
Some basic fields within the new project Wizard are common to all project types and include:

  - **Project Name**: A project name that is unique to the current <abbr title="See Glossary.">workspace</abbr>. Projects can be namespaced by adding dots in the name. Project names may not begin with a number and may not contain special characters other than dots for the namespace. Namespacing projects provides the ability to uniquely identify projects that share the same base name but have different implementations and also provides a logical grouping of resources. For example, the REDHAWK basic assets are namspaced as `rh.xxxx`.
  
> **TIP**  
> The Wizard dialog detects errors and informs the user if there is an invalid entry.  

  - **Location**: By default the location of the project is set to the current workspace. This may be changed by deselecting the **Use default location** check box and providing a custom location.

  - **Working sets**: A working set is an Eclipse concept and provides an additional layer of organization to the project workspace. A user may put common projects into a single "working set" to visually organize the **<abbr title="See Glossary.">Project Explorer</abbr>**. Additional options become available when projects are placed in working sets allowing the user to build, refresh, and search based on a specific working set. While it may resemble a folder, a working set does not create a new directory on the file system and the same project may belong to multiple working sets.

### REDHAWK Component Project

The **New Component Project Wizard** is a three page wizard that walks the user through creating a new REDHAWK <abbr title="See Glossary.">component</abbr>, selecting a programming language, and selecting a code generator. Many of the fields found within the component project wizard are also found within the device project wizard. Fields common to all new project wizards are defined in [Common Fields](#common-fields).

Page one of the component wizard contains:

  - **Contents**: A REDHAWK component is defined by an Software Package Descriptor (SPD) file. By default, when creating a new component, the SPD file created is an empty skeleton. A new component may be based off of a previous component's SPD file by selecting it from the file system.
  - **Component ID**: Every SoftPkg element contains an ID which is used for universally unique identification. The IDE generates a DCE compliant UUID by default.

Page two of the wizard defines the component's implementation, the programming language used and the code generation template.

The content found on this page includes:

  - **Prog. Lang**: A component's implementation may be written in either C++, Java, or Python.
  - **Code Generator**: The REDHAWK IDE provides a single code generator for each of the programming languages. It is possible to augment the IDE with additional code generators. A user may choose to forgo automatic code generation and instead choose to create their implementation manually.

  - **ID**: Each language implementation has a project unique ID. By default the language chosen is used as the ID.
  - **Description**: A component's implementation may contain a description. The description is written into the component's SPD file.

Page three of the wizard allows the user to customize the properties of the code generator and template.

Wizard page three contains fields for:

  - **Generator**: A read only label displaying the code generator being configured. This is the code generator that was chosen on the previous wizard page.
  - **Template**: The code generator may provide multiple templates to choose from. Each template may contain unique properties that may be set from the properties section.
  - **Output Directory**: The folder created to house the generated code.
  - **Package**: Available only for Java implementations. The Java package to place auto-generated code within.
  - **Properties**: Different code generation templates may provide configuration properties. Mouse hovering over the individual property provides information about the property.

Click **Finish** to complete the new project creation wizard.

### REDHAWK Control Panel Project

The **New Control Panel Project Wizard** is a two to three page wizard that walks the user through creating a new Plug-in project and optionally, generating a fully functioning project based off of supplied template projects. The New Control Panel Project Wizard is taken directly from the Eclipse New Plug-in Project Wizard and contains identical fields. Refer to the Eclipse documentation for more information.

### REDHAWK Device Project

The **New Device Project Wizard** is a three page wizard that walks the user through creating a new REDHAWK <abbr title="See Glossary.">device</abbr>, selecting a programming language, and selecting a code generator. Many of the fields found within the device project wizard are also found within the component project wizard. Fields common to all new project wizards are defined in [Common Fields](#common-fields).

Page one of the wizard contains:

  - **Device**: The user may select whether this is a standard device, Loadable device, or an Executable device. The checkbox below indicates if this is an Aggregate device.
  - **Contents**: A REDHAWK device is defined by an SPD file. By default, when creating a new device, the SPD file created is an empty skeleton. The new device may be based off of a previous device's SPD file by selecting it from the file system.
  - **Device ID**: Every SoftPkg element contains an ID which is used for universally unique identification. The IDE generates a DCE compliant UUID by default.

Page two of the wizard defines the device's implementation, the programming language used and the code generation template.

Wizard page two contains:

  - **Prog. Lang**: A device's implementation may be written in either C++, Java, or Python.
  - **Code Generator**: The REDHAWK IDE provides a single code generator for each of the programming languages. It is possible to augment the IDE with additional code generators. A user may choose to forgo automatic code generation and instead choose to create their implementation manually.
  - **ID**: Each language implementation has a project unique ID, by default the language chosen is used as the ID.
  - **Description**: A description for this implementation may be provided. The description is written into the device's SPD file.

Page three of the wizard allows the user to customize the properties of the code generator and template.

Wizard page three contains:

  - **Generator**: A read only label displaying the code generator being configured. This is the code generator that was chosen on the previous wizard page.
  - **Template**: The code generator may provide multiple templates to choose from. Each template may contain unique properties which may be set in the properties section.
  - **Output Directory**: The folder created to house the generated code.
  - **Package**: Available only for Java implementations. The Java package to place auto-generated code within.
  - **Properties**: Different code generation templates may provide configuration properties. Mouse hovering over the individual property provides information about the property.

Click **Finish** to complete the new project creation wizard.

### REDHAWK FrontEnd Device Project

The [**FEI Wizard**](../devices/interacting-with-hardware/creating-fei-device-ide.html#using-the-fei-wizard-to-create-an-fei-device) enables users to quickly create an FrontEnd Interfaces (FEI) compliant RX or TX tuner device. In the wizard, the user specifies the physical properties of the device, including whether the device ingests or outputs GPS and if the device has digital or analog input and output <abbr title="See Glossary.">ports</abbr>. Additionally, the user can choose to augment the required tuner status <abbr title="See Glossary.">properties</abbr> with additional optional properties.

### REDHAWK Interface Description Language (IDL) Project

The **IDL Project Wizard** is a one page wizard that walks the user through creating a new REDHAWK IDL project. Fields common to all new project wizards are defined in [Common Fields](#common-fields).


> **NOTE**  
> When creating a new IDL project, avoid using reserved names as specified in the Object Management Group Language Mapping Specifications (IDL to Java Language Mapping, Python, and C++).  

Page one of the wizard contains:

  - **Module Name**: The module to place this IDL into.
  - **Version**: IDL Version
  - **Import existing IDL Files**: Specifies IDL files on disk which are included in this new IDL.

Click **Finish** to complete the new project creation wizard.


> **NOTE**  
> If you upgrade to REDHAWK version 2.0, and your custom IDL project references the Core Framework (CF) IDLs, compiler errors may be displayed. To resolve the errors, create a new IDL project and import your IDL files into the new project. The new project contains an updated `Makefile.am` that accounts for changes in the REDHAWK 2.0 IDL files.  

### REDHAWK Node Project

The **New Node Project Wizard** is a one to two page wizard that walks the user through creating a new REDHAWK <abbr title="See Glossary.">node</abbr> and optionally, placing devices into the new node. Fields common to all new project wizards are defined in [Common Fields](#common-fields).

Page one of the wizard contains:

  - **Domain Manager**: Select the <abbr title="See Glossary.">Domain Manager</abbr> this node is assigned.

  - **Contents**: A REDHAWK node is defined by a Device Configuration Descriptor (DCD) file. By default, when creating a new node, the DCD file created is an empty skeleton. The new node may be based off of a previous node's DCD file by selecting it from the file system.

  - **Node ID**: Every DCD contains an ID which is used for universally unique identification. The IDE generates a DCE compliant UUID by default.

The wizard may be completed at this point by clicking **Finish** or the user may continue to the optional second page.

On page two the user chooses what devices/services make up this node. This may be changed after the node's creation from the <abbr title="See Glossary.">Node Editor</abbr>.

Select **Finish** to complete the new project creation wizard.

### REDHAWK Octave Project

The [**Octave Wizard**](octave-wizard.html) enables users to import existing Octave M-files for easy conversion into REDHAWK C++ components. The user imports an existing M-file, as well as any required dependent M-files, and then maps the M-file's inputs and outputs to REDHAWK ports and properties.

### REDHAWK Service Project

The **New Service Project Wizard** is a three page wizard that walks the user through creating a new REDHAWK <abbr title="See Glossary.">service</abbr> selecting a programming language, and selecting a code generator. Many of the fields found within the service project wizard are also found within the component and device project wizard. Fields common to all new project wizards are defined in [Common Fields](#common-fields).

Page one of the wizard contains:

  - **Service Interface**: Select from the list of installed IDLs the interface which this service uses.
  - **Contents**: A REDHAWK service is defined by an SPD file. By default, when creating a new service, the SPD file created is an empty skeleton. The new service may be based off of a previous service's SPD file by selecting it from the file system.
  - **Service ID**: Every SoftPkg element contains an ID which is used for universally unique identification. The IDE generates a DCE compliant UUID by default.

Page two of the wizard defines the service's implementation, the programming language used and the code generation template.

Wizard page two contains:

  - **Prog. Lang**: The service's implementation may be written in either C++, Java, or Python.
  - **Code Generator**: A Python code generator is available for REDHAWK service code generation. It is possible to augment the IDE with additional code generators.
  - **ID**: Each language implementation has a project unique ID,  by default the language chosen is used as the ID.
  - **Description**: A description may be provided for this implementation. The description is written into the service's SPD file.

Page three of the wizard provides customization to the configuration values of this implementation's code generation properties and template.

Wizard page three contains:

  - **Generator**: A read only label displaying the code generator being configured. This is the code generator that was chosen on the previous wizard page.
  - **Template**: The code generator may provide multiple templates to choose from. Each template may contain unique properties which may be set in the properties section.
  - **Output Directory**: The folder created to house the generated code.
  - **Properties**: Different code generation templates may provide configuration properties. Mouse hovering over the individual property provides information about the property.

Select **Finish** to complete the new project creation wizard.

## REDHAWK Shared Library Project

The [**REDHAWK Shared Library Project Wizard**](../shared-libraries/creating-a-shared-library-project.html) enables users to quickly create a C++ shared library for use in REDHAWK. In the wizard, the user specifies the project name and can then generate a simple set of code files to begin adding in library functions.

## REDHAWK Waveform Project

The **New Waveform Project Wizard** is a one to two page wizard that walks the user through creating a new REDHAWK <abbr title="See Glossary.">waveform</abbr> and optionally, assigning an <abbr title="See Glossary.">Assembly Controller</abbr>. Fields common to all new project wizards are defined in [Common Fields](#common-fields).

Page one of the wizard contains:

  - **Contents**: An waveform is defined by an Software Assembly Descriptor (SAD) file. By default, when creating a new waveform, the SAD file created is an empty skeleton. The new waveform may be based off of a previous waveform's SAD file by selecting it from the file system.
  - **REDHAWK Waveform ID**: Every SAD file contains an ID which is used for universally unique identification. The IDE generates a DCE compliant UUID by default.

The user may choose to complete the wizard at this point or continue on to the optional second page. Page two allows the user to set an Assembly Controller for the new waveform. The Assembly Controller may be set, or changed after the waveforms creation from within the Assembly Controller editor.

Click **Finish** to complete the new project creation wizard.
