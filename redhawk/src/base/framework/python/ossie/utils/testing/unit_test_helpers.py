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

# Utility library to assist with the construction of component/device/waveform unittests

import os
import time
import threading
from new import classobj
import unittest
import signal
import subprocess
import omniORB
import sys
from omniORB import any, CORBA
import CosNaming, CosNaming__POA # This must be imported after importing omniORB
import ossie.parsers.spd as SPDParser
import ossie.parsers.prf as PRFParser
import ossie.parsers.scd as SCDParser
import logging
from ossie.cf import CF, CF__POA
from ossie.cf import ExtendedCF
import copy
from ossie import properties
from ossie.utils import model
from ossie.utils import prop_helpers
from ossie.utils import sb
from ossie.utils.idllib import IDLLibrary
import getopt
from ossie.properties import mapComplexType
from ossie.utils.prop_helpers import parseComplexString
from omniORB import any, CORBA, tcInternal

import rhunittest

# These global methods are here to allow other modules to modify the global variables IMPL_ID and SOFT_PKG
# TestCase setUp() method doesn't allow passing in arguments to the test case so global values are needed
def setImplId(impl_id):
    global IMPL_ID
    IMPL_ID = impl_id

def getImplId():
    return IMPL_ID

def setSoftPkg(soft_pkg):
    global SOFT_PKG
    SOFT_PKG = soft_pkg

def getSoftPkg():
    return SOFT_PKG 

# this variable is used by the sandbox to kick off a component through the IDE
#IDE_REF_ENV = os.getenv('IDE_REF')
IDE_REF_ENV = None
if IDE_REF_ENV != None:
    sb.setIDE_REF(sb.orb.string_to_object(IDE_REF_ENV)._narrow(ExtendedCF.Sandbox))

def stringToComplex(value, type):
    real, imag = parseComplexString(value, type)
    if isinstance(real, basestring):
        real = int(real)
        imag = int(imag)
    return complex(real, imag)
    
class ScaComponentTestCase(unittest.TestCase):
    """
    Class used to test independent implementations of a component. It starts an
    implementation and runs tests that are common among components.
    
    """
    STOP_SIGNALS = ((signal.SIGINT, 1),
                    (signal.SIGTERM, 5),
                    (signal.SIGKILL, None))
        
    # Stores the implementation ID of the implementation specific test
    SPC_ID = None
    
#    def __init__(self):
#        """
#        Instantiates a new object and sets the Software Package Descriptor file
#        used to retrieve all the component characteristics.
#        
#        Input:
#            <spd_file>    The component Software Package Descriptor file
#        """
#        unittest.TestCase.__init__(self)
        
    def setUp(self):
        """
        Starts the component whose id matches the one stored in the IMPL_ID
        """
        
        signal.signal(signal.SIGINT, self.tearDown)
        signal.signal(signal.SIGTERM, self.tearDown)
        signal.signal(signal.SIGQUIT, self.tearDown)
        
        global SOFT_PKG
        global IMPL_ID
        self.comp_obj = None
        self.comp = None
        # Use the globals by default
        self.impl = IMPL_ID
        self.spd_file = SOFT_PKG
        self.spd = SPDParser.parse(SOFT_PKG)
        
        try:
            self.prf_file = self.spd.get_propertyfile().get_localfile().get_name()
            if (self.prf_file[0] != '/'):
                self.prf_file = os.path.join(os.path.dirname(self.spd_file), self.prf_file)
            self.prf = PRFParser.parse(self.prf_file)
        except:
            self.prf_file = None
            self.prf = None
        
        self.scd_file = self.spd.get_descriptor().get_localfile().get_name()
        if (self.scd_file[0] != '/'):
            self.scd_file = os.path.join(os.path.dirname(self.spd_file), self.scd_file)
        self.scd = SCDParser.parse(self.scd_file)

        # create a map between prop ids and names
        if self.prf:
            self._props = prop_helpers.getPropNameDict(self.prf)

    def tearDown(self):
        """
        Stops, release, and kill the process running the component
        """
        if self.comp_obj != None and self.scd.get_componenttype() in \
            ("resource", "device", "loadabledevice", "executabledevice"):
            try:
                self.comp.releaseObject()
            except CORBA.Exception:
                pass
        else: # this is where Services would be handled
            pass
        self.comp_obj = None
            
    def launch(self, execparams={}, ossiehome=None, configure={}, initialize=True, objType=None, debugger=None):
        """
        Launch the component. The component will be executed as a child process,
        then (optionally) initialized and configured.

        Arguments:
          execparams - Execparams to override on component execution.
          ossiehome  - Base location of REDHAWK installation for finding IDL files.
                       Default location is determined from $OSSIEHOME environment
                       variable.
          configure  - If a dictionary, a set of key/value pairs to override the
                       initial configuration values of the component.
                       If None, skip initial configuration.
          initialize - If true, call initialize() after launching the component.
                       If false, skip initialization.
          objType    - Object type to be launched. Could be a component, device or service.
       """
        if IDE_REF_ENV == None:
            if ossiehome:
                model._idllib = IDLLibrary()
                model._idllib.addSearchPath(str(ossiehome)+'/idl')
            component = sb.launch(self.spd_file, impl=self.impl, execparams=execparams,
                                  configure=configure, initialize=initialize, objType=objType, debugger=debugger)
        else:
            # spd file path passed in to unit test is relative to current component tests directory (i.e. "..")
            # IDE unit test requires spd file path relative to redhawk file system
            componentName = str(self.spd.get_name())
            rh_file_system_spd_file = "components/" + componentName + "/" + self.spd_file[3:]
            component = sb.launch(rh_file_system_spd_file, impl=self.impl, execparams=execparams,
                                  configure=configure, initialize=initialize, objType=objType, debugger=debugger)
        self.comp_obj = component.ref
        self.comp = component
            
    def isMatch(self, prop, modes, kinds, actions):
        if prop.get_mode() == None:
            m = "readwrite"
        else:
            m = prop.get_mode()
        matchMode = (m in modes)

        if prop.__class__ in (PRFParser.simple, PRFParser.simpleSequence):
            if prop.get_action() == None:
                a = "external"
            else:
                a = prop.get_action().get_type()
            matchAction = (a in actions)

            matchKind = False
            if prop.get_kind() == None:
                k = ["configure", "property"]
            else:
                k = prop.get_kind()
            for kind in k:
                if kind.get_kindtype() in kinds:
                    matchKind = True

        elif prop.__class__ in (PRFParser.struct, PRFParser.structSequence):
            matchAction = True # There is no action, so always match

            matchKind = False
            if prop.get_configurationkind() == None:
                k = ["configure", "property"]
            else:
                k = prop.get_configurationkind()
            for kind in k:
                if kind.get_kindtype() in kinds:
                    matchKind = True

            if k in kinds:
                matchKind = True


        return matchMode and matchKind and matchAction
       
    def getPropertySet(self, kinds=("configure","property",), \
                             modes=("readwrite", "writeonly", "readonly"), \
                             action="external", \
                             includeNil=True,
                             commandline=False):
        """
        A useful utility function that extracts specified property types from
        the PRF file and turns them into a CF.PropertySet
        """
        propertySet = []

        # Simples
        if self.prf != None:
          for prop in self.prf.get_simple():
            if self.isMatch(prop, modes, kinds, (action,)): 
                if prop.get_value() is not None:    
                    if prop.complex.lower() == "true":
                        type = mapComplexType(prop.get_type())
                        value = stringToComplex(prop.get_value(), type)
                    else:
                        type = prop.get_type()
                        value = prop.get_value()
                    dt = properties.to_tc_value(value, type)
                elif not includeNil:
                    continue
                else:
                    dt = any.to_any(None)
                p = CF.DataType(id=str(prop.get_id()), value=dt)
                propertySet.append(p)

          # Simple Sequences
          for prop in self.prf.get_simplesequence():
            if self.isMatch(prop, modes, kinds, (action,)): 
                if prop.get_values() is not None:
                    seq = []
                    if prop.complex.lower() == "true":
                        type = mapComplexType(prop.get_type())
                        for v in prop.get_values().get_value():
                            seq.append(stringToComplex(v, type))
                        expectedType = properties.getTypeCode(type)
                        expectedTypeCode = tcInternal.createTypeCode(
                            (tcInternal.tv_sequence, expectedType._d, 0))
                        dt = CORBA.Any(expectedTypeCode, 
                                       [properties._convertComplexToCFComplex(item, type) 
                                            for item in seq])
                    else:
                        type = prop.get_type()
                        for v in prop.get_values().get_value():
                            value = v
                            seq.append(properties.to_pyvalue(value, type))
                        dt = any.to_any(seq)
                elif not includeNil:
                    continue
                else:
                    dt = any.to_any(None)
                p = CF.DataType(id=str(prop.get_id()), value=dt)
                propertySet.append(p)

          # Structures
          for prop in self.prf.get_struct():
            if self.isMatch(prop, modes, kinds, (action,)): 
                if (prop.get_simple() is not None) or (prop.get_simplesequence() is not None):
                    fields = []
                    hasValue = False
                    if prop.get_simple() is not None:
                        for p in prop.get_simple():
                            if p.get_value() is not None:
                                hasValue = True
                            dt = properties.to_tc_value(p.get_value(), p.get_type())
                            fields.append(CF.DataType(id=str(p.get_id()), value=dt))
                    if prop.get_simplesequence() is not None:
                        for p in prop.get_simplesequence():
                            if p.get_values() is not None:
                                hasValue = True
                            dt = properties.to_tc_value(p.get_values(), p.get_type())
                            fields.append(CF.DataType(id=str(p.get_id()), value=dt))
                    if not hasValue and not includeNil:
                        continue
                    dt = any.to_any(fields)
                else:
                    dt = any.to_any(None)
                p = CF.DataType(id=str(prop.get_id()), value=dt)
                propertySet.append(p)
                
          # Struct Sequence
          for prop in self.prf.get_structsequence():
            if self.isMatch(prop, modes, kinds, (action,)):
              baseProp = []
              if prop.get_struct() != None:
                fields = []
                for internal_prop in prop.get_struct().get_simple():
                    fields.append(CF.DataType(id=str(internal_prop.get_id()), value=any.to_any(None)))
                for internal_prop in prop.get_struct().get_simplesequence():
                    fields.append(CF.DataType(id=str(internal_prop.get_id()), value=any.to_any(None)))
              for val in prop.get_structvalue():
                baseProp.append(copy.deepcopy(fields))
                for entry in val.get_simpleref():
                  val_type = None
                  for internal_prop in prop.get_struct().get_simple():
                      if str(internal_prop.get_id()) == entry.refid:
                          val_type = internal_prop.get_type()
                  for subfield in baseProp[-1]:
                      if subfield.id == entry.refid:
                          subfield.value = properties.to_tc_value(entry.get_value(), val_type)
                for entry in val.get_simplesequenceref():
                  val_type = None
                  for internal_prop in prop.get_struct().get_simplesequence():
                      if str(internal_prop.get_id()) == entry.refid:
                          val_type = internal_prop.get_type()
                  for subfield in baseProp[-1]:
                      if subfield.id == entry.refid:
                          subfield.value = properties.to_tc_value(entry.get_values(), val_type)
              anybp = []
              for bp in baseProp:
                  anybp.append(properties.props_to_any(bp))
              p = CF.DataType(id=str(prop.get_id()), value=any.to_any(anybp))
              propertySet.append(p)

        return propertySet
    
    def runConfigurationTest(self, spd_file, prf_file):
        """
        Tests all the properties defined in the PRF file and attempts to 
        configure each property.  If the property is of kind 'configure' and 
        the mode is not readonly, then it compares the results with what is
        expected.  Otherwise it expects an exception to be thrown
        
        Inputs:
            <spd_file>    The component's SPD file
            <prf_file>    The component's properties file
        """        
        self.launch()
        prf = PRFParser.parse(prf_file)
        props = prf.get_simple()
        int_off = 1
        frac_off = 1.0
        # iterating through all the properties
        for prop in props:
            id_ = prop.get_id()
            mode_ = prop.get_mode()
            val_  = prop.get_value()
            type_ = prop.get_type()
            
            kinds = prop.get_kind()
            is_configurable = False
            
            # check each kind to see if at least one of them is configure
            for kind in kinds:
                kind_type = kind.get_kindtype()
                if kind_type == 'configure':
                    is_configurable = True
                    break
            # generate some bogus value
            if type_ == 'double' or type_ == 'float':
                new_val = frac_off + float(val_)
            elif type_ == 'integer' or type_ == 'long':
                new_val = int_off + int(val_)
            else:
                new_val = '%s_%d' % (val_, int_off)
            
            conf_prop = CF.DataType(id=str(id_), value=any.to_any(new_val))                
            
            # if it can be configured, test that is the case                                        
            if is_configurable and mode_ != 'readonly':
                self.comp_obj.configure([conf_prop])
                q_value = self.comp_obj.query([conf_prop])
                self.assertEqual(len(q_value), 1)
                self.assertEqual(conf_prop.id, q_value[0].id)
                self.assertEqual(conf_prop.value._v, q_value[0].value._v)
            
            # this property should be be configurable
            else:
                try:
                    self.comp_obj.configure([conf_prop])
                    self.assertFalse()
                except:
                    # Got an expected error, so we should continue
                    continue

class RHComponentTestCase(ScaComponentTestCase):
    pass

class ScaComponentTestProgram(unittest.TestProgram):
    USAGE = """\
Usage: %(progName)s [options] [test] [...]

Options:
  -h, --help       Show this message
  -v, --verbose    Verbose output
  -q, --quiet      Minimal output
  -i, --impl       Specify a specific component implementation, by default all impls are tested

Examples:
  %(progName)s                               - run default set of tests
  %(progName)s MyTestSuite                   - run suite 'MyTestSuite'
  %(progName)s MyTestCase.testSomething      - run MyTestCase.testSomething
  %(progName)s MyTestCase                    - run all 'test*' test methods
                                               in MyTestCase
"""
    def __init__(self, spd_file, module='__main__', defaultTest=None,
                argv=None, testRunner=None, testLoader=None, impl=None):
        self.spd_file = spd_file
        self.impl = impl
        self.results = []
        if testLoader is None:
            testLoader = rhunittest.RHTestLoader(impl)
        unittest.TestProgram.__init__(self, module, defaultTest, argv, testRunner, testLoader)

    def parseArgs(self, argv):
        try:
            options, args = getopt.getopt(argv[1:], 'hHvqi:',
                                          ['help','verbose','quiet','impl='])
            for opt, value in options:
                if opt in ('-h','-H','--help'):
                    self.usageExit()
                if opt in ('-q','--quiet'):
                    self.verbosity = 0
                if opt in ('-v','--verbose'):
                    self.verbosity = 2
                if opt in ('-i','--impl'):
                    self.impl = value
            if len(args) == 0 and self.defaultTest is None:
                self.test = self.testLoader.loadTestsFromModule(self.module)
                return
            if len(args) > 0:
                self.testNames = args
            else:
                self.testNames = (self.defaultTest,)
            self.createTests()
        except getopt.error, msg:
            self.usageExit(msg)
    
    def runTests(self):
        # Setup some globals to pass to the TestCase
        global SOFT_PKG
        global IMPL_ID
        SOFT_PKG = self.spd_file
        spd = SPDParser.parse(self.spd_file)
        
        if self.testRunner is None:
            try:
                import xmlrunner
                self.testRunner = xmlrunner.XMLTestRunner(verbosity=self.verbosity)
            except ImportError:
                self.testRunner = unittest.TextTestRunner(verbosity=self.verbosity)
        
        for implementation in spd.get_implementation():
            IMPL_ID = implementation.get_id()
            if IMPL_ID == self.impl or self.impl is None:
                result = self.testRunner.run(self.test)
                self.results.append(result)
        #sys.exit(not result.wasSuccessful())

def main(spd_file=None, module='__main__', defaultTest=None, argv=None, testRunner=None,
         testLoader=unittest.defaultTestLoader, impl=None):
    if spd_file is None:
        return rhunittest.RHTestProgram(module, defaultTest, argv, testRunner, impl)
    else:
        return ScaComponentTestProgram(spd_file, module, defaultTest, argv, testRunner, testLoader, impl)
