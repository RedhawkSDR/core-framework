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

import time
import os
import unittest
from _unitTestHelpers import scatest
from ossie.cf import CF
from omniORB import URI
import CosNaming
import CosEventChannelAdmin
from ossie.utils import sb
import os
import contextlib
from ossie.utils import redhawk

@contextlib.contextmanager
def stdout_redirect(where):
    sys.stdout = where
    try:
        yield where
    finally:
        sys.stdout = sys.__stdout__

@scatest.requireLog4cxx
class CppLoggingConfig(scatest.CorbaTestCase):
    def setUp(self):
        self.cname = "TestLoggingAPI"
        self.comp = sb.launch(self.cname)

    def _try_config_test(self, logcfg, epattern, foundTest=None ):

        with stdout_redirect(cStringIO.StringIO()) as new_stdout:
            ossie.utils.log4py.config.strConfig(logcfg,None)

        new_stdout.seek(0)
        found = []
        epats=[]
        if type(epattern) == str:
            epats.append(epattern)
        else:
            epats = epattern
        if foundTest == None:
            foundTest = len(epats)*[True]
        for x in new_stdout.readlines():
            for epat in epats:
                m=re.search( epat, x )
                if m :
                    found.append( True )

        self.assertEqual(found, foundTest )
        
    def tearDown(self):
        self.comp.releaseObject()
        sb.release()

        # Try to clean up the event channel, if it was created
        context = None
        try:
            channel = self._root.resolve(URI.stringToName('TEST_APPENDER/TEST_EVT_CH1'))
            channel = channel._narrow(CosEventChannelAdmin.EventChannel)
            channel.destroy()
        except:
            pass

        # Clean up naming context, too
        try:
            context = self._root.resolve(URI.stringToName('TEST_APPENDER'))
            context = context._narrow(CosNaming.NamingContext)
            self._root.unbind(URI.stringToName('TEST_APPENDER'))
            context.destroy()
        except:
            pass

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)


    def test_log_level(self):
        self.comp.ref._set_log_level( CF.LogLevels.OFF )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.OFF )
        self.comp.ref._set_log_level( CF.LogLevels.FATAL )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.FATAL)
        self.comp.ref._set_log_level( CF.LogLevels.ERROR )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.ERROR)
        self.comp.ref._set_log_level( CF.LogLevels.WARN )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.WARN)
        self.comp.ref._set_log_level( CF.LogLevels.INFO )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.INFO)
        self.comp.ref._set_log_level( CF.LogLevels.DEBUG )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.DEBUG)
        self.comp.ref._set_log_level( CF.LogLevels.TRACE )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.TRACE)
        self.comp.ref._set_log_level( CF.LogLevels.ERROR )

    def test_default_logconfig(self):
        cfg = "log4j.rootLogger=INFO,STDOUT\n" + \
              "# Direct log messages to STDOUT\n" + \
              "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
              "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
              "log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{3}:%L - %m%n\n"

        c_cfg=self.comp.ref.getLogConfig()

        ## remove extra white space
        cfg=cfg.replace(" ","")
        c_cfg=c_cfg.replace(" ","")
        self.assertEquals( cfg, c_cfg)


    def test_logconfig(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT\n" + \
            "# Direct log messages to STDOUT\n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{1}:%L - %m%n\n"

        self.comp.ref.setLogConfig(cfg)
        
        c_cfg=self.comp.ref.getLogConfig()
        cfg=cfg.replace(" ","")
        c_cfg=c_cfg.replace(" ","")
        self.assertEquals( cfg, c_cfg)


    def test_comp_macro_config(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT\n " + \
            "# Direct log messages to STDOUT\n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=@@@COMPONENT.NAME@@@\n"

        self.comp.ref.setLogConfig(cfg)
        
        c_cfg=self.comp.ref.getLogConfig()

        res=c_cfg.find(self.cname)

        self.assertNotEquals( res, -1 )

    def test_comp_macro_config2(self):
        cfg = "@@@COMPONENT.NAME@@@"
        self.comp.ref.setLogConfig(cfg)
        c_cfg=self.comp.ref.getLogConfig()
        res=c_cfg.find(self.cname)
        self.assertNotEquals( res, -1 )


    def test_comp_log_event_appender(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT,pse\n" + \
            "# Direct log messages to STDOUT \n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=@@@COMPONENT.NAME@@@\n" + \
            "# Direct log messages to event channel\n" + \
            "log4j.appender.pse=org.ossie.logging.RH_LogEventAppender\n" + \
            "log4j.appender.pse.name_context=TEST_APPENDER\n" + \
            "log4j.appender.pse.event_channel=TEST_EVT_CH1\n" + \
            "log4j.appender.pse.producer_id=PRODUCER1\n" + \
            "log4j.appender.pse.producer_name=THE BIG CHEESE\n" + \
            "log4j.appender.pse.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.pse.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n\n"

        c_cfg=self.comp.ref.setLogConfig(cfg)
        c_cfg=self.comp.ref.start()
        c_cfg=self.comp.ref.stop()


@scatest.requireJava
class JavaLoggingConfig(scatest.CorbaTestCase):
    def setUp(self):
        self.cname = "TestLoggingAPI"
        self.comp = sb.launch(self.cname, impl="java" )

        
    def tearDown(self):
        self.comp.releaseObject()
        sb.release()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def test_log_level(self):
        self.comp.ref._set_log_level( CF.LogLevels.OFF )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.OFF )
        self.comp.ref._set_log_level( CF.LogLevels.FATAL )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.FATAL)
        self.comp.ref._set_log_level( CF.LogLevels.ERROR )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.ERROR)
        self.comp.ref._set_log_level( CF.LogLevels.WARN )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.WARN)
        self.comp.ref._set_log_level( CF.LogLevels.INFO )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.INFO)
        self.comp.ref._set_log_level( CF.LogLevels.DEBUG )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.DEBUG)
        self.comp.ref._set_log_level( CF.LogLevels.TRACE )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.TRACE)
        self.comp.ref._set_log_level( CF.LogLevels.ERROR )

    def test_default_logconfig(self):
        cfg = "log4j.rootLogger=INFO,STDOUT\n" + \
	    "# Direct log messages to STDOUT\n" + \
	    "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
	    "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
	    "log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{3}:%L - %m%n\n"

        c_cfg=self.comp.ref.getLogConfig()
        cfg=cfg.replace(" ","")
        c_cfg=c_cfg.replace(" ","")
        self.assertEquals( cfg, c_cfg)


    def test_logconfig(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT\n" + \
            "# Direct log messages to STDOUT\n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{1}:%L - %m%n\n"

        self.comp.ref.setLogConfig(cfg)
        
        c_cfg=self.comp.ref.getLogConfig()
        cfg=cfg.replace(" ","")
        c_cfg=c_cfg.replace(" ","")

        self.assertEquals( cfg, c_cfg)


    def test_comp_macro_config(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT\n " + \
            "# Direct log messages to STDOUT\n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=@@@COMPONENT.NAME@@@\n"

        self.comp.ref.setLogConfig(cfg)
        
        c_cfg=self.comp.ref.getLogConfig()

        res=c_cfg.find(self.cname)

        self.assertNotEquals( res, -1 )

    def test_comp_macro_config2(self):
        cfg = "@@@COMPONENT.NAME@@@"
        self.comp.ref.setLogConfig(cfg)
        c_cfg=self.comp.ref.getLogConfig()
        res=c_cfg.find(self.cname)
        self.assertNotEquals( res, -1 )


    def test_comp_log_event_appender(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT,pse\n" + \
            "# Direct log messages to STDOUT\n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=@@@COMPONENT.NAME@@@\n" + \
            "# Direct log messages to event channel\n" + \
            "log4j.appender.pse=org.ossie.logging.RH_LogEventAppender\n" + \
            "log4j.appender.pse.name_context=TEST_APPENDER\n" + \
            "log4j.appender.pse.event_channel=TEST_EVT_CH1\n" + \
            "log4j.appender.pse.producer_id=PRODUCER1\n" + \
            "log4j.appender.pse.producer_name=THE BIG CHEESE\n" + \
            "log4j.appender.pse.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.pse.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n\n"

        c_cfg=self.comp.ref.setLogConfig(cfg)
        c_cfg=self.comp.ref.start()
        c_cfg=self.comp.ref.stop()

class FileLoggingConfig(scatest.CorbaTestCase):
    def setUp(self):
        self.cname = "TestLoggingAPI"
        self.comp = None
        
    def tearDown(self):
        if self.comp:
           self.comp.releaseObject()
        sb.release()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def test_comp_macro_directories_config_python(self):
        file_loc = os.getcwd()
        self.comp = sb.launch(self.cname, impl="python", execparams={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/logconfig.cfg'} )
        fp = None
        try:
            fp = open('foo/bar/test.log','r')
        except:
            pass
        try:
            os.remove('foo/bar/test.log')
        except:
            pass
        try:
            os.rmdir('foo/bar')
        except:
            pass
        try:
            os.rmdir('foo')
        except:
            pass
        self.assertNotEquals(fp, None)

    @scatest.requireLog4cxx
    def test_comp_macro_directories_config_cpp(self):
        file_loc = os.getcwd()
        self.comp = sb.launch(self.cname, impl="cpp", execparams={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/logconfig.cfg'} )
        fp = None
        try:
            fp = open('foo/bar/test.log','r')
        except:
            pass
        try:
            os.remove('foo/bar/test.log')
        except:
            pass
        try:
            os.rmdir('foo/bar')
        except:
            pass
        try:
            os.rmdir('foo')
        except:
            pass
        self.assertNotEquals(fp, None)

    @scatest.requireJava
    def test_comp_macro_directories_config_java(self):
        file_loc = os.getcwd()
        self.comp = sb.launch(self.cname, impl="java", execparams={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/logconfig.cfg'} )
        fp = None
        try:
            fp = open('foo/bar/test.log','r')
        except:
            pass
        try:
            os.remove('foo/bar/test.log')
        except:
            pass
        try:
            os.rmdir('foo/bar')
        except:
            pass
        try:
            os.rmdir('foo')
        except:
            pass
        self.assertNotEquals(fp, None)


class PythonLoggingConfig(scatest.CorbaTestCase):
    def setUp(self):
        self.cname = "TestLoggingAPI"
        self.comp = sb.launch(self.cname, impl="python", instanceName="TestLoggingAPI_1" )

    def tearDown(self):
        self.comp.releaseObject()
        sb.release()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def test_log_level(self):
        self.comp = sb.launch(self.cname, impl="python", execparams={'DISABLE_CB': 'True'} )
        self.comp.ref._set_log_level( CF.LogLevels.OFF )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.OFF )
        self.comp.ref._set_log_level( CF.LogLevels.FATAL )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.FATAL)
        self.comp.ref._set_log_level( CF.LogLevels.ERROR )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.ERROR)
        self.comp.ref._set_log_level( CF.LogLevels.WARN )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.WARN)
        self.comp.ref._set_log_level( CF.LogLevels.INFO )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.INFO)
        self.comp.ref._set_log_level( CF.LogLevels.DEBUG )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.DEBUG)
        self.comp.ref._set_log_level( CF.LogLevels.TRACE )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.TRACE)
        self.comp.ref._set_log_level( CF.LogLevels.ERROR )

    def test_default_logconfig(self):
        cfg = "log4j.rootLogger=INFO,STDOUT\n" + \
	    "# Direct log messages to STDOUT\n" + \
	    "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
	    "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
	    "log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n\n"

        c_cfg=self.comp.ref.getLogConfig()
        cfg=cfg.replace(" ","")
        c_cfg=c_cfg.replace(" ","")
        self.assertEquals( cfg, c_cfg)


    def test_logconfig(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT\n" + \
            "# Direct log messages to STDOUT\n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{1}:%L - %m%n\n"

        self.comp.ref.setLogConfig(cfg)
        
        c_cfg=self.comp.ref.getLogConfig()
        cfg=cfg.replace(" ","")
        c_cfg=c_cfg.replace(" ","")
        self.assertEquals( cfg, c_cfg)


    def test_comp_macro_config(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT\n" + \
            "# Direct log messages to STDOUT\n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=@@@COMPONENT.NAME@@@\n"

        self.comp.ref.setLogConfig(cfg)
        
        c_cfg=self.comp.ref.getLogConfig()

        res=c_cfg.find(self.cname)

        self.assertNotEquals( res, -1 )


    def test_comp_log_event_appender(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT,pse\n" + \
            "# Direct log messages to STDOUT \n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=@@@COMPONENT.NAME@@@\n" + \
            "# Direct log messages to event channel\n" + \
            "log4j.appender.pse=org.ossie.logging.RH_LogEventAppender\n" + \
            "log4j.appender.pse.name_context=TEST_APPENDER\n" + \
            "log4j.appender.pse.event_channel=TEST_EVT_CH1\n" + \
            "log4j.appender.pse.producer_id=PRODUCER1\n" + \
            "log4j.appender.pse.producer_name=THE BIG CHEESE\n" + \
            "log4j.appender.pse.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.pse.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n\n"

        c_cfg=self.comp.ref.setLogConfig(cfg)
        c_cfg=self.comp.ref.start()
        c_cfg=self.comp.ref.stop()


    def test_log_callback(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT,pse\n" + \
            "# Direct log messages to STDOUT \n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=@@@COMPONENT.NAME@@@\n" + \
            "# Direct log messages to event channel\n" + \
            "log4j.appender.pse=org.ossie.logging.RH_LogEventAppender\n" + \
            "log4j.appender.pse.name_context=TEST_APPENDER\n" + \
            "log4j.appender.pse.event_channel=TEST_EVT_CH1\n" + \
            "log4j.appender.pse.producer_id=PRODUCER1\n" + \
            "log4j.appender.pse.producer_name=THE BIG CHEESE\n" + \
            "log4j.appender.pse.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.pse.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n\n"

        exp_cfg = "log4j.rootLogger=ERROR,STDOUT,pse\n" + \
            "# Direct log messages to STDOUT \n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=TestLoggingAPI_1\n" + \
            "# Direct log messages to event channel\n" + \
            "log4j.appender.pse=org.ossie.logging.RH_LogEventAppender\n" + \
            "log4j.appender.pse.name_context=TEST_APPENDER\n" + \
            "log4j.appender.pse.event_channel=TEST_EVT_CH1\n" + \
            "log4j.appender.pse.producer_id=PRODUCER1\n" + \
            "log4j.appender.pse.producer_name=THE BIG CHEESE\n" + \
            "log4j.appender.pse.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.pse.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n\n"

        # change log level, callback will intercept
        orig = self.comp.ref._get_log_level()
        self.comp.ref._set_log_level( CF.LogLevels.TRACE )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, orig )

        # change config
        c_cfg=self.comp.ref.setLogConfig(cfg)
        self.assertEquals( self.comp.new_log_cfg, exp_cfg)
        

class PythonDeviceLoggingConfig(scatest.CorbaTestCase):
    def setUp(self):
        self.cname = "python_dev"
        self.comp = sb.launch(self.cname, impl="python" )
        
    def tearDown(self):
        self.comp.releaseObject()
        sb.release()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)


    def test_log_level(self):
        self.comp = sb.launch(self.cname, impl="python", execparams={'DISABLE_CB': 'True'} )
        self.comp.ref._set_log_level( CF.LogLevels.OFF )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.OFF )
        self.comp.ref._set_log_level( CF.LogLevels.FATAL )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.FATAL)
        self.comp.ref._set_log_level( CF.LogLevels.ERROR )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.ERROR)
        self.comp.ref._set_log_level( CF.LogLevels.WARN )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.WARN)
        self.comp.ref._set_log_level( CF.LogLevels.INFO )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.INFO)
        self.comp.ref._set_log_level( CF.LogLevels.DEBUG )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.DEBUG)
        self.comp.ref._set_log_level( CF.LogLevels.TRACE )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, CF.LogLevels.TRACE)
        self.comp.ref._set_log_level( CF.LogLevels.ERROR )

    def test_default_logconfig(self):
        cfg = "log4j.rootLogger=INFO,STDOUT\n" + \
	    "# Direct log messages to STDOUT\n" + \
	    "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
	    "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
	    "log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n\n"

        c_cfg=self.comp.ref.getLogConfig()
        cfg=cfg.replace(" ","")
        c_cfg=c_cfg.replace(" ","")
        self.assertEquals( cfg, c_cfg)


    def test_logconfig(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT\n" + \
            "# Direct log messages to STDOUT\n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{1}:%L - %m%n\n"

        self.comp.ref.setLogConfig(cfg)
        
        c_cfg=self.comp.ref.getLogConfig()
        cfg=cfg.replace(" ","")
        c_cfg=c_cfg.replace(" ","")
        self.assertEquals( cfg, c_cfg)


    def test_comp_macro_config(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT\n" + \
            "# Direct log messages to STDOUT\n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=@@@DEVICE.NAME@@@\n"

        self.comp.ref.setLogConfig(cfg)
        
        c_cfg=self.comp.ref.getLogConfig()

        res=c_cfg.find(self.cname)

        self.assertNotEquals( res, -1 )


    def test_comp_log_event_appender(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT,pse\n" + \
            "# Direct log messages to STDOUT \n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=@@@DEVICE.NAME@@@\n" + \
            "# Direct log messages to event channel\n" + \
            "log4j.appender.pse=org.ossie.logging.RH_LogEventAppender\n" + \
            "log4j.appender.pse.name_context=TEST_APPENDER\n" + \
            "log4j.appender.pse.event_channel=TEST_EVT_CH1\n" + \
            "log4j.appender.pse.producer_id=PRODUCER1\n" + \
            "log4j.appender.pse.producer_name=THE BIG CHEESE\n" + \
            "log4j.appender.pse.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.pse.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n\n"

        c_cfg=self.comp.ref.setLogConfig(cfg)
        c_cfg=self.comp.ref.start()
        c_cfg=self.comp.ref.stop()


    def test_log_callback(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT,pse\n" + \
            "# Direct log messages to STDOUT \n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=@@@DEVICE.NAME@@@\n" + \
            "# Direct log messages to event channel\n" + \
            "log4j.appender.pse=org.ossie.logging.RH_LogEventAppender\n" + \
            "log4j.appender.pse.name_context=TEST_APPENDER\n" + \
            "log4j.appender.pse.event_channel=TEST_EVT_CH1\n" + \
            "log4j.appender.pse.producer_id=PRODUCER1\n" + \
            "log4j.appender.pse.producer_name=THE BIG CHEESE\n" + \
            "log4j.appender.pse.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.pse.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n\n"

        exp_cfg = "log4j.rootLogger=ERROR,STDOUT,pse\n" + \
            "# Direct log messages to STDOUT \n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=python_dev_1\n" + \
            "# Direct log messages to event channel\n" + \
            "log4j.appender.pse=org.ossie.logging.RH_LogEventAppender\n" + \
            "log4j.appender.pse.name_context=TEST_APPENDER\n" + \
            "log4j.appender.pse.event_channel=TEST_EVT_CH1\n" + \
            "log4j.appender.pse.producer_id=PRODUCER1\n" + \
            "log4j.appender.pse.producer_name=THE BIG CHEESE\n" + \
            "log4j.appender.pse.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.pse.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n\n"


        # change log level, callback will intercept
        orig = self.comp.ref._get_log_level()
        self.comp.ref._set_log_level( CF.LogLevels.TRACE )
        lvl = self.comp.ref._get_log_level()
        self.assertEquals( lvl, orig )

        # change config
        c_cfg=self.comp.ref.setLogConfig(cfg)
        self.assertEquals( self.comp.new_log_cfg, exp_cfg)

class DomainTestLogEventAppender(scatest.CorbaTestCase):
    def setUp(self):
        self.stderr_filename = 'stderr.out'
        self.output_file = open(self.stderr_filename,'w')
        nb, self._domMgr = self.launchDomainManager(stderr=self.output_file)
        nb, self._devMgr = self.launchDeviceManager('/nodes/test_PortTestDevice_node/DeviceManager.dcd.xml')
        self.dom=redhawk.attach(self._domMgr._get_name() )
        fp = open('loggers/syncappender/log4j.appender', 'r')
        self.logconfig = fp.read()
        fp.close()

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)
        try:
            self.output_file.close()
        except:
            pass
        try:
            os.remove(self.stderr_filename)
        except:
            pass

    def test_logeventappenderDomainManager(self):
        self.dom.setLogConfig(self.logconfig)
        fp = open(self.stderr_filename, 'r')
        contents = fp.read()
        fp.close()
        self.assertEquals(len(contents), 0)

class LoggingConfigCategory(scatest.CorbaTestCase):
    def setUp(self):
        self.comp=None
        pass

    def tearDown(self):
        if self.comp:
            self.comp.releaseObject()
        sb.release()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def _test_LoggingCategory(self):
        # change root logger level state from WARN to INFO
        orig=self.comp.log_level()
        x='\n\nlog4j.rootLogger=WARN,stdout\n\n \
    # Direct log messages to stdout\n \
    log4j.appender.stdout=org.apache.log4j.ConsoleAppender\n \
    log4j.appender.stdout.Target=System.out\n \
    log4j.appender.stdout.layout=org.apache.log4j.PatternLayout\n \
    log4j.appender.stdout.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{1}:%L - %m%n\n \
    log4j.category.TestLoggingAPI_1=INFO,stdout\n\n'

        self.comp.setLogConfig(x)
        lvl=self.comp.log_level()
        self.assertEquals( orig, lvl )


        # change component's  logger level state from WARN to TRACE
        proj=CF.LogLevels.TRACE
        y='\n\nlog4j.rootLogger=WARN,stdout\n\n \
    # Direct log messages to stdout\n \
    log4j.appender.stdout=org.apache.log4j.ConsoleAppender\n \
    log4j.appender.stdout.Target=System.out\n \
    log4j.appender.stdout.layout=org.apache.log4j.PatternLayout\n \
    log4j.appender.stdout.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{1}:%L - %m%n\n \
    log4j.category.TestLoggingAPI_1=TRACE,stdout\n\n'
        self.comp.setLogConfig(y)
        lvl=self.comp.log_level()
        self.assertEquals( proj, lvl )

    @scatest.requireJava
    def test_LoggingCategoryJava(self):
        self.cname = "TestLoggingAPI"
        self.comp = sb.launch(self.cname, impl="java" )
        self._test_LoggingCategory()

    def test_LoggingCategoryPython(self):
        self.cname = "TestLoggingAPI"
        self.comp = sb.launch(self.cname, impl="python", execparams={'DISABLE_CB':True } )
        self._test_LoggingCategory()

    @scatest.requireLog4cxx
    def test_LoggingCategoryCpp(self):
        self.cname = "TestLoggingAPI"
        self.comp = sb.launch(self.cname, impl="cpp" )
        self._test_LoggingCategory()

        
if __name__ == "__main__":
  # Run the unittests
  unittest.main()
