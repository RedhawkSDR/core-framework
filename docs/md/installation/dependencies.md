# External Dependencies

The following sections explain how to install the dependencies from the Fedora Extra Packages for Enterpise Linux (EPEL) and Red Hat/CentOS repositories. Dependencies not available from either of those sources are [included with REDHAWK](redhawk-yum.html#dependencies-packaged-with-redhawk).

## Installing the EPEL Repository

This section explains how to install the EPEL repository.


> **NOTE**  
> For more information on the Fedora EPEL project, refer to <http://fedoraproject.org/wiki/EPEL>.  

### From the Fedora Downloads Site

Install the EPEL repository on your system from the Fedora downloads site.

For RHEL/CentOS 7:

```bash
sudo yum install https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
```

## Installing the Software Collections (SCL) Repository

This section explains how to install the SCL repository.

The REDHAWK Framework is compiled with C++14, which the default compiler on CentOS 7 (g++ 4.x.x) does not support.  REDHAWK supports C++14 through the SCL repository, which provides `devtoolset-<ver>`.


> **NOTE**  
> For more information on the SCL project, refer to <https://www.softwarecollections.org/en/>.  

Once the EPEL repository is installed, the following command will install the SCL repository.

For RHEL/CentOS 7:

```bash
sudo yum install centos-release-scl
```

## Runtime-only Dependencies

The following dependencies are required for REDHAWK runtime installations.

### Dependencies for RHEL/CentOS 7

The following runtime-only dependencies are required for RHEL/CentOS 7 systems.

  - `gstreamer-python`
  - `gstreamer1-plugins-base`
  - `python-matplotlib-qt4`
  - `log4cxx`
  - `util-linux`
  - `numpy`
  - `python36-jinja2`
  - `binutils`
  - `numactl`
  - `numactl-libs`
  - `sqlite`
  - `bash-completion`

To install the dependencies for RHEL/CentOS 7, enter the following commands:

```bash
sudo yum install gstreamer-python \
     gstreamer1-plugins-base \
     python-matplotlib-qt4 \
     log4cxx \
     util-linux \
     numpy \
     python36-jinja2 \
     binutils \
     numactl \
     numactl-libs \
     sqlite \
     bash-completion
```

## Dependencies for Development and Building from Source

The following dependencies are required for development with the REDHAWK Framework and building REDHAWK from source.

### Dependencies for RHEL/CentOS 7

The following dependencies are required for development on RHEL/CentOS 7 with the REDHAWK Framework and building REDHAWK from source.

  - `gstreamer-python`
  - `libuuid-devel`
  - `boost-devel`
  - `cppunit-devel`
  - `autoconf automake libtool`
  - `expat-devel`
  - `gcc-c++`
  - `java-1.8.0-openjdk-devel`
  - `junit4`
  - `log4cxx-devel`
  - `python-devel`
  - `python-setuptools`
  - `PyQt4`
  - `python36-jinja2`
  - `xsd`
  - `uhd`
  - `uhd-devel`
  - `uhd-doc`
  - `uhd-firmware`
  - `numactl-devel`
  - `sqlite-devel`
  - `autoconf-archive`
  - `devtoolset-9-gcc-c++`
  - `yaml-cpp-devel`

To install the dependencies above for RHEL/CentOS 7, ensure the SCL repository is installed, then enter the following command:

```bash
sudo yum install gstreamer-python \
     libuuid-devel \
     boost-devel \
     cppunit-devel \
     autoconf automake libtool \
     expat-devel \
     gcc-c++ \
     java-1.8.0-openjdk-devel \
     junit4 \
     log4cxx-devel \
     python-devel \
     python-setuptools \
     PyQt4 \
     python36-jinja2 \
     xsd \
     uhd \
     uhd-devel \
     uhd-doc \
     uhd-firmware \
     numactl-devel \
     sqlite-devel \
     autoconf-archive \
     devtoolset-9-gcc-c++ \
     yaml-cpp-devel
```


> **NOTE**  
> The `CentOS-SCLo-scl-rh.repo` yum repository is required to install `devtoolset-9-gcc-c++`  

### Optional Dependencies for Development

The following dependencies are required for Octave <abbr title="See Glossary.">component</abbr> development.

  - `octave-devel`

To install the dependencies, enter the following command:

```bash
sudo yum install octave-devel
```
