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
from ossie.utils import sb
import time


class BaseVectorPort(unittest.TestCase):
    KEYS = ['c_name', 'c_inport', 'c_outport', 'sink_inport']
    PORTS = { 
               'Int8' : [ 'dataCharIn', 'dataCharOut', 'charIn' ],
               'UInt8' : [ 'dataOctetIn', 'dataOctetOut', 'charIn' ],
               'Int16' : [ 'dataShortIn', 'dataShortOut', 'shortIn' ],
               'UInt16' : [ 'dataUShortIn', 'dataUShortOut', 'shortIn' ],
               'Int32' : [ 'dataLongIn', 'dataLongOut', 'longIn' ],
               'UInt32' : [ 'dataULongIn', 'dataULongOut', 'longIn' ],
               'Int64' : [ 'dataLongLongIn', 'dataLongLongOut', 'longlongIn' ],
               'UInt64' : [ 'dataULongLongIn', 'dataULongLongOut', 'longlongIn' ],
               'Float' : [ 'dataFloatIn', 'dataFloatOut', 'floatIn' ],
               'Double' : [ 'dataDoubleIn', 'dataDoubleOut', 'doubleIn' ]
               }
    
    def __init__(self, methodName='runTest'):
        unittest.TestCase.__init__(self, methodName)
        ##self.cname = 'CPP_Ports'
        ##self.ptype = 'Int8'
        self.inport = None
        self.outport = None
        self.sink_port_name = None
        self.ctx = dict().fromkeys(BaseVectorPort.KEYS)
        self.setContext()

    def getPortFlow(self, ptype='Int8' ):
        return BaseVectorPort.PORTS[ptype]

    def setContext(self, ptype=None, cname=None, ctx=None ):
        if cname:
            self.ctx[ BaseVectorPort.KEYS[0] ] = cname
        else:
            self.ctx[ BaseVectorPort.KEYS[0] ] = self.cname

        if ptype == None:
            ptype = self.ptype

        self.ctx[ BaseVectorPort.KEYS[1] ] = BaseVectorPort.PORTS[ptype][0]
        self.ctx[ BaseVectorPort.KEYS[2] ] = BaseVectorPort.PORTS[ptype][1]
        self.ctx[ BaseVectorPort.KEYS[3] ] = BaseVectorPort.PORTS[ptype][2]

        tmp=self.ctx
        if ctx:
            tmp = ctx
        try:
            self.cname = tmp['c_name']
            self.inport = tmp['c_inport']
            self.outport = tmp['c_outport']
            self.sink_port_name = tmp['sink_inport']
        except:
            pass
        

    def setUp(self):
        self.seq = range(10)
        self.setContext()

    def test_vector(self):
        dsource=sb.DataSource()
        dsink=sb.DataSink()
        test_comp=sb.Component(self.cname)
        data=range(100)
        dsource.connect(test_comp, providesPortName=self.inport )
        test_comp.connect(dsink, providesPortName=self.sink_port_name, usesPortName=self.outport)
        sb.start()
        dsource.push(data,EOS=True)
        dest_data=dsink.getData(eos_block=True)
        sb.stop()

        self.assertEqual(data, dest_data)

class Base_CPP_Port(BaseVectorPort):
    C_NAME='CPP_Ports'
    def __init__(self, ptype='Int8', methodName='runTest'):
        self.cname = Base_CPP_Port.C_NAME
        self.ptype = ptype
        BaseVectorPort.__init__(self, methodName)

class Base_Python_Port(BaseVectorPort):
    C_NAME='Python_Ports'
    def __init__(self, ptype='Int8', methodName='runTest'):
        self.cname = Base_Python_Port.C_NAME
        self.ptype = ptype
        BaseVectorPort.__init__(self, methodName)

class Base_Java_Port(BaseVectorPort):
    C_NAME='Java_Ports'
    def __init__(self, methodName='runTest'):
        self.cname = Base_Java_Port.C_NAME
        self.ptype = 'Int8'
        BaseVectorPort.__init__(self, methodName)

class Test_CPP_Int8(Base_CPP_Port):
    def __init__(self, methodName='runTest'):
        Base_CPP_Port.__init__(self, 'Int8', methodName)

class Test_CPP_Int16(Base_CPP_Port):
    def __init__(self, methodName='runTest'):
        Base_CPP_Port.__init__(self, 'Int16', methodName)

class Test_CPP_Int32(Base_CPP_Port):
    def __init__(self, methodName='runTest'):
        Base_CPP_Port.__init__(self, 'Int32', methodName)

class Test_CPP_Int64(Base_CPP_Port):
    def __init__(self, methodName='runTest'):
        Base_CPP_Port.__init__(self, 'Int64', methodName)

class Test_CPP_Float(Base_CPP_Port):
    def __init__(self, methodName='runTest'):
        Base_CPP_Port.__init__(self, 'Float', methodName)

class Test_CPP_Double(Base_CPP_Port):
    def __init__(self, methodName='runTest'):
        Base_CPP_Port.__init__(self, 'Double', methodName)


if __name__ == '__main__':
#    unittest.main()
    suite = unittest.TestLoader().loadTestsFromTestCase(Base_CPP_Port)
    unittest.TextTestRunner(verbosity=2).run(suite)

##python -m unittest test_module1 test_module2
##python -m unittest test_module.TestClass
##python -m unittest test_module.TestClass.test_method

##You can pass in a list with any combination of module names, and fully qualified class or method names.

##You can run tests with more detail (higher verbosity) by passing in the -v flag:

##python -m unittest -v test_module

##For a list of all the command-line options:

##python -m unittest -h

