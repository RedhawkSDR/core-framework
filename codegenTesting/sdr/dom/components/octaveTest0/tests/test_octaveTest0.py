#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK codegenTesting.
#
# REDHAWK codegenTesting is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK codegenTesting is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import unittest
import ossie.utils.testing
import os
from omniORB import any

from ossie.utils import sb
import time
import copy

PAUSE = .1  # seconds to send to sleep statements

class ResourceTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all resource implementations in octaveTest0"""

    def pushDataAndCheck(
            self,
            inputData        = range(100),
            expectedOutput   = range(100),
            pause            = PAUSE,
            streamID         = 'defaultStreamID',
            sampleRate       = 1.0,
            complexData      = False,
            SRIKeywords      = [],
            usesPortName     = "vectorOut0",
            providesPortName = "vectorIn0"):

        dataFormat = "double"
        source = sb.StreamSource(streamID=streamID, format=dataFormat)
        sink = sb.StreamSink(format=dataFormat)
        source.xdelta = 1.0 / sampleRate
        source.complex = complexData
        interleaved = False
        if source.complex:
            interleaved = True
        for kw in SRIKeywords:
            source.setKeyword(kw._name, kw._value, format=kw._format)

        # connect source -> component -> sink
        source.connect(self.comp)
        self.comp.connect(sink)

        # Start all
        source.start()
        self.comp.start()
        sink.start()

        source.write(inputData, interleaved=interleaved)

        # Give the component some time to process the data
        time.sleep(pause)
        streamData = sink.read()
        retData = []
        retSri = None
        if streamData:
            retData = streamData.data
            retSri = streamData.sri
        self.assertEquals(retData, expectedOutput)

        # SRI tests
        self.assertEquals(streamData.eos, False)
        self.assertEquals(retSri.streamID, streamID)

        # sample rate is checked via Xdelta
        self.assertEquals(retSri.xdelta, 1./sampleRate)
        self.assertEquals(retSri.mode, complexData)

        # SRI keywords
        if len(SRIKeywords) != len(retSri.keywords):
            self.fail("Incorrect number of SRI keywords returned by component.")
        for expectedKeyword in SRIKeywords:
            found = False
            for retKeyword in retSri.keywords:
                if retKeyword.id == expectedKeyword._name:
                    self.assertEquals(str(retKeyword.value),
                                      str(any.to_any(expectedKeyword._value)))
                    found = True
                    break
            if not found:
                self.fail("SRI keyword not found: " + expectedKeyword.id)

        # Check for eos
        source.close()
        streamData = sink.read(timeout=pause, eos=True)
        retData = []
        retSri = None
        if streamData:
            retData = streamData.data
            retSri = streamData.sri
        self.assertEquals(streamData.eos, True)
        self.assertEquals(retSri.streamID, streamID)

        source.releaseObject()
        sink.releaseObject()

    def test0_vectorData(self):
        # Launch the resource with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",),
                                         modes=("readwrite", "writeonly"),
                                         includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])

        self.launch(execparams)

        inputData = range(100)

        # test with default prop setting
        #
        # get a copy of the prop value so that we don't query in every iteration
        # when creating expectedOutput
        constIn0 = self.comp.constIn0
        expectedOutput = [x+constIn0 for x in inputData]
        self.pushDataAndCheck(
            inputData      = inputData,
            expectedOutput = expectedOutput)

        # test with modified prop setting
        #
        # set the property to some arbitrary value that is not the default
        self.comp.constIn0 = 0.5
        # get a copy of the prop value so that we don't query in every iteration
        # when creating expectedOutput
        constIn0 = self.comp.constIn0
        expectedOutput = [x+constIn0 for x in inputData]
        self.pushDataAndCheck(
            inputData      = inputData,
            expectedOutput = expectedOutput)

        # test complex IO
        expectedOutput = []
        i = 0
        while i < len(inputData) - 1:
            cplx = complex(inputData[i], inputData[i + 1])
            cplx += constIn0
            expectedOutput.append(cplx)
            i += 2

        self.pushDataAndCheck(
            inputData      = inputData,
            expectedOutput = expectedOutput,
            complexData    = True)

        # Simulate regular resource shutdown
        self.comp.releaseObject()


    def test1_configureQuery(self):
        # Launch the resource with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",),
                                         modes=("readwrite", "writeonly"),
                                         includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)

        # test string props
        self.comp.stringPropIO1 = "foo"
        # need to push some data so that feval gets called
        self.pushDataAndCheck()
        self.assertEquals(self.comp.stringPropIO1, "foo")
        self.assertEquals(self.comp.stringPropOutput1, "output")

        # complex props
        self.comp.complexPropIO1 = complex(1.5, 2.5)
        # need to push some data so that feval gets called
        self.pushDataAndCheck()
        self.assertEquals(self.comp.complexPropIO1, complex(1.5, 2.5))
        self.assertEquals(self.comp.complexPropOutput1, complex(1, 2))

        # test vector props
        self.comp.vectorPropIO1 = range(10)
        complexVect = [complex(-1,-1), complex(0, 2), complex(5.0, 6)]
        self.comp.complexVectorPropIO1 = complexVect
        # need to push some data so that feval gets called
        self.pushDataAndCheck()
        self.assertEquals(self.comp.vectorPropIO1, range(10))
        self.assertEquals(self.comp.vectorPropOutput1, [-1, 0, 1, 2.5])
        self.assertEquals(self.comp.complexVectorPropIO1, complexVect)
        self.assertEquals(
            self.comp.complexVectorOutput1, 
            [complex(-1,-1), complex(0,0), complex(1,0), complex(2,2)])

        # test SRI keywords
        inputData = xrange(100)
        expectedOutput = []
        i = 0
        while i < len(inputData) - 1:
            cplx = complex(inputData[i], inputData[i + 1])
            expectedOutput.append(cplx)
            i += 2
        SRIKeywords = [sb.io_helpers.SRIKeyword('val1', 1, 'long'),
                       sb.io_helpers.SRIKeyword('val2', 'foo', 'string')]
        self.pushDataAndCheck(streamID       = "testSRI",
                              expectedOutput = expectedOutput,
                              sampleRate     = 50,
                              complexData    = True,
                              SRIKeywords    = SRIKeywords)

        # Simulate regular resource shutdown
        self.comp.releaseObject()


    def testScaBasicBehavior(self):
        #######################################################################
        # Launch the resource with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)

        #######################################################################
        # Verify the basic state of the resource
        self.assertNotEqual(self.comp, None)
        self.assertEqual(self.comp.ref._non_existent(), False)

        self.assertEqual(self.comp.ref._is_a("IDL:CF/Resource:1.0"), True)

        #######################################################################
        # Validate that query returns all expected parameters
        # Query of '[]' should return the following set of properties
        expectedProps = []
        expectedProps.extend(self.getPropertySet(kinds=("configure", "execparam"), modes=("readwrite", "readonly"), includeNil=True))
        expectedProps.extend(self.getPropertySet(kinds=("allocate",), action="external", includeNil=True))
        props = self.comp.query([])
        props = dict((x.id, any.from_any(x.value)) for x in props)
        # Query may return more than expected, but not less
        for expectedProp in expectedProps:
            self.assertEquals(props.has_key(expectedProp.id), True)

        #######################################################################
        # Verify that all expected ports are available
        for port in self.scd.get_componentfeatures().get_ports().get_uses():
            port_obj = self.comp.getPort(str(port.get_usesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a("IDL:CF/Port:1.0"),  True)

        for port in self.scd.get_componentfeatures().get_ports().get_provides():
            port_obj = self.comp.getPort(str(port.get_providesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a(port.get_repid()),  True)

        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.start()
        self.comp.stop()

        #######################################################################
        # Simulate regular resource shutdown
        self.comp.releaseObject()

if __name__ == "__main__":
    ossie.utils.testing.main("../octaveTest0.spd.xml") # By default tests all implementations
