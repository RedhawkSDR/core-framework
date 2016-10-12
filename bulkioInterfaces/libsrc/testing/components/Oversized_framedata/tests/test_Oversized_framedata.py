#!/usr/bin/env python
import unittest
import ossie.utils.testing
import os
from omniORB import any
from ossie.utils import sb

class ResourceTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all resource implementations in Oversized_framedata"""

    def testConsistentSize(self):
        #######################################################################
        # Launch the resource with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        snk=sb.DataSink()
        snk.start()
        self.comp.connect(snk)

        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.start()
        
        (retval, timestamps) = snk._sink.retrieveData(20000000)
        self.assertEquals(timestamps[1][0]%1024,0) 

        #######################################################################
        # Simulate regular resource shutdown
        snk.releaseObject()
        self.comp.releaseObject()
        
    # TODO Add additional tests here
    #
    # See:
    #   ossie.utils.bulkio.bulkio_helpers,
    #   ossie.utils.bluefile.bluefile_helpers
    # for modules that will assist with testing resource with BULKIO ports

if __name__ == "__main__":
    ossie.utils.testing.main("../Oversized_framedata.spd.xml") # By default tests all implementations
