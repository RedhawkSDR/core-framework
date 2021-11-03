# Introduction

### What is REDHAWK?
REDHAWK is a software package that supports the design, development, deployment, management, and upgrading of real-time, network-enabled Software-Defined Radios (SDR). This chapter provides a high-level overview of REDHAWK and its capabilities.

#### Overview

The REDHAWK software package is comprised of four major pieces:

  - A set of programs to manage distributed deployment of software applications.
  - A set of tools that allow developers to easily create software that is deployable within the REDHAWK environment.
  - A set of tools for introspecting a running REDHAWK system.
  - A set of signal processing building blocks that developers can compose into larger, customized applications.

A signal processing application developed in REDHAWK can be deployed on anything from a single Linux computer to network-enabled system of Linux computers. Typically, the integration of hardware and software required for such a form of multi-asset computing is a non-trivial undertaking, requiring a significant expenditure of resources. REDHAWK takes care of the complicated "under the hood" hardware/software integration challenges so that developers can focus on application development: an in-depth understanding of hardware and software systems is not required for basic REDHAWK use.

REDHAWK also standardizes data interfaces, hardware management, and configuration management, which benefits non-distributed application developers. This standardization, coupled with the provided toolkit, reduces integration cost for both legacy capabilities and future development.

#### Applications of REDHAWK

REDHAWK was designed for the development of SDRs. SDRs have the advantage of being highly reconfigurable relative to their hardware-defined counterparts. This flexibility comes at the cost of increased size, weight, and power consumption, which leads to the use of larger, sometimes distributed, computing systems to perform the computationally-intensive signal processing tasks associated with radios. Through the use of REDHAWK, an SDR developer can focus on signal processing algorithms without worrying about the onus of deploying such algorithms in a network environment.

The process of software and hardware integration among disparate project teams and vendors can be a major undertaking that is often resolved with one-off, custom solutions. While designed to support the data-streaming needs of SDRs, REDHAWK also facilitates integrating additional software and hardware assets into computing systems through features such as well-defined common interfaces. REDHAWK provides a streamlined and consistent methodology to the otherwise tedious and difficult process of assimilating new technologies.
