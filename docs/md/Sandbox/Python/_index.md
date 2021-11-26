# Python Sandbox

The Python-based <abbr title="See Glossary.">sandbox</abbr> is a Python package that is imported as any other Python package. Included in the Python package are tools that provide a means for passing Bulk Input/Output (BulkIO) data to and from <abbr title="See Glossary.">components</abbr> or <abbr title="See Glossary.">devices</abbr>. Plotting is also supported from the Python package. This section discusses how to instantiate and test a component using the provided sandbox tools.

The sandbox is Python-centric, so its use requires some basic Python knowledge.

### Setup

In order to run the sandbox, REDHAWK must be installed and the `OSSIEHOME` and `SDRROOT` environment variables must be [set correctly](../../Install/install-from-source.html#installation-steps).

To use the sandbox *none* of the following programs need to be running: omniNames, omniEvents, <abbr title="See Glossary.">Domain Manager</abbr>, <abbr title="See Glossary.">Device Manager</abbr>, nodeBooter

The Python-based sandbox includes basic analytical plotting tools based on the matplotlib Python plotting library. To use these plots, the following Python packages must be installed:

- `matplotlib`
- `PyQt4`

### Starting the Sandbox

The Python-based sandbox, like any other Python module, may be used in another Python program or run directly within the Python interpreter. The following examples assume that the sandbox is being used within the Python interpreter.

To begin, start a Python interpreter session and import the sandbox:

```python
>>> from ossie.utils import sb
```

As with any other Python module, the built-in functions `help()` and `dir()` are useful when searching for available commands.

```python
>>> help(sb)
>>> dir(sb)
```

To exit the sandbox, press `Ctrl+D` at the Python prompt. On exit, the sandbox terminates and cleans up all of the components and helpers it creates.

### Running the Sandbox

Within the sandbox, components and helpers are launched in an idle state. In order to begin processing (e.g., `serviceFunction()` in C++ components), each object must be started.

After the desired components and helpers are created, the `start()` method starts all registered objects:

```python
>>> sb.start()
```

It is not necessary to stop the sandbox prior to exiting, as this is handled as part of normal exit cleanup. However, if you wish to stop processing, the `stop()` method stops all registered objects:

```python
>>> sb.stop()
```

In non-interactive scripts, the Python interpreter exits after the last statement. This may be undesirable, such as in the case where a set of components should run for an arbitrary amount of time, with their output monitored via plots. The built-in Python method `raw_input()` can be used to prevent the interpreter from exiting:

```python
>>> raw_input()
```


> **NOTE**  
> When using plots, calling `raw_input()` instead of `time.sleep()` allows the UI thread to continue updating.  

Items launched in the sandbox are registered in the sandbox's internal state. To view the items that are deployed in the sandbox, use the `show` command:

```python
>>> sb.show()
```

The items shown by the `show` command are referenced by a unique name. To recover an item by its sandbox internal name, use the `getComponent` function:

```python
>>> comp = sb.getComponent("name")
```

#### Connecting to a Running Domain

The Python sandbox allows a developer to not only launch components, but it also allows one to interact with a live Domain. This is accomplished through the [redhawk package](../../Runtime-Environment/inspection.html#redhawk-module).
