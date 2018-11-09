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

<!DOCTYPE softwareassembly PUBLIC '-//JTRS//DTD SCA V2.2.2 SAD//EN' 'softwareassembly.dtd'>
<softwareassembly id="DCE:cbb6ec77-c38e-4cf8-8905-dbbb47503466" name="MessageEventTestPy">
  <componentfiles>
    <componentfile id="MessageSenderPy" type="SPD">
        <localfile name="/components/huge_msg_python/huge_msg_python.spd.xml"/>
    </componentfile>
  </componentfiles>
  <partitioning>
    <componentplacement>
        <componentfileref refid="MessageSenderPy"/>
        <componentinstantiation id="DCE:52597d1d-d064-4536-9d0e-14bb7724ccdc">
            <usagename>MessageSenderPy_1</usagename>
            <findcomponent>
                <namingservice name="MessageSenderPy_1"/>
            </findcomponent>
        </componentinstantiation>
    </componentplacement>
  </partitioning>
  <assemblycontroller>
      <componentinstantiationref refid="DCE:52597d1d-d064-4536-9d0e-14bb7724ccdc"/>
  </assemblycontroller>
  <connections>
      <connectinterface>
          <usesport>
              <usesidentifier>output</usesidentifier>
              <componentinstantiationref refid="DCE:52597d1d-d064-4536-9d0e-14bb7724ccdc"/>
          </usesport>
          <findby>
              <domainfinder type="eventchannel" name="message_events"/>
          </findby>
      </connectinterface>
  </connections>
</softwareassembly>
