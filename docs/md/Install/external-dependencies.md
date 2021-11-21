# External Dependencies

This section explains how to install the dependencies that come from Redhat related sources such as (1) the Redhat/CentOS repositories, (2) the Fedora Extra Packages for Enterpise Linux ([EPEL](http://fedoraproject.org/wiki/EPEL)) repositories, and (3) the Redhat Software Collections ([SCL](https://www.softwarecollections.org/en/)) repository.

There are other dependencies that are not available from those sources because they are created or modified specifically for REDHAWK.  To install those see the [Yum Repository](redhawk-yum-archive.html#dependencies-packaged-with-redhawk) section.

## Install the EPEL Repository

For RHEL/CentOS 7:
```bash
sudo yum install epel-release
```

## Install the Software Collections (SCL) Repository

REDHAWK is compiled with C++14, which is not supported by the default compiler on CentOS 7 (g++ 4.x.x).  REDHAWK supports C++14 through the SCL repository, which provides `devtoolset-<ver>`.

> **NOTE**:  The EPEL repository is required to install the SCL repository.

For RHEL/CentOS 7:
```bash
sudo yum install centos-release-scl
```

## Runtime-only Dependencies

To install dependencies required for RHEL/CentOS 7 runtime:
```bash
sudo yum install gstreamer-python \
     gstreamer1-plugins-base \
     java-11-openjdk-devel \
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

The following dependencies are required, in addition to the runtime dependencies, for development and building REDHAWK from source.

> **NOTE**:  The `CentOS-SCLo-scl-rh.repo` yum repository is required to install `devtoolset-9-gcc-c++`  

```bash
sudo yum install gstreamer-python \
     libuuid-devel \
     boost-devel \
     cppunit-devel \
     autoconf automake libtool \
     expat-devel \
     gcc-c++ \
     java-1.8.0-openjdk-devel \
     java-11-openjdk-devel \
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

### Optional Dependencies for Octave Development

The following dependencies are required for Octave <abbr title="See Glossary.">component</abbr> development:

```bash
sudo yum install octave-devel
```
