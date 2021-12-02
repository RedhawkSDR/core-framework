# Install from Source

This section describes how to build and install REDHAWK from source and use the environment variables to run REDHAWK.

The overall process includes:
- Install [external dependencies](external-dependencies.html).
- Setup the [REDHAWK YUM repository](redhawk-yum-repo.html) and install included dependencies.
- Build and install REDHAWK software (below).
- Setup the user environment (below).

> **NOTE**:  Building REDHAWK from source requires external dependencies beyond those required to run REDHAWK.

## Background

### C++ Language Standard Support

See the section on [C++](cpp.html).

### Java Version

See the section on [Java and JacORB](java-and-jacorb.html).

## Installation Steps

To install the Core Framework (CF) from source, download `redhawk-src-<version>.tar.gz`.

```bash
wget https://github.com/RedhawkSDR/redhawk/releases/download/<version>/redhawk-src-<version>.tar.gz
```

Set the `OSSIEHOME` and `SDRROOT` environment variables (recommended defaults shown below) before running the installation script. You must have write permission for the locations of `OSSIEHOME` and `SDRROOT` or the installation will not work.

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

> **NOTE**:  Part of the environment used to compile the source is needed at runtime.  To enable that environment at each login, add the following lines to `~/.bashrc`:  
> ```bash
> export OSSIEHOME=/usr/local/redhawk/core
> export SDRROOT=/var/redhawk/sdr
> . $OSSIEHOME/environment-setup
> ```

To build the source code with or without optional features, provide the appropriate build option to the `configure` setup. The following table describes some common options.

### Common `configure` Options

| **Option**   | **Description** |
| :---- | :-------- |
| `--disable-affinity` | Affinity processing is enabled by default. This option will disable <abbr title="See Glossary.">NUMA</abbr> affinity processing.  |
| `--enable-persistence=<type>` | Enable persistence support. Supported types: `bdb`, `gdbm`, `sqlite`.  |
| `--disable-persistence` | Disable persistence support. This may be desired for specialized builds to eliminate the additional database dependency.  |
| `--disable-log4cxx`  | Disable log4cxx support.   |

> **NOTE**:  As of REDHAWK 2.2.1, the default setting is to enable persistence using `sqlite`.  
> It is only necessary to specify `--enable-persistence` to use a different backend database.  
> The default setting from prior versions is selectable with `--disable-persistence`.

To view a complete list of configurations, enter the following commands:

```bash
cd <core-framework source directory>
cd redhawk/src
./reconf
./configure --help
```
To provide any of the build options, edit the `redhawk-install.sh` script and change the following line to include the appropriate option:

```bash
./configure
```

## Environment Variables

REDHAWK expects several environment variables to be set to run. REDHAWK installs a set of scripts that appropriately set these variables in the `etc/profile.d` directory in your installation. Source the appropriate files for your shell before running. For example, if you installed to the default `OSSIEHOME` location (`/usr/local/redhawk/core`) and are using bash/dash:

```bash
. /usr/local/redhawk/core/etc/profile.d/redhawk.sh
. /usr/local/redhawk/core/etc/profile.d/redhawk-sdrroot.sh
```
or copy them to your system's `/etc/profile.d` directory to make them global for all users:
```bash
sudo cp /usr/local/redhawk/core/etc/profile.d/* /etc/profile.d
```

> **NOTE**:  Remember to restart your terminal if you modify the system's `/etc/profile.d` directory for changes to take effect.  

## Configure omniORB

Refer to [Configuring omniORB](install-from-rpm.html#configure-omniorb) for information on how to edit the omniORB configuration file (`/etc/omniORB.cfg`) to provide information about how to reach the CORBA <abbr title="See Glossary.">Event Service</abbr>.
