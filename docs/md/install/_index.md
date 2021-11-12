# Install

This chapter explains how to install the Core Framework (CF), the IDE, and the basic assets. The CF is the software back-end of REDHAWK. The IDE is a GUI for development and interaction with REDHAWK systems. The basic assets are a collection of <abbr title="See Glossary.">components</abbr>, <abbr title="See Glossary.">devices</abbr>, and <abbr title="See Glossary.">waveforms</abbr> that developers can use to create simple software-defined radio applications.

To configure and install REDHAWK and associated dependencies, you must have root permissions. The REDHAWK installation is compatible with  CentOS 7.  The current REDHAWK release was tested against CentOS 7.9.

## Install REDHAWK from RPMs

This section provides step-by-step instructions for installing a REDHAWK release using the YUM command-line package management tool. The installation process includes:

  - Configuring the host system to install REDHAWK dependencies from Fedora EPEL
  - Downloading and configuring the REDHAWK YUM repository on the host system
  - Installing the REDHAWK software
  - Setting up the user environment for immediate REDHAWK runtime or development use


> **NOTE**  
> Before beginning the installation process, if you are upgrading from a 1.8.x version of REDHAWK or for more information about external dependencies, refer to [External Dependencies](dependencies.html).  

### Setup to Install EPEL Dependencies

REDHAWK has several open-source software dependencies from the EPEL repository. If your system is not configured to receive software packages from EPEL, you can configure it as follows:

For RHEL/CentOS 7:

```bash
sudo yum install https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
```

### Download and Configure the Yum Repository

The following conventions are used in the instructions that follow.

| **Variable** | **Description**                                 | **Example**          |
| :----------- | :---------------------------------------------- | :------------------- |
| `<version>`  | REDHAWK version                                 | `3.0.0`              |
| `<dist>`     | Linux distribution as represented by rpm macros | `el7` (for CentOS 7) |
| `<arch>`     | host architecture                               | `x86_64`            |

Adjust the variables to match the desired REDHAWK version, host Linux distribution, and host machine architecture. For example, for REDHAWK version 3.0.0, CentOS 7, `redhawk-yum-3.0.0-el7-x86_64.tar.gz`.

#### Download the YUM Archive of REDHAWK

Download the archive of the desired version of REDHAWK for your host OS and architecture.

```bash
wget https://github.com/RedhawkSDR/redhawk/releases/download/<version>/redhawk-yum-<version>-<dist>-<arch>.tar.gz
```

#### Setup the REDHAWK Repository

1.  In the directory that you want to use for the REDHAWK yum repository, extract the contents of the tar file.

    ```bash
    tar xzvf redhawk-yum-<version>-<dist>-<arch>.tar.gz
    cd redhawk-<version>-<dist>-<arch>
    ```

2.  Install the redhawk-release package (containing the REDHAWK GPG signing key):

    ```bash
    sudo yum install -y redhawk-release*.rpm
    ```

3.  Enter the following commands to add the following file, `/etc/yum.repos.d/redhawk.repo`:

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

### Install REDHAWK

Use one of the following options to install the IDE, CF, and accompanying dependencies from RPMs.


  - To install only the runtime REDHAWK software, enter the following command:

    ```bash
    sudo yum groupinstall "REDHAWK Runtime"

    ```

  - To install the REDHAWK development software, enter the following command:

    ```bash
    sudo yum groupinstall "REDHAWK Development"

    ```

> **NOTE**  
> If you want to be more selective about the packages you install, refer to [REDHAWK Yum Repository and Packages](redhawk-yum.html) for a list of packages that can be installed. You can also [install a stand-alone IDE](ide.html).  Additionally, individual RPMs released in the REDHAWK Updates repository may be installed. For more information, refer to [Installing RPMs from the REDHAWK Updates Repository](redhawk-yum.html#installing-rpms-from-the-redhawk-updates-repository).


> **NOTE**  
> For installation issues with the GPP, refer to [Troubleshooting](troubleshooting.html).  

### Setup the User Environment

1.  Enter the following commands to set up the environment variables:

    ```bash
    . /etc/profile.d/redhawk.sh
    . /etc/profile.d/redhawk-sdrroot.sh

    ```

2.  Use the following command to add each REDHAWK user to the `redhawk` group:

    ```bash
    sudo /usr/sbin/usermod -a -G redhawk <username>

    ```

    Where `<username>` is the name of a user to add to the group. If you are logged into an account that you modify with `usermod`, you must log out and back in for the changes to take effect.

## Configure omniORB

The omniORB configuration file (`/etc/omniORB.cfg`) must be edited to provide information about how to reach the <abbr title="See Glossary.">CORBA Name Service</abbr>. By default, the config file contains the following entries:

```bash
InitRef = NameService=corbaname::127.0.0.1:2809
supportBootstrapAgent = 1
```

> **NOTE**  
> The `NameService` line provides information about how to reach the CORBA Naming Service. The number is an IP address followed by a colon and a port number. The port number is used as a default if no other number is specified. `SupportBootstrapAgent` is a server side option. This enables omniORB servers and Java clients to work together. When set to 1, an omniORB server responds to a bootstrap agent request.  

1.  Add the following line to the config file to configure the CORBA <abbr title="See Glossary.">Event Service</abbr> (this requires root permissions):

    ```bash
    InitRef = EventService=corbaloc::127.0.0.1:11169/omniEvents
    ```

    The first number is the IP address followed by a colon and a port number. `omniEvents` is the object key.

1.  Enter the following command to start the `omniNames` and `omniEvents` services:

    ```bash
    sudo $OSSIEHOME/bin/cleanomni
    ```

1.  For CentOS 7 systems, to have `omniNames` and `omniEvents` start automatically at system boot (recommended), enter the following commands:

    ```bash
    sudo systemctl enable omniNames.service
    sudo systemctl enable omniEvents.service
    ```

For more information about omniORB configuration file settings (`/etc/omniORB.cfg`), refer to Chapter 4 of the omniORB [User's Guide](http://omniorb.sourceforge.net/omni42/omniORB/omniORB004.html), or on your local system, [here](file:///usr/share/doc/omniORB-devel-4.2.0/doc/omniORB/omniORB004.html).

### Configure omniORB for Distributed Systems

If you want to run a <abbr title="See Glossary.">Domain Manager</abbr> and <abbr title="See Glossary.">Device Manager</abbr> from two different computers, the following procedure explains how to configure omniORB for distributed systems.

1.  On the computer from which you want to run the Domain Manager, start `omniNames` and `omniEvents` and then launch a Domain Manager.

> **NOTE**  
> The firewall may need to be disabled to allow the Device Manager to connect.  

2.  On the computer from which you want to run the Device Manager, modify the `omniORB.cfg` file so that the IP address for the `NameService` and `EventService` is the address of the computer running the Domain Manager.

    The following example is a modified Domain Manager `omniORB.cfg` file:

    ```bash
    InitRef = NameService=corbaname::127.0.0.1:
    InitRef = EventService=corbaloc::127.0.0.1:11169/omniEvents
    ```

    The following example is a modified Device Manager `omniORB.cfg` file:

    ```bash
    InitRef = NameService=corbaname::<IP address of Domain Manager>:
    InitRef = EventService=corbaloc::<IP address of Domain Manager>:11169/omniEvents
    ```

> **NOTE**  
> Neither `omniEvents` nor `omniNames` needs to be running on this computer.  

3.  On the computer running the Device Manager, test that you can see the Domain Manager by running `nameclt list`.

    The name of the Domain Manager is displayed.

4.  Start the Device Manager.

    Any devices in the <abbr title="See Glossary.">node</abbr> are registered with the Domain Manager.

5.  To verify that you can view both the Device Manager and Domain Manager, from either computer, run `nameclt list <Domain Manager Name>`.

    The Device Manager and Domain Manager are displayed.

> **NOTE**  
> omniORB may have trouble automatically resolving its location. In this case, it may be necessary to set the endpoints in the `omniORB.cfg` files by adding the following to each `omniORB.cfg` file: `endpoint = giop:tcp:<IP address of machine>`. You must restart `omniEvents` and `omniNames` for these changes to take effect.  


> **NOTE**  
> Run `rh_net_diag` to help diagnose any problems. Refer to [Diagnosing Problems Using the `rh_net_diag` Script](../appendices/troubleshooting/connections.html#diagnosing-problems-using-the-rh_net_diag-script) for more information on how to use `rh_net_diag`.  

## Configure JacORB to Support the IDE

The IDE uses JacORB version 3.3 for CORBA communication. The IDE includes a configuration file for JacORB in the IDE's directory (in `configuration/jacorb.properties`). The file includes explanations and examples for many of JacORB's configuration options. For more information, refer to chapter 3 of the JacORB 3.3 Programming Guide. Typically, there is no need to adjust any JacORB settings.
