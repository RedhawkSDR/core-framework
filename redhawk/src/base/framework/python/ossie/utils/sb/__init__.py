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
 - launch():
     Execute a softpackage and return a proxy object for managing it
 - getComponent():
     Retrieve a pointer to one of the running components

Component, Device and Service proxy classes are used to manage executable softpackages:
  - The launch() function will kick off the executable and return the appropriate object type.
    - It can take an SPD XML file or a component name found in the defined SDRROOT.
  - SCD (XML) describes the interfaces for the instantiated component.
  - The destructor cleans up the launched component process.
  - releaseObject removes the object (and destroys the component instance)

Component and Device classes support additional functionality:
  - Properties are attributes of the instantiated object.
  - connect() is called on the component with the uses port and the component with the provides port is the argument
    - Ambiguities can be resolved with usesPortName and/or providesPortName.

Helpers are provided to manage data
  - DataSource(), DataSink():
      Push vectors from Python to components and back.
  - FileSource(), FileSink():
      Push data from a file into components and back.
  - MessageSource(), MessageSink():
      Push messages from Python to components and back.
  - SoundSink():
      Playback audio data from BULKIO streams
  - compareSRI():
      Compares the content of two SRI values
  - orb:
      OmniORB instance

Plotting (matplotlib/PyQt4-based):
  - LinePlot():
      Line (X/Y) plot of input signal(s)
  - LinePSD():
      Line (X/Y) plot of power spectral density (PSD) of input signal(s)
  - RasterPlot():
      Falling raster (2D image) plot of input signal
  - RasterPSD():
      Falling raster (2D image) plot of the power spectral density (PSD) of input signal

Plotting (REDHAWK IDE-based)
  - Plot():
      Provides a way to display data from a particular port.
      Requires:
        - Eclipse Redhawk IDE must be installed
        - Environment variable RH_IDE must be set defining the path to the main eclipse directory (/data/eclipse for example)

General usage examples:
    # Launch component by the component name and change a property value
    >>> a = launch('<component>')
    >>> a.some_prop = 5.0
    >>> print a.some_prop
    5.0

    # Launch component by the SPD XML file of a component
    >>> b = launch('/absolute/path/to/component/descriptor/<component>.spd.xml')

    # Provide file input to a component and plot output from a given output port
    >>> a = FileSource('<input filename here>','<data type here>')
    >>> b = launch('<component name here>')
    >>> c = LinePlot()
    >>> a.connect(b)
    >>> b.connect(c)
    >>> start()
    >>> stop()

    # Continuously pushing an array of data to a plot
    >>> a = DataSource()
    >>> b = LinePlot()
    >>> a.connect(b,usesPortName='shortOut')
    >>> data = range(1000)
    >>> start()
    >>> a.push(data,loop=True)
    >>> stop()
"""
from domainless import *
from io_helpers import *
from block_process import *
try:
    from bulkio.bulkioInterfaces import BULKIO
except:
    # BULKIO is not installed
    pass

from plots import *
from audio import *
