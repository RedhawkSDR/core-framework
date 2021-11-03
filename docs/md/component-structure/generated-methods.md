# Generated Component Methods

This section provides an overview of noteworthy methods provided in the auto-generated <abbr title="See Glossary.">component</abbr> files. In some cases, the names of the methods vary by language.

### `serviceFunction()`

The core functionality of a component resides in the `serviceFunction()` method in C++, the `process()` method in Python, and the `serviceFunction()` method in Java. The `serviceFunction()` is invoked on a recurring basis after `start()` is called on the component's base class.


> **NOTE**  
> When `serviceFunction` returns `NORMAL`, it will immediately loop back. In Java, if objects are created in the `serviceFunction` and no blocking calls are made, garbage collection will be deferred until no more heap space is available. Under these conditions, there are likely to be substantial CPU and JVM memory utilization issues.  

### `constructor()`

This is the component/<abbr title="See Glossary.">device</abbr> constructor. When this function is invoked, <abbr title="See Glossary.">properties</abbr> of kind property are initialized to their default or overloaded state.
