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
#
#
#

#
# base_port
#  runs same set of tests for each type of port specified...
#

from base_ports  import *


class Test_CPP_Int8(BaseVectorPort):
    def __init__(self, *args, **kwargs):
        BaseVectorPort.__init__(self, 'Int8', 'CPP_Ports', *args, **kwargs)

class Test_CPP_Int16(BaseVectorPort):
    def __init__(self, *args, **kwargs):
        BaseVectorPort.__init__(self, 'Int16', 'CPP_Ports', *args, **kwargs)

class Test_CPP_Int32(BaseVectorPort):
    def __init__(self, *args, **kwargs):
        BaseVectorPort.__init__(self, 'Int32', 'CPP_Ports', *args, **kwargs)

class Test_CPP_Int64(BaseVectorPort):
    def __init__(self, *args, **kwargs):
        BaseVectorPort.__init__(self, 'Int64', 'CPP_Ports', *args, **kwargs)

class Test_CPP_Float(BaseVectorPort):
    def __init__(self, *args, **kwargs):
        BaseVectorPort.__init__(self, 'Float', 'CPP_Ports', *args, **kwargs)

class Test_CPP_Double(BaseVectorPort):
    def __init__(self, *args, **kwargs):
        BaseVectorPort.__init__(self, 'Double', 'CPP_Ports', *args, **kwargs)

class Test_CPP_File(BaseVectorPort):
    _sample = "The quick brown fox jumped over the lazy dog"
    def __init__(self, *args, **kwargs):
        BaseVectorPort.__init__(self, 'File', 'CPP_Ports', *args, srcData=Test_CPP_File._sample, **kwargs)


if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ Test_CPP_Int8, Test_CPP_Int16,  Test_CPP_Int32, Test_CPP_Int64, Test_CPP_Float, Test_CPP_Double, Test_CPP_File ]:
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    try:
        import xmlrunner
        runner = xmlrunner.XMLTestRunner(verbosity=2)
    except ImportError:
        runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite)
