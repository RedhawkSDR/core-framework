# README Redhawk Docs

## How to read and search the docs

### For RPM Install
Point browser to `$OSSIEHOME/docs/html/index.html`.

Or view `$OSSIEHOME/docs/RedhawkManual-<ver>.pdf`.

The docs are originally written in `Markdown`, then converted to `html`.  
The `.md` files are distributed to make it easier to search, eg with `grep`.  
Search can be done in the pdf, or in `$OSSIEHOME/docs/md/`.

### For Source Install
```
cd core-framework/docs
make
make pdf  # optional
# After OSSIEHOME is created, during the install of `redhawk`:
make install  # optional
```

## Content Caveats
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
