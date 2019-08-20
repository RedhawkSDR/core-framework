#!/usr/bin/env python

import ossie.utils.testing
from ossie.utils import sb
import frontend
from omniORB import CORBA, any
from ossie.cf import CF
from ossie.properties import *
from ossie.utils import uuid

class HwLoadRequest(object):
    request_id = simple_property(
                                 id_="hw_load_request::request_id",
                                 name="request_id",
                                 type_="string")

    requester_id = simple_property(
                                   id_="hw_load_request::requester_id",
                                   name="requester_id",
                                   type_="string")

    hardware_id = simple_property(
                                  id_="hw_load_request::hardware_id",
                                  name="hardware_id",
                                  type_="string")

    load_filepath = simple_property(
                                    id_="hw_load_request::load_filepath",
                                    name="load_filepath",
                                    type_="string")

    def __init__(self, **kw):
        """Construct an initialized instance of this struct definition"""
        for classattr in type(self).__dict__.itervalues():
            if isinstance(classattr, (simple_property, simpleseq_property)):
                classattr.initialize(self)
        for k,v in kw.items():
            setattr(self,k,v)

    def __str__(self):
        """Return a string representation of this structure"""
        d = {}
        d["request_id"] = self.request_id
        d["requester_id"] = self.requester_id
        d["hardware_id"] = self.hardware_id
        d["load_filepath"] = self.load_filepath
        return str(d)

    @classmethod
    def getId(cls):
        return "hw_load_request"

    @classmethod
    def isStruct(cls):
        return True

    def getMembers(self):
        return [("request_id",self.request_id),("requester_id",self.requester_id),("hardware_id",self.hardware_id),("load_filepath",self.load_filepath)]

hw_load_request = struct_property(id_="hw_load_request",
                                  name="hw_load_request",
                                  structdef=HwLoadRequest,
                                  configurationkind=("property",),
                                  mode="readwrite")


class HwLoadStatus(object):
    request_id = simple_property(
                                 id_="hw_load_status::request_id",
                                 name="request_id",
                                 type_="string")

    requester_id = simple_property(
                                   id_="hw_load_status::requester_id",
                                   name="requester_id",
                                   type_="string")

    hardware_id = simple_property(
                                  id_="hw_load_status::hardware_id",
                                  name="hardware_id",
                                  type_="string")

    load_filepath = simple_property(
                                    id_="hw_load_status::load_filepath",
                                    name="load_filepath",
                                    type_="string")

    state = simple_property(
                            id_="hw_load_status::state",
                            name="state",
                            type_="short")

    def __init__(self, **kw):
        """Construct an initialized instance of this struct definition"""
        for classattr in type(self).__dict__.itervalues():
            if isinstance(classattr, (simple_property, simpleseq_property)):
                classattr.initialize(self)
        for k,v in kw.items():
            setattr(self,k,v)

    def __str__(self):
        """Return a string representation of this structure"""
        d = {}
        d["request_id"] = self.request_id
        d["requester_id"] = self.requester_id
        d["hardware_id"] = self.hardware_id
        d["load_filepath"] = self.load_filepath
        d["state"] = self.state
        return str(d)

    @classmethod
    def getId(cls):
        return "hw_load_status"

    @classmethod
    def isStruct(cls):
        return True

    def getMembers(self):
        return [("request_id",self.request_id),("requester_id",self.requester_id),("hardware_id",self.hardware_id),("load_filepath",self.load_filepath),("state",self.state)]


hw_load_status = struct_property(id_="hw_load_status",
                                  name="hw_load_status",
                                  structdef=HwLoadStatus,
                                  configurationkind=("property",),
                                  mode="readwrite")

hw_load_requests = structseq_property(id_="hw_load_requests",
                                              name="hw_load_requests",
                                              structdef=HwLoadRequest,
                                              defvalue=[],
                                              configurationkind=("property",),
                                              mode="readwrite")



class DeviceTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the device.
    SPD_FILE = '../ProgrammableDevice.spd.xml'

    # setUp is run before every function preceded by "test" is executed
    # tearDown is run after every function preceded by "test" is executed

    # self.comp is a device using the sandbox API
    # to create a data source, the package sb contains sources like StreamSource or FileSource
    # to create a data sink, there are sinks like StreamSink and FileSink
    # to connect the component to get data from a file, process it, and write the output to a file, use the following syntax:
    #  src = sb.FileSource('myfile.dat')
    #  snk = sb.StreamSink()
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

    def setUp(self):
        # Launch the device, using the selected implementation
        self.comp = sb.launch(self.spd_file, impl=self.impl)
        pass

    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        sb.release()

    def test_hw_load_request(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions

        my_request = HwLoadRequest()
        my_request.request_id = str(uuid.uuid1())
        my_request.requestor_id = "PG_TESTER"
        my_request.hardware_id = "PG_TESTER:1"
        my_request.load_filepath = "/the/path/file/to/load.bin"

        my_request_any =  CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), struct_to_props(my_request))

        my_requests = CF.DataType(id='hw_load_requests',
                                  value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/AnySeq:1.0"),
                                                  [ my_request_any ] ))


        hw_load_requests = structseq_property(id_="hw_load_requests",
                                              name="hw_load_requests",
                                              structdef=HwLoadRequest,
                                              defvalue=[],
                                              configurationkind=("property",),
                                              mode="readwrite")

        self.comp.start()

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
