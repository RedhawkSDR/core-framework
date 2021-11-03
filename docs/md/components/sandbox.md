# Sandbox

This section briefly describes how to use the <abbr title="See Glossary.">sandbox</abbr> either from a Python shell or via the REDHAWK IDE; a more detailed description is available in the [Sandbox](../sandbox/_index.html) chapter.

### Python Sandbox

The sandbox can be accessed directly from the command line and is used to manipulate new <abbr title="See Glossary.">components</abbr> and <abbr title="See Glossary.">waveforms</abbr>. This provides a very powerful means of testing and scripting tests for REDHAWK systems.

1. Open a Python session and import the sandbox:

    ```python
    >>> from ossie.utils import sb
    ```

1. Running a component

    To get a list of available components, type:

    ```python
    >>> sb.catalog()
    ['rh.HardLimit', 'rh.SigGen', ...]
    ```

    > **NOTE**  
    > The displayed list is derived by scanning `$SDRROOT`. `HardLimit` and `SigGen` are examples of existing components.

    Create the component object by typing:

    ```python
    >>> hardLimit = sb.launch("rh.HardLimit")
    >>> sigGen = sb.launch("rh.SigGen")
    ```

    > **NOTE**  
    > After the constructor is finished, the components run as their own processes. If an absolute file path for a component's Software Package Descriptor (SPD) file (`<component.spd.xml`) is given as a constructor argument for the component instance, then that component is started irrespective of whether or not it is present in `$SDRROOT`.

1. Support widgets are available in the sandbox to help the developer interact with running components. There are a variety of widgets, including data sources and sinks, a speaker interface, and plotters. In this example, the plot widget is used.

    To use plotting, the path to the eclipse directory of the installed IDE must be specified in the sandbox (this can also be done by setting the `RH_IDE` environment variable to the absolute path of the Eclipse directory prior to starting the Python session):

    ```python
    >>> sb.IDELocation("/path/to/ide/eclipse")
    >>> plot = sb.Plot()
    ```

    Create two plot objects by instantiating the `Plot` class twice and assigning the objects to local variables:

    ```python
    >>> plot1 = sb.Plot()
    >>> plot2 = sb.Plot()
    ```

1. Connect the components together and connect the plots. The `connect()` method tries to match the <abbr title="See Glossary.">port</abbr>; ambiguities can be resolved with the parameters `usesPortName` and `providesPortName`. Connecting the plots to the components displays the **Plot Application** window.

    ```python
    >>> sigGen.connect(hardLimit)
    >>> sigGen.connect(plot1)
    >>> hardLimit.connect(plot2)
    ```

1. In this tutorial, the `SigGen` component is configured before it is started to get a better visual. Set the frequency of the `SigGen` Component equal to *5006*:

    ```python
    >>> sigGen.frequency = 5006
    ```

1. Start everything in the sandbox:

    ```python
    >>> sb.start()
    ```

1. The <abbr title="See Glossary.">property</abbr> values can now be set on the `HardLimit` component. The plot reflects the property change:

    ```python
    >>> hardLimit.upper_limit = .8
    ```

1. To inspect the properties and input/output ports of a component, invoke the component's `api()` method:

    ```python
    >>> sigGen.api()
    ```

1.  To clean up, type `Ctrl+D` to end the Python session. The Python sandbox releases all components and cleans up whatever plots or additional widgets were created during the session.

### The IDE Sandbox

This section provides an overview of how to use the sandbox in the REDHAWK IDE. The IDE sandbox provides a graphical environment for launching, inspecting, and debugging components, <abbr title="See Glossary.">devices</abbr>, <abbr title="See Glossary.">nodes</abbr>, <abbr title="See Glossary.">services</abbr>, and waveforms. In addition, via the **<abbr title="See Glossary.">REDHAWK Console</abbr>**, you can use the Python-based sandbox API to interact with objects that are running within the IDE sandbox.

The following procedure provides an example of launching and interacting with a component in the sandbox in the REDHAWK IDE.

1. In the <abbr title="See Glossary.">REDHAWK Explorer view</abbr>:

    1. Expand the **Sandbox** to expose the **Chalkboard**.

    1. Double-click the **Chalkboard**.

      ![Open Chalkboard](images/REDHAWK_Example_HardLimit_Chalkboard_Highlighted.png)

1. From the **Chalkboard**:

    1. Select the palette on the right; drag the `SigGen (cpp)` component onto the **Chalkboard**. The component will initially be gray in color until launching is complete. When the component is finished loading, its background color is blue. `SigGen` will be used to help test the new `HardLimit` component.

    1. If the `SigGen` component is not displayed, left-click the `rh` folder to display the list of available components.

    1. If the **cpp** implementation is not displayed, expand the list of implementations by left-clicking the arrow to the left of the component name and select the **cpp** implementation.

        > **NOTE**  
        > To zoom in and out on the diagram, press and hold Ctrl; then scroll up or down. Alternatively, press and hold Ctrl; then press + or -.  

    1. Right-click the `SigGen` component, and select **Start**.

        ![Start Component](images/REDHAWK_Example_chalkboard_Start_Comp.png)

    1. Left-click the **dataFloat_out** port to select it, then right-click the port to open the port context menu, and select **Plot Port Data**.

        ![Plot Port Data](images/REDHAWK_Example_chalkboard_Plot_Port.png)

    1. Open the <abbr title="See Glossary.">Properties view</abbr> and change the following signal properties:
        - frequency: 100
        - magnitude: 10

        ![Change Property Value](images/REDHAWK_Example_chalkboard_Prop_Change.png)

    1. Select the palette on the right, drag the `HardLimit` component onto the **Chalkboard**.

    1. Right-click the `HardLimit` component, click **Start**.

    1. Connect the **dataFloat_out** port on the `SigGen` component to the **dataFloat_in** port on the `HardLimit` component by clicking and dragging from the solid black output port to the input port.

        ![Connect Ports](images/REDHAWK_Example_chalkboard_Connect_Comp.png)

    1. Right-click the **dataFloat_out** port on the `HardLimit` component, click **Plot Port Data**

    1. Notice that the output has been hard limited by the `HardLimit` component
