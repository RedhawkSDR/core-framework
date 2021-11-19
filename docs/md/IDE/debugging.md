# Debugging REDHAWK Components and Devices with Eclipse

The REDHAWK IDE uses the debugging capabilities from the JDT, CDT, <abbr title="See Glossary.">PyDev</abbr>, and REDHAWK <abbr title="See Glossary.">sandbox</abbr>. The debugger provides tools to detect and diagnose errors in an <abbr title="See Glossary.">application</abbr> during execution. The debugger allows control of execution of the program by setting breakpoints, suspending launched programs, stepping through source code, and examining the contents of variables.

For more details on debugging concepts, consult the Eclipse documentation at <http://help.eclipse.org/> or view the embedded documentation from within the REDHAWK IDE by selecting **Help > Help Contents**.

In the Eclipse documentation browser refer to the following sections for additional debugging concepts and details:

  - **Java development user guide > Concepts > Debugger**
  - **Java development user guide > Concepts > Breakpoints**
  - **Java development user guide > Tasks > Running and Debugging**
  - **C/C++ Development User Guide > Concepts > Debug**
  - **C/C++ Development User Guide > Tasks > Running and debugging projects > Debugging**

### Running Unit Tests

<abbr title="See Glossary.">Component</abbr> and <abbr title="See Glossary.">device</abbr> projects created with the IDE have a `tests` folder that contains a functional example test case. This test case checks that the resource can be started, stopped, and released without error.

To run the unit test case:

1.  From the <abbr title="See Glossary.">**Project Explorer** view</abbr>, expand the project folder then the `tests` folder to display the existing test cases.
2.  Right-click the test script, select **Run As > Python unit-test**.
3.  Check the test results from the **<abbr title="See Glossary.">Console</abbr>** and **PyUnit** views.

### Running a Component or Device from the REDHAWK Sandbox

The REDHAWK sandbox provides an environment to run components and devices without the need for a <abbr title="See Glossary.">domain</abbr> or <abbr title="See Glossary.">Device Manager</abbr>. When running a component or device from the sandbox, it is started as its own forked process. A new **Console** view is created for logging and error messages.

To launch a REDHAWK component or device in the sandbox:

1.  Open the project's `spd.xml` file.
2.  From the **Overview** tab, in the **Testing** section, click **Launch a local component**.
3.  From the <abbr title="See Glossary.">REDHAWK Explorer view</abbr>, expand the **Sandbox**.
4.  To view a running component, expand or double click the **Chalkboard**. To view a running device, expand the **Device Manager**.
5.  To display the corresponding console, right-click the component or device, select **Show Console**.

#### Releasing a Component/Device from the REDHAWK Sandbox

Releasing a resource invokes a graceful shutdown, which follows the object lifecycle sequence. Release makes a call to the resource's `releaseObject()` method to initiate the sequence. There are two ways to release a component using the IDE and one to release a device:

  - Releasing using **REDHAWK Explorer** view:

    1.  In the **REDHAWK Explorer** view, expand **Sandbox > Chalkboard** to display the running component or **Sandbox > Device Manager** to display a running device.
    2.  Right-click the component/device, select **Release**.

  - Releasing using **Chalkboard** diagram:

    1.  In the **REDHAWK Explorer** view, expand the **Sandbox**, double-click **Chalkboard**.
    2.  In the **Chalkboard** diagram, right-click the REDHAWK component, select **Release**.

#### Terminating a Component from the REDHAWK Sandbox

Sometimes a REDHAWK component fails to respond to input. In these cases the component may need to be terminated. Terminate kills the processes of the resource.

To terminate a component:

  - Terminating using **REDHAWK Explorer** view:

    1.  In the **REDHAWK Explorer** view, expand the **Sandbox > Chalkboard**.
    2.  Right-click the REDHAWK component, select **Terminate**.

  - Terminating using the **Chalkboard**:

    1.  In the **REDHAWK Explorer** view, expand the **Sandbox**, double-click **Chalkboard**.
    2.  In the **Chalkboard** diagram, right-click the REDHAWK component, select **Terminate**.

  - Terminating using the **Console** view:

    1.  In the **Console** view, click the **Display Selected Console** drop down.
    2.  Select the launched REDHAWK component from the drop-down list to switch to its console.
    3.  Click the **Terminate** icon, indicated by a red square.

### Using the Debugger with the Sandbox

The REDHAWK IDE provides an infrastructure-free way to use, test, and debug a REDHAWK components. This section describes how to use the debugging features of the IDE.

#### Setting Breakpoints in Component source code

A breakpoint suspends the execution of a program at the location where the breakpoint is set. Breakpoints can be enabled and disabled via the **Breakpoints** view or from the source code editor.

To set a breakpoint from the source code editor:

1.  Open the source code file.
2.  Choose a line to set the breakpoint.
3.  Directly to the left of the line of code, in the vertical marker bar, perform one of the following actions:
      - Right-click, select **Add Breakpoint** or **Toggle Breakpoint**.
      - Double-click in the marker bar.
4.  A small solid blue (for C/C++/Java code) or green (for Python code) circle marks the breakpoint location.

#### Launching a Component in the REDHAWK Sandbox in Debug Mode

To use the REDHAWK IDE's debugger, a REDHAWK component can be launched in the sandbox in debug mode.

A component can be launched in debug mode in several ways:

  - From the Software Package Descriptor (SPD) file Editor:
    1.  Open the project's spd.xml file.
    2.  From the **Overview** tab of the <abbr title="See Glossary.">SoftPkg Editor</abbr>, in the **Testing** section, click **Debug a component in the Sandbox**.
    3.  The REDHAWK component is now launched in the sandbox in debug mode.

  - From the Project Explorer:
    1.  In the **Project Explorer** view, expand the project.
    2.  Right-click project's `spd.xml` file and select **Debug As > 1 Launch component in Sandbox**.
    3.  The REDHAWK component is now launched in the sandbox in debug mode.

There are two ways to confirm that the REDHAWK component has been launched in debug mode:

  - From the **Chalkboard** diagram, there is a small bug icon at the top right corner of the REDHAWK component.
  - From the **REDHAWK Explorer** view, expand the **Sandbox > Chalkboard** and a **\<DEBUGGING\>** decorator is displayed to the right of the REDHAWK component.

The debugger can switch between Python, C++, and Java while debugging. The REDHAWK IDE can also launch other components in the sandbox to interact with components in debug mode.

Note that there are three separate debuggers for C++, Python, and Java. They all support the basic debugging capabilities such as setting breakpoints, stepping through executing code, and viewing variables. However, some debuggers have additional features that are not available to the others. For example, the Java debugger has the ability to hot swap code as it is executing.

If there are breakpoints set and triggered, a **Confirm Perspective Switch** dialog prompts the user to open the Debug <abbr title="See Glossary.">perspective</abbr>. Clicking **Yes** rearranges the <abbr title="See Glossary.">workbench</abbr> so it is geared towards debugging source code.

The Debug perspective displays these additional views:

  - **Debug** view: The **Debug** view displays the stack frame for the suspended threads in debug mode. Each thread in the program appears as a node in the tree. It displays the process for each target that is running. Refer to **Help > Help Contents** or the Eclipse documentation for more details.
  - **Variables** view: The **Variables** view displays information about the variables associated with the stack frame selected in the **Debug** view. New values can be assigned to variables while stepping through the source code. Refer to **Help > Help Contents** or the Eclipse documentation for more details.
  - **Breakpoints** view: The **Breakpoints** view lists all the breakpoints currently set in the workspace. Double-click a breakpoint to display its location in the editor. Breakpoints can be enabled, disabled, deleted, created, grouped by working set, triggered by a user supplied hit count, triggered only in certain conditions, or triggered by exceptions. Refer to **Help > Help Contents** or the Eclipse documentation for more details.
