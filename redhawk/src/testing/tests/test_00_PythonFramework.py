#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

from ossie.resource import Resource, usesport, providesport
import os.path
import scatest
from ossie.cf import CF, CF__POA
from ossie.properties import *
from ossie.events import *
from omniORB import any, CORBA
import sets
import logging
import time
import CosEventChannelAdmin
import CosNaming
import CosLifeCycle

logging.basicConfig()
logging.getLogger().setLevel(logging.ERROR)

###############################################################################   
# Here is an example of a component that uses a combination of both the 
# old-style 'tuple-based' and the new-style 'property-based' methods
# for populating the PropertyStore
PROPERTIES = (
              (
               u'DCE:6b298d70-6735-43f2-944d-06f754cd4eb9', # ID
               u'no_default_prop', # NAME
               u'string', # TYPE
               u'readwrite', # MODE
               None, # DEFAULT
               None, # UNITS
               'external', # ACTION
               (u'configure',), # KINDS
              ),
              (
               u'DCE:456310b2-7d2f-40f5-bfef-9fdf4f3560ea', # ID
               u'default_prop', # NAME
               u'string', # TYPE
               u'readwrite', # MODE
               'default', # DEFAULT
               None, # UNITS
               'external', # ACTION
               (u'configure',), # KINDS
              ),
              (
               u'DCE:4a23ad60-0b25-4121-a630-68803a498f75', # ID
               u'os_name', # NAME
               u'string', # TYPE
               u'readonly', # MODE
               'Linux', # DEFAULT
               None, # UNITS
               u'eq', # ACTION
               (u'allocation',), # KINDS
              ),
              (
               u'DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b', # ID
               u'processor_name', # NAME
               u'string', # TYPE
               u'readonly', # MODE
               'x86', # DEFAULT
               None, # UNITS
               u'eq', # ACTION
               (u'allocation',), # KINDS
              ),
              (
               u'DCE:7f36cdfb-f828-4e4f-b84f-446e17f1a85b', # ID
               u'DeviceKind', # NAME
               u'string', # TYPE
               u'readonly', # MODE
               'BasicTestDevice', # DEFAULT
               None, # UNITS
               u'eq', # ACTION
               (u'allocation',), # KINDS
              ),
              (
               u'DCE:64303822-4c67-4c04-9a5c-bf670f27cf39', # ID
               u'RunsAs', # NAME
               u'string', # TYPE
               u'readonly', # MODE
               'root', # DEFAULT
               None, # UNITS
               u'ne', # ACTION
               (u'allocation',), # KINDS
              ),
              (
               u'DCE:021f10cf-7a05-46ec-a507-04b513b84bd4', # ID
               u'HasXMIDAS', # NAME
               u'boolean', # TYPE
               u'readonly', # MODE
               True, # DEFAULT
               None, # UNITS
               u'eq', # ACTION
               (u'allocation',), # KINDS
              ),
              (
               u'DCE:ac73446e-f935-40b6-8b8d-4d9adb6b403f', # ID
               u'ProvidedCpuCores', # NAME
               u'short', # TYPE
               u'readonly', # MODE
               8, # DEFAULT
               None, # UNITS
               u'ge', # ACTION
               (u'allocation',), # KINDS
              ),
              (
               u'DCE:dd339b67-b387-4018-94d2-9a72955d85b9', # ID
               u'CoresClockRateGHz', # NAME
               u'float', # TYPE
               u'readonly', # MODE
               3.0, # DEFAULT
               None, # UNITS
               u'le', # ACTION
               (u'allocation',), # KINDS
              ),
              (
               u'DCE:8dcef419-b440-4bcf-b893-cab79b6024fb', # ID
               u'memCapacity', # NAME
               u'long', # TYPE
               u'readonly', # MODE
               None, # DEFAULT
               None, # UNITS
               'external', # ACTION
               (u'allocation',), # KINDS
              ),
              (
               u'DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8', # ID
               u'BogoMipsCapacity', # NAME
               u'long', # TYPE
               u'readonly', # MODE
               None, # DEFAULT
               None, # UNITS
               'external', # ACTION
               (u'allocation',), # KINDS
              ),
              (
               u'DCE:c03e148f-e9f9-4d70-aa00-6e23d33fa648', # ID
               u'SomeConfigFileLocation', # NAME
               u'string', # TYPE
               u'readonly', # MODE
               'notyourfile', # DEFAULT
               None, # UNITS
               'external', # ACTION
               (u'execparam',), # KINDS
              ),
              (
               u'DCE:dc4289a8-bb98-435b-b914-305ffaa7594f', # ID
               u'ImplementationSpecificProperty', # NAME
               u'string', # TYPE
               u'readonly', # MODE
               'DefaultValueNoGood', # DEFAULT
               None, # UNITS
               'external', # ACTION
               (u'execparam',), # KINDS
              ),
              (
               u'DCE:716ea1c4-059a-4b18-8b66-74804bd8d435', # ID
               u'ImplementationSpecificProperty2', # NAME
               u'long', # TYPE
               u'readonly', # MODE
               0, # DEFAULT
               None, # UNITS
               'external', # ACTION
               (u'execparam',), # KINDS
              ),
             )

class TestResource(Resource):
            # Include both old style and new style propertydefs
            def __init__(self, execparams={}):
                Resource.__init__(self, "TestResource", execparams, propertydefs=PROPERTIES)
                # For port defintions that use fget, this is illegal syntax and
                # will throw an AttributeError
                self.port_input2 = TestResource.Input2Port()
                self._input = TestResource.InputPort()
                self._output = TestResource.OutputPort()


            # You can define a port using an fget.  This let's you
            # control the lifecyle of the port.  The fget function
            # *MUST* return a CORBA client reference (i.e. the value
            # returned from _this
            class InputPort(CF__POA.LifeCycle):
                def nonCorbaCall(self):
                    return True

            def get_port_input(self):
                return self._input

            port_input = providesport(name="input", 
                                      repid="IDL:CF/LifeCycle:1.0", 
                                      type_="control",
                                      fget=get_port_input)

            class OutputPort(CF__POA.Port):
                def nonCorbaCall(self):
                    return True

            def get_port_output(self):
                return self._output

            port_output = usesport(name="output", 
                                   repid="IDL:CF/LifeCycle:1.0", 
                                   type_="control",
                                   fget=get_port_output)

            # You can also define a port without an fget
            # When you do so, your constructor must set
            # the class attribute with the 'servant' object
            class Input2Port(CF__POA.LifeCycle):
                def nonCorbaCall(self):
                    return True

            port_input2 = providesport(name="input2", 
                                      repid="IDL:CF/LifeCycle:1.0", 
                                      type_="control")
           

            def set_someprop(self, value):
                self._someprop = value

            def get_someprop(self):
                return self._someprop

            def runIntenalTests(self, unittest):
                """Run some tests to see if the properties behave as expected when
                accessed from inside the object."""
                self.someprop = 12345
                self._props["DCE:456310b2-7d2f-40f5-bfef-9fdf4f3560ea"] = "Hello"
                self._props["someprop2"] = 5432.1
                
                unittest.assertEqual(self._props["default_prop"], "Hello")
                unittest.assertEqual(self.someprop, 12345)
                unittest.assertEqual(self._props["someprop"], 12345)
                unittest.assertEqual(self.someprop2, 5432.1)
                unittest.assertEqual(self._props["someprop2"], 5432.1)

                unittest.assertEqual(self._props["astruct"].field1, 123.4)
                unittest.assertEqual(self._props["astruct"].field2, 88)
                unittest.assertEqual(self._props["astruct"].field3, "some value")
                unittest.assertEqual(self.struct.field1, 123.4)
                unittest.assertEqual(self.struct.field2, 88)
                unittest.assertEqual(self.struct.field3, "some value")

                self.struct.field1 = 987.6
                self.struct.field2 = 55
                self.struct.field3 = "nothing"

                unittest.assertEqual(self._props["astruct"].field1, 987.6)
                unittest.assertEqual(self._props["astruct"].field2, 55)
                unittest.assertEqual(self._props["astruct"].field3, "nothing")
                unittest.assertEqual(self.struct.field1, 987.6)
                unittest.assertEqual(self.struct.field2, 55)
                unittest.assertEqual(self.struct.field3, "nothing")

                # Ensure that the "port" attributes are python objects
                # and not CORBA client refs
                unittest.assertEqual(self.port_input.nonCorbaCall(), True)
                unittest.assertEqual(self.port_input2.nonCorbaCall(), True)
                unittest.assertEqual(self.port_output.nonCorbaCall(), True)
               
                unittest.assertEqual(type(self.invalidprop), str) 
                self.invalidprop = 50 # Change the type of invalidprop from string to int
                unittest.assertEqual(type(self.invalidprop), int) 

            # An example that uses custom setters/getters
            someprop = simple_property(\
                id_='DCE:fa8c5924-845c-484a-81df-7941f2c5baa9',
                type_="long",
                name="someprop",
                defvalue=10,
                fset=set_someprop,
                fget=get_someprop)

            # An example that uses built in value storage
            someprop2 = simple_property(\
                id_='DCE:cf623573-a09d-4fb1-a2ae-24b0b507115d',
                type_="double",
                name="someprop2",
                defvalue=50.0)

            # This propery should be a string, but the component
            # will set it to a integer...the properties.py should
            # fix that so the returned any has a string type code
            invalidprop = simple_property(\
                id_='DCE:19a51bb2-1fd2-4c53-9215-a29bc10638e3',
                type_="string",
                name="invalidprop",
                defvalue="100")

            invalidprop2 = simple_property(\
                id_='DCE:3107571b-bde0-41c6-a840-c473b1a41b25',
                type_="short",
                name="invalidprop2",
                defvalue=10)
            
            # An exec-parameter
            execparam3 = simple_property(\
                id_='EXEC_PARAM_3',
                type_="float",
                name="Parameter 3",
                defvalue=3.125,
                kinds=("execparam",),
                mode="readwrite")

            execparam4 = simple_property(\
                id_='EXEC_PARAM_4',
                type_="float",
                name="Parameter 4",
                defvalue=1.0,
                kinds=("execparam",),
                mode="writeonly")

            # A standard sequence property that doesn't support
            # operators
            noopseqprop = simpleseq_property(\
                id_='noopseqprop',
                type_="long",
                name="noopseqprop",
                defvalue=(0, 1, 2, 3))
        
            # A sequence property backed by a list
            seqprop = simpleseq_property(\
                id_='seqprop[]',
                type_="float",
                name="seqprop",
                defvalue=(0.0, 1.0, 2.0, 3.0))
            
            # A sequence property backed by a list
            emptyseqprop = simpleseq_property(\
                id_='emptyseqprop[]',
                type_="float",
                name="emptyseqprop",
                defvalue=None)
            
            # A sequence property backed by a dictionary
            dictprop = simpleseq_property(\
                id_='dictprop[]',
                type_="long",
                name="dictprop",
                defvalue={"val1": 1, "val2": 2, "val3": 3})

            class SomeStruct(object):
                field1 = simple_property(id_="f1",
                                         name="Field 1",
                                         type_="double",
                                         defvalue=123.4)
                field2 = simple_property(id_="f2",
                                         name="Field 2",
                                         type_="long",
                                         defvalue=88)
                field3 = simple_property(id_="f3",
                                         name="Field 3",
                                         type_="string",
                                         defvalue="some value")

            struct = struct_property(id_="astruct", 
                                     name="Demonstration of a Struct", 
                                     structdef=SomeStruct)


###############################################################################   
# In the case of auto-generated code, TestResource is the base-class;
# therefore users need to be able to change the fset/fget behavior
# in their defined class so they can avoid modifying the generated code
#
# This class also demonstrates a method to port old-style code that
# uses on_query, on_configure calls
class TestResource2(TestResource):
    def __init__(self, execparams={}):
        TestResource.__init__(self, execparams)
        
    # Test that implicit callbacks still work on old-style and new-style
    # properties

    # Old-tuple style
    def onconfigure_prop_no_default_prop(self, oldvalue, value):
        if value:
            self._props["no_default_prop"] = "__" + value + "__"
                            
    def query_prop_no_default_prop(self):
        return self._props["no_default_prop"]

    # New-style
    def onconfigure_prop_someprop2(self, oldvalue, value):
        self.someprop2 = value + 1
    
    def query_prop_someprop2(self):
        return self.someprop2

    # Test support for rebinding to explicit callbacks
    def set_someprop(self, value):
        self._someprop = value

    def get_someprop(self):
        return self._someprop

    someprop = rebind(TestResource.someprop, fset=set_someprop, fget=get_someprop)
    
    def runIntenalTests(self, unittest):
        """Run some tests to see if the properties behave as expected when
        accessed from inside the object."""
        self.someprop = 12345
        self._props["DCE:456310b2-7d2f-40f5-bfef-9fdf4f3560ea"] = "Hello"
        self._props["someprop2"] = 5432.1
        
        unittest.assertEqual(self._props["default_prop"], "Hello")
        unittest.assertEqual(self.someprop, 12345)
        unittest.assertEqual(self._props["someprop"], 12345)
        unittest.assertEqual(self.someprop2, 5432.1)
        unittest.assertEqual(self._props["someprop2"], 5432.1)
        
    # You should also be able to define additional properties in your derived class
    someprop3 = simple_property(\
        id_='someprop3',
        type_="long",
        name="someprop3",
        defvalue=1)
            
###############################################################################            
## THE UNIT TESTS THEMSELVES
class TestPythonFramework(scatest.OssieTestCase):

    def test_TupleInitialization(self):
        tr = TestResource()
        tr.initialize()  # Initialize *must* be called, normally this is done by the coreframework
        self.assertEqual(tr._props.has_key("InvalidKey"), False)
        for p in PROPERTIES:
            self.assert_(tr._props.isValidPropId(p[0]))
            self.assert_(tr._props.has_key(p[0]))
            self.assert_(tr._props.has_key(p[1]))
            self.assertEqual(tr._props[p[0]], tr._props[p[1]])

            configurable = ("configure" in p[7])
            queryable = ("configure" in p[7]) or ("execparam" in p[7]) or ("allocation" in p[7])
            allocatable = ("allocation" in p[7])
            writable = p[3] in ("writeonly", "readwrite")
            readable = p[3] in ("readonly", "readwrite")
            external = p[6] == "external"

            self.assertEqual(tr._props.isConfigurable(p[0]), configurable and external and writable)
            self.assertEqual(tr._props.isQueryable(p[0]), queryable and external and readable)
            self.assertEqual(tr._props.isAllocatable(p[0]), allocatable and external)

            self.assertEqual(tr._props.isWritable(p[0]), p[3] in ("writeonly", "readwrite"))
            self.assertEqual(tr._props.isReadable(p[0]), p[3] in ("readonly", "readwrite"))


            value = tr._props[p[0]]
            if value != None:
                if p[2] == "boolean":
                    self.assertEqual(type(value), bool)
                elif p[2] in ("short", "long", "ulong", "ushort"):
                    self.assertEqual(type(value), int)
                elif p[2] == "double":
                    self.assertEqual(type(value), double)
                elif p[2] == "float":
                    self.assertEqual(type(value), float)
                else:
                    self.assertEqual(type(value), str)


    def test_PropertyType(self):
        
        EXECPARAMS = {"DCE:c03e148f-e9f9-4d70-aa00-6e23d33fa648": "myfile",
                      "DCE:716ea1c4-059a-4b18-8b66-74804bd8d435": "100"}
        tr = TestResource(EXECPARAMS)
        tr.initialize()  # Initialize *must* be called, normally this is done by the coreframework
         
        self.assertEqual(tr._props["DCE:c03e148f-e9f9-4d70-aa00-6e23d33fa648"], "myfile")
        self.assertEqual(tr._props["DCE:716ea1c4-059a-4b18-8b66-74804bd8d435"], 100)

        # Test that configure works
        self.assertEqual(tr._props["no_default_prop"], None)
        newvalue = CF.DataType(id="DCE:6b298d70-6735-43f2-944d-06f754cd4eb9", value=any.to_any("SomeString"))
        tr.configure([newvalue])
        newvalue2 = tr.query([newvalue])
        self.assertEqual(len(newvalue2), 1)
        self.assertEqual(newvalue.id, newvalue2[0].id)
        self.assertEqual(newvalue.value, newvalue2[0].value)

        # Testing query 
        props = tr.query([])
        props = dict([(p.id, p.value._v) for p in props])
        self.assertEqual(props.has_key("EXEC_PARAM_3"), True)
 
    def test_NewStyle(self):
         # Test the new behaviors
        EXECPARAMS = {"DCE:c03e148f-e9f9-4d70-aa00-6e23d33fa648": "myfile",
                      "DCE:716ea1c4-059a-4b18-8b66-74804bd8d435": "100"}
        tr = TestResource(EXECPARAMS)
        tr.initialize()  # Initialize *must* be called, normally this is done by the coreframework
        
        self.assertEqual(tr._props["DCE:c03e148f-e9f9-4d70-aa00-6e23d33fa648"], "myfile")
        self.assertEqual(tr._props["DCE:716ea1c4-059a-4b18-8b66-74804bd8d435"], 100)

        # Test that configure works
        self.assertEqual(tr._props["no_default_prop"], None)
        newvalue = CF.DataType(id="DCE:6b298d70-6735-43f2-944d-06f754cd4eb9", value=any.to_any("SomeString"))
        tr.configure([newvalue])
        newvalue2 = tr.query([newvalue])
        self.assertEqual(len(newvalue2), 1)
        self.assertEqual(newvalue.id, newvalue2[0].id)
        self.assertEqual(newvalue.value, newvalue2[0].value)
        self.assertEqual(tr._props["DCE:6b298d70-6735-43f2-944d-06f754cd4eb9"], "SomeString")
        self.assertEqual(tr._props["no_default_prop"], "SomeString")
        
        # Test that we cannot configure readonly
        newvalue = CF.DataType(id="DCE:c03e148f-e9f9-4d70-aa00-6e23d33fa648", value="SomeString")
        self.assertRaises(CF.PropertySet.InvalidConfiguration, tr.configure, [newvalue])

        # Test that a property using fset and fget get the default value
        self.assertEqual(tr._someprop, 10)
        self.assertEqual(tr.someprop, 10)
        self.assertEqual(tr._props["someprop"], 10)
        tr.someprop = 100
        self.assertEqual(tr._someprop, 100)
        self.assertEqual(tr.someprop, 100)
        self.assertEqual(tr._props["someprop"], 100)

        # Test that a property using internal __value storage works when
        # accessed as an attribute
        self.assertEqual(tr.someprop2, 50.0)
        self.assertEqual(tr._props["someprop2"], 50.0)
        tr.someprop2 = 99.0
        self.assertEqual(tr.someprop2, 99.0)
        self.assertEqual(tr._props["someprop2"], 99.0)

        # Run some tests on the struct
        self.assertEqual(tr.struct.field1, 123.4)
        self.assertEqual(tr.struct.field2, 88)
        self.assertEqual(tr.struct.field3, "some value")
        newvalue2 = tr.query([CF.DataType(id="astruct", value=any.to_any(None))])
        # Convert the struct into a list of dictionaries
        struct_propseq = any.from_any(newvalue2[0].value)
        # Extract the CF.DataType to a dict using from_any
        d = dict([(d["id"], d["value"]) for d in struct_propseq])
        self.assertEqual(d, {"f1": 123.4, "f2": 88, "f3": "some value"})
       
        # Query invalid prop to ensure it is still a string 
        queryResults = tr.query([CF.DataType(id="DCE:19a51bb2-1fd2-4c53-9215-a29bc10638e3", 
                                 value=any.to_any(None))])
        self.assertEqual(len(queryResults), 1)
        prop = queryResults[0]
        self.assertEqual(prop.value.typecode(), CORBA.TC_string)
        prop_val = any.from_any(prop.value)
        self.assertEqual(type(prop_val), str)

        # Query invalid prop2 to ensure it is still a short
        queryResults = tr.query([CF.DataType(id="DCE:3107571b-bde0-41c6-a840-c473b1a41b25", 
                                 value=any.to_any(None))])
        self.assertEqual(len(queryResults), 1)
        prop = queryResults[0]
        self.assertEqual(prop.value.typecode(), CORBA.TC_short)
        prop_val = any.from_any(prop.value)
        self.assertEqual(type(prop_val), int)

        # Run some tests that explore the behavior from the inside of the class
        tr.runIntenalTests(self)

        # Check that changing a struct field internally is represented in the next query
        self.assertEqual(tr.struct.field1, 987.6)
        self.assertEqual(tr.struct.field2, 55)
        self.assertEqual(tr.struct.field3, "nothing")
        newvalue2 = tr.query([CF.DataType(id="astruct", value=any.to_any(None))])
        # Convert the struct into a list of dictionaries
        struct_propseq = any.from_any(newvalue2[0].value)
        # Extract the CF.DataType to a dict using from_any
        d = dict([(d["id"], d["value"]) for d in struct_propseq])
        self.assertEqual(d, {"f1": 987.6, "f2": 55, "f3": "nothing"})

        # Check that we can configure a struct property
        newvalue = [CF.DataType(id="f1", value=any.to_any(555.5)), 
                    CF.DataType(id="f2", value=any.to_any(33)),
                    CF.DataType(id="f3", value=any.to_any("another value"))]
        tr.configure([CF.DataType(id="astruct", value=any.to_any(newvalue))])

        self.assertEqual(tr.struct.field1, 555.5)
        self.assertEqual(tr.struct.field2, 33)
        self.assertEqual(tr.struct.field3, "another value")
        newvalue2 = tr.query([CF.DataType(id="astruct", value=any.to_any(None))])
        struct_propseq = any.from_any(newvalue2[0].value)
        d = dict([(d["id"], d["value"]) for d in struct_propseq])
        self.assertEqual(d, {"f1": 555.5, "f2": 33, "f3": "another value"})
        
        # Check that we can query a "readwrite" execparam
        props = tr.query([CF.DataType(id="EXEC_PARAM_3", value=any.to_any(None))])
        self.assertEqual(props[0].value._v, 3.125)

        props = tr.query([])
        props = dict([(p.id, p.value._v) for p in props])
        self.assertEqual(props.has_key("EXEC_PARAM_3"), True)
        self.assertEqual(props["EXEC_PARAM_3"], 3.125)
        self.assertEqual(props.has_key("EXEC_PARAM_4"), False)

        # Check that we cannot query a "writeonly" execparam
        self.assertRaises(CF.UnknownProperties, tr.query, [CF.DataType(id="EXEC_PARAM_4", value=any.to_any(None))])
       
        # Check that we can't configure execparams
        self.assertRaises(CF.PropertySet.InvalidConfiguration, tr.configure, [CF.DataType(id="EXEC_PARAM_3", value=any.to_any(None))])

        # Check that we can't query invalid property id's
        self.assertRaises(CF.UnknownProperties, tr.query, [CF.DataType(id="INVALID_PARAM_4", value=any.to_any(None))])
       
        # Check that we can't query by property name
        self.assertRaises(CF.UnknownProperties, tr.query, [CF.DataType(id="SomeConfigFileLocation", value=any.to_any(None))])

        # Query invalid prop to ensure it is still a string 
        queryResults = tr.query([CF.DataType(id="DCE:19a51bb2-1fd2-4c53-9215-a29bc10638e3", 
                                 value=any.to_any(None))])
        self.assertEqual(len(queryResults), 1)
        prop = queryResults[0]
        self.assertEqual(prop.value.typecode(), CORBA.TC_string)
        prop_val = any.from_any(prop.value)
        self.assertEqual(type(prop_val), str)

        # Check that the type is valid:
        # (boolean|char|double|float|short|long|longlong|objref|octet|string|ulong|ushort)
        # any other is invalid
        kwds = {'id_':'DCE:fa8c5924-845c-484a-81df-7941f2c5baa9',
                'type_':'superlonglong',
                'name':"someprop",
                'defvalue':10}
        # Expects a ValueError Exception, fails if not raised
        self.failUnlessRaises(ValueError, simple_property, **kwds)
        

        # Check that a valid type does not raise an exception valid type
        simple_property(\
                id_='DCE:a950cf39-e7e3-4132-aa6e-526d5dc6eea9',
                type_="long",
                name="good_property",
                defvalue=10)

        # Check that type None not raise an exception valid type
        simple_property(\
                id_='DCE:dbaea3ce-2f44-401f-93be-984e6b2a4047',
                type_=None,
                name="valid_property",
                defvalue=10)

                
                
    def test_DerivedClass(self):
         # Test the new behaviors
        EXECPARAMS = {"DCE:c03e148f-e9f9-4d70-aa00-6e23d33fa648": "myfile",
                      "DCE:716ea1c4-059a-4b18-8b66-74804bd8d435": "100"}
        tr = TestResource2(EXECPARAMS)
        tr.initialize()  # Initialize *must* be called, normally this is done by the coreframework
        
        self.assertEqual(tr._props["DCE:c03e148f-e9f9-4d70-aa00-6e23d33fa648"], "myfile")
        self.assertEqual(tr._props["DCE:716ea1c4-059a-4b18-8b66-74804bd8d435"], 100)

        # Test that configure works
        self.assertEqual(tr._props["no_default_prop"], None)
        newvalue = CF.DataType(id="DCE:6b298d70-6735-43f2-944d-06f754cd4eb9", value=any.to_any("SomeString"))
        tr.configure([newvalue])
        newvalue2 = tr.query([newvalue])
        self.assertEqual(len(newvalue2), 1)
        self.assertEqual(newvalue.id, newvalue2[0].id)
        self.assertEqual(newvalue.value, newvalue2[0].value)
       
        # If the user uses the old-style onconfigure/query callbacks then they
        # won't update the dictionary unless the user explicitly does it
        self.assertEqual(tr._props["DCE:6b298d70-6735-43f2-944d-06f754cd4eb9"], "__SomeString__")
        self.assertEqual(tr._props["no_default_prop"], "__SomeString__")
        
        # Test that we cannot configure readonly
        newvalue = CF.DataType(id="DCE:c03e148f-e9f9-4d70-aa00-6e23d33fa648", value="SomeString")
        self.assertRaises(CF.PropertySet.InvalidConfiguration, tr.configure, [newvalue])

        # Test that a property using fset and fget get the default value
        self.assertEqual(tr._someprop, 10)
        self.assertEqual(tr.someprop, 10)
        self.assertEqual(tr._props["someprop"], 10)
        tr.someprop = 100
        self.assertEqual(tr._someprop, 100)
        self.assertEqual(tr.someprop, 100)
        self.assertEqual(tr._props["someprop"], 100)

        # Test that a property using internal __value storage works when
        # accessed as an attribute
        self.assertEqual(tr.someprop2, 50.0)
        self.assertEqual(tr._props["someprop2"], 50.0)
        tr.someprop2 = 99.0
        self.assertEqual(tr.someprop2, 99.0)
        self.assertEqual(tr._props["someprop2"], 99.0)
        newvalue = CF.DataType(id="DCE:cf623573-a09d-4fb1-a2ae-24b0b507115d", value=any.to_any(10.0))
        tr.configure([newvalue])
        tr.someprop2 = 11.0
        self.assertEqual(tr.someprop2, 11.0)
        self.assertEqual(tr._props["someprop2"], 11.0)
        
        self.assertEqual(tr.someprop3, 1)
        self.assertEqual(tr._props["someprop3"], 1)
        tr.someprop3 = 2
        self.assertEqual(tr.someprop3, 2)
        self.assertEqual(tr._props["someprop3"], 2)


        tr.runIntenalTests(self)
       
        
    def test_SequenceOperators(self):
        tr = TestResource()
        tr.initialize() # Initialize *must* be called, normally this is done by the coreframework
        
        # Test that operators don't work unless explictly declared 
        self.assertEqual(tr.noopseqprop, (0, 1, 2, 3))

        self.assertRaises(CF.UnknownProperties, tr.query, [CF.DataType(id="noopseqprop[]", value=any.to_any(None))])
        self.assertRaises(CF.UnknownProperties, tr.query, [CF.DataType(id="noopseqprop[*]", value=any.to_any(None))])
        self.assertRaises(CF.UnknownProperties, tr.query, [CF.DataType(id="noopseqprop[@]", value=any.to_any(None))])
        newvalue = [CF.DataType(id="1", value=any.to_any(1)), 
                    CF.DataType(id="3", value=any.to_any(3))]
        self.assertRaises(CF.PropertySet.InvalidConfiguration, tr.configure, [CF.DataType(id="noopseqprop[]", value=any.to_any(newvalue))])

        # Test the sequence property backed by a list
        self.assertEqual(tr.seqprop, (0.0, 1.0, 2.0, 3.0))
        seqvalue = tr.query([CF.DataType(id="seqprop[]", value=any.to_any(None))])
        self.assertEqual(seqvalue[0].value._v, [0.0, 1.0, 2.0, 3.0])
        
        seqvalue = tr.query([CF.DataType(id="seqprop[*]", value=any.to_any(None))])
        self.assertEqual(seqvalue[0].value._v, [0.0, 1.0, 2.0, 3.0])
        
        seqvalue = tr.query([CF.DataType(id="seqprop[0]", value=any.to_any(None))])
        self.assertEqual(seqvalue[0].value._v, [0.0])
        
        seqvalue = tr.query([CF.DataType(id="seqprop[3]", value=any.to_any(None))])
        self.assertEqual(seqvalue[0].value._v, [3.0])
        
        seqvalue = tr.query([CF.DataType(id="seqprop[1:3]", value=any.to_any(None))])
        self.assertEqual(seqvalue[0].value._v, [1.0, 2.0])
        
        seqvalue = tr.query([CF.DataType(id="seqprop[1:]", value=any.to_any(None))])
        self.assertEqual(seqvalue[0].value._v, [1.0, 2.0, 3.0])
        
        seqvalue = tr.query([CF.DataType(id="seqprop[:2]", value=any.to_any(None))])
        self.assertEqual(seqvalue[0].value._v, [0.0, 1.0])
        
        seqvalue = tr.query([CF.DataType(id="seqprop[?]", value=any.to_any(None))])
        self.assertEqual(seqvalue[0].value._v, [0,1,2,3])
        
        seqvalue = tr.query([CF.DataType(id="seqprop[@]", value=any.to_any(None))])
        
        # Extract the CF.DataType to a dict
        # Note that the even though the property is an array, the id is still a
        # string. This conversion was not done in the past, which worked when
        # called locally via the Python interface but raised a BAD_PARAM
        # exception when called via CORBA.
        d = dict([(d.id, d.value._v) for d in seqvalue[0].value._v])
        self.assertEqual(d, {'0': 0.0, '1': 1.0, '2': 2.0, '3': 3.0})
        
        seqvalue = tr.query([CF.DataType(id="emptyseqprop[*]", value=any.to_any(None))])
        self.assertEqual(seqvalue[0].value._v, None)

        seqvalue = tr.query([CF.DataType(id="seqprop[#]", value=any.to_any(None))])
        self.assertEqual(len(seqvalue), 1)
        seqlen = any.from_any(seqvalue[0].value)
        self.assertEqual(seqlen, 4)

        # Test that we can configure using the '@' operator
        newvalue = [CF.DataType(id="1", value=any.to_any(10.0)), 
                    CF.DataType(id="3", value=any.to_any(30.0))]
        tr.configure([CF.DataType(id="seqprop[@]", value=any.to_any(newvalue))])

        self.assertEqual(tr.seqprop, (0.0, 10.0, 2.0, 30.0))
        seqvalue = tr.query([CF.DataType(id="seqprop[]", value=any.to_any(None))])
        self.assertEqual(seqvalue[0].value._v, [0.0, 10.0, 2.0, 30.0])
        
        # Test the sequence property backed by a dictionary
        self.assertEqual(tr.dictprop, {"val1": 1, "val2": 2, "val3": 3})
        seqvalue = tr.query([CF.DataType(id="dictprop[]", value=any.to_any(None))])
        s1 = sets.Set(seqvalue[0].value._v)
        s2 = sets.Set([1, 2, 3])
        self.assertEqual(s1, s2)
        
        seqvalue = tr.query([CF.DataType(id="dictprop[*]", value=any.to_any(None))])
        s1 = sets.Set(seqvalue[0].value._v)
        self.assertEqual(s1, s2)
        
        seqvalue = tr.query([CF.DataType(id="dictprop[val1]", value=any.to_any(None))])
        self.assertEqual(seqvalue[0].value._v, [1])
        
        seqvalue = tr.query([CF.DataType(id="dictprop[val3]", value=any.to_any(None))])
        self.assertEqual(seqvalue[0].value._v, [3])
        
        seqvalue = tr.query([CF.DataType(id="dictprop[val1, val2]", value=any.to_any(None))])
        self.assertEqual(seqvalue[0].value._v, [1, 2])
        
        seqvalue = tr.query([CF.DataType(id="dictprop[val1, val3]", value=any.to_any(None))])
        self.assertEqual(seqvalue[0].value._v, [1, 3])
                
        seqvalue = tr.query([CF.DataType(id="dictprop[@]", value=any.to_any(None))])
        # Extract the CF.DataType to a dict using _v
        d = dict([(d.id, d.value._v) for d in seqvalue[0].value._v])
        self.assertEqual(d, {"val1": 1, "val2": 2, "val3": 3})
        # Extract the CF.DataType to a dict using from_any
        d = dict([(d["id"], d["value"]) for d in any.from_any(seqvalue[0].value)])
        self.assertEqual(d, {"val1": 1, "val2": 2, "val3": 3})

        newvalue = [CF.DataType(id="val2", value=any.to_any(20)), 
                    CF.DataType(id="val3", value=any.to_any(30))]
        tr.configure([CF.DataType(id="dictprop[@]", value=any.to_any(newvalue))])

        self.assertEqual(tr.dictprop, {"val1": 1, "val2": 20, "val3": 30})
        seqvalue = tr.query([CF.DataType(id="dictprop[@]", value=any.to_any(None))])
        # Extract the CF.DataType to a dict using _v
        d = dict([(d.id, d.value._v) for d in seqvalue[0].value._v])
        self.assertEqual(d, {"val1": 1, "val2": 20, "val3": 30})
        
        seqvalue = tr.query([CF.DataType(id="dictprop[#]", value=any.to_any(None))])
        self.assertEqual(len(seqvalue), 1)
        seqlen = any.from_any(seqvalue[0].value)
        self.assertEqual(seqlen, 3)

        # Verify that we cannot add keys via configure()
        newvalue = [CF.DataType(id="val4", value=any.to_any(4)), 
                    CF.DataType(id="val5", value=any.to_any(5))]
        self.assertRaises(CF.PropertySet.InvalidConfiguration, tr.configure, [CF.DataType(id="dictprop[@]", value=any.to_any(newvalue))])
        self.assertEqual(tr.dictprop, {"val1": 1, "val2": 20, "val3": 30})

    def test_InvalidStructDef(self):
        try:
            class BadStructResource(Resource):
                """Test that strutdef constraints of being new-style are validated in the struct_property constructor"""
                def __init__(self, execparams={}):
                    Resource.__init__(self, "BadStructResource", execparams)

                class SomeStruct:
                    field1 = simple_property(id_="f1",
                                             name="Field 1",
                                             type_="double",
                                             defvalue=123.4)

                struct = struct_property(id_="astruct", 
                                         name="Demonstration of a Struct", 
                                         structdef=SomeStruct)
        except ValueError:
            pass
        else:
            self.fail("Expected ValueError")

    def test_ValidStructDef(self):
        """
        Tests the rebind function added to the struct_property object
        """        

        class StructPropStruct(object): 
            field_one = simple_property(id_="DCE:7f1e487a-5b24-4493-9f78-8493be12cc95",
                                          name="field_one", 
                                          type_="string", 
                                          defvalue="The first field", 
                                          )
 
            field_two = simple_property(id_="DCE:25e3aaf2-063d-4140-b4e3-1b24f4fcea9a",
                                          name="field_two", 
                                          type_="double", 
                                          defvalue=2.0, 
                                          )
 
            field_three = simple_property(id_="DCE:262a5cb3-f3ac-4eeb-a16f-b77490d97d33",
                                          name="field_three", 
                                          type_="long", 
                                          defvalue=333, 
                                          )

        struct_prop = struct_property(id_="DCE:70f920b8-51df-4231-97c7-9a89fd45d20c",
                                          name="struct_prop", 
                                          structdef=StructPropStruct)
                
        def getter(id_):            
            for fld_id, fld_val in struct_prop.fields.iteritems():
                if fld_id == id_:
                    return fld_val[1]
                
        def setter(id_, prop):
            for fld_id, fld_val in struct_prop.fields.iteritems():
                if fld_id == id_:
                    struct_prop.fields[prop.id_] = (prop.name, prop)
                    del struct_prop.fields[fld_id]
                                
        def validator(id_, value):            
            for fld_id, fld_val in struct_prop.fields.iteritems():
                if fld_id == id_:
                    fld_val[1].defvalue = value

        
        # making sure the object was generated properly
        self.assertNotEqual(struct_prop, None)
        # making sure fget, fset, and fval are None
        self.assertEqual(struct_prop.fget, None)
        self.assertEqual(struct_prop.fset, None)
        self.assertEqual(struct_prop.fval, None)
        
        struct_prop = struct_prop.rebind(getter, setter, validator)
        # making sure fget, fset, and fval are NOT None
        self.assertNotEqual(struct_prop.fget, None)
        self.assertNotEqual(struct_prop.fset, None)
        self.assertNotEqual(struct_prop.fval, None)
        
        # getting the first property
        prop = struct_prop.fget("DCE:7f1e487a-5b24-4493-9f78-8493be12cc95")
        self.assertEqual(prop.name, 'field_one')
        
        prop = simple_property(id_="DCE:7f1e487a-5b24-4493-9f78-8493be12cc97",
                               name="field_one_replacement", 
                               type_="string", 
                               defvalue="The first field replaced", 
                              )
        
        # setting the property and removing field_one
        struct_prop.fset('DCE:7f1e487a-5b24-4493-9f78-8493be12cc95', prop)
        cmp_prop = struct_prop.fget('DCE:7f1e487a-5b24-4493-9f78-8493be12cc97')
        self.assertEqual("field_one_replacement", cmp_prop.name)
        
        # making sure I still have only three fields
        self.assertEqual(3, len(struct_prop.fields))
        
        # replacing the value from property field_three
        struct_prop.fval('DCE:262a5cb3-f3ac-4eeb-a16f-b77490d97d33', 444)
        cmp_prop = struct_prop.fget('DCE:262a5cb3-f3ac-4eeb-a16f-b77490d97d33')
        self.assertEqual(444, cmp_prop.defvalue)
        

        
    def test_MagicCalls(self):
        """
        Tests the onconfigure, onquery, and onvalue magic calls in all different 
        properties
        """
        import pprint
        
        class StructPropStruct(object): 
            field_one = simple_property(id_="DCE:7f1e487a-5b24-4493-9f78-8493be12cc95",
                                          name="Field One", 
                                          type_="string", 
                                          defvalue="The first field", 
                                          )
 
            field_two = simple_property(id_="DCE:25e3aaf2-063d-4140-b4e3-1b24f4fcea9a",
                                          name="Field Two", 
                                          type_="double", 
                                          defvalue=2.0, 
                                          )
 
            field_three = simple_property(id_="DCE:262a5cb3-f3ac-4eeb-a16f-b77490d97d33",
                                          name="Field Three", 
                                          type_="long", 
                                          defvalue=333, 
                                          )

 
 
 
        class MagicCallsResource(Resource):

            # You can define a port using an fget.  This let's you
            # control the lifecyle of the port.  The fget function
            # *MUST* return a CORBA client reference (i.e. the value
            # returned from _this
            class InputPort(CF__POA.LifeCycle):
                def nonCorbaCall(self):
                    return True

            class OutputPort(CF__POA.Port):
                def nonCorbaCall(self):
                    return True

            # Include both old style and new style propertydefs
            def __init__(self, execparams={}):
                Resource.__init__(self, "TestResource", execparams)
                # For port defintions that use fget, this is illegal syntax and
                # will throw an AttributeError
                self._input = TestResource.InputPort()
                self._output = TestResource.OutputPort()

                                                                                  
            def get_port_input(self):
                return self._input


            def get_port_output(self):
                return self._output



            ######################################################
            ##         SIMPLE PROPERTY MAGIC CALLBACKS          ##
            ######################################################
            def query_prop_prop_one(self):
                return self.prop_one

            def validate_prop_prop_one(self, value):
                if isinstance(value, float):
                    return True
                else:
                    return False

            def onconfigure_prop_prop_one(self, oldvalue, value):
                if value:
                    self.prop_one = value
                            


            ######################################################
            ##        SEQUENCE PROPERTY MAGIC CALLBACKS         ##
            ######################################################
            def query_prop_seq_prop(self):
                return self.seq_prop

            def validate_prop_seq_prop(self, value):
                if isinstance(value, list) or isinstance(value, tuple):
                    return True
                else:
                    return False

            def onconfigure_prop_seq_prop(self, oldvalue, value):
                if value:
                    self.seq_prop = value
                            


            ######################################################
            ##         STRUCT PROPERTY MAGIC CALLBACKS          ##
            ######################################################     
            def query_prop_test_struct(self):
                result = []
                for name, attr in self.struct_prop.__dict__.items():
                    if name.startswith('__') and name.endswith('__'):
                        name = name[2:-2]
                        result.append(CF.DataType(id=name, value=any.to_any(attr)))

                return any.to_any(result)

            def validate_prop_test_struct(self, value):
                if not isinstance(value, StructPropStruct):
                    return False

                return True

            def onconfigure_prop_test_struct(self, oldvalue, value):
                if value:
                    self.struct_prop = value



            port_input = providesport(name="input", 
                                      repid="IDL:CF/LifeCycle:1.0", 
                                      type_="control",
                                      fget=get_port_input)


            port_output = usesport(name="output", 
                                   repid="IDL:CF/LifeCycle:1.0", 
                                   type_="control",
                                   fget=get_port_output)




            struct_prop = struct_property(id_="DCE:70f920b8-51df-4231-97c7-9a89fd45d20c",
                                          name="test_struct", 
                                          structdef=StructPropStruct)
                                              
            seq_prop = simpleseq_property(id_="DCE:1d4b16f0-97e0-4ff7-b14e-9e83511e9ee9",
                                          name="seq_prop",   
                                          type_="long", 
                                          defvalue=(1, 2, 3, 4, 5, 6, 7, ),
                                          mode="readwrite",  
                                          action="external",                 
                                          kinds=("configure", )
                                          )

            prop_one = simple_property(id_="DCE:25e3aaf2-063d-4140-b4e3-1b24f4fcea9a",
                                       name="prop_one", 
                                       type_="double", 
                                       defvalue=2.0, 
                                       )




        tr = MagicCallsResource()
        # Initialize *must* be called, normally this is done by the coreframework
        tr.initialize()     
        
        sim_id = "DCE:25e3aaf2-063d-4140-b4e3-1b24f4fcea9a"
        
        seq_id = "DCE:1d4b16f0-97e0-4ff7-b14e-9e83511e9ee9"

        str_id = "DCE:70f920b8-51df-4231-97c7-9a89fd45d20c"

        #######################################################################
        ##                                Testing Query
        #######################################################################
        # Checking that I can run query in a simple property
        props = tr.query([CF.DataType(id=sim_id, value=any.to_any(None))])        
        self.assertEqual(len(props), 1)
        self.assertEqual(props[0].id, 'DCE:25e3aaf2-063d-4140-b4e3-1b24f4fcea9a')
        self.assertEqual(props[0].value.value(), 2.0)
        
        # Checking that I can run query in a sequence property
        props = tr.query([CF.DataType(id=seq_id, value=any.to_any(None))])
        self.assertEqual(len(props), 1)
        self.assertEqual(props[0].id, 'DCE:1d4b16f0-97e0-4ff7-b14e-9e83511e9ee9')
        self.assertEqual(props[0].value.value(), [1, 2, 3, 4, 5, 6, 7])

        # Checking that I can run query in a struct property
        props = tr.query([CF.DataType(id=str_id, value=any.to_any(None))])
        self.assertEqual(len(props), 1)
        self.assertEqual(props[0].id, 'DCE:70f920b8-51df-4231-97c7-9a89fd45d20c')
        # I should have 3 fields inside the struct
        self.assertEqual(len(props[0].value.value()), 3)
        
        # checking item by item inside the struct
        for item in props[0].value.value():
            val = any.from_any(item.value)
            
            if item.id == 'DCE:7f1e487a-5b24-4493-9f78-8493be12cc95':
                self.assertEqual('The first field', val)
            elif item.id == 'DCE:25e3aaf2-063d-4140-b4e3-1b24f4fcea9a':
                self.assertEqual(2.0, val)
            
            elif item.id == 'DCE:262a5cb3-f3ac-4eeb-a16f-b77490d97d33':
                self.assertEqual(333, val)
        
        
        #######################################################################
        ##            Testing Validation and Configuration
        #######################################################################

            
        newvalue = CF.DataType(id=sim_id, value=any.to_any(5.0))
        tr.configure([newvalue])
        props = tr.query([CF.DataType(id=sim_id, value=any.to_any(None))])        
        self.assertEqual(len(props), 1)
        self.assertEqual(props[0].id, 'DCE:25e3aaf2-063d-4140-b4e3-1b24f4fcea9a')
        self.assertEqual(props[0].value.value(), 5.0)
                            
        newvalue = CF.DataType(id=seq_id, value=any.to_any((11,22,33,44,55,66,77,88,)))
        tr.configure([newvalue])

        # Checking that I can run query in a sequence property
        props = tr.query([CF.DataType(id=seq_id, value=any.to_any(None))])
        self.assertEqual(len(props), 1)
        self.assertEqual(props[0].id, 'DCE:1d4b16f0-97e0-4ff7-b14e-9e83511e9ee9')
        self.assertEqual(props[0].value.value(), [11,22,33,44,55,66,77,88])

        val = StructPropStruct()
        val.field_one = 'replaced_string'
        val.field_two = 77.88
        val.field_three = 99
        newvalue = CF.DataType(id=str_id, value=struct_to_any(val))
        tr.configure([newvalue])

        # Checking that I can run query in a struct property
        props = tr.query([CF.DataType(id=str_id, value=any.to_any(None))])
        self.assertEqual(len(props), 1)
        self.assertEqual(props[0].id, 'DCE:70f920b8-51df-4231-97c7-9a89fd45d20c')
        # I should have 3 fields inside the struct
        self.assertEqual(len(props[0].value.value()), 3)
        
        # checking item by item inside the struct
        for item in props[0].value.value():
            val = any.from_any(item.value)
            
            if item.id == 'DCE:7f1e487a-5b24-4493-9f78-8493be12cc95':
                self.assertEqual('replaced_string', val)
            elif item.id == 'DCE:25e3aaf2-063d-4140-b4e3-1b24f4fcea9a':
                self.assertEqual(77.88, val)            
            elif item.id == 'DCE:262a5cb3-f3ac-4eeb-a16f-b77490d97d33':
                self.assertEqual(99, val)
 
 


    def test_PortHandling(self):
        # To test ports we need an ORB
        orb = CORBA.ORB_init()
        obj_poa = orb.resolve_initial_references("RootPOA")
        poaManager = obj_poa._get_the_POAManager()
        poaManager.activate()

        tr = TestResource()
        tr.initialize() # Initialize *must* be called, normally this is done by the coreframework

        input = tr.getPort("input")
        self.assert_(input != None)
        self.assert_(input._is_a("IDL:CF/LifeCycle:1.0"))

        input = tr.getPort("input2")
        self.assert_(input != None)
        self.assert_(input._is_a("IDL:CF/LifeCycle:1.0"))

        output = tr.getPort("output")
        self.assert_(output != None)
        self.assert_(output._is_a("IDL:CF/Port:1.0"))

        self.assertRaises(CF.PortSupplier.UnknownPort, tr.getPort, "nonexistant")

    # Only define event-related tests if events are enabled.
    if scatest.getBuildDefineValue("ENABLE_EVENTS") == "1":
        def test_eventChannelManager(self):
            orb = CORBA.ORB_init()

            chanMgr = ChannelManager(orb)
            factory = chanMgr.getEventChannelFactory()
            self.assertNotEqual(factory, None)
            self.assertNotEqual(factory._narrow(CosLifeCycle.GenericFactory), None)
            
            # Force creation
            channel1 = chanMgr.createEventChannel("TestChan", force=True)
            self.assertNotEqual(channel1, None)
            self.assertNotEqual(channel1._narrow(CosEventChannelAdmin.EventChannel), None)

            # Without force, we should get the previously created one
            channel2 = chanMgr.createEventChannel("TestChan", force=False)
            self.assertNotEqual(channel2, None)
            self.assertNotEqual(channel2._narrow(CosEventChannelAdmin.EventChannel), None)
            self.assert_(channel1._is_equivalent(channel2))

            # Get should return the right one
            channel3 = chanMgr.getEventChannel("TestChan")
            self.assertNotEqual(channel3, None)
            self.assertNotEqual(channel3._narrow(CosEventChannelAdmin.EventChannel), None)
            self.assert_(channel1._is_equivalent(channel3))

            # This should create a new channel 
            channel4 = chanMgr.createEventChannel("TestChan", force=True)
            self.assertNotEqual(channel4, None)
            self.assertNotEqual(channel4._narrow(CosEventChannelAdmin.EventChannel), None)

            chanMgr.destroyEventChannel("TestChan")

            channel5 = chanMgr.getEventChannel("TestChan")
            self.assertEqual(channel5, None)
            
            channel6 = chanMgr.createEventChannel("TestChan", force=True)
            self.assertNotEqual(channel6, None)
            self.assertNotEqual(channel6._narrow(CosEventChannelAdmin.EventChannel), None)
            
            chanMgr.destroyEventChannel("TestChan")

    def test_SimpleSequenceTypeChecking(self):
        # Test case for ticket #2426
        class TestClass(object):
            def __init__(self):
                self._props = ossie.properties.PropertyStorage(self, [], {})
                self._props.initialize()
            string_seq = simpleseq_property(id_="string_seq", type_="string")
            double_seq = simpleseq_property(id_="double_seq", type_="double")
            int_seq = simpleseq_property(id_="int_seq", type_="long")
            char_seq = simpleseq_property(id_="char_seq", type_="char")
            octet_seq = simpleseq_property(id_="octet_seq", type_="octet")
        
        t = TestClass()
        t.string_seq = [["1", "2", "3"], ["4", "5", "6"]]
        # Although the value isn't a sequence of strings, it will
        # get coerced into the correctly value
        p = TestClass.string_seq.query(t)
        self.assertEqual(p._t.content_type(), CORBA.TC_string)
        v = any.from_any(p)
        for e in v:
            self.assertEqual(type(e), str)
            
        # This can also be coerced at query time into the correct type
        t.int_seq = ["4", "5", "6"]
        p = TestClass.int_seq.query(t)
        self.assertEqual(p._t.content_type(), CORBA.TC_long)
        v = any.from_any(p)
        for e in v:
            self.assertEqual(type(e), int)
            
        # This cannot be coerced into a double, so query will
        # throw a type error
        t.double_seq = [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]
        try:
            p = TestClass.double_seq.query(t)
        except TypeError:
            pass
        else:
            self.fail("Expected a type error")

        # Obviously, a list of doubles will work
        t.double_seq = [1.0, 2.0, 3.0]
        p = TestClass.double_seq.query(t)
        self.assertEqual(p._t.content_type(), CORBA.TC_double)
        v = any.from_any(p)
        for e in v:
            self.assertEqual(type(e), float)
            
        t.char_seq = "ABCDE"
        p = TestClass.char_seq.query(t)
        self.assertEqual(p._t._t, CORBA.TypeCode(CORBA.CharSeq)._t)
        v = any.from_any(p)
        for e in v:
            self.assertEqual(type(e), str)
            
        t.octet_seq = "FGHI"
        p = TestClass.octet_seq.query(t)
        self.assertEqual(p._t._t, CORBA.TypeCode(CORBA.OctetSeq)._t)
        v = any.from_any(p)
        for e in v:
            self.assertEqual(type(e), str)

    def test_PythonToAnyConversion(self):
        # Basic conversion 
        result = ossie.properties.to_tc_value(1.0, 'double')
        self.assertTrue(result.typecode().equal(CORBA.TC_double))
        self.assertEqual(any.from_any(result), 1.0)

        # Ensure that pre-formatted Any values are unchanged (1.8.8 regression)
        value = CORBA.Any(CORBA.TC_double, 1.0)
        result = ossie.properties.to_tc_value(value, 'double')
        self.assertTrue(result.typecode().equal(CORBA.TC_double))
        self.assertEqual(any.from_any(result), 1.0)

        # Type change
        result = ossie.properties.to_tc_value(1.25, 'long')
        self.assertTrue(result.typecode().equal(CORBA.TC_long))
        self.assertEqual(any.from_any(result), 1)

        # Type change from Any (1.8.8 regression)
        value = CORBA.Any(CORBA.TC_double, 1.25)
        result = ossie.properties.to_tc_value(value, 'long')
        self.assertTrue(result.typecode().equal(CORBA.TC_long))
        self.assertEqual(any.from_any(result), 1)
