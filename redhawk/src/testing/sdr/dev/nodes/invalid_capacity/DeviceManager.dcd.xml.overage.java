<?xml version="1.0" encoding="UTF-8"?>
<!--
This file is protected by Copyright. Please refer to the COPYRIGHT file
distributed with this source distribution.

This file is part of REDHAWK core.

REDHAWK core is free software: you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.

REDHAWK core is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
details.

You should have received a copy of the GNU Lesser General Public License along
with this program.  If not, see http://www.gnu.org/licenses/.
-->

<!DOCTYPE deviceconfiguration PUBLIC "-//JTRS//DTD SCA V2.2.2 DCD//EN" "deviceconfiguration.dtd">
<deviceconfiguration id="DCE:f8bdc21c-5bd1-43e8-b364-060bf0960496" name="devc_node">
  <devicemanagersoftpkg>
    <localfile name="/mgr/DeviceManager.spd.xml"/>
  </devicemanagersoftpkg>
  <componentfiles>
    <componentfile id="devc_file_1" type="SPD">
      <localfile name="/devices/DevC/DevC.spd.xml.java"/>
    </componentfile>
  </componentfiles>
  <partitioning>
    <componentplacement>
      <componentfileref refid="devc_file_1"/>
      <componentinstantiation id="devc_node:devc_1">
        <usagename>devc_1</usagename>
        <componentproperties>
          <simpleref refid="myulong" value="3"/>
        </componentproperties>
      </componentinstantiation>
    </componentplacement>
  </partitioning>
  <domainmanager>
    <namingservice name="REDHAWK_DEV/REDHAWK_DEV"/>
  </domainmanager>
</deviceconfiguration>
