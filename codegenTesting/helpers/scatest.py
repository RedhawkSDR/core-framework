#!/usr/bin/env python
from _unitTestHelpers.scatest import *
import commands
from subprocess import Popen
from xml.dom.minidom import parse

import common

def setupDeviceAndDomainMgrPackage():
    # override the method defined in _unitTestHelpers.  We do not wish to
    # use a local Domain or Device Manager profile.
    #
    # This will cause us to use the domain/device managers installed 
    # in OSSIEHOME
    pass

class CodeGenTestCase(OssieTestCase):
    """A helper class for test cases which need to generate and compile code."""
    artifact_name = None
    artifact_dir = None
    language = None
    languages = None
    output_dir = 'build'

    def __init__(self, methodName='runTest'):
        unittest.TestCase.__init__(self, methodName)
        self._orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
        self._poa = self._orb.resolve_initial_references("RootPOA")
        self._poa._get_the_POAManager().activate()
        self._ns = self._orb.resolve_initial_references("NameService")
        self._root = self._ns._narrow(CosNaming.NamingContext)

    def __str__(self):
        try:
            methodName = self._testMethodName
        except AttributeError:
            # On Python < 2.5
            methodName = self._TestCase__testMethodName
        return methodName + ' [' + str(self.artifact_type) + ': ' + str(self.artifact_name) + ']'

    def setUp(self):
        validLanguageKeys = []
        if self.language == 'C++':
            validKeys = common.cpp_usage
        elif self.language == 'Java':
            validKeys = common.java_usage
        elif self.language == 'Python':
            validKeys = common.python_usage
        # Loadable/Executable device not supported for java
        elif self.artifact_name == 'basic_executable_device' or self.artifact_name == 'basic_loadable_device':
            validKeys = common.cpp_usage + common.python_usage
            self.languages = ('C++', 'Python')
        else:
            validKeys = common.cpp_usage + common.java_usage + common.python_usage

        self.impl_dict = self.getImplementationIDs(validKeys)

        self.source_dir = os.path.join(self.artifact_dir, self.artifact_name)
        self.build_dir = os.path.join(self.output_dir, self.artifact_name)

        # Use the build directory to find the unit test, in case the project
        # doesn't have a pre-generated one of its own
        self.test_dir = os.path.join(self.build_dir, 'tests')
        sys.path.insert(0, self.test_dir)

        if not os.path.isdir(self.output_dir):
            os.mkdir(self.output_dir)

    def tearDown(self):
        sys.path.remove(self.test_dir)


    def generateCode(self, impls=None):
        spdfile = os.path.join(self.source_dir, self.artifact_name + '.spd.xml')
        codegen = os.path.join(os.environ['OSSIEHOME'], 'bin', 'redhawk-codegen')

        #generate only for this language if a language was specified
        if self.language:
            languages = [self.language]
        elif self.languages:
            languages = self.languages
        else:
            languages = ('C++', 'Java', 'Python')

        if impls:
             for impl in impls:
                    opts = [('-C', self.build_dir), ('--impl', impl)]
                    opts = ' '.join(' '.join(item) for item in opts)
                    _codegen = ' '.join((codegen, opts, spdfile))
                    (status,output) = commands.getstatusoutput(_codegen)
                    print output
                    self.assertEquals(status, 0, msg='failed for impl=' + str(impl) + '\n\n' + output)
        else:
          for lang in languages:
             if self.impl_dict.has_key(lang):
                for impl in self.impl_dict[lang]:
                    implId = impl['id']
                    opts = [('-C', self.build_dir), ('--impl', implId)]
                    opts = ' '.join(' '.join(item) for item in opts)
                    _codegen = ' '.join((codegen, opts, spdfile))

                    (status,output) = commands.getstatusoutput(_codegen)
                    self.assertEquals(status, 0, msg='failed for lang=' + lang + '\n\n' + output)

    def compileProject(self, file_dir):
        start_dir = os.getcwd();
        os.chdir(file_dir)
        command = './reconf; ./configure; make'
        (status,output) = commands.getstatusoutput(command)
        os.chdir(start_dir)
        self.assertEquals(status, 0 ,msg=output)

    def getImplementationIDs(self, validLanguageKeys):
        parent_dir = os.getcwd()
        artifact_dir = parent_dir + '/' + self.artifact_dir + '/' + self.artifact_name
        try:
            os.chdir(artifact_dir)
        except:
            self.fail('No ' + self.artifact_type + ' named <' + str(self.artifact_name) + '> found')

        impl_dict = {}
        spd_file_name = self.artifact_name + ".spd.xml"

        try:
            spd_dom = parse(spd_file_name)
            implementation_id_nodes = spd_dom.getElementsByTagName("implementation")

            if len(implementation_id_nodes) > 0:

                for node in implementation_id_nodes:
                    id = node.attributes['id'].nodeValue
                    pl_node = node.getElementsByTagName('programminglanguage')[0]
                    pl = pl_node.attributes['name'].nodeValue

                    local_node = node.getElementsByTagName('localfile')[0]
                    local_file_dir = local_node.attributes['name'].nodeValue

                    entry_point_node = node.getElementsByTagName('entrypoint')[0]
                    entry_point = entry_point_node.firstChild.nodeValue

                    if pl in validLanguageKeys:
                        lang = str(pl)
                        impl = {'id': id,
                                'name': entry_point}
                        if lang not in impl_dict:
                            impl_dict[lang] = []
                        impl_dict[lang].append(impl)

            else:
                print "No implementations found in SPD file for " + self.artifact_type + " [" + self.artifact_name + "]"

        except Exception, e:
            print "No SPD file found for " + self.artifact_type + " [" + self.artifact_name + "]"

        os.chdir(parent_dir)
        return impl_dict

    def runUnitTests(self):
        failures = []
        for lang in self.impl_dict:
            for impl in self.impl_dict[lang]:
                impl_id = impl['id']
                impl_name = impl['name']
                spd_file = os.path.join(self.build_dir, self.artifact_name+'.spd.xml')
                retval = ossie.utils.testing.ScaComponentTestProgram(spd_file,
                                                                     module='test_'+self.artifact_name,
                                                                     impl=impl_id)
                for result in retval.results:
                    if result.errors or result.failures:
                        if not lang in failures:
                            failures.append(lang)

        self.assertEqual(len(failures), 0, msg='failed for ' + ', '.join(failures))
