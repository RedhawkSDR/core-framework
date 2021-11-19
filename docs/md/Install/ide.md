# IDE

The REDHAWK IDE leverages Java technologies and requires Java 8. The IDE also includes native libraries that allow the IDE to have a look and feel appropriate for the OS. A minimum of 2 GB of RAM is required, but 4 GB+ is recommended.

> **WARNING**:  The IDE should no longer be used for code generation.  Instead, use the IDE to generate the PRF, SCD, and SPD XML files.  Then, run `redhawk-codegen` directly on that SPD file.

To install a stand-alone IDE:

1. Ensure your system has the appropriate dependencies installed.

    On RHEL/CentOS 7:

    ```bash
    sudo yum install java-1.8.0-openjdk-devel PackageKit-gtk3-module libcanberra-gtk3 libwebkit2gtk
    ```  

1. Download the appropriate archive from the REDHAWK [release page](https://github.com/RedhawkSDR/redhawk/releases) on GitHub.  

    We do not currently plan to release an IDE for REDHAWK 3 and above.
    As of 2021-Nov, the latest IDE is from version 2.3.0.
    If necessary, click on the Assets heading to expand the list or archives to show the IDE.

1. Choose a location and extract the archive:

    ```bash
    tar zxf redhawk-ide-<version>-linux.gtk.x86_64.tar.gz
    ```

> **NOTE**:  Before running the IDE, follow the instructions in section 'Java and JacORB'.

1. Start the IDE by running the `eclipse` executable in the eclipse directory.
