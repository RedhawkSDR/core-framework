# Basic Sandbox Example

The fastest and easiest way to get a REDHAWK application started is through the REDHAWK sandbox, which is a Python package.  It runs REDHAWK components outside the runtime environment, which limits the components to a single host computer.

> **NOTE**:  REDHAWK 3 switched to Python 3 from Python 2.

You can use the sandbox in a Python 3 script or session (shown below).  First, start a Python 3 session from the command line:
```bash
python3
>>>
```

Then, import the sandbox module:
```python
>>> from ossie.utils import sb
```

The sandbox has commands to list components available for use and to create an instance of a component by passing its name as an argument:
```python
>>> sb.catalog()
```

- output:

    ```python
    ['rh.HardLimit', 'rh.SigGen']
    ```

```python
>>> sigGen = sb.launch("rh.SigGen")
>>> hardLimit = sb.launch("rh.HardLimit")
```

The REDHAWK IDE plotting tool can be used in the sandbox to display data graphically. The path to the Eclipse directory of the installed IDE must be specified in the sandbox (this can also be done by setting the RH_IDE environment variable to the REDHAWK path prior to starting the python session):

```python
>>> sb.IDELocation("/path/to/ide/eclipse")
>>> plot = sb.Plot()
```

The two components need to be connected, and HardLimit must be connected to the plotter to display the results.

```python
>>> sigGen.connect(hardLimit)
>>> hardLimit.connect(plot)
```

Once the HardLimit component is connected to the plotter, a window appears that plots any data coming from the output port. Once the sandbox is started, the plot begins to display data:

```python
>>> sb.start()
```

Component properties can be modified as attributes of the component. The HardLimit property upper_limit can be changed to set an upper limit on the data that is displayed:

```python
>>> hardLimit.upper_limit = .8
```

For further reading on application development, the component and sandbox chapters of the REDHAWK manual provide a more thorough description of the makeup of components and how to test them in the REDHAWK environment.
