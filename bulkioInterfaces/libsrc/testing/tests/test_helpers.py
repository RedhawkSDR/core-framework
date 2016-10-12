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
import random
import unittest
import sys
import time
import types
from ossie.utils import sb

# Add the local search paths to find local IDL files
from ossie.utils import model
from ossie.utils.idllib import IDLLibrary
model._idllib = IDLLibrary()
model._idllib.addSearchPath('../../../idl')
model._idllib.addSearchPath('/usr/local/redhawk/core/share/idl')

# add local build path to test out api, issue with bulkio.<library> and bulkio.bulkioInterfaces... __init__.py
# differs during build process
sys.path = [ '../../build/lib' ] + sys.path

import bulkio

def str_to_class(s):
    if s in globals() and isinstance(globals()[s], types.ClassType):
        return globals()[s]
    return None

class SRI_Tests(unittest.TestCase):
    def __init__(self, methodName='runTest'):
        unittest.TestCase.__init__(self, methodName)

    def setUp(self):
        self.seq = range(10)

    def test_create(self):
        sri = bulkio.sri.create()
        
        self.assertEqual( sri.hversion, 1, "Version Incompatable" )

if __name__ == '__main__':
    if len(sys.argv) < 2 :
        unittest.main()
    else:
        suite = unittest.TestLoader().loadTestsFromTestCase(globals()[sys.argv[1]] ) 
        unittest.TextTestRunner(verbosity=2).run(suite)

##python -m unittest test_module1 test_module2
##python -m unittest test_module.TestClass
##python -m unittest test_module.TestClass.test_method
##You can pass in a list with any combination of module names, and fully qualified class or method names.
##You can run tests with more detail (higher verbosity) by passing in the -v flag:
##python -m unittest -v test_module
##For a list of all the command-line options:
##python -m unittest -h

