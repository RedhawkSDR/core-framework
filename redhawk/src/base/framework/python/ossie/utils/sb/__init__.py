#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

"""
Python command line environment where Redhawk components can be launched, connected, and run without a Domain Manager or a Device Manager

Sandbox functions:
 - show():
     Show current list of components running and component connections
 - generateSADXML():
     Generate an xml string describing the current graph
 - loadSADFile():
     Load and interconnect the components described in the file
 - setSDRROOT():
     Set a new SDRROOT for the current sandbox session
 - catalog():
     Search SDRROOT (or the given argument) for components that can be launched
 - setDEBUG():
     Enable debug output
 - reset():
     Put components back into their initial state
 - start()/stop():
     Invoke start or stop on all components and helpers
 - getComponent():
     Retrieve a pointer to one of the running components

The Component class is used to manage a component
  - The constructor can take an SPD XML file or a component name found in the defined SDRROOT.
  - SCD (XML) describes the interfaces for the instantiated component.
  - Properties are attributes of the instantiated object.
  - The destructor cleans up the launched component process.
  - connect() is called on the component with the uses port and the component with the provides port is the argument
    - Ambiguities can be resolved with usesPortName and/or providesPortName.
  - releaseObject removes the object (and destroys the component instance)

Helpers are provided to manage data
  - InputData(), OutputData():
      Push vectors from Python to components and back.
  - InputFile(), OutputFile():
      Push data from a file into components and back.
  - The Plot():
      Provides a way to display data from a particulare port.
      Requires:
        - Eclipse Redhawk IDE must be installed
        - Environment variable RH_IDE must be set defining the path to the main eclipse directory (/data/eclipse for example)
  - compareSRI():
      Compares the content of two SRI values
  - orb:
      OmniORB instance

General usage examples:
    # Launch component by the component name and change a property value
    >>> a = Component(\"<component>\")
    >>> a.some_prop = 5.0
    >>> print a.some_prop
    5.0

    # Launch component by the SPD XML file of a component
    >>> b = Component(\"/absolute/path/to/component/descriptor/<component>.spd.xml\")

    # Provide file input to a component and plot output from a given output port
    >>> a = InputFile(\"<input filename here>\",\"<data type here>\")
    >>> b = Component(\"<component name here>\")
    >>> c = Plot()
    >>> a.connect(b)
    >>> b.connect(c)
    >>> start()
    >>> stop()

    # Continuously pushing an array of data to a plot
    >>> a = InputData()   # default data type is short
    >>> b = Plot()
    >>> a.connect(b,usesPortName=\"shortOut\")
    >>> data = range(1000)
    >>> a.push(data,loop=True)
"""
from domainless import *
from io_helpers import *
try:
    from bulkio.bulkioInterfaces import BULKIO
except:
    # BULKIO is not installed
    pass

