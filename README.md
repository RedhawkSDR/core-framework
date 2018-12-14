# REDHAWK Core Framework

## Description
REDHAWK is a software-defined radio (SDR) framework designed to support the development, deployment, and management of real-time software radio applications. To support the design and development of software applications, REDHAWK provides tools that allow development and testing of software modules called "Components" and composition of Components into "Waveform Applications" that can be seamlessly deployed on a single computer or multiple network-enabled computers.

REDHAWK 2.1.0 added support for new C++ Components to be created as shared libraries, which allow multiple Components to be deployed into the same process and enable faster, lower-cost I/O. For documentation on this beta feature, refer to docs/shared-address.

## Subdirectories

* redhawk - Contains the REDHAWK Core Framework base libraries and system software.
* bulkioInterfaces - Contains the IDLs and build scripts required for the installation and use of the BulkIO interface library.
* burstioInterfaces - Contains the IDLs and build scripts required for the installation and use of the BurstIO interface library.
* frontendInterfaces - Contains the source and build script for the REDHAWK Frontend Interfaces.
* redhawk-codegen - REDHAWK C++, Python, and Java Code Generators.
* GPP - Creates the GPP Device for the systems and installs within the system SDR Root.
* throughput - Measures the average and peak throughput of raw sockets, CORBA and BulkIO to help quantify the performance of a given system.
* codegenTesting - Unit tests for verifying the operation of the Code Generators.

## REDHAWK Documentation

REDHAWK Website: [www.redhawksdr.org](http://www.redhawksdr.org)

## Copyrights

This work is protected by Copyright. Please refer to the [Copyright File](COPYRIGHT) for updated copyright information.                                                                                                                

## License

The REDHAWK Core Framework is licensed under the GNU Lesser General Public License (LGPL).
