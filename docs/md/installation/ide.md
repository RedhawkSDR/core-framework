# IDE

The REDHAWK IDE leverages Java technologies and requires Java 8. The IDE also includes native libraries that allow the IDE to have a look and feel appropriate for the OS. A minimum of 2 GB of RAM is required, but 4 GB+ is recommended.

The following procedure explains how to install a stand-alone IDE.

1.  Ensure your system has the appropriate dependencies installed.

    On RHEL/CentOS 7:

    ```bash
    sudo yum install java-1.8.0-openjdk-devel PackageKit-gtk3-module libcanberra-gtk3 libwebkit2gtk
    ```  
    On RHEL/CentOS 6:

    ```bash
    sudo yum install java-1.8.0-openjdk-devel PackageKit-gtk-module libcanberra-gtk2 webkitgtk
    ```

2.  Locate the appropriate archive from the REDHAWK release page on GitHub (<https://github.com/RedhawkSDR/redhawk/releases/>`<version>`). (Where `<version>` corresponds to the version of the REDHAWK IDE. For example, <https://github.com/RedhawkSDR/redhawk/releases/2.0.3>).

3.  Run the following command:

    ```bash
    tar zxf redhawk-ide-<version>-linux.gtk.<arch>.tar.gz
    ```

    Where `<version>` corresponds to the version of the REDHAWK IDE and `<arch>` is x86_64 for 64-bit systems.

4.  Start the IDE by running the eclipse executable in the eclipse directory.
