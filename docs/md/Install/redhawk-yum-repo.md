# Yum Repository

This section describes the REDHAWK yum packages and dependencies provided via a yum archive.

> **NOTE**:  [External dependencies](external-dependencies.html) are also necessary for development with REDHAWK and for building REDHAWK from source.  

## Download the Archive

REDHAWK yum packages, and some specific dependencies, are distributed in an archived yum repository.  To download the archive:
```bash
wget https://github.com/RedhawkSDR/redhawk/releases/download/<version>/\
redhawk-yum-<version>-<dist>-<arch>.tar.gz
```
Adjust the variables to match the desired REDHAWK version, host Linux distribution, and host machine architecture, according to this:

| **Variable** | **Description**                                 | **Example**          |
| :----------- | :---------------------------------------------- | :------------------- |
| `<version>`  | REDHAWK version                                 | `3.0.0`              |
| `<dist>`     | Linux distribution as represented by rpm macros | `el7` (for CentOS 7) |
| `<arch>`     | host architecture                               | `x86_64`            |

For example, for REDHAWK version 3.0.0, CentOS 7, the filename is `redhawk-yum-3.0.0-el7-x86_64.tar.gz`.

## Setup Local Yum Repository

1. In the directory that you want to use for the REDHAWK yum repository, extract the tar file.

    ```bash
    tar xzvf redhawk-yum-<version>-<dist>-<arch>.tar.gz
    cd redhawk-<version>-<dist>-<arch>
    ```

1. Install the redhawk-release package (containing the REDHAWK GPG signing key):

    ```bash
    sudo yum install -y redhawk-release*.rpm
    ```

1. Add the file `/etc/yum.repos.d/redhawk.repo`:

    ```bash
    cat<<EOF|sed 's@LDIR@'`pwd`'@g'|sudo tee /etc/yum.repos.d/redhawk.repo
    [redhawk]
    name=REDHAWK Repository
    baseurl=file://LDIR/
    enabled=1
    gpgcheck=1
    gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-redhawk
    EOF
    ```

    Once this is done, your local `yum` command will be able to use the REDHAWK Repository.

## Dependencies Included

These dependencies are required for RHEL/CentOS 7 systems.  Since they have been modified for REDHAWK, they are not available from a generic CentOS, or similar, repository.  So, they are distributed with REDHAWK in the yum archive.

  - `jacorb`
  - `libomniEvents2-devel`
  - `libomniEvents2`
  - `omniEvents-bootscripts`
  - `omniEvents-debuginfo`
  - `omniEvents-doc`
  - `omniEvents-server`
  - `omniEvents-utils`
  - `omniORB-debuginfo`
  - `omniORB-devel`
  - `omniORB-doc`
  - `omniORB-servers`
  - `omniORB-utils`
  - `omniORB`
  - `omniORBpy-debuginfo`
  - `omniORBpy-devel`
  - `python3-omniORB`

To install the dependencies for RHEL/CentOS 7:

```bash
sudo yum install jacorb libomniEvents2 libomniEvents2-devel omniEvents-bootscripts \
    omniEvents-debuginfo omniEvents-doc omniEvents-server omniEvents-utils \
    omniORB omniORB-debuginfo omniORB-devel omniORB-doc omniORB-servers omniORB-utils \
    omniORBpy-debuginfo omniORBpy-devel python3-omniORB
```

## REDHAWK Yum Groups

The repository contains two yum groups (REDHAWK Runtime and REDHAWK Development).

### REDHAWK Runtime

The REDHAWK Runtime yum group includes the following packages:

  - `bulkioInterfaces`
  - `burstioInterfaces`
  - `frontendInterfaces`
  - `GPP-profile`
  - `GPP`
  - `redhawk-docs`
  - `redhawk-sdrroot-dev-mgr`
  - `redhawk-sdrroot-dom-mgr`
  - `redhawk-sdrroot-dom-profile`
  - `redhawk`

### REDHAWK Development

The REDHAWK Development yum group includes the following packages:

  - `redhawk-basic-components`
  - `redhawk-basic-devices`
  - `redhawk-basic-waveforms`
  - `redhawk-codegen`
  - `redhawk-devel`
  - `redhawk-ide`
  - `redhawk-qt-tools`

> **NOTE**:  The Development group will install the Runtime group packages as dependencies.

## Selective Installation

After you set up the REDHAWK yum repository, you can also install individual packages via yum for selective installations.

For example, to perform a selective installation that includes the <abbr title="See Glossary.">GPP</abbr>, enter the following command:

```bash
sudo yum install GPP
```

To perform a selective development installation that includes the REDHAWK IDE, enter the following command:

```bash
sudo yum install redhawk-ide
```

## The Updates Repository

You can install RPMs from the REDHAWK Updates repository. The following steps and examples explain how to set up the REDHAWK Updates repository and install the individual RPMs.

> **WARNING**:  Updating with these RPMs may break API compatibility with your current REDHAWK LTS version.  

### Setup

1. In the directory that you want to use for the REDHAWK Updates yum repository, extract the contents of the tar file.

    ```bash
    tar xzvf redhawk-updates-yum-rh2.2-<updates-version>-<dist>-<arch>.tar.gz
    cd redhawk-updates-rh2.2-<updates-version>-<dist>-<arch>
    ```

1. Enter the following commands to add the following file, `/etc/yum.repos.d/redhawk-updates.repo`:

    ```bash
    cat<<EOF|sed 's@LDIR@'`pwd`'@g'|sudo tee /etc/yum.repos.d/redhawk-updates.repo
    [redhawk-updates]
    name=REDHAWK Updates Repository
    baseurl=file://LDIR/
    enabled=0
    gpgcheck=1
    gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-redhawk
    EOF
    ```

### Usage

After adding the `redhawk-updates.repo` file, you can update any of the available packages. For example, you can update to the latest version of a package using the `yum update <package name>` command. Enter the following command to update to the latest version of `rh.sinksocket`:

```bash
sudo yum --enablerepo=redhawk-updates update rh.sinksocket
```

Alternatively, you can update to a specific version of a package using the `yum install <full rpm name>` command. Enter the following commands to update to this specfic version of `rh.sinksocket`:

```bash
sudo yum --showduplicates list rh.sinksocket
sudo yum --enablerepo=redhawk-updates install rh.sinksocket-2.1.0.0.rh2.2.el7.x86_64
```
