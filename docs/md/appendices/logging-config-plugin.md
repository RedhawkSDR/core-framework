# Logging Configuration Plugin

The <abbr title="See Glossary.">Domain Manager</abbr> may be extended to use a loadable library that can assist in the resolution of the `LOGGING_CONFIG_URI` parameter during <abbr title="See Glossary.">application</abbr> deployments. The following code and build files provide a template to build the loadable library: `libossielogcfg.so`.

The library should be installed in `$OSSIEHOME/lib64` or `$OSSIEHOME/lib` depending on your hardware and operating system. If you choose to install the library in a different directory, you will need to add this path to `LD_LIBRARY_PATH` before starting the DomainManager. To enable this feature in the DomainManager, use the `--useloglib` option when launching the `nodeBooter` program.

```bash
nodeBooter -D --useloglib
```

### `LogConfigUriResolver` Class and Example

The `LogConfigUriResolver` class is the base class that your customized class will inherit. The class contains a single method `get_uri`, that will be used to resolve the logging configuration file location during deployment. The method accepts a single parameter `path` that describes the resource's path in the <abbr title="See Glossary.">Domain</abbr>. The following list describes the different resource paths:

- <abbr title="See Glossary.">Component</abbr>
  - syntax
  ```
  rsc:<DomainName>/<Application ID>/<Component's Naming Service Name>
  ```

  - example
  ```
  rsc:REDHAWK_DEV/TestWave_2/MyComp_4
  ```

- <abbr title="See Glossary.">Device</abbr>
  - syntax
  ```
  dev:<DomainName>/<Node ID>/<Device's Naming Service Name>
  ```

  - example
  ```
  dev:REDHAWK_DEV/Node_1/MyDevice_4
  ```

- <abbr title="See Glossary.">Service</abbr>
  - syntax
  ```
  svc:<DomainName>/<Node ID>/<Service's Naming Service Name>
  ```

  - example
  ```
  svc:REDHAWK_DEV/Node_1/MyRedis_3
  ```

Using this path, the customized code should return the location of a logging configuration file or an empty string (use current default resolution method). Logging configuration file locations should be formatted as follows:

  - `sca://path/to/config/file`
  ```
   sca://logcfg/comp.log.cfg
  ```

  - `file:///absolute/path/to/config/file`
  ```
  file:///var/redhawk/sdr/dom/logcfg/comp.log.cfg
  ```

The following example code creates a custom resolver class that provides logging configuration files from the local `SDRROOT` directory (file:///var/redhawk/sdr/dom/logcfg/device.log.cfg). The macro `MAKE_FACTORY` allows the class to be dynamically loaded by the Domain Manager.

```cpp

#include <ossie/logging/LogConfigUriResolver.h>
#include <iostream>
#include <sstream>

class CustomLogConfigResolver : public ossie::logging::LogConfigUriResolver {

public:

  CustomLogConfigResolver() {};

  virtual ~CustomLogConfigResolver(){};

  /**
    get_uri

    Return a string object that will be passed to a resource on the command line as
    LOGGING_CONFIG_URI parameter. An empty string will be ignored by the DomainManager.

    @param path  Path of the resource in the domain.
                 for components, rsc:<domain name>/<application name>/<component Naming Service name>
                 e.g.            rsc:REDHAWK_DEV/TestWave_2/MyComp_4

                 for devices, dev:<domain name>/<node name>/<device's Naming Service name>
                 e.g.            dev:REDHAWK_DEV/Node_1/MyDevice_4

                 for service, svc:<domain name>/<node name>/<service's Naming Service name>
                 e.g.            svc:REDHAWK_DEV/Node_1/MyRedis_1
  */

  std::string get_uri( const std::string &path ) {

    std::string sdrroot("");
    if ( ::getenv("SDRROOT")){
      sdrroot = ::getenv("SDRROOT");
    }

    if ( path.find("dev:") != std::string::npos ) {
      std::ostringstream os;
      os << "file://" << sdrroot << "/dom/logcfg/device.log.cfg";
      return std::string(os.str());
    }
    if ( path.find("svc:") != std::string::npos ) {
      std::ostringstream os;
      os << "file://" << sdrroot << "/dom/logcfg/serviceq.log.cfg";
      return std::string(os.str());
    }

    if ( path.find("rsc:") != std::string::npos ) {
      std::ostringstream os;
      os << "file://" << sdrroot << "/dom/logcfg/comp.log.cfg";
      return std::string(os.str());
    }
    // an empty string return value will be ignored by the DomainManager
    return std::string("");
  };

};

MAKE_FACTORY(CustomLogConfigResolver);
```

## Build Files

Use the following build files to compile and build the above example code in a file called `ossielogcfg.cpp`. The compiled code produces a library called `libossielogcfg.so` that is installed in `$OSSIEHOME/lib` or `$OSSIEHOME/lib64`. The build process follows the same paradigm as standard REDHAWK generated software: `reconf; configure; make install`.

### `reconf`

Save the following code to a file called `reconf`, and make the file executable.

```bash
#!/bin/sh

rm -f config.cache
[ -d m4 ] || mkdir m4
autoreconf -i
```

### `configure.ac`

Save the following code to a file called `configure.ac`.

```bash
AC_INIT(ossielogcfg, 1.0.0)
AM_INIT_AUTOMAKE([nostdinc foreign])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AC_CORBA_ORB
OSSIE_CHECK_OSSIE
# TODO: Make this an installed macro
OSSIE_SDRROOT_AS_PREFIX
prefix="${OSSIEHOME}"
libdir="${OSSIEHOME}/lib"
AS_IF( [ test `uname -i` == "x86_64"  ], [ libdir="$prefix/lib64" ], [ libdir="$prefix/lib" ] )

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Dependencies
PKG_CHECK_MODULES([REDHAWK], [ossie >= 2.0])
OSSIE_ENABLE_LOG4CXX
AX_BOOST_BASE([1.41])
AX_BOOST_SYSTEM
AX_BOOST_THREAD
AX_BOOST_REGEX

AC_CONFIG_FILES([Makefile ])
AC_OUTPUT
```

### `Makefile.am`

Save the following code to a file called `Makefile.am`.


> **NOTE**  
> Remember to replace the spaces before `rm -rf m4` with a tab to prevent the `Makefile.am` file from crashing.  

```bash
ACLOCAL_AMFLAGS = -I m4 -I${OSSIEHOME}/share/aclocal/ossie
AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libossielogcfg.la

xmldir = $(prefix)
dist_xml_DATA =

distclean-local:
    rm -rf m4

libossielogcfg_la_SOURCES = ossielogcfg.cpp
libossielogcfg_la_LIBADD = $(REDHAWK_LIBS)
libossielogcfg_la_CPPFLAGS = -I . -I $(srcdir)/include $(REDHAWK_CFLAGS) $(BOOST_CPPFLAGS)
libossielogcfg_la_CXXFLAGS = -Wall
```
