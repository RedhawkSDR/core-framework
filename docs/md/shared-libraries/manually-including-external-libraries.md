# Manually Including External Libraries

Occasionally, a C++ <abbr title="See Glossary.">component</abbr> may require building and linking with a library that is not packaged as a REDHAWK shared library. This section details how to manually configure the compiler and linker flags. Two examples are given:

  - using a `pkg-config` (`.pc`) file to find and link against a library - enables your project to check for the presence of the library and issue an error while running configure if the library is not found, and enables you to avoid hard-coded options.
  - directly linking against a library - enables you to directly supply the compiler/linker flags and can be used if a `pkg-config` file is not available.

### Adding a Library by Referencing a `pkg-config` File

To add a library by referencing a `pkg-config` (`.pc`) file, edit the `configure.ac` file in your component's implementation directory.

1.  Open the `configure.ac` file in your component's implementation directory.

2.  Locate the following line referencing `PROJECTDEPS` in the code:

```cpp
PKG_CHECK_MODULES([PROJECTDEPS], [ossie >= 2.0 omniORB4 >= 4.1.0])
```

3.  Add your library to the list of requirements. For example, if you need version 1.2.3 or greater of the `foo` library:

```cpp
PKG_CHECK_MODULES([PROJECTDEPS], [
  ossie >= 2.0
  omniORB4 >= 4.1.0
  foo >= 1.2.3
])
```

4.  If your `pkg-config` file is not on the `pkg-config` path, you can augment the `pkg-config` search path by adding a line just before the call to `PKG_CHECK_MODULES`:

```bash
export PKG_CONFIG_PATH=/custom/path:$PKG_CONFIG_PATH
```

## Adding a Library Directly

To add a library, edit the `Makefile.am` file in your component's implementation directory.

1.  Open the `Makefile.am` file in your component's implementation directory.

2.  Append compiler and linker flags to the end of the CXXFLAGS and LDADD lines, respectively. For example:

    ```make
    MyComponent_CXXFLAGS += -I/usr/local/include/foo
    MyComponent_LDADD += -L/usr/local/lib64 -lfoo
    ```
