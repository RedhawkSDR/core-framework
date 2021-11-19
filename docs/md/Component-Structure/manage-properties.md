# Managing Properties

<abbr title="See Glossary.">Properties</abbr> are defined by their structure, kind, and type. The four different property structures include:

  - `simple` - single value such as `1.0`, or `"a string"`
  - `simple sequence` - list/array of zero or more simples such as `[1, 2, 3]`, or `["first", "second"]`
  - `struct` - groups several simples and simple sequences together
  - `struct sequence` - list/array of zero or more instances of a struct

Three commonly used kinds of properties in REDHAWK include:

  - `property` - denotes properties that are used for configuration and status
  - `allocation` - expresses requirements that will be satisfied by a REDHAWK <abbr title="See Glossary.">device</abbr> (For more information about allocations, refer to [Devices](../sandbox/python/devices.html), [The Allocation Manager](../runtime-environment/allocation-manager.html), and [FrontEnd Interfaces](../appendices/fei.html)).
  - `message` - used only with structs and indicates that the struct will be used as an event <abbr title="See Glossary.">message</abbr> within REDHAWK (For more information about messages, refer to [Messaging](../connections/messaging/_index.html)).

The property's type corresponds with basic programming language primitive types such as floats, long integers, booleans, etc. Additionally, numeric types can be complex.

Through the use of generated code and the REDHAWK libraries, manipulation of properties uses fundamental types provided by C++, Python, or Java, as seen in [Properties](base-component-members.html#properties).  For example, a `simple sequence`, complex-float property is manipulated via a `std::vector< std::complex<float> >` variable in C++ and a list of Python complex objects in Python. Generated <abbr title="See Glossary.">component</abbr> code provides a class data field representing each property for that component.

The primitive data types supported for `simple` and `simple sequence` properties are: `boolean`, `octet`, `float`, `double`, `short`, `ushort`, `long`, `longlong`, `ulong`, `ulonglong`, `string`, `objref`, `char`, and `utctime`. The `utctime` type is used to describe time and can be used to synchronize property change events and queries on the component or device. To set a default value for a time as a property, use a string of the form "YYYY:MM:DD::hh:mm:ss.sss" where YYYY is the year, MM is the month, DD is the day, hh is the hour (0-23), mm is the minutes, and ss.sss is the fractional seconds.

In some cases, it is desirable for the `utctime` property to be initialized to the current time. To do so, the default value (in either the component's default property value or as an overload at the <abbr title="See Glossary.">waveform</abbr> level) is set to "now", which is the time when the component is deployed. The string "now" can also be used in the Python <abbr title="See Glossary.">sandbox</abbr> to set the `utctime` property's value to the current time. Inside the component code, helpers are available to set the `utctime` property value to the current time; for example, in C++, the following code sets the property to now:

```cpp
my_prop = redhawk::time::utils::now();
```

The following primitive data types can be marked as complex values: `boolean`, `octet`, `float`, `double`, `short`, `ushort`, `long`, `longlong`, `ulong`, and `ulonglong`.

Each component implements the `CF::PropertySet` interface, which provides remote access to the component's properties through the `query()` and `configure()` methods. The `query()` method provides a means for reading a component's current property settings and the `configure()` method provides a means for setting a component's property values. Properties identified in these methods will use the property identifier value to resolve identifier access. The property `mode` attribute (`readonly`, `writeonly`, or `readwrite`) controls access to the `query()` and the `configure()` methods.

The REDHAWK library base classes provide a complete implementation of `configure()`, with the creation of specific properties handled per component by the generated base classes. Beyond the basic updating of local values, the standard `configure()` implementation provides:

  - Thread-safe updates via mutual exclusion
  - Automatic conversion of numeric types
  - Notification on changes to property values
  - External reporting of changes via events
  - Exception throwing for invalid input

Because of these enhancements, developers are strongly discouraged from overloading either the `query()` or `configure()` methods.

For more information about managing property changes and customizing `query()` and `configure()`, refer to [Property Change Listeners](#property-change-listeners) and [Customizing Query and Configure](#customizing-query-and-configure).

### Property ID

Properties are identified by ID and name. The ID must be unique to the scope of the component or device. This uniqueness applies to all properties, including the members of `struct` and `struct sequence` properties. Therefore, if two different `struct` properties in the same component each have a member with the name `abc`, both members cannot use the ID `abc`.

To eliminate ID conflicts, REDHAWK provides a naming convention that allows for multiple `struct` properties to use the same member names without creating ID conflicts. For members of a `struct`, the ID is created by combining the name of the member and the ID of the `struct`. For example, if `struct` property `foo` has a simple member `bar`, the member would have the name `bar` and ID `foo::bar`. The naming convention also applies to `struct sequence` properties as well.

### Property Name

The property name, if provided, is used for member variables in generated code and for display within the IDE. If not provided, the ID is used instead.

### Property Access

The `mode` setting applies only to properties of kind `property`. The value of `mode` (`readonly`, `writeonly`, or `readwrite`)  determines the property's ability to support `configure()` or `query()`.  


> **WARNING**  
> Do not `configure()` or `query()` properties for which the kind setting does not include `property`.  The action may have no effect, or worse, return some undefined value.  

If no value is provided for a property, it receives a default value.
Alternately, the user may set a value in the IDE, or equivalently, in the PRF.
This value (default or user-provided) can be considered to be set at component definition time.
For properties with the kind setting `property`, this initial value can be overwritten later, at component usage time, in the following:

- the Python sandbox, in the `properties` argument to the `sb.launch()` command
- a SAD file

This overwrite happens after the language-provided constructor runs and before the generated `constructor()` function runs.
Consequently, it is not recommended to access properties in the language-provided constructor.
Instead, wait until the property value overwrites are complete.
Then, access properties in the generated `constructor()` function.

### Property Change Listeners

Often, it is useful to trigger additional actions when the value of a property changes. Components support a type of notification called property change listeners that enable the developer to register callback methods that are executed whenever `configure()` is called with new values for the particular property.

Property change listeners are executed while holding the lock that protects access to all properties for the component. This ensures that no outside changes can occur while responding to property changes. The callback may alter the value of the property or call additional functions; however, avoid computationally expensive or blocking operations.

#### C++

C++ components support notification of property value changes using member function callbacks.

The following example explains how to add a property change listener for the `freqMHz` simple property of type `float`, of a component named `MyComponent`.

In `[component].h`, add a private method declaration for your callback. The callback receives two arguments, the old and new values:

```cpp
void freqMHz_changed(float oldValue, float newValue);
```

Implement the function in `[component].cpp`.

Then, in the component `constructor()`, register the change listener:

```cpp
this->addPropertyListener(freqMHz, this, &MyComponent_i::freqMHz_changed);
```

`addPropertyListener` takes three arguments: the property's member variable, the target object (typically `this`) and a pointer to a member function.

When defining a property listener for `struct` or `sequence` property, the new and old values are passed by `const` reference:

```cpp
void taps_changed(const std::vector<float>& oldValue, const std::vector<float>& newValue);
```

#### Python

Like C++, Python components allow registering listeners by property. The callback is typically a member function.

The following example explains how to add a property change listener for the `freqMHz` property.

Define the callback as a member function on your component. Excluding the implicit `self` argument, the callback receives three arguments: the property ID and the old and new values.

```python
def freqMHz_changed(self, propid, oldval, newval):
    # Perform action based on change
```

In your component `constructor()` method, register the change listener:

```python
self.addPropertyChangeListener("freqMHz", self.freqMHz_changed)
```

#### Java

Java properties support an idiomatic listener interface for responding to changes. As opposed to C++ and Python, listener registration is performed directly on the property object.

The following example explains how to add a property change listener for the `freqMHz` property of a component named `MyComponent`.

Define your callback that will respond to changes for the property as a member function on your component class. For simple numeric properties, the old and new value arguments can be the primitive type (for example, `float`):

```java
private void freqMHz_changed(float oldValue, float newValue) {
    // Perform action based on change
}
```

In your component's `constructor()` method, define an anonymous subclass of `org.ossie.properties.PropertyListener` that connects the property's change notification to your callback. For simple numeric properties, the type parameter of the `PropertyListener` class must be the boxed type (for example, `Float`).

```java
this.freqMHz.addChangeListener(new PropertyListener<Float>() {
    public void valueChanged(Float oldValue, Float newValue) {
        MyComponent.this.freqMHz_changed(oldValue, newValue);
    }
});
```

### Customizing Query and Configure


> **NOTE**  
> This feature is only available in C++.  

The REDHAWK libraries and generated component code automatically handle `query()` and `configure()` for all defined properties. However, in some cases, it may be preferable to retrieve the current value of a property in response to a `query()`, such as when fetching status from an external library. A developer may also want more control over how the property value is set. Components support per-property callbacks to customize query and configure behavior.

The query callback is called when the component receives a `query()` for that property, in lieu of consulting the local state. Likewise, the configure callback is called when the component receives a `configure()` for that property, instead of updating the component local state.


> **NOTE**  
> Unlike property listeners, the configure callback is always called regardless of whether the new value is equal to the old value.  

Query and configure callbacks are executed while holding the lock that protects access to all properties for the component. This ensures that the callback has exclusive access to the component properties. If possible, avoid computationally expensive or blocking operations to ensure that the component remains responsive.

#### C++

In C++, query and configure callbacks are registered on the components. Registering a new callback replaces the old one.

##### Query Callbacks

To create a query callback, in `[component].h`, add a private member function declaration. It takes no arguments and returns the value:

```cpp
float get_freqMHz();
```

Implement the function in `[component].cpp`.

Then, in the body of `constructor()`, register the query function:

```cpp
this->setPropertyQueryImpl(freqMHz, this, &MyComponent_i::get_freqMHz);
```

`setPropertyQueryImpl` takes three arguments: the property's member variable, the target object (typically `this`) and a pointer to a member function.

##### Configure Callbacks

To create a configure callback, in `[component].h`, add a private member function declaration. It takes one argument, the new value, and returns `void`:

```cpp
void set_freqMHz(float value);
```

Implement the function in `[component].cpp`.

Then, in the body of `constructor()`, register the configure function:

```cpp
this->setPropertyConfigureImpl(freqMHz, this, &MyComponent_i::set_freqMHz);
```

`setPropertyConfigureImpl` takes three arguments: the property's member variable, the target object (typically `this`) and a pointer to a member function.

When a configure callback is set, the member variable is not updated automatically. It is up to the component developer to update the member variable, if desired.

#### Overriding the `configure()` Method

For the vast majority of cases, the standard `configure()` implementation is sufficient. Developers are strongly discouraged from overriding `configure()`. However, in the event that additional functionality beyond what is provided is required, the overridden method should call the base class `configure()` method to ensure that the behavior expected by the library and framework is preserved. Whether the base class method is pre- or post-extended is left to the discretion of the component developer.


### Synchronization

External listeners to properties can be informed of changes in component properties by using the `registerPropertyListener` function on the component. The `registerPropertyListener` function allows an event consumer to register with the component. Upon registration, the component begins a thread that monitors the value of the requested properties. When the value of any of the monitored properties changes, an event is issued notifying the consumer what property changed on what component, when, and to what new value.

To maintain synchronization between property change events and `query` calls to the component, it is possible to add a `QUERY_TIMESTAMP` property to the query. The `QUERY_TIMESTAMP` property on the `query()` is populated with the timestamp for this `query`. The returned timestamp can be compared to asynchronously received property change events to assess what is the most recent known value for the requested property.


### Querying and Configuring Components and Devices

The previous sections explain the component developer's ability to define properties and respond to external requests to query the property value or change its value through a configure call. This section focuses on the process of invoking a query or configure call from an external source.

Properties are packed as a sequence of `CF::DataType` structures, where each `CF::DataType` structure is composed of the string element `id` and the `CORBA::Any` element `value`, forming an id/value pair for any one property. The `CORBA::Any` element is a construct that can hold any arbitrary data type (including custom-defined structures and objects); this construct holds both the value itself and information regarding the type for the value. REDHAWK struct properties are packed as nested sequences of `CF::DataType`. The outer construct is the property, and the `value` element contains a sequence of `CF::DataType` elements, one for each member of the structure.

Properties in REDHAWK are strongly typed, so the data type for the `value` element must match the type that the component or device expects for that particular property. If the wrong type is packed into the `CORBA::Any`, the property will fail to configure. For example, if a property is defined as type `long` and the value packed into the `value` element is of type `short`, then the operation will fail.

Only use `query()` and `configure()` on a property if it's kind includes 'property'.

#### Access from C++
Accessing component or device properties from a C++ program can be awkward because it requires the developer to comply with the CORBA API. To simplify the manipulation of properties, REDHAWK includes `redhawk::PropertyMap` which overlays the `std::map` API onto `CF::DataType` sequences enabling the developer to inspect, add, or remove a property from a property sequence.

Configure a value:
```cpp
include <ossie/PropertyMap.h>

CF::Properties my_props;
redhawk::PropertyMap &tmp = redhawk::PropertyMap::cast(my_props);

short num_value = 2;
std::string str_value("hello");

tmp["property_id_a"] = num_value;
tmp["property_id_b"] = str_value;

comp->configure(my_props);

short retval = tmp["property_id"].toShort();

```

Query a value:
```cpp
include <ossie/PropertyMap.h>

CF::Properties my_props;
redhawk::PropertyMap &tmp = redhawk::PropertyMap::cast(my_props);

tmp["property_id_a"] = redhawk::Value();
tmp["property_id_b"] = redhawk::Value();
comp->query(my_props);

short num_value = tmp["property_id_a"].toShort();
std::string str_value = tmp["property_id_b"].toString();

```

Additional convenience functions are declared in the header `ossie/CorbaUtils.h`. These functions make it easier to interact directly with the `CORBA::Any` type, but they have been superseded by `redhawk::PropertyMap` and are included only to maintain API compatibility with older software.

#### Access from Java

To make the interaction with the `Any` type simpler, REDHAWK includes the package `org.ossie.properties.AnyUtils`. This package includes helper functions converting primitive property types, complex values, and sequences to and from an `Any` object.

Configure a value:
```java
import CF.DataType;
import org.ossie.properties.AnyUtils;
import java.util.ArrayList;

short num_value = 2;
String str_value = "hello";

ArrayList<DataType> props = new ArrayList<DataType>();
props.add(new DataType("property_id_a", AnyUtils.toAny((Object)num_value, "short")));
props.add(new DataType("property_id_b", AnyUtils.toAny((Object)str_value, "string")));
try {
        comp.configure((DataType[])props.toArray());
} catch (CF.PropertySetPackage.PartialConfiguration|CF.PropertySetPackage.InvalidConfiguration e) {
}
```

Query a value:
```java
import org.omg.CORBA.ORB;
import CF.DataType;
import CF.UnknownProperties;
import org.ossie.properties.AnyUtils;

PropertiesHolder props_h = new PropertiesHolder();
props.add(new DataType("property_id_a", ORB.init().create_any()));
props.add(new DataType("property_id_b", ORB.init().create_any()));
props_h.value = (DataType[])props.toArray();
try {
        comp.query(props_h);
} catch (CF.UnknownProperties e) {
}
short num_value = (short) AnyUtils.convertAny(props_h.value[0].value);
String str_value = (String) AnyUtils.convertAny(props_h.value[1].value);
```

#### Access from Python

Accessing component or device properties, or any other control functionality in REDHAWK, is simplest from a Python program. The Python sandbox is an environment that presents REDHAWK as Pythonic elements, making it easy to access elements in the framework. The Python sandbox was designed to support the development of command-and-control software that performs functions like querying or configuring properties.

Configure a value:
```python
from ossie.utils import sb, redhawk
# the sb package is used to launch component instances
# the redhawk package is used to connect to a running domain

comp.property_id_a = 2
comp.property_id_b = "hello"
```

Query a value:
```python
from ossie.utils import sb, redhawk
# the sb package is used to launch component instances
# the redhawk package is used to connect to a running domain

print comp.property_id_a      # prints "2" to standard out
2

print comp.property_id_b      # prints "hello" to standard out
hello

num_value = comp.property_id_a.queryValue() # queryValue is needed to assign the value rather than the container
str_value = comp.property_id_b.queryValue() # queryValue is needed to assign the value rather than the container
```

Helper functions designed to support the development of Python software, such as `ossie.properties.props_to_dict` and `ossie.properties.props_from_dict`, are still available in compliance with REDHAWK's API support policy; however, REDHAWK does not recommend using these helper functions in any new software. Python is not strongly typed like C++ or Java, making packing values into an `Any` object error-prone. Functions such as `ossie.properties.props_from_dict` do not have enough information regarding property data types and often pack the property using the wrong native type (for example, using `double` instead of `float`), which leads to difficulty when debugging problems.
