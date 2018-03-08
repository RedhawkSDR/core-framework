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

import bulkio.sri

class StreamBase(object):
    def __init__(self, sri):
        self._sri = sri

    @property
    def streamID(self):
        return self._sri.streamID

    @property
    def sri(self):
        return self._sri

    @property
    def xstart(self):
        return self._sri.xstart

    @property
    def xdelta(self):
        return self._sri.xdelta

    @property
    def xunits(self):
        return self._sri.xunits

    @property
    def subsize(self):
        return self._sri.subsize

    @property
    def ystart(self):
        return self._sri.ystart

    @property
    def ydelta(self):
        return self._sri.ydelta

    @property
    def yunits(self):
        return self._sri.yunits

    @property
    def complex(self):
        return self._sri.mode != 0

    @property
    def blocking(self):
        return self._sri.blocking

    @property
    def keywords(self):
        return self._sri.keywords

    def hasKeyword(self, name):
        return bulkio.sri.hasKeyword(self._sri, name)

    def getKeyword(self, name):
        return bulkio.sri.getKeyword(self._sri, name)
