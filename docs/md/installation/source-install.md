# Build and Install from Source

## Building the Framework

This section describes how to build and install REDHAWK from source and use the environment variables to run REDHAWK.

### Installing Build Dependencies

Building REDHAWK from source requires a few additional dependencies beyond those required to run REDHAWK.  The following procedure explains how to install the additional build dependencies.

1.  First, ensure your system has the necessary [dependency software](dependencies.html) provided by RHEL / CentOS and Fedora EPEL.
2.  Ensure the REDHAWK Yum repository is set up using the process described in [Setting Up the REDHAWK Repostiory](../installation/_index.html#setting-up-the-redhawk-repository).  Install the [dependencies](../appendices/redhawk-yum.html#dependencies-packaged-with-redhawk) distributed with the REDHAWK tarball.

### Installing the Framework from Source

#### Compilation Considerations

This section discusses two compiler considerations for REDHAWK:  (1) C++ language standard support, and (2) development tools setup.

##### C++ Language Standard Support

To ensure compatibility, it is important that your compiler has complete support for your C++ language standard.  For example, `g++` version 4.x.x has complete support for C++98/C++03, while `g++` version 5.x.x has complete support for C++11.  If object A was compiled with `g++` 4.8.5 at `-std=gnu++03` and object B was compiled with `g++` 5.1.0 at `-std=gnu++11`, then these objects are ABI compatible and can be linked.

As a counter-example, `g++` version 4.x.x does not have complete support for C++11.  If object A was compiled with `g++` 4.8.5 at `-std=gnu++11` and object B was compiled with `g++` 5.1.0 at `-std=gnu++11`, then these objects are ABI incompatible and should not be linked.

To determine whether a version of `g++` has complete support for a language standard, look [here](https://gcc.gnu.org/onlinedocs/libstdc++/manual/api.html).

REDHAWK is developed and tested using the C++14 standard and may not compile in C++17 or newer modes.  To configure GCC to use the C++14 standard, set the `CXXFLAGS` environment variable before building REDHAWK from source:
```bash
export CXXFLAGS="--std=gnu++14"
```

##### Development Tools Setup

REDHAWK supports CentOS 7, for which the default C++ compiler is `g++` version 4.x.x, with complete support for C++98/C++03, but not for later versions.  REDHAWK requires a compiler with complete support for the C++14 standard.  For CentOS 7, that is provided by Software Collections (SCL), in the `devtoolset-<ver>-gcc-c++` rpm package.

The `devtoolset-<ver>-gcc-c++` rpm package installs to the `/opt/` directory a set of GNU software development tools such as `g++`, `gdb`, `ld`, and `autoconf`.  The use of these tools is enabled by prepending several environment PATH-like variable with `/opt/rh/devtoolset-<ver>`.  After completing the `./redhawk-install.sh` command below, these commands will be available to enable and disable them:
```bash
. $OSSIEHOME/bin/redhawk-devtoolset-enable.sh
. $OSSIEHOME/bin/redhawk-devtoolset-disable.sh
```

After that, if you want these tools enabled all the time, you can do this:
```bash
sudo cp $OSSIEHOME/bin/redhawk-devtoolset-enable.sh /etc/profile.d/
```

#### Installation Steps

To install the Core Framework (CF) from source, the `redhawk-src-<version>.tar.gz` must be downloaded.

```bash
wget https://github.com/RedhawkSDR/redhawk/releases/download/<version>/redhawk-src-<version>.tar.gz
```

You must set the `OSSIEHOME` and `SDRROOT` environment variables (recommended defaults shown below) before running the installation script. You must have write permission for the locations of `OSSIEHOME` and `SDRROOT` or the installation will not work.

To compile the source, execute the following commands:

```bash
export OSSIEHOME=/usr/local/redhawk/core
export SDRROOT=/var/redhawk/sdr
tar zxvf redhawk-src-<version>.tar.gz
cd redhawk-src-<version>/
./redhawk-install.sh
. $OSSIEHOME/redhawk-devtoolset-disable.sh  # optional
. $OSSIEHOME/environment-setup
```

> **NOTE**  
> Part of the environment used to compile the source is needed at runtime.  To enable that environment at each login, add the following lines to `.bashrc`:  
> ```bash
> export OSSIEHOME=/usr/local/redhawk/core
> export SDRROOT=/var/redhawk/sdr
> . $OSSIEHOME/environment-setup
> ```

To build the source code with or without optional features, provide the appropriate build option to the `configure` setup. The following table describes some common options.

##### Common Build Options for the `Configure` Command
| **Option**   | **Description** |
| :-------------- | :-------- |
| `--disable-affinity` | Affinity processing is enabled by default. This option will disable <abbr title="See Glossary.">NUMA</abbr> affinity processing.  |
| `--enable-persistence=<type>` | Enable persistence support. Supported types: `bdb`, `gdbm`, `sqlite`.  |
| `--disable-persistence` | Disable persistence support. This may be desired for specialized builds to eliminate the additional database dependency.  |
| `--disable-log4cxx`  | Disable log4cxx support.   |

> **NOTE**  
> As of REDHAWK 2.2.1, the default setting is to enable persistence using `sqlite`.  
> It is only necessary to specify `--enable-persistence` to use a different backend database.  
> The default setting from prior versions is selectable with `--disable-persistence`.

To view a complete list of configurations, enter the following commands:

```bash
cd <redhawk src directory>
cd redhawk/src
./reconf
./configure --help
```
To provide any of the build options, edit the `redhawk-install.sh` script and change the following line to include the appropriate option:

```bash
./configure
```

### Setting Environment Variables

REDHAWK expects several environment variables to be set to run. REDHAWK installs a set of scripts that appropriately set these variables in the `etc/profile.d` directory in your installation. Source the appropriate files for your shell before running. For example, if you installed to the default `OSSIEHOME` location (`/usr/local/redhawk/core`) and are using bash/dash:

```bash
. /usr/local/redhawk/core/etc/profile.d/redhawk.sh
. /usr/local/redhawk/core/etc/profile.d/redhawk-sdrroot.sh
```

or copy them to your system's `/etc/profile.d` directory to make them global for all users:

```bash
sudo cp /usr/local/redhawk/core/etc/profile.d/* /etc/profile.d
```


> **NOTE**  
> Remember to restart your terminal if you modify the system's `/etc/profile.d` directory for changes to take effect.  

### Configuring omniORB

Refer to [Configuring omniORB](../installation/_index.html#configuring-omniorb) for information on how to edit the omniORB configuration file (`/etc/omniORB.cfg`) to provide information about how to reach the CORBA <abbr title="See Glossary.">Event Service</abbr>.
