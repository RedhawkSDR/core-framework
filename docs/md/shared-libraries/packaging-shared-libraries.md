# Packaging Shared Libraries

The REDHAWK code generators for <abbr title="See Glossary.">components</abbr> support locating and building against REDHAWK shared libraries in C++, Python, and Java. This section covers the conventions for packaging shared libraries to allow the REDHAWK build system to find, build, and run with shared libraries. The REDHAWK IDE provides a C++ shared library project type that automatically manages the build and installation of C++ libraries; however, in some cases, it may be necessary to create a REDHAWK shared library manually.

## General

A shared library has a Software Package Descriptor (SPD) but does not include a Properties File (PRF) or Software Component Descriptor (SCD).

The implementation has a localfile element but no entrypoint. The localfile points to the directory or file that contains the library.

Shared libraries are installed in `$SDRROOT/dom/deps/<library name>/`. Libraries may be namespaced; for example, a library named `rh.example` would be installed to `$SDRROOT/dom/deps/rh/example/`.

## C++ Libraries

The implementation of a C++ shared library is a `.so` file, or a directory containing multiple `.so` files. For building and linking components, C++ shared libraries also contain header files and a `.pc` file.

Header files are installed in `include/` under the top-level soft package directory. Libraries are installed in the implementation directory, typically `cpp/lib/`. The shared library must include a `.pc` file that is installed in `pkgconfig/` under the library directory (for example, `cpp/lib/pkgconfig/mylib.pc`).

The following example shows a file list for a C++ library named `mylib`:

  - `$SDRROOT/dom/deps/mylib/mylib.spd.xml`
  - `$SDRROOT/dom/deps/mylib/include/mylib.h`
  - `$SDRROOT/dom/deps/mylib/cpp/lib/libmylib.so`
  - `$SDRROOT/dom/deps/mylib/cpp/lib/pkgconfig/mylib.pc`

At runtime, the implementation directory is added to `LD_LIBRARY_PATH`.

## Python Libraries

The implementation of a Python shared library is one or more `.py` modules. The modules may be standalone, or organized into packages.

Python code is installed in the implementation directory, `python`. The library must be importable from that location.

The following example shows a file list for a Python library named `mylib`:

  - `$SDRROOT/dom/deps/mylib/mylib.spd.xml`
  - `$SDRROOT/dom/deps/mylib/python/mylib/__init__.py`
  - `$SDRROOT/dom/deps/mylib/python/mylib/utils.py`

At runtime, the implementation directory is added to `PYTHONPATH`.

## Java Libraries

The implementation of a Java shared library is a single `.jar` file.

The `.jar` file is installed in the implementation directory, `java/`.

The following example shows a file list for a Java library named `mylib`:

  - `$SDRROOT/dom/deps/mylib/mylib.spd.xml`
  - `$SDRROOT/dom/deps/mylib/java/mylib.jar`

At runtime, the `.jar` file is added to `CLASSPATH`.

## Octave Libraries

The implementation of an Octave shared library is a directory of `.m` files.

Octave code is installed in the implementation directory (typically `noarch`).

The following example shows a file list for an Octave library named `mylib`:

  - `$SDRROOT/dom/deps/mylib/mylib.spd.xml`
  - `$SDRROOT/dom/deps/mylib/noarch/func1.m`
  - `$SDRROOT/dom/deps/mylib/noarch/func2.m`

At runtime, the implementation directory is added to `OCTAVE_PATH`.
