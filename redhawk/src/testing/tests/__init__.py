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
import os
import sys

# The testing directory should be one level above this package
testdir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))

# Explicitly set the test logging config properties (if not already set)
if 'OSSIEUNITTESTSLOGCONFIG' not in os.environ:
    os.environ['OSSIEUNITTESTSLOGCONFIG'] = os.path.join(testdir, "runtest.props")

# Add the testing directory to the Python system path so that we can import
# runtests as a module, and the _unitTestHelpers package
sys.path.append(testdir)

# Set up the system paths (LD_LIBRARY_PATH, PYTHONPATH, CLASSPATH), IDL paths
# and SDRROOT to allow testing against an uninstalled framework.
import runtests
runtests.configureTestPaths()

from _unitTestHelpers import scatest
scatest.createTestDomain()
