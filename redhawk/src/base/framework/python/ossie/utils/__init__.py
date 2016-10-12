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

from popen import Popen

# Try to use the uuid module from Python 2.5 or newer; if that fails, use our
# fallback compatibility module. Should the minimum Python version ever be
# raised to 2.5, the compatibility module can be eliminated.
try:
    import uuid
except ImportError:
    import _uuid as uuid

# Manually insert the uuid module into sys.modules to allow statements such as
# "import ossie.utils.uuid" to work as expected.
import sys
sys.modules['ossie.utils.uuid'] = uuid
del sys
