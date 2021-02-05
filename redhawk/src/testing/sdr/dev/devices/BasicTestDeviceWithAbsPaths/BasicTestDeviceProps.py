#!/usr/bin/env python3
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

#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: BasicTestDevice.prf.xml
# Generated on: Mon Feb  1 15:20:42 2010

PROPERTIES = (
              (
               'DCE:6b298d70-6735-43f2-944d-06f754cd4eb9', # ID
               'no_default_prop', # NAME
               'string', # TYPE
               'readwrite', # MODE
               None, # DEFAULT
               None, # UNITS
               'external', # ACTION
               ('configure',), # KINDS
              ),
              (
               'DCE:456310b2-7d2f-40f5-bfef-9fdf4f3560ea', # ID
               'default_prop', # NAME
               'string', # TYPE
               'readwrite', # MODE
               'default', # DEFAULT
               None, # UNITS
               'external', # ACTION
               ('configure',), # KINDS
              ),
              (
               'DCE:4a23ad60-0b25-4121-a630-68803a498f75', # ID
               'os_name', # NAME
               'string', # TYPE
               'readonly', # MODE
               'Linux', # DEFAULT
               None, # UNITS
               'eq', # ACTION
               ('allocation',), # KINDS
              ),
              (
               'DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b', # ID
               'processor_name', # NAME
               'string', # TYPE
               'readonly', # MODE
               'x86', # DEFAULT
               None, # UNITS
               'eq', # ACTION
               ('allocation',), # KINDS
              ),
              (
               'DCE:7f36cdfb-f828-4e4f-b84f-446e17f1a85b', # ID
               'DeviceKind', # NAME
               'string', # TYPE
               'readonly', # MODE
               'BasicTestDevice', # DEFAULT
               None, # UNITS
               'eq', # ACTION
               ('allocation',), # KINDS
              ),
              (
               'DCE:64303822-4c67-4c04-9a5c-bf670f27cf39', # ID
               'RunsAs', # NAME
               'string', # TYPE
               'readonly', # MODE
               'root', # DEFAULT
               None, # UNITS
               'ne', # ACTION
               ('allocation',), # KINDS
              ),
              (
               'DCE:021f10cf-7a05-46ec-a507-04b513b84bd4', # ID
               'HasXMIDAS', # NAME
               'boolean', # TYPE
               'readonly', # MODE
               True, # DEFAULT
               None, # UNITS
               'eq', # ACTION
               ('allocation',), # KINDS
              ),
              (
               'DCE:ac73446e-f935-40b6-8b8d-4d9adb6b403f', # ID
               'ProvidedCpuCores', # NAME
               'short', # TYPE
               'readonly', # MODE
               8, # DEFAULT
               None, # UNITS
               'ge', # ACTION
               ('allocation',), # KINDS
              ),
              (
               'DCE:dd339b67-b387-4018-94d2-9a72955d85b9', # ID
               'CoresClockRateGHz', # NAME
               'float', # TYPE
               'readonly', # MODE
               3.0, # DEFAULT
               None, # UNITS
               'le', # ACTION
               ('allocation',), # KINDS
              ),
              (
               'DCE:8dcef419-b440-4bcf-b893-cab79b6024fb', # ID
               'memCapacity', # NAME
               'long', # TYPE
               'readonly', # MODE
               None, # DEFAULT
               None, # UNITS
               'external', # ACTION
               ('allocation',), # KINDS
              ),
              (
               'DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8', # ID
               'BogoMipsCapacity', # NAME
               'long', # TYPE
               'readonly', # MODE
               None, # DEFAULT
               None, # UNITS
               'external', # ACTION
               ('allocation',), # KINDS
              ),
              (
               'DCE:c03e148f-e9f9-4d70-aa00-6e23d33fa648', # ID
               'SomeConfigFileLocation', # NAME
               'string', # TYPE
               'readonly', # MODE
               'notyourfile', # DEFAULT
               None, # UNITS
               'external', # ACTION
               ('execparam',), # KINDS
              ),
              (
               'DCE:dc4289a8-bb98-435b-b914-305ffaa7594f', # ID
               'ImplementationSpecificProperty', # NAME
               'string', # TYPE
               'readonly', # MODE
               'DefaultValueNoGood', # DEFAULT
               None, # UNITS
               'external', # ACTION
               ('execparam',), # KINDS
              ),
              (
               'DCE:716ea1c4-059a-4b18-8b66-74804bd8d435', # ID
               'ImplementationSpecificProperty2', # NAME
               'string', # TYPE
               'readonly', # MODE
               'DefaultValueNoGood2', # DEFAULT
               None, # UNITS
               'external', # ACTION
               ('execparam',), # KINDS
              ),
             )

