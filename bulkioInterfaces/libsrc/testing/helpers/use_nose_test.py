try:
    import nose
    import nose.plugins.xunit
    import logging
    import ossie.utils.testing
    import ossie.parsers.spd as SPDParser
    import sys
    import codecs
    from nose.pyversion import force_unicode, format_exception

    #log=logging.getLogger('nose.plugins.MyPlugin')
    class TestCfgPlugin(nose.plugins.Plugin):
        name="cfg"
        def __init__(self,prog,spd_file, impl=None):
            self.prog=prog
            self.spd_file= spd_file
            self.impl = impl
            self.enabled=True
            self.score=None
            super(TestCfgPlugin,self).__init__()

        def options(self, parser, env):
            super(TestCfgPlugin,self).options(parser,env)
            parser.add_option('--impl',
                          dest='impl', 
                          default=None,
                          help='Specify implementation to run')

        def configure(self, options, conf):
            super(TestCfgPlugin,self).configure(options,conf)
            self.enabled=True
            self.impl=options.impl

        def setOutputStream(self, stream):
	    self.stream=stream

    class TestXunitPlugin(nose.plugins.xunit.Xunit):
        name="xunit"
        def __init__(self,prog,spd_file, impl=None):
            self.prog=prog
            self.spd_file= spd_file
            self.impl = impl
            self.cnt=0
            self.mode="w"
            self.score=None
            super(TestXunitPlugin,self).__init__()

        def options(self, parser, env):
            super(TestXunitPlugin,self).options(parser,env)

        def configure(self, options, config):
            super(TestXunitPlugin,self).configure( options, config)
            self.enabled=True            

        def resetReport(self):
            self.mode="w"
            self.stats = {'errors': 0,
                          'failures': 0,
                          'passes': 0,
                          'skipped': 0
                          }
            self.errorlist = []

        def report(self, stream):
            """Writes an Xunit-formatted XML file

            The file includes a report of test errors and failures.

            """
            efname = self.error_report_file_name.replace("%IMPL%", str(self.impl))
            print "Report ---->>>>>> ", efname, " IMP ", self.impl, " CNT ", self.cnt,  " Mode ", self.mode
            self.error_report_file = codecs.open(efname, self.mode,
                                                 self.encoding, 'replace')

            self.stats['testsuite_name'] = self.xunit_testsuite_name+'-'+self.impl
            self.stats['total'] = (self.stats['errors'] + self.stats['failures']
                                   + self.stats['passes'] + self.stats['skipped'])
            if self.cnt == 0:
                self.error_report_file.write( u'<?xml version="1.0" encoding="%s"?>' % self.encoding )
            
            self.error_report_file.write(            
                u'<testsuite name="%(testsuite_name)s" tests="%(total)d" '
                u'errors="%(errors)d" failures="%(failures)d" '
                u'skip="%(skipped)d">' % self.stats)

            self.error_report_file.write(u''.join([force_unicode(e, self.encoding)
                                                   for e in self.errorlist]))
            self.error_report_file.write(u'</testsuite>')
            self.error_report_file.close()
            if self.config.verbosity > 1:
                stream.writeln("-" * 70)
                stream.writeln("XML: %s" % self.error_report_file.name)


    #
    # Only runs tests from __main__
    #
    class NoseTestProgram(nose.core.TestProgram):
        def __init__(self,spd,**kw):
            self.mycfg=TestCfgPlugin(self,spd)
            self.myxunit=TestXunitPlugin(self,spd)
            self.results=[]
            super(NoseTestProgram,self).__init__(addplugins=[self.mycfg,self.myxunit],**kw)

        def runTests(self):
            # parse components.spd file
            spd = SPDParser.parse(self.mycfg.spd_file)

            #log.debug("runTests called")
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
                    self.myxunit.impl=IMPL_ID
                    result = self.testRunner.run(self.test)
                    self.success = self.success and result.wasSuccessful()
                    self.results.append(result)
                    self.myxunit.resetReport()

            if self.exit:
                sys.exit(not self.success)
            return self.success
except ImportError:
    pass

