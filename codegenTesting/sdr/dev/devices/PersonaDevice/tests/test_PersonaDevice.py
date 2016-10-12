#!/usr/bin/env python

from ossie.utils.testing import RHComponentTestCase
from ossie.utils import sb

class ResourceTests(RHComponentTestCase):
    # setUp is run before every function preceded by "test" is executed
    # tearDown is run after every function preceded by "test" is executed
    
    # self.comp is a component using the sandbox API
    # to create a data source, the package sb contains data sources like DataSource or FileSource
    # to create a data sink, there are sinks like DataSink and FileSink
    # to connect the component to get data from a file, process it, and write the output to a file, use the following syntax:
    #  src = sb.FileSource('myfile.dat')
    #  snk = sb.DataSink()
    #  src.connect(self.comp)
    #  self.comp.connect(snk)
    #  sb.start()
    #
    # components/sources/sinks need to be started. Individual components or elements can be started
    #  src.start()
    #  self.comp.start()
    #
    # every component/elements in the sandbox can be started
    #  sb.start()

    # NOTE: Persona devices cannot be launched in the sandbox, because they are
    # hosted by a programmable device. This means we cannot test in isolation,
    # so this test is currently empty.
    pass

if __name__ == "__main__":
    ossie.utils.testing.main("../PersonaDevice.spd.xml") # By default tests all implementations
