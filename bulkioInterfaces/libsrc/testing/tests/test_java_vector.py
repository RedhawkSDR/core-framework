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


class Test_Java_Int8(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int8', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname )
        pass

class Test_Java_Int16(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int16', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname,
                                bio_in_module=bulkio.InShortPort,
                                bio_out_module=bulkio.OutShortPort )
        pass

class Test_Java_Int32(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int32', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname,
                                bio_in_module=bulkio.InLongPort,
                                bio_out_module=bulkio.OutLongPort )
        pass

class Test_Java_Int64(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int64', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname,
                                bio_in_module=bulkio.InLongLongPort,
                                bio_out_module=bulkio.OutLongLongPort )
        pass

class Test_Java_Float(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Float', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname,
                                bio_in_module=bulkio.InFloatPort,
                                bio_out_module=bulkio.OutFloatPort )
        pass

class Test_Java_Double(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Double', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname,
                                bio_in_module=bulkio.InDoublePort,
                                bio_out_module=bulkio.OutDoublePort )
        pass


if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ Test_Java_Int8, Test_Java_Int16,  Test_Java_Int32, Test_Java_Int64, Test_Java_Float, Test_Java_Double ]:
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    try:
        import xmlrunner
        runner = xmlrunner.XMLTestRunner(verbosity=2)
    except ImportError:
        runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite)
