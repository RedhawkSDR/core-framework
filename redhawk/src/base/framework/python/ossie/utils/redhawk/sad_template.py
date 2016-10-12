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


class sad:
    def __init__(self):
        self.template = '<?xml version="1.0" encoding="UTF-8"?>\n<!DOCTYPE softwareassembly PUBLIC "-//JTRS//DTD SCA V2.2.2 SAD//EN" "softwareassembly.dtd">\n<softwareassembly id="DCE:@__UUID__@" name="@__NAME__@">\n  <componentfiles>\n@__COMPONENTFILE__@  </componentfiles>\n  <partitioning>\n@__COMPONENTPLACEMENT__@  </partitioning>\n@__ASSEMBLYCONTROLLER__@  <connections>\n@__CONNECTINTERFACE__@  </connections>\n@__EXTERNALPORTS__@</softwareassembly>\n'

        self.componentfile='\n    <componentfile id="@__SPDFILEID__@" type="SPD">\n      <localfile name="@__SPDFILE__@"/>\n    </componentfile>\n'

        self.componentplacement='\n    <componentplacement>\n        <componentfileref refid="@__SPDFILEID__@"/>\n        <componentinstantiation id="DCE:@__COMPONENTINSTANCE__@">\n            <usagename>@__COMPONENTNAME__@</usagename>\n            <findcomponent>\n                <namingservice name="@__COMPONENTNAME__@"/>\n            </findcomponent>\n      </componentinstantiation>\n    </componentplacement>\n'

        self.assemblycontroller='\n  <assemblycontroller>\n      <componentinstantiationref refid="DCE:@__COMPONENTINSTANCE__@"/>\n  </assemblycontroller>\n'

        self.externalport='\n  <externalports>\n@__USESPORT__@\n@__PROVIDESPORT__@\n  </externalports>\n'

        self.usesport='\n      <usesport>\n        <usesidentifier>@__PORTNAME__@</usesidentifier>\n        <componentinstantiationref refid="DCE:@__COMPONENTINSTANCE__@"/>\n      </usesport>\n'

        self.providesport='\n      <providesport>\n        <providesidentifier>@__PORTNAME__@</providesidentifier>\n        <componentinstantiationref refid="DCE:@__COMPONENTINSTANCE__@"/>\n      </providesport>\n'

        self.componentsupportedinterface='\n      <componentsupportedinterface>\n        <supportedidentifier>@__PORTINTERFACE__@</supportedidentifier>\n        <componentinstantiationref refid="DCE:@__COMPONENTINSTANCE__@"/>\n      </componentsupportedinterface>\n'

        self.externalusesport='\n      <port>\n        <usesidentifier>@__PORTNAME__@</usesidentifier>\n        <componentinstantiationref refid="DCE:@__COMPONENTINSTANCE__@"/>\n      </port>\n'

        self.externalprovidesport='\n      <port>\n        <providesidentifier>@__PORTNAME__@</providesidentifier>\n        <componentinstantiationref refid="DCE:@__COMPONENTINSTANCE__@"/>\n      </port>\n'

        self.connectinterface='\n    <connectinterface id="DCE:@__CONNECTID__@">\n@__USESPORT__@\n@__PROVIDESPORT__@\n    </connectinterface>\n'
