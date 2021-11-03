# Components

A <abbr title="See Glossary.">component</abbr> is a modular building block that can be inserted into any number of signal processing <abbr title="See Glossary.">applications</abbr> to perform a specific and reusable function. A component is fully defined by its interfaces, <abbr title="See Glossary.">properties</abbr>, and functionality. Examples include a component that tunes, filters, and decimates a wideband signal and one that performs a FM demodulation. Some components inevitably need to be custom implementations, but a majority of signal processing functions can be reused and shared.

REDHAWK allows for developers to create components in either C++, Python, or Java. C++ is recommended for most computationally-intensive tasks, whereas Python or Java work well for handling metadata manipulation or command and control tasks.

Components can be interconnected together within a <abbr title="See Glossary.">waveform</abbr> to create a complete signal processing application or can be run independently in the REDHAWK <abbr title="See Glossary.">sandbox</abbr> to perform trivial tasks on a local host. The figure below depicts the composition of components into a waveform.

![REDHAWK Workflow](images/REDHAWK_Component_Workflow_Graphic.png)

By using the REDHAWK Framework, basic processing elements may be encapsulated as components and reused by other REDHAWK compliant systems. Using the REDHAWK IDE and the included code generators, much of the code for control and input/output can be auto-generated. The figure below depicts the encapsulation of an arbitrary processing algorithm into an auto-generated REDHAWK component wrapper.

![Components](images/REDHAWK_Component_Container_Graphic.png)

