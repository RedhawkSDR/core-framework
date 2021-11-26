# README Redhawk Docs

## How to read and search the docs

### For RPM Install
Point browser to `/usr/local/redhawk/core/docs/html/index.html`.

We do not intend to polish the pdf version of the manual.  It should be sufficient to guide the user to install REDHAWK.

The docs are originally written in `Markdown`, then converted to `html`.  
The `.md` files are distributed to make it easier to search, eg with `grep`.  
Search in `/usr/local/redhawk/core/docs/md`.

### For Source Install
```
cd core-framework/docs
make
make pdf  # optional
# After OSSIEHOME is created:
make install  # optional
```

> **NOTE**:  `make install` may fail on systems with root squash.  
> Or if the user running `make` is not the owner of `OSSIEHOME`.

## Content Caveats
Redhawk 3.0 includes changes for which the docs have not yet been updated.
Readers should keep in mind these changes:

- change from C++ 03 to C++ 14
- change from Java 1.8 to Java 11
- change from Python 2 to Python 3
- The IDE was not completely updated (and is not planned to be).

In general, new topics are covered by new documentation, eg:

- FEI 3.0
- Java 11 and JacORB
- Shared Address and Shared Memory

However, older documentation with example code has mostly not been reviewed or updated.
