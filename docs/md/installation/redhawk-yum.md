# Yum Repository

The following sections describe the REDHAWK packages and provided dependencies.


> **NOTE**  
> [External dependencies](../appendices/dependencies.html) are also necessary for development with REDHAWK and for building REDHAWK from source.  

## REDHAWK Yum Repository

All of the REDHAWK packages are included in a yum repository, which can be configured as described in [Setting Up the REDHAWK Repository](../installation/_index.html#setting-up-the-redhawk-repository).

### REDHAWK Yum Groups

The repository contains two yum groups (REDHAWK Runtime and REDHAWK Development). The yum groups are described in the following sections.

#### REDHAWK Runtime

The REDHAWK Runtime yum group includes the following packages:

  - `bulkioInterfaces`
  - `burstioInterfaces`
  - `frontendInterfaces`
  - `GPP`
  - `GPP-profile`
  - `redhawk-sdrroot-dev-mgr`
  - `redhawk-sdrroot-dom-mgr`
  - `redhawk-sdrroot-dom-profile`
  - `redhawk`

#### REDHAWK Development

The REDHAWK Development yum group includes the following packages:

  - `redhawk-basic-components`
  - `redhawk-basic-devices`
  - `redhawk-basic-waveforms`
  - `redhawk-codegen`
  - `redhawk-devel`
  - `redhawk-ide`
  - `redhawk-qt-tools`


> **NOTE**  
> The Development group will install the Runtime group packages as dependencies.  

### Dependencies Packaged with REDHAWK

This section identifies the dependencies that are packaged with REDHAWK.

#### Dependencies for RHEL/CentOS 6

The following list of dependencies are required for RHEL/CentOS 6 systems.

  - `libomniEvents2`
  - `libomniEvents2-devel`
  - `log4cxx`
  - `log4cxx-devel`
  - `omniEvents-bootscripts`
  - `omniEvents-debuginfo`
  - `omniEvents-doc`
  - `omniEvents-server`
  - `omniEvents-utils`
  - `omniORBpy-debuginfo`
  - `omniORBpy-devel`
  - `omniORBpy-libs`
  - `python-omniORB`
  - `uhd`
  - `uhd-debuginfo`
  - `uhd-devel`
  - `uhd-doc`
  - `uhd-firmware`

To install the dependencies for RHEL/CentOS 6, enter the following commands:

```bash
sudo yum install libomniEvents2 libomniEvents2-devel log4cxx \
    log4cxx-devel omniEvents-bootscripts omniEvents-debuginfo omniEvents-doc \
    omniEvents-server omniEvents-utils omniORBpy-debuginfo omniORBpy-devel \
    omniORBpy-libs python-omniORB uhd uhd-debuginfo uhd-devel uhd-doc uhd-firmware
```

#### Dependencies for RHEL/CentOS 7

The following list of dependencies are required for RHEL/CentOS 7 systems.

  - `libomniEvents2`
  - `libomniEvents2-devel`
  - `omniEvents-bootscripts`
  - `omniEvents-debuginfo`
  - `omniEvents-doc`
  - `omniEvents-server`
  - `omniEvents-utils`
  - `omniORB`
  - `omniORB-debuginfo`
  - `omniORB-devel`
  - `omniORB-doc`
  - `omniORB-servers`
  - `omniORB-utils`
  - `omniORBpy-debuginfo`
  - `omniORBpy-devel`
  - `omniORBpy-libs`
  - `python-omniORB`

To install the dependencies for RHEL/CentOS 7, enter the following commands:

```bash
sudo yum install libomniEvents2 libomniEvents2-devel omniEvents-bootscripts \
    omniEvents-debuginfo omniEvents-doc omniEvents-server omniEvents-utils \
    omniORB omniORB-debuginfo omniORB-devel omniORB-doc omniORB-servers omniORB-utils \
    omniORBpy-debuginfo omniORBpy-devel omniORBpy-libs python-omniORB
```

### Selective Installation

After you set up the REDHAWK yum repository as described in [Setting Up the REDHAWK Repository](/manual/installation/#setting-up-the-redhawk-repository), you can also install individual packages via yum for selective installations.

For example, to perform a selective installation that includes the <abbr title="See Glossary.">GPP</abbr>, enter the following command:

```bash
sudo yum install GPP
```

To perform a selective development installation that includes the REDHAWK IDE, enter the following command:

```bash
sudo yum install redhawk-ide
```

### Installing RPMS from the REDHAWK Updates Repository

You can install RPMs from the REDHAWK Updates repository. The following steps and examples explain how to set up the REDHAWK Updates repository and install the individual RPMs.


> **WARNING**  
> Updating with these RPMs may break API compatibility with your current REDHAWK LTS version.  

#### Setting Up the REDHAWK Updates Repository

1.  In the directory that you want to use for the REDHAWK Updates yum repository, extract the contents of the tar file.

    ```bash
    tar xzvf redhawk-updates-yum-rh2.2-<updates-version>-<dist>-<arch>.tar.gz
    cd redhawk-updates-rh2.2-<updates-version>-<dist>-<arch>
    ```

2.  Enter the following commands to add the following file, `/etc/yum.repos.d/redhawk-updates.repo`:

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

#### Using the REDHAWK Updates Repository

After adding the `redhawk-updates.repo` file, you can update any of the available packages. For example, you can update to the latest version of a package using the `yum update <package name>` command. Enter the following command to update to the latest version of `rh.sinksocket`:

```bash
sudo yum --enablerepo=redhawk-updates update rh.sinksocket
```

Alternatively, you can update to a specific version of a package using the `yum install <full rpm name>` command. Enter the following commands to update to this specfic version of `rh.sinksocket`:

```bash
sudo yum --showduplicates list rh.sinksocket
sudo yum --enablerepo=redhawk-updates install rh.sinksocket-2.1.0.0.rh2.2.el7.x86_64
```
