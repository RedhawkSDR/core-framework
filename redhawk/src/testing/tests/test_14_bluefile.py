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

import unittest
import tempfile
import shutil
import os
from ossie.utils.bluefile import bluefile

class BluefileTest(unittest.TestCase):

    def test_formWritePath (self):
        """
        Ensure that bluefile.form_write_path does not throw an exception on a
        filename with no path.
        """
        try:
            bluefile.form_write_path('out')
        except AttributeError:
            self.fail('form_write_path raises an exception with no path')

