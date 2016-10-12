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

from base_ports  import *
from  bulkio import *
from ossie.cf import CF
from omniORB import any

class Test_Python_Int8(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int8', cname='Python_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname  )
        pass

class Test_Python_Int16(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int16', cname='Python_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname,  bio_in_module=InShortPort, bio_out_module=OutShortPort )
        pass

class Test_Python_Int32(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int32', cname='Python_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname, bio_in_module=InLongPort, bio_out_module=OutLongPort )
        pass

class Test_Python_Int64(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int64', cname='Python_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname,  bio_in_module=InLongLongPort, bio_out_module=OutLongLongPort )
        pass

class Test_Python_Float(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Float', cname='Python_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname,  bio_in_module=InFloatPort, bio_out_module=OutFloatPort )
        pass

class Test_Python_Double(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Double', cname='Python_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname,  bio_in_module=InDoublePort, bio_out_module=OutDoublePort )
        pass

if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ Test_Python_Int8, Test_Python_Int16,  Test_Python_Int32, Test_Python_Int64, Test_Python_Float, Test_Python_Double ]:
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    try:
        import xmlrunner
        runner = xmlrunner.XMLTestRunner(verbosity=2)
    except ImportError:
        runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite)
