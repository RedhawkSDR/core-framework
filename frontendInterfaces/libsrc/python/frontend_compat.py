#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK frontendInterfaces.
#
# REDHAWK frontendInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK frontendInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import warnings
warnings.warn("Using deprecated syntax for port variables",DeprecationWarning)
from frontend import *
class PortFRONTENDFrontendTunerIn_i(InFrontendTunerPort):
    def __init__(self,parent,name):
        InFrontendTunerPort.__init__(self,name)
class PortFRONTENDAnalogTunerIn_i(InAnalogTunerPort):
    def __init__(self,parent,name):
        InAnalogTunerPort.__init__(self,name)
class PortFRONTENDDigitalTunerIn_i(InDigitalTunerPort):
    def __init__(self,parent,name):
        InDigitalTunerPort.__init__(self,name)
class PortFRONTENDGPSIn_i(InGPSPort):
    def __init__(self,parent,name):
        InGPSPort.__init__(self,name)
class PortFRONTENDRFInfoIn_i(InRFInfoPort):
    def __init__(self,parent,name):
        InRFInfoPort.__init__(self,name)
class PortFRONTENDRFSourceIn_i(InRFSourcePort):
    def __init__(self,parent,name):
        InRFSourcePort.__init__(self,name)
class PortFRONTENDNavDataIn_i(InNavDataPort):
    def __init__(self,parent,name):
        InNavDataPort.__init__(self,name)
class PortFRONTENDFrontendTunerOut_i(OutFrontendTunerPort):
    def __init__(self,parent,name):
        OutFrontendTunerPort.__init__(self,name)
class PortFRONTENDAnalogTunerOut_i(OutAnalogTunerPort):
    def __init__(self,parent,name):
        OutAnalogTunerPort.__init__(self,name)
class PortFRONTENDDigitalTunerOut_i(OutDigitalTunerPort):
    def __init__(self,parent,name):
        OutDigitalTunerPort.__init__(self,name)
class PortFRONTENDGPSOut_i(OutGPSPort):
    def __init__(self,parent,name):
        OutGPSPort.__init__(self,name)
class PortFRONTENDRFInfoOut_i(OutRFInfoPort):
    def __init__(self,parent,name):
        OutRFOutfoPort.__init__(self,name)
class PortFRONTENDRFSourceOut_i(OutRFSourcePort):
    def __init__(self,parent,name):
        OutRFSourcePort.__init__(self,name)
class PortFRONTENDNavDataOut_i(OutNavDataPort):
    def __init__(self,parent,name):
        OutNavDataPort.__init__(self,name)

