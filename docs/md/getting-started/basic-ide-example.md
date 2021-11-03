# Basic IDE Example

This section provides a simple example of the REDHAWK signal processing development environment, though it is far from a comprehensive guide to all of the features available in the REDHAWK IDE. While this guide assumes the user has no prior knowledge of the REDHAWK Core Framework, certain concepts and skills are required to fully understand the material.

The minimum technical requirements include:

  - Object-oriented programming experience
  - Linux/Unix experience
  - Engineering/Computer Science background
  - Understanding of basic communication theory

In addition to the minimum set of technical requirements, the following prerequisites must be met before beginning the following procedure:

  - REDHAWK Core Framework and IDE installed
  - Example components installed:
      - SigGen
      - HardLimit

## Basic IDE Use

The following sections discuss how to launch the REDHAWK IDE, how to open the Chalkboard, how to create a signal generator, and how to test the input/output response of a component.

### Launching the REDHAWK IDE

1.  Start the REDHAWK IDE with the following command:
    ```bash
    rhide
    ```

2.  If prompted to specify a workspace location, select an appropriate location and select **OK**.

### Opening the Chalkboard

From the **REDHAWK Explorer** view expand **Sandbox**, and double-click **Chalkboard**.
![Chalkboard](./images/REDHAWK_DoubleClickChalkboard.png "Chalkboard")

### Creating a Signal Generator

1.  From the **Chalkboard** **Palette**, drag the **SigGen (python)** component into the **Chalkboard** canvas.

    1.  In the **Palette**, if the **SigGen** component is not displayed, under **Components**, left-click the **rh** folder to display the list of available components.
    2.  If the Python implementation is not displayed, expand the list of implementations by left-clicking the arrow to the left of the component name. After the list is displayed, left-click the desired implementation.
    3.  When the component is finished loading, its background color is blue.

2.  Right-click the **SigGen** component, and click **Start**.

3.  Right-click the **SigGen** component, and click **Show Properties**.

4.  From the **Properties** view, change the **frequency** to 20Hz.

5.  From the **Properties** view, change the **magnitude** to 1.

6.  Right-click **SigGen**'s **dataFloat_out** port, and click **Plot Port Data**.

7.  Right-click the **SigGen** component, and click **Stop**.

### Testing the Input/Output Response of a Component

1.  From the **Chalkboard** **Palette**, drag the **HardLimit (python)** component into the **Chalkboard** canvas.

    1.  In the **Palette**, if the **HardLimit** component is not displayed, under **Components**, left-click the **rh** folder to display the list of available components.
    2.  If the Python implementation is not displayed, expand the list of implementations by left-clicking the arrow to the left of the component name. After the list is displayed, left-click the desired implementation.
    3.  When the component is finished loading, its background color is blue.
2.  Click-and-drag from **SigGen**'s **dataFloat_out** port to the **HardLimit** **dataFloat_in** port.

3.  Right-click the **SigGen** component, and click **Start**.

4.  Right-click the **HardLimit** component, and click **Start**.

5.  Right-click the **HardLimit**'s **dataFloat_out** port, and click **Plot Port Data**.

> **NOTE**  
> Two **Plot Port** views are now open, one for each of the plotted ports.  

6.  Right-click the **SigGen** component, and click **Show Properties**.

7.  From the **Properties** view, change the **magnitude** to 5.

    The **Plot Port** view for the **HardLimit dataFloat_out** port is now limiting the output to 1.
