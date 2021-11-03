# README Redhawk Docs

# How to read the docs
The docs are installed in `$OSSIEHOME/docs/`.  Point a browser to the `index.html` file there.  

# How to search the docs
The docs are originally written in `Markdown`, then converted to `html`.
The `.md` files are distributed to make it easier to search, eg with `grep`.

## Caveats
Redhawk 3.0 includes changes for which the docs have not yet been updated.
Readers should keep in mind these changes:

- change from C++ 03 to C++ 14
- change from Java 1.8 to Java 11
- change from Python 2 to Python 3
- The IDE was not completely updated (and is not planned to be).

In general, new topics are covered by new documentation, eg:

- FEI 3.0
- how to install and configure JacORB for Java 11 openjdk

However, older documentation with example code has mostly not been reviewed or updated.
