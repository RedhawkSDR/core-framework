# Persona Device Pattern

In REDHAWK, you can manage and maintain the lifecycle of <abbr title="See Glossary.">programmable hardware</abbr>, such as FPGAs, by implementing a specific design pattern: the <abbr title="See Glossary.">Persona</abbr> Pattern. This design pattern is rooted heavily in the concept of REDHAWK <abbr title="See Glossary.">devices</abbr>, proxies used to interface physical hardware with the REDHAWK Framework.

To accurately represent the dynamic nature of programmable hardware, two unique roles have been established: the programmable role and the persona role. These roles are represented in REDHAWK as two separate REDHAWK devices: <abbr title="See Glossary.">programmable devices</abbr> and <abbr title="See Glossary.">persona devices</abbr>.

## Potential Benefits

Persona devices provide the following benefits:

  - Enables the REDHAWK Framework to manage and maintain the lifecycle of programmable hardware.
  - Ability to add/remove programmable hardware loads without the need to modify/rebuild source.
  - Reduced development time and costs.
  - Support for reuse and portability.
  - Scalability across embedded, lightweight, and heavyweight architectures.

## Theory of Operation

This section describes the recommended methods to implement programmable and persona devices in REDHAWK.

### The Programmable and Persona Roles

The programmable role and the persona role attempt to model the polymorphic behavior of programmable hardware within the REDHAWK Framework. The programmable role can be thought of as a controller that may also provide generic functionality, whereas the persona role can be thought of as a standalone representation of a single hardware load. This delineation provides two distinct areas to accurately define functionality and assign responsibilities.

#### The Programmable Role

The programmable role has a handful of responsibilities associated with it, but most important are its controller responsibilities. The programmable role has the ability to control which persona has permission to load the programmable hardware while also blocking subsequent personas from attempting to re-program loaded, running hardware. This simple controller role may also be extended to contain generic functionality that is common among all personas or that does not apply to the persona role.

#### The Persona Role

The persona role is responsible for defining the hardware load and any interfaces pertaining to that specific load. This may include register definitions and/or data IO channels. The persona role supplies the bulk of the control over the programmable hardware, representing the state of the loaded programmable hardware.

### REDHAWK Devices

The programmable and persona roles can be implemented via REDHAWK devices. Representing these roles as their own independent devices allows for more granular control and for a more precise representation of each role without needing to shoe-horn the two independent roles together into a single, complex entity.
![Relationship between Programmable Devices and Persona Devices](../images/personadiagram.png)

#### REDHAWK Programmable Device

The programmable role may be implemented in REDHAWK as a standard REDHAWK executable device. The <abbr title="See Glossary.">Device Manager</abbr> is responsible for launching this device, following the typical REDHAWK device lifecycle. What makes the programmable device unique is that it provides hooks for maintaining and managing the lifecycle of the persona devices registered to it.

REDHAWK programmable devices also implement functionality to instantiate REDHAWK persona devices from shared libraries. This is achieved by using `dlopen` to load the shared library and `dlsym` to access/instantiate the device object contained within the shared library. This mechanism occurs when the following requirements are met:

  - Programmable or persona devices are associated together using the `compositepartofdevice` tag in Device Configuration Descriptor (DCD) file.
  - Aggregate (parent) device is an executable device.
  - Composite (child) device is a shared library.

#### REDHAWK Persona Device

A REDHAWK persona device is simply the XML representation of one specific behavior that may be assigned to a programmable device. This XML representation may include unique <abbr title="See Glossary.">properties</abbr> and/or <abbr title="See Glossary.">ports</abbr> that are only relevant to the persona. Each persona device is visible in the REDHAWK <abbr title="See Glossary.">domain</abbr>, which allows for run-time configuration of persona devices in a standardized way.

REDHAWK persona devices may be built as shared objects rather than executables, therefore, allowing persona devices to exist within the same process space as a programmable device.
![Dynamic Loading of Persona Devices onto a Programmable Device](../images/persona_load.png)

### Associating Programmable/Persona Devices

Programmable devices and persona devices are linked together via the REDHAWK DCD files. Within these DCD files, the aggregate device relationship tag defines which persona devices should be associated to a programmable device. The advantage of describing this relationship within the DCD file comes with the added ability to dynamically add/remove/modify the possible personalities of programmable hardware without the need to rebuild source.

#### Aggregate Persona Device

In the Software Communications Architecture (SCA) 2.2.2, aggregate devices are defined as devices that are loosely coupled together within a parent-child relationship. The extent of this relationship is left up to the developer and the persona pattern attempts to take full advantage of this loose coupling.

Because a single piece of programmable hardware can take on many personalities, the programmable device is said to have a one-to-many relationship with its loads. The DCD file, via the aggregate relationship tags, enables users to properly define this one-to-many relationship between a single programmable device and many available persona devices.

#### DCD File Example

In the following example DCD file, the `compositepartofdevice` tag is used to associate `TestPersonaChild1` and `TestPersonaChild2` to `PersonaParent1`.

```xml
...
<componentfiles>
  <componentfile id="test_persona_parent" type="SPD">
    <localfile name="/devices/programmable/programmable.spd.xml"/>
  </componentfile>
  <componentfile id="test_persona_child" type="SPD">
    <localfile name="/devices/persona_device/persona_device.spd.xml"/>
  </componentfile>
</componentfiles>
<partitioning>
  <componentplacement>
    <componentfileref refid="test_persona_parent"/>
    <componentinstantiation id="persona_node:persona_parent_1">
      <usagename>persona_parent_1</usagename>
    </componentinstantiation>
  </componentplacement>
  <componentplacement>
    <componentfileref refid="test_persona_child"/>
    <compositepartofdevice refid="persona_node:persona_parent_1"/>
    <componentinstantiation id="persona_node:persona_child_1">
      <usagename>test_persona_child_1</usagename>
    </componentinstantiation>
  </componentplacement>
  <componentfileref refid="test_persona_child"/>
    <compositepartofdevice refid="persona_node:persona_parent_1"/>
    <componentinstantiation id="persona_node:persona_child_2">
      <usagename>test_persona_child_2</usagename>
    </componentinstantiation>
  </componentplacement>
</partitioning>
...
```

### Hardware-Accelerated Components

<abbr title="See Glossary.">Hardware-accelerated components</abbr> are REDHAWK <abbr title="See Glossary.">components</abbr> that are proxies into only a portion of hardware. An example for FPGAs is having multiple demodulator components within the fabric that all have their own register sets and data IO channels. Each individual register set and/or data IO channel may be represented within a single REDHAWK component that has a dependency on a specific behavior or persona.

The pattern used for hardware-accelerated components is very similar to the pattern used for persona devices. These components may be built as shared libraries and share state the same way programmable and persona devices share state.
![Dynamic Loading of Hardware-Accelerated Components to a Persona Device](../images/full_load.png)

## IDE and Code-Generation Support

The persona code-generation support includes templates for both the persona device and the programmable device. These templates have been set up to allow for the easy redefinition of the entry-point method to share state from device-to-device as well as device-to-component. The code generation provides the following benefits:

  - Provides C++ and Python templates for persona/programmable devices and hardware-accelerated components.
  - Allows for custom definition of the entry-point method to pass state and/or other parameters from device-to-device as well as device-to-component.
  - Provides ease of development with generic shared library functionality within base classes.
  - Manages and maintains generic functionality between device-to-device and device-to-component behavior.


### Creating a Programmable Device with the IDE and Code Generators

The following steps explain how to create a programmable device with the IDE and code generators.

1. To create a new device, in the IDE, select **File > New > REDHAWK Device Project**.  

2. In the Create a REDHAWK Device Project window, enter a project name, and under **Device**, select **Executable** and check the **Aggregate Device** checkbox.

3. Continue to specify the device in the wizard, and when you are finished, click **Finish**.

    The device project is created, and the **Overview** tab of the project is displayed.

4. Click the **Generate all Implementations** button.  

    The generated files are necessary, but not sufficient for a programmable device.  They **must** be customized in the next step.

5. In a terminal window, from the programmable device project's directory in the current IDE workspace, enter the following command:

    ```sh
    $ redhawk-codegen -f --pgdevice <device_name>.spd.xml
    ```

### Create a Persona Device with the IDE and Code Generators

The following steps explain how to create a persona device with the IDE and code generators.

1. To create a new device, in the IDE, select **File > New > REDHAWK Device Project**.  

> **NOTE**  
> You may create a persona device in any of REDHAWK's supported implementation languages.  For demonstration purposes, this procedure uses the highly likely case that the implementation language is C++.  Similarly, while a persona device may be an executable, in this procedure, it is a shared library.  

2. In the Create a REDHAWK Device Project window, enter a project name and click **Next**.

3. In the New Implementation window, in the **Prog. Lang** field, select **C++** and click **Next**.

4. Continue to specify the device in the wizard, and when you are finished, click **Finish**.

    The device project is created, and the **Overview** tab of the project is displayed.

5. Open the **Implementations** tab, and on the right half of the page, scroll down and open the **Code** section.  

6. In the **Type** selection box, select **SharedLibrary**.  

7. In the **Code** section, locate the **Entry Point** and **File** text boxes.  Initially, these are set to `cpp/<device_name>`.  Change both to `cpp/.libs/<device_name>.so`.  Optionally, the same information can be set in the SPD file:

    ```xml
      <implementation id="cpp">
        <code type="SharedLibrary">
          <localfile name="cpp/.libs/<device_name>.so"/>
          <entrypoint>cpp/.libs/<device_name>.so</entrypoint>
    ```

8. On the **Implementations** tab, click the **Generate all Implementations** button. Then, click **Yes** when prompted to save the changes, and in the Regenerate Files dialog box, click **OK**.

    The generated files are necessary, but not sufficient for a persona device.  They **must** be customized in the next step.

9. In a terminal window, navigate to the persona device project's directory in the current IDE workspace. Then, navigate to the `cpp` directory and edit the generated `.md5sums` file to remove the line containing `Makefile.am`.

10. Enter the following command:

    ```sh
    $ redhawk-codegen -f --persona ../<device_name>.spd.xml
    ```

## Persona Pattern Development

This section describes the recommended design patterns for developing programmable and persona devices.

### Programmable Device Development

Developing REDHAWK programmable devices is based heavily on interfacing with a manufacturer's driver/API and/or custom drivers/APIs. If the driver/API is to be shared, the programmable device may be responsible for the initial construction/opening of the driver/API and the destruction/closing at the end of the lifecycle. The REDHAWK programmable device must also include properties that exist on that specific hardware (for example, temperatures) and any allocation properties that may exist (for example, tuner cards using FrontEnd Interfaces (FEI) 2.0). Any persona device that attempts to reserve the hardware may configure/allocate these properties to put the device into a known usable state for that persona.

### Persona Device Development

Developing persona devices uses the code-generation support to abstract away the hooks and functionality used to associate persona devices with their parent programmable device.

Persona devices, in theory, must contain any ports and/or properties that are strictly related to the hardware personality. These ports and properties must be unique to the load and must not overlap with the programmable device. The persona device may then choose to access the driver/API as needed for operations such as reading/writing registers.

### System Developers

System developers may assign the programmable device behavior on the fly while also maintaining persona sets for a specific programmable device dynamically. The relationship between persona devices and programmable devices is defined at the <abbr title="See Glossary.">node</abbr> level within the DCD file. With programmable devices and persona devices defined, a system developer may chose to create/modify DCD files to dynamically add/remove/modify the personas that may be loaded onto the programmable hardware without the need to generate/build source.

## When to Use the Persona Pattern

Persona devices are useful for programming hardware dynamically but may not be ideal for every programmable device situation. The following cases describe some common scenarios and explain why the persona pattern may or may not be practical for the scenarios.

  - **Case 1:** You have a single FPGA that you want to always operate as X.
    - **Solution:** Develop a single REDHAWK device without persona devices. In this scenario, a single device is adequate enough to represent both the hardware and the programmable behavior of the hardware. We are essentially treating the hardware and programmable behavior as a single unit because that behavior is static.

  - **Case 2:** You have a single FPGA that you want to operate like X, Y, and Z, where X, Y, and Z do not exhibit any of the same behavior.
    - **Solution:** Develop a REDHAWK programmable device that interfaces with the hardware and develop X, Y, and Z REDHAWK persona devices that represent the unique behaviors. Using persona X, Y, and Z allows for all differences between X, Y, and Z to be represented properly while also allowing generic functionality to be retained within the programmable device.

  - **Case 3:** You have a single FPGA that you want to operate like X1, X2, and X3, where X1, X2, and X3 exhibit common behavior.
    - **Solution:** There are multiple solutions and the best solution depends on the specific use-case and whether the behaviors are similar. Some possible solutions include:
      - Solution 1: Develop a REDHAWK programmable device that interfaces with the hardware and develop a REDHAWK persona device for each behavior. This requires the most amount of effort but will yield the most amount of flexibility if the behaviors wind up not being as similar as originally expected
      - Solution 2: Develop a single REDHAWK device that interfaces with the hardware and maps the differences in X1, X2, and X3 behavior via the standard REDHAWK properties. This simplest solution can quickly yield "spaghetti" code if the house-keeping becomes more complex.
      - Solution 3: Develop a REDHAWK programmable device that interfaces with the hardware and develop a dynamic REDHAWK persona device that can be configured to represent X1, X2, and/or X3. This approach allows the REDHAWK programmable device to maintain its appropriate role while offloading the housekeeping code to the dynamic REDHAWK persona device. This option also lends itself to adding additional REDHAWK persona devices further down the road to include a new behavior that was not originally intended.

## Sharing Hardware Driver/API

When using a single driver/API, it is possible to pass a single instance of the driver/API to each persona device and/or hardware-accelerated component. The programmable device is the layer closest to the physical hardware and is typically in charge of opening/instantiating the driver/API.

The REDHAWK Framework allows for SoftPkg dependencies, packages used to distribute shared libraries/headers amongst numerous resources. The persona pattern does not mandate the use of SoftPkg, but it may be beneficial to create a REDHAWK SoftPkg dependency for the driver/API libraries/headers instead of manually including/linking headers/libraries.

Once the headers and libraries are shared, it is possible to share a single instance of the library/API by using one of the following options.

### Option 1: Modifying the `construct` Method

Because persona devices are shared objects, you can modify the entry-point method into those shared objects and append any additional arguments. The following procedure explains how to modify the `construct` method.

1.  For the programmable device, update any entry-point `typedefs` and pass in additional arguments to the entry-point method.
2.  For the persona devices, update the extern C `construct` method as follows:

```cpp
extern "C" {
  Device_impl* construct(int argc,
             char* argv[],
             Device_impl*  myParent,
             MyNewArg1Ptr* myNewArg1,
             MyNewArg2Ptr* myNewArg2){
    .
    .
    // Standard construct logic
    .
    .

    // Use myNewArg1 and myNewArg2 here
    devicePtr->setMyArg1(myNewArg1);
    devicePtr->setMyArg2(myNewArg2);
  }
}
```

### Option 2: Defining a Programmable Device Interface

The following procedure explains how to define a programmable device interface.

1.  Create a programmable device interface that exposes desired state.
2.  Include the interface with the persona devices and cast the parent reference into the interface.
3.  Update the persona device entry-point `construct` method as follows:

```cpp
extern "C" {
  Device_impl* construct(int argc,
             char* argv[],
             Device_impl* myParent) {
    .
    .
    // Standard construct logic
    .
    .

    // Cast Programmable Device into interface
    Interface myDevice = static_cast<Interface>(myParent);

    // Give Programmable Device interface to persona
    devicePtr->setMyDeviceInterface(myDevice);
  }
}
```
