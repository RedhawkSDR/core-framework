#! /usr/bin/env python
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


from distutils.core import setup
import sys

try:
    import tabnanny
except ImportError:
    pass
else:
    import StringIO
    stdout, stderr = sys.stdout, sys.stderr
    sys.stdout = co = StringIO.StringIO()
    sys.stderr = ce = StringIO.StringIO()
    # Tabnanny doesn't provide any mechanism other than print outs so we have
    # to capture the output
    tabnanny.check("_unitTestHelpers")
    sys.stdout = stdout
    sys.stderr = stderr
    if len(co.getvalue().strip()) != 0:
        print "Incosistent tab usage:"
        print co.getvalue()
        sys.exit(-1)

unitTestHelper = [
    '_unitTestHelpers.scatest',
    '_unitTestHelpers.runtestHelpers',
    '_unitTestHelpers.buildconfig']

version='1.10.1'

setup(
    name='unitTestHelper',
    version=version,
    description='Unit Test Helpers',
    py_modules=unitTestHelper,
    scripts=[],
)
