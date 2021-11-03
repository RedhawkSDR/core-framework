# Component Implementations

<abbr title="See Glossary.">Components</abbr> may specify particular dependencies such as OS, processor architecture, or required <abbr title="See Glossary.">device</abbr> <abbr title="See Glossary.">properties</abbr> (e.g., processor speed or memory capacity). Setting these dependencies ensures that a component is deployed to an appropriate device at runtime.

While REDHAWK supports multiple implementations for a single component, it can be confusing, especially when debugging a system. Except for some limited scenarios, it is recommended that developers associate a single implementation with each component.
