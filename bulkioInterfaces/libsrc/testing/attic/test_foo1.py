#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import sys
from ossie.utils import sb

dsource=sb.DataSource()
dsink=sb.DataSink()
test_comp=sb.Component('Foo')
data=range(100)
dsource.connect(test_comp, providesPortName='dataShortIn')
test_comp.connect(dsink, providesPortName='shortIn', usesPortName='dataShortOut')
sb.start()
dsource.push(data)
dest_data=dsink.getData()
