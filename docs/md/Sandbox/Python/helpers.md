# Helpers

The Python <abbr title="See Glossary.">sandbox</abbr> provides a variety of helpers to both simplify the interaction with REDHAWK subsystems as well as increase the reliability of interactions with these subsystems.

 A common problem is type matching in <abbr title="See Glossary.">properties</abbr>. While Python is very flexible with types, other languages, like C++ and Java, are not. When properties are set on <abbr title="See Glossary.">components</abbr>, the type for the value has to match the type that is expected by the component. Python, given its dynamic type system, will pack the value in what it deems appropriate, which may or may not match the expected type. While the property mapping performs this matching automatically, it is sometimes desirable to create a set of properties for other systemic uses. For example, it may be desirable to use the <abbr title="See Glossary.">Allocation Manager</abbr>, making it impossible for the Python script to know which <abbr title="See Glossary.">device</abbr> will satisfy the `allocateCapacity` call. To support this need, the `createProps` function was created. `createProps` is given a dictionary of the required properties, and it also takes an optional Properties File (PRF). Using the PRF, the property dictionary is converted to the appropriate format, where the filename is the location on the local filesystem.

 The following example demonstrates how to use the `createProps` function with an existing PRF  to generate a set of properties.

 ```python
  >>> from ossie.utils import allocations
  >>> prop = allocations.createProps({'some_prop':[1.0,2.0]}, prf='/var/tmp/sdr/dev/devices/my_dev/my_dev.prf.xml')
  ```
