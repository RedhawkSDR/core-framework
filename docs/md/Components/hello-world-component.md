# A Hello World Component

Use the following procedure to create a simple <abbr title="See Glossary.">component</abbr> that prints `hello world` to the terminal upon startup.

1.  Create a new REDHAWK component Project:
  - **File > New > REDHAWK Component Project**

1.  Name the project: *HelloWorld*

1.  Click **Next**.

1.  Select:
  - **Prog. Lang: C++**

1.  Click **Next**.

1.  Click **Finish**.
  - If a dialog asks to switch to CPP <abbr title="See Glossary.">perspective</abbr>, click **No**.

1.  Generate Code:
  - In the editor tool bar, click **Generate All Component Implementations**

1.  In the `HelloWorld.cpp` file, add the following include to the beginning of the file:
```cpp
#include <iostream>
```

1.  In the `HelloWorld.cpp` file, add the following code to the `serviceFunction()` method:
```cpp
std::cout << "Hello world" << std::endl;
```

1. Compile the project:
  - **Project > Build Project**

1. Drag the project to the <abbr title="See Glossary.">**Target SDR**</abbr> section of the <abbr title="See Glossary.">**REDHAWK Explorer**</abbr>.

1. On a terminal, start a Python session.

1. Run the following commands:
```python
>>> from ossie.utils import sb
>>> hello_world = sb.launch("HelloWorld")
>>> sb.start()
```

