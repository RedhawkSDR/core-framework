# IDE

The REDHAWK IDE is a tool that enables developers to create, test, and deploy software for REDHAWK systems. This tool is built on Eclipse, which is a generic, extensible IDE that allows developers to add custom modules. The REDHAWK IDE, therefore, leverages all of the features available in a bare-bones Eclipse IDE while also providing customized features specific to REDHAWK development.

The following features are included in the REDHAWK IDE:

  - Eclipse - <http://eclipse.org/>
  - Eclipse JDT - Java Development Tooling for Eclipse: <http://www.eclipse.org/jdt/>
  - Eclipse CDT - C++ Development Tooling for Eclipse. Includes tools for autoconf, rpm spec file editing, and many debugging tools: <http://www.eclipse.org/cdt/>
  - Eclipse CORBA - Interface Description Language (IDL) Editor for Eclipse: <http://eclipsecorba.sourceforge.net/>
  - PyDev - Python Development Tooling for Eclipse: <http://pydev.org/>
  - Eclipse Git - Git Tooling for Eclipse: <https://www.eclipse.org/egit/>

For signal processing developers, the REDHAWK IDE provides graphical interfaces for the auto-generation of <abbr title="See Glossary.">component</abbr>, <abbr title="See Glossary.">device</abbr>, and <abbr title="See Glossary.">service</abbr> code in C++, Python, and Java. Built on top of the Eclipse JDT, Eclipse <abbr title="See Glossary.">PyDev</abbr>, and Eclipse CDT, the IDE provides feature-rich editors for code written in any of the three languages. In addition, a drag-and-drop environment for testing these modules and constructing them into <abbr title="See Glossary.">waveforms</abbr> and <abbr title="See Glossary.">nodes</abbr> is available.

For system developers, the IDE provides an environment for deploying and maintaining <abbr title="See Glossary.">applications</abbr> onto fielded systems. Furthermore, the pluggable nature of Eclipse allows system developers to add customized system user-interface modules.


> **NOTE**  
> The <abbr title="See Glossary.">REDHAWK Explorer</abbr> is a lighter-weight application that includes a subset of the REDHAWK IDE's functionality. The REDHAWK Explorer can be used to connect to a running REDHAWK <abbr title="See Glossary.">domain</abbr>, navigate the contents, launch and view waveforms, and so forth. It provides no development functionality, only runtime interaction with REDHAWK domains.  

