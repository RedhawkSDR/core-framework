# Working with Components, Devices, and Services

The <abbr title="See Glossary.">sandbox</abbr> contains the following commands for working with <abbr title="See Glossary.">components</abbr>, <abbr title="See Glossary.">devices</abbr>, and <abbr title="See Glossary.">services</abbr>:

- `show()`
- `catalog()`
- `api()`
- `launch()`

The `show()` command displays running components, connections between components, and the `SDRROOT`:

```python
>>> sb.show()
```

The `catalog()` command displays which components, devices, and services are available in `SDRROOT`. To determine what types are displayed, use the `objType` argument (by default `objType="components"`) as shown below:

```python
>>> sb.catalog()
>>> sb.catalog(objType="devices")
>>> sb.catalog(objType="services")
```
An alternative to `catalog()` is `browse()`. The function `browse()` provides formatted human-readable text that describes the items that are installed on the system rather than the computer-friendly list that `catalog()` provides.

The `api()` method displays the <abbr title="See Glossary.">ports</abbr> and <abbr title="See Glossary.">properties</abbr> for a running component:

```python
>>> comp.api()
Component [FloatToShort]:
Provides (Input) Ports ==============
Port Name       Port Interface
---------       --------------
float_in        IDL:BULKIO/dataFloat:1.0

Uses (Output) Ports ==============
Port Name       Port Interface
---------       --------------
short_out       IDL:BULKIO/dataShort:1.0

Properties ==============
Property Name   (Data Type)     [Default Value] Current Value
-------------   -----------     --------------- -------------
max_value       (float/SF/32f)  1.0             1.0
min_value       (float/SF/32f)  -1.0            -1.0
```

The `launch()` command launches components, devices, and services. The first argument identifies the object to launch. It may be either a path to an Software Package Descriptor (SPD) file or, for objects installed to the current `SDRROOT`, the name of the object as given in the SPD. The path can be absolute or relative and does not need to reside in `SDRROOT`.

The following example demonstrates how to launch a device from the current working directory using the path:

```python
>>> my_dev = sb.launch("./MyDevice.spd.xml")
```

The following example demonstrates how to launch a device named `SigGen` from the current working directory using the name:

```python
>>> my_dev = sb.launch("rh.SigGen")
```

A component's properties, either normal or with the `commandline` attribute, may be overridden at launch time by passing a dictionary of property IDs and values to the keyword argument `properties`. These values override the default values listed in the Properties File (PRF) file.

```python
>>> my_comp = sb.launch("MyComponent", properties={"EXECPARAM_1": "value",
                                                   "normalparam": "somevalue"})
```

> **NOTE**  
> The `execparams` named parameter is deprecated but may be in use by legacy systems.  Refer to the documentation for pre-REDHAWK 2.0 releases for details about its use.  

In the case of components and devices, after the process is launched and the component is initialized, the component's properties are set to their default values as listed in the PRF file. The default values for this initial call to `initializeProperties()` may be overridden by passing a Python dictionary of property names and values to the keyword argument `properties`:

```python
>>> my_comp = sb.launch("MyComponent", properties={"normalparam": "somevalue",
...                                         "frequency":22e3,
...                                         "sample_rate": 1000000})
```


> **NOTE**  
> The initial `configure()` call has been superseded by the use of the `initializeProperties()` call.  

By default, the sandbox launches the first component implementation with an entry point that exists on the file system. A particular implementation may be specified by passing the implementation ID to the `impl` argument:

```python
>>> my_comp = sb.launch("MyComponent", impl="cpp")
```

The sandbox includes limited support for attaching a debugger to a component process. The debugger console opens in a new XTerm window to allow continued interaction on the Sandbox Console.

In the case of C++, to launch a component and attach `gdb` to the process, enter the following command:

```python
>>> my_comp = sb.launch("./MyComponent.spd.xml", debugger="gdb")
```

The component and `gdb` are run in separate processes. Exiting gdb closes the window, but the component continues to function.

The debugger argument also supports `jdb` (Java), `pdb` (Python), and `valgrind` (Valgrind, a tool used for diagnostics such as memory leak detection).

To provide arguments to the supported debuggers, the debugger needs to be instantiated outside the scope of the launch function. For example, to perform a full leak check using Valgrind, use the following argument:

```python
>>> from ossie.utils.sandbox.debugger import GDB, JDB, PDB, Valgrind
>>> vg_option = {'leak-check':'full'}
>>> vg = Valgrind(**vg_option)
>>> my_comp = sb.launch("./MyComponent.spd.xml", debugger=vg)
```

If incorrect arguments are passed, the component fails to deploy. Note that the Python debugger does not take arguments.

#### Properties

In addition to the standard REDHAWK `query()` and `configure()` functions, the sandbox presents a simplified interface to properties for components and devices. Properties can be accessed as attributes on the component object:

```python
>>> my_comp.string_prop = "Hello World!"
>>> my_comp.long_prop
1
```

Property names are taken from the component PRF file, with any characters that are invalid for Python identifiers replaced by an underscore.

The current value of properties with a mode of "readonly" or "readwrite" may be inspected. Properties with a mode of "readwrite" or "writeonly" can be assigned a new value.

To view the properties that are available for a given component, along with their types and current and default values, use the `api()` function.

Simple properties with numeric types can be assigned from any Python numeric type:

```python
>>> my_comp.float_prop = 3
>>> my_comp.long_prop = 1.0e3
```

The value is range checked and coerced into the desired type before being configured on the component:

```python
>>> my_comp.ushort_prop = -1
ossie.utils.type_helpers.OutOfRangeException: ...
```

Floating point values are truncated, not rounded, during conversion to integer types:

```python
>>> my_comp.long_value = 1.5
>>> my_comp.long_value
1
```

A simple property with a complex value type can be assigned from a Python complex or two-item sequence. The numeric conversion of the real and imaginary components is identical to that of single numeric values.

```python
>>> my_comp.complex_prop = 1.0+2.5j
>>> my_comp.complex_prop = (1, 2)
```

Complex properties support assignment from single numeric values; the imaginary component is assumed to be 0.

```python
>>> my_comp.complex_prop = 1
>>> my_comp.complex_prop
1+0j
```

Properties that have enumerated values in the component's PRF support assignment using the enumerated name as a Python string:

```python
>>> siggen.shape = "triangle"
```

Struct properties can be set with a Python dictionary:

```python
>>> my_comp.struct_prop = {"item_string": "value",
...                        "item_long": 100}
```

The dictionary keys are the IDs of the simple properties that make up the struct. Each value is converted to the appropriate type following the same rules as simple properties. Any struct members that are not in the dictionary retain their current values.

Individual struct members may be set directly, using the simple property name:

```python
>>> my_comp.struct_prop.item_string = "new value"
```

> **NOTE**  
> Properties have a mode (readwrite, readonly, writeonly), and for compatibility reasons, the mode is a member of the Python Struct property container and cannot change. If the Struct property has a member called "mode", requesting the member "mode" from the property will return its access mode rather than the content of the property member. Access the value of any element of a property with a reserved word as its name as follows:
> ```python
> >>> my_comp.struct_prop["mode"] = "Hello World!"
> ```


> **NOTE**  
> Setting a struct member as an attribute uses the simple property's name, while setting the member via a dictionary uses the simple property's ID.  

Both simple and struct sequence properties may be manipulated as lists. Sequence properties support the common Python list operations, such as slicing and in-place modifiers:

```python
>>> my_comp.long_seq = [1, 2, 3, 4]
>>> my_comp.long_seq[2:]
[3, 4]
>>> my_comp.long_seq.append(5)
>>> my_comp.long_seq
[1, 2, 3, 4, 5]
>>> my_comp.long_seq[2:4] = [6]
>>> my_comp.long_seq
[1, 2, 6, 5]
```

The items of simple sequences follow the same conversion rules as the corresponding simple property:

```python
>>> my_comp.long_seq = [1.5, 2.5, 3.5]
>>> my_comp.long_seq
[1, 2, 3]
```

Each item in a struct sequence works identically to a single struct property:

```python
>>> my_comp.struct_seq
[{'a':'first', 'b':1}, {'a':'second', 'b':2}]
>>> my_comp.struct_seq[0].a = "new value"
>>> my_comp.struct_seq[1] = {"a":"third", "b":3}
>>> my_comp.struct_seq
[{'a':'new value', 'b':1}, {'a':'third', 'b':3}]
```

The sandbox generates a low-level CORBA `configure()` call each time a property value is set. However, sandbox components also support setting multiple property values at once using a Python dictionary:

```python
>>> my_comp.configure({"long_prop":1, "string_prop":"new value"})
```

The keys may be either the property names or IDs. The values are converted in the same manner as setting the individual property directly.

#### Property Listener

It is possible to asynchronously listen to changes in properties such that it is not necessary to poll the component to see the state of a particular property. This is done through property change listeners. To implement this listener, create a property change listener and register it with the component. Note that the listener can be a local object or an <abbr title="See Glossary.">event channel</abbr>.

```python
>>> def property_change_callback(self, event_id, registration_id, resource_id, properties, timestamp):
        print event_id, registration_id, resource_id, properties, timestamp
>>> listener = sb.PropertyChangeListener(changeCallbacks={'prop_1':callback_fn})
>>> comp.registerPropertyListener(listener, ['prop_1'], 0.5)  # check the property every 0.5 seconds
```
