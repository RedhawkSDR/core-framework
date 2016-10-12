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

import unittest
from _unitTestHelpers import scatest
from ossie.cf import CF
from omniORB import URI
import CosNaming
import CosEventChannelAdmin
from ossie.utils import sb

class CppLoggingConfig(scatest.CorbaTestCase):
    def setUp(self):
        self.cname = "TestLoggingAPI"
        self.comp = sb.launch(self.cname)

        
    def tearDown(self):
        self.comp.releaseObject()

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


    def test_setNamedLogLevel(self):
        self.comp.ref.setLogLevel( "", CF.LogLevels.OFF );
        self.comp.ref.setLogLevel( "", CF.LogLevels.FATAL );
        self.comp.ref.setLogLevel( "", CF.LogLevels.ERROR );
        self.comp.ref.setLogLevel( "", CF.LogLevels.WARN );
        self.comp.ref.setLogLevel( "", CF.LogLevels.INFO );
        self.comp.ref.setLogLevel( "", CF.LogLevels.DEBUG );
        self.comp.ref.setLogLevel( "", CF.LogLevels.TRACE );

    def test_log_level(self):
        self.comp.ref._set_log_level( CF.LogLevels.OFF );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.OFF );
        self.comp.ref._set_log_level( CF.LogLevels.FATAL );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.FATAL);
        self.comp.ref._set_log_level( CF.LogLevels.ERROR );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.ERROR);
        self.comp.ref._set_log_level( CF.LogLevels.WARN );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.WARN);
        self.comp.ref._set_log_level( CF.LogLevels.INFO );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.INFO);
        self.comp.ref._set_log_level( CF.LogLevels.DEBUG );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.DEBUG);
        self.comp.ref._set_log_level( CF.LogLevels.TRACE );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.TRACE);
        self.comp.ref._set_log_level( CF.LogLevels.ERROR );

    def test_default_logconfig(self):
        cfg = "log4j.rootLogger=INFO,STDOUT\n" + \
              "# Direct log messages to STDOUT\n" + \
              "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
              "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
              "log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{1}:%L - %m%n\n"

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
            "# Direct log messages to stdout\n" + \
            "log4j.appender.pse=RH_LogEventAppender\n" + \
            "log4j.appender.pse.name_context=TEST_APPENDER\n" + \
            "log4j.appender.pse.event_channel=TEST_EVT_CH1\n" + \
            "log4j.appender.pse.producer_id=PRODUCER1\n" + \
            "log4j.appender.pse.producer_name=THE BIG CHEESE\n" + \
            "log4j.appender.pse.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.pse.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n\n"

        c_cfg=self.comp.ref.setLogConfig(cfg)
        c_cfg=self.comp.ref.start()
        c_cfg=self.comp.ref.stop()



class JavaLoggingConfig(scatest.CorbaTestCase):
    def setUp(self):
        self.cname = "TestLoggingAPI"
        self.comp = sb.launch(self.cname, impl="java" )

        
    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)


    def test_setNamedLogLevel(self):
        self.comp.ref.setLogLevel( "", CF.LogLevels.OFF );
        self.comp.ref.setLogLevel( "", CF.LogLevels.FATAL );
        self.comp.ref.setLogLevel( "", CF.LogLevels.ERROR );
        self.comp.ref.setLogLevel( "", CF.LogLevels.WARN );
        self.comp.ref.setLogLevel( "", CF.LogLevels.INFO );
        self.comp.ref.setLogLevel( "", CF.LogLevels.DEBUG );
        self.comp.ref.setLogLevel( "", CF.LogLevels.TRACE );

    def test_log_level(self):
        self.comp.ref._set_log_level( CF.LogLevels.OFF );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.OFF );
        self.comp.ref._set_log_level( CF.LogLevels.FATAL );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.FATAL);
        self.comp.ref._set_log_level( CF.LogLevels.ERROR );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.ERROR);
        self.comp.ref._set_log_level( CF.LogLevels.WARN );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.WARN);
        self.comp.ref._set_log_level( CF.LogLevels.INFO );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.INFO);
        self.comp.ref._set_log_level( CF.LogLevels.DEBUG );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.DEBUG);
        self.comp.ref._set_log_level( CF.LogLevels.TRACE );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.TRACE);
        self.comp.ref._set_log_level( CF.LogLevels.ERROR );

    def test_default_logconfig(self):
        cfg = "log4j.rootLogger=INFO,STDOUT\n" + \
	    "# Direct log messages to STDOUT\n" + \
	    "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
	    "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
	    "log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{1}:%L - %m%n\n"

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


    def disable_tbd_test_comp_log_event_appender(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT,pse\n" + \
            "# Direct log messages to STDOUT\n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=@@@COMPONENT.NAME@@@\n" + \
            "# Direct log messages to stdout\n" + \
            "log4j.appender.pse=RH_LogEventAppender\n" + \
            "log4j.appender.pse.name_context=TEST_APPENDER\n" + \
            "log4j.appender.pse.event_channel=TEST_EVT_CH1\n" + \
            "log4j.appender.pse.producer_id=PRODUCER1\n" + \
            "log4j.appender.pse.producer_name=THE BIG CHEESE\n" + \
            "log4j.appender.pse.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.pse.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n\n"

        c_cfg=self.comp.ref.setLogConfig(cfg)
        c_cfg=self.comp.ref.start()
        c_cfg=self.comp.ref.stop()


class PythonLoggingConfig(scatest.CorbaTestCase):
    def setUp(self):
        self.cname = "TestLoggingAPI"
        self.comp = sb.launch(self.cname, impl="python" )

        
    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)


    def test_setNamedLogLevel(self):
        self.comp.ref.setLogLevel( "", CF.LogLevels.OFF );
        self.comp.ref.setLogLevel( "", CF.LogLevels.FATAL );
        self.comp.ref.setLogLevel( "", CF.LogLevels.ERROR );
        self.comp.ref.setLogLevel( "", CF.LogLevels.WARN );
        self.comp.ref.setLogLevel( "", CF.LogLevels.INFO );
        self.comp.ref.setLogLevel( "", CF.LogLevels.DEBUG );
        self.comp.ref.setLogLevel( "", CF.LogLevels.TRACE );

    def test_log_level(self):
        self.comp.ref._set_log_level( CF.LogLevels.OFF );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.OFF );
        self.comp.ref._set_log_level( CF.LogLevels.FATAL );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.FATAL);
        self.comp.ref._set_log_level( CF.LogLevels.ERROR );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.ERROR);
        self.comp.ref._set_log_level( CF.LogLevels.WARN );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.WARN);
        self.comp.ref._set_log_level( CF.LogLevels.INFO );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.INFO);
        self.comp.ref._set_log_level( CF.LogLevels.DEBUG );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.DEBUG);
        self.comp.ref._set_log_level( CF.LogLevels.TRACE );
        lvl = self.comp.ref._get_log_level();
        self.assertEquals( lvl, CF.LogLevels.TRACE);
        self.comp.ref._set_log_level( CF.LogLevels.ERROR );

    def test_default_logconfig(self):
        cfg = "log4j.rootLogger=INFO,STDOUT\n" + \
	    "# Direct log messages to STDOUT\n" + \
	    "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
	    "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
	    "log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{1}:%L - %m%n\n"

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


    def disable_tbd_test_comp_log_event_appender(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT,pse\n" + \
            "# Direct log messages to STDOUT \n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=@@@COMPONENT.NAME@@@\n" + \
            "# Direct log messages to stdout\n" + \
            "log4j.appender.pse=RH_LogEventAppender\n" + \
            "log4j.appender.pse.name_context=TEST_APPENDER\n" + \
            "log4j.appender.pse.event_channel=TEST_EVT_CH1\n" + \
            "log4j.appender.pse.producer_id=PRODUCER1\n" + \
            "log4j.appender.pse.producer_name=THE BIG CHEESE\n" + \
            "log4j.appender.pse.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.pse.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n\n"

        c_cfg=self.comp.ref.setLogConfig(cfg)
        c_cfg=self.comp.ref.start()
        c_cfg=self.comp.ref.stop()



        
if __name__ == "__main__":
  # Run the unittests
  unittest.main()
