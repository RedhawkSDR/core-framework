#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK throughput.
#
# REDHAWK throughput is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK throughput is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import math

_SI_PREFIXES = ['', 'K', 'M', 'G', 'T']

def _from_si_prefix(value, base):
    if value[-1].isalpha():
        suffix = value[-1].upper()
        value = value[:-1]
        index = _SI_PREFIXES.index(suffix)
        scale = base**index
    else:
        scale = 1
    return value, scale

def _to_si_prefix(value, base):
    index = int(math.floor(math.log(value, base)))
    return (value/math.pow(base, index), _SI_PREFIXES[index])

def from_binary(value):
    value, scale = _from_si_prefix(value, 1024)
    return int(value)*scale

def to_binary(value):
    return '%d%s' % _to_si_prefix(value, 1024)

def time_to_sec(value):
    scale = 1.0
    if value[-1].isalpha():
        suffix = value[-1].lower()
        value = value[:-1]
        if suffix == 'm':
            scale = 60.0
        elif suffix == 's':
            scale = 1.0
    return float(value)*scale

def to_gbps(value):
    return '%.2f' % (value/(1024**3))

def to_percent(value):
    return '%.1f' % (value*100.0)

