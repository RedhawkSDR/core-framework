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

class BaseFailPort(unittest.TestCase):        
    KEYS = ['c_name', 'c_inport', 'c_outport', 'sink_inport']
    PORT_FLOW = {
               'Int8' : [ 'dataCharIn', 'dataCharOut', 'charIn' ],
               'UInt8' : [ 'dataOctetIn', 'dataOctetOut', 'charIn' ],
               'Int16' : [ 'dataShortIn', 'dataShortOut', 'shortIn' ],
               'UInt16' : [ 'dataUShortIn', 'dataUShortOut', 'shortIn' ],
               'Int32' : [ 'dataLongIn', 'dataLongOut', 'longIn' ],
               'UInt32' : [ 'dataULongIn', 'dataULongOut', 'longIn' ],
               'Int64' : [ 'dataLongLongIn', 'dataLongLongOut', 'longlongIn' ],
               'UInt64' : [ 'dataULongLongIn', 'dataULongLongOut', 'longlongIn' ],
               'Float' : [ 'dataFloatIn', 'dataFloatOut', 'floatIn' ],
               'Double' : [ 'dataDoubleIn', 'dataDoubleOut', 'doubleIn' ],
               'File' : [ 'dataFileIn', 'dataFileOut', 'fileIn' ],
               'Xml' : [ 'dataXMLIn', 'dataXMLOut', 'xmlIn' ]
               }

    def __init__(
            self,
            methodName='runTest',
            ptype='Int8',
            cname=None,
            srcData=None,
            cmpData=None,
            bio_in_module=bulkio.InCharPort,
            bio_out_module=bulkio.OutCharPort ):
        unittest.TestCase.__init__(self, methodName)
        self.c_dir = 'components'
        self.c_name = cname
        self.ptype = ptype
        self.execparams = {}
        self.c_inport = None
        self.c_outport = None
        self.sink_inport = None
        self.srcData = srcData
        self.cmpData = cmpData
        self.ctx = dict().fromkeys(BaseVectorPort.KEYS)
        self.bio_in_module = bio_in_module
        self.bio_out_module = bio_out_module

    def getPortFlow(self, ptype='Int8' ):
        return BaseVectorPort.PORT_FLOW[ptype]

    def setContext(self, ctx=None):
        self.ctx[ BaseVectorPort.KEYS[0] ] = self.c_name
        self.ctx[ BaseVectorPort.KEYS[1] ] = BaseVectorPort.PORT_FLOW[self.ptype][0]
        self.ctx[ BaseVectorPort.KEYS[2] ] = BaseVectorPort.PORT_FLOW[self.ptype][1]
        self.ctx[ BaseVectorPort.KEYS[3] ] = BaseVectorPort.PORT_FLOW[self.ptype][2]
        tmp=self.ctx
        if ctx:
            tmp = ctx
        try:
            self.c_inport = tmp['c_inport']
            self.c_outport = tmp['c_outport']
            self.sink_inport = tmp['sink_inport']
        except:
            pass


    def setUp(self):
        self.setContext()
        if self.srcData:
            self.seq = self.srcData
        else:
            self.seq = range(100)
        self.launchedComps = []

    def tearDown(self):
        for comp in self.launchedComps:
            comp.releaseObject()
        self.launchedComps = []

    def launch(self, *args, **kwargs):
        comp = sb.launch(*args, **kwargs)
        self.launchedComps.append(comp)
        return comp

    def test_connection_fail(self):
        import os
        in_sri = bulkio.sri.create()
        in_sri.streamID = "VECTOR-PUSHPACKET-SIMPLE"
        in_sri.mode = 0
        in_sri.xdelta  = 1/33.0
        dsource=sb.DataSource()
        dsink=sb.DataSink()
        c_spd_xml = test_dir + self.c_dir + '/' + self.c_name + '/' + self.c_name + '.spd.xml'
        print "Test Component:" + c_spd_xml
        self.execparams = { 'LOGGING_CONFIG_URI' : 'file://'+os.getcwd()+'/log4j.ex1' }
        data=self.seq
        c=self.launch( c_spd_xml, execparams=self.execparams)
        c1=self.launch( c_spd_xml, execparams=self.execparams)

        dsource.connect(c, providesPortName=self.c_inport )
        c.connect(c1,usesPortName=self.c_outport,providesPortName=self.c_inport)
        c.connect(dsink, providesPortName=self.sink_inport, usesPortName=self.c_outport)
        os.kill(c1._pid, 9)
        while ( c1._process.isAlive() == True ) : time.sleep(.5)
        dsource.start()
        dsource.push( data, EOS=False, streamID=in_sri.streamID, sampleRate=33.0, complexData=(in_sri.mode==1))
        dsource.push( data, EOS=False)
        dsource.push( data, EOS=False)
        dsource.push( data, EOS=False)
        dsource.push( data, EOS=False)
        dsource.push( data, EOS=False)
        dsource.push( data, EOS=False)
        dsource.push( data, EOS=False)
        dsource.push( data, EOS=False)
        dsource.push( data, EOS=True)
        c.start()
        adata=dsink.getData(eos_block=True)


class Test_Int8_Fail_CPP(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Int8', cname='CPP_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname )
        pass

class Test_Int16_Fail_CPP(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Int16', cname='CPP_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname ,
                                bio_in_module=bulkio.InShortPort,
                                bio_out_module=bulkio.OutShortPort )
        pass

class Test_Int32_Fail_CPP(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Int32', cname='CPP_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname,
                                bio_in_module=bulkio.InLongPort,
                                bio_out_module=bulkio.OutLongPort )
        pass

class Test_Int64_Fail_CPP(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Int64', cname='CPP_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname,
                                bio_in_module=bulkio.InLongLongPort,
                                bio_out_module=bulkio.OutLongLongPort )
        pass

class Test_Float_Fail_CPP(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Float', cname='CPP_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname ,
                                bio_in_module=bulkio.InFloatPort,
                                bio_out_module=bulkio.OutFloatPort )
        pass

class Test_Double_Fail_CPP(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Double', cname='CPP_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname,
                                bio_in_module=bulkio.InDoublePort,
                                bio_out_module=bulkio.OutDoublePort )
        pass

class Test_File_Fail_CPP(BaseFailPort):
    _sample = "The quick brown fox jumped over the lazy dog"
    def __init__(self, methodName='runTest', ptype='File', cname='CPP_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname, srcData=Test_File_Fail_CPP._sample,
                                bio_in_module=bulkio.InFilePort,
                                bio_out_module=bulkio.OutFilePort )
        pass


class Test_Int8_Fail_Java(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Int8', cname='Java_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname )
        pass

class Test_Int16_Fail_Java(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Int16', cname='Java_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname ,
                                bio_in_module=bulkio.InShortPort,
                                bio_out_module=bulkio.OutShortPort )
        pass

class Test_Int32_Fail_Java(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Int32', cname='Java_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname,
                                bio_in_module=bulkio.InLongPort,
                                bio_out_module=bulkio.OutLongPort )
        pass

class Test_Int64_Fail_Java(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Int64', cname='Java_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname,
                                bio_in_module=bulkio.InLongLongPort,
                                bio_out_module=bulkio.OutLongLongPort )
        pass

class Test_Float_Fail_Java(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Float', cname='Java_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname ,
                                bio_in_module=bulkio.InFloatPort,
                                bio_out_module=bulkio.OutFloatPort )
        pass

class Test_Double_Fail_Java(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Double', cname='Java_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname,
                                bio_in_module=bulkio.InDoublePort,
                                bio_out_module=bulkio.OutDoublePort )
        pass

class Test_File_Fail_Java(BaseFailPort):
    _sample = "The quick brown fox jumped over the lazy dog"
    def __init__(self, methodName='runTest', ptype='File', cname='Java_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname, srcData=Test_File_Fail_Java._sample,
                                bio_in_module=bulkio.InFilePort,
                                bio_out_module=bulkio.OutFilePort )
        pass



class Test_Int8_Fail_Python(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Int8', cname='Python_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname )
        pass

class Test_Int16_Fail_Python(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Int16', cname='Python_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname ,
                                bio_in_module=bulkio.InShortPort,
                                bio_out_module=bulkio.OutShortPort )
        pass

class Test_Int32_Fail_Python(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Int32', cname='Python_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname,
                                bio_in_module=bulkio.InLongPort,
                                bio_out_module=bulkio.OutLongPort )
        pass

class Test_Int64_Fail_Python(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Int64', cname='Python_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname,
                                bio_in_module=bulkio.InLongLongPort,
                                bio_out_module=bulkio.OutLongLongPort )
        pass

class Test_Float_Fail_Python(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Float', cname='Python_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname ,
                                bio_in_module=bulkio.InFloatPort,
                                bio_out_module=bulkio.OutFloatPort )
        pass

class Test_Double_Fail_Python(BaseFailPort):
    def __init__(self, methodName='runTest', ptype='Double', cname='Python_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname,
                                bio_in_module=bulkio.InDoublePort,
                                bio_out_module=bulkio.OutDoublePort )
        pass

class Test_File_Fail_Python(BaseFailPort):
    _sample = "The quick brown fox jumped over the lazy dog"
    def __init__(self, methodName='runTest', ptype='File', cname='Python_Ports' ):
        BaseFailPort.__init__(self, methodName, ptype, cname, srcData=Test_File_Fail_Python._sample,
                                bio_in_module=bulkio.InFilePort,
                                bio_out_module=bulkio.OutFilePort )
        pass


if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ Test_Int8_Fail_CPP, Test_Int16_Fail_CPP, Test_Int32_Fail_CPP, Test_Double_Fail_CPP, Test_Float_Fail_CPP, Test_File_Fail_CPP,
               Test_Int8_Fail_Java, Test_Int16_Fail_Java, Test_Int32_Fail_Java, Test_Double_Fail_Java, Test_Float_Fail_Java, Test_File_Fail_Java,
               Test_Int8_Fail_Python, Test_Int16_Fail_Python, Test_Int32_Fail_Python, Test_Double_Fail_Python, Test_Float_Fail_Python, Test_File_Fail_Python ]:
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    try:
        import xmlrunner
        runner = xmlrunner.XMLTestRunner(verbosity=2)
    except ImportError:
        runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite)
