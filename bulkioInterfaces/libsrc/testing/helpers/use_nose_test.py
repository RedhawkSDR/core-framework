try:
    import nose
    import logging
    import ossie.utils.testing
    import ossie.parsers.spd as SPDParser
    import sys

    #log=logging.getLogger('nose.plugins.MyPlugin')
    class TestCfgPlugin(nose.plugins.Plugin):
        name="TestCfgPlugin"
        def __init__(self,prog,spd_file, impl=None):
            self.prog=prog
            self.spd_file= spd_file
            self.impl = impl
            self.enabled=True
            self.score=None
            super(TestCfgPlugin,self).__init__()

        def options(self, parser, env={}):
            super(TestCfgPlugin,self).options(parser,env)
            parser.add_option('--impl',
                          dest='impl', 
                          default=None,
                          help='Specify implementation to run')

        def configure(self, options, conf):
            super(TestCfgPlugin,self).configure(options,conf)
            self.impl=options.impl

    #
    # Only runs tests from __main__
    #
    class NoseTestProgram(nose.core.TestProgram):
        def __init__(self,spd,**kw):
            self.mycfg=TestCfgPlugin(self,spd)
            self.results=[]
            super(NoseTestProgram,self).__init__(addplugins=[self.mycfg],**kw)

        def TworunTests(self):
            spd = SPDParser.parse(self.mycfg.spd_file)

            impl = self.mycfg.impl
            self.success=True
            self.exit=False
            for implementation in spd.get_implementation():
                IMPL_ID = implementation.get_id()
                if IMPL_ID == impl or impl is None:
                    ossie.utils.testing.setImplId(IMPL_ID)
                    ossie.utils.testing.setSoftPkg(self.mycfg.spd_file)
                    impl_ret=super(NoseTestProgram,self).runTests()
                    self.success = self.success and impl_ret

            if self.exit:
                sys.exit(not self.success)

        def runTests(self):
            # parse components.spd file
            spd = SPDParser.parse(self.mycfg.spd_file)

            #log.debug("runTests called")
            if self.testRunner is None:
                self.testRunner = nose.core.TextTestRunner(stream=self.config.stream,
                                             verbosity=self.config.verbosity,
                                             config=self.config)
            plug_runner = self.config.plugins.prepareTestRunner(self.testRunner)
            if plug_runner is not None:
                self.testRunner = plug_runner
            impl = self.mycfg.impl
            self.success=True
            self.exit=False
            for implementation in spd.get_implementation():
                IMPL_ID = implementation.get_id()
                if IMPL_ID == impl or impl is None:
                    ossie.utils.testing.setImplId(IMPL_ID)
                    ossie.utils.testing.setSoftPkg(self.mycfg.spd_file)
                    result = self.testRunner.run(self.test)
                    self.success = self.success and result.wasSuccessful()
                    self.results.append(result)

            if self.exit:
                sys.exit(not self.success)
            return self.success
except ImportError:
    pass

