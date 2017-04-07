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

import unittest
from ossie.utils.log4py import config
from ossie.utils.log4py import *
import time
import os
import sys
import contextlib
import cStringIO
import re

logcfg="""
log4j.rootLogger=INFO, CONSOLE
log4j.appender.CONSOLE=org.apache.log4j.ConsoleAppender
log4j.appender.CONSOLE.layout=org.apache.log4j.PatternLayout
log4j.appender.CONSOLE.layout.ConversionPattern="""

conversions = { 'date1' : ( "%d{yyyy-MM-dd}", '%Y-%m-%d' ),
                'date2' : ( "%d{yy-MM-D}", '%y-%m-%j' ),
                'month1' : ( "%d{MMM}", '%b' ),
                'month2' : ( "%d{MMMM}", '%B' ),
                'month3' : ( "%d{MM}", '%m' ),
                'month4' : ( "%d{M}", '%m' ),
                'day1' : ( "%d{EEE}", '%a' ),
                'day2' : ( "%d{EEEE}", '%A' ),
                'day3' : ( "%d{DDD}", '%j' ),
                'day4' : ( "%d{D}", '%j' ),
                'week1' : ( "%d{F}", '%w' ),
                'week2' : ( "%d{w}", '%U' ),
                'hour1' : ( "%d{HH}", '%H' ),
                'hour2' : ( "%d{H}", '%H' ),
                'minute1' : ( "%d{mm}", '%M' ),
                'minute2' : ( "%d{mmm}", '%M' ),
                'week2' : ( "%d{w}", '%U' ),
                'tz4' : ( "%d{zzzz}", '%Z' ),
                'tz3' : ( "%d{zzz}", '%Z' ),
                'tz1' : ( "%d{z}", '%Z' ),
                'ap1' : ( "%d{a}", '%p' ),
                'iso8601' : ( "%d{ISO8601}", '%Y-%m-%d %H:%M:%S', ',' ),
               }

@contextlib.contextmanager
def stdout_redirect(where):
    sys.stdout = where
    try:
        yield where
    finally:
        sys.stdout = sys.__stdout__

class Test_Log4py_DateFormat(unittest.TestCase):

    def setUp(self):
        self.tfile = os.tmpfile()

    def _run_test(self, fmt_key):
        conv=conversions[fmt_key]
        config.strConfig(logcfg+conv[0])
        self.logger=logging.getLogger('')
        self.console=self.logger.handlers[0]
        self.console.stream=self.tfile
        
        pval=time.strftime(conv[1])
        
        # 
        self.logger.info('test1')
        self.tfile.seek(0)
        logline=self.tfile.read()
        logline=logline.strip()
        if len(conv) > 2:
            logline = logline.split(conv[2])[0]
            pval = pval.split(conv[2])[0]
        self.assertEquals( pval, logline)


    def test_date1(self):
        self._run_test('date1')

    def test_date2(self):
        self._run_test('date2')

    def test_month1(self):
        self._run_test('month1')

    def test_month2(self):
        self._run_test('month2')

    def test_month3(self):
        self._run_test('month3')

    def test_month4(self):
        self._run_test('month4')

    def test_day1(self):
        self._run_test('day1')

    def test_day2(self):
        self._run_test('day2')

    def test_day3(self):
        self._run_test('day3')

    def test_day4(self):
        self._run_test('day4')

    def test_week1(self):
        self._run_test('week1')

    def test_week2(self):
        self._run_test('week2')

    def test_hour1(self):
        self._run_test('hour1')

    def test_hour2(self):
        self._run_test('hour2')

    def test_minute1(self):
        self._run_test('minute1')

    def test_minute2(self):
        self._run_test('minute2')

    def test_timezone1(self):
        self._run_test('tz4')

    def test_timezone2(self):
        self._run_test('tz3')

    def test_timezone3(self):
        self._run_test('tz1')

    def test_iso8601(self):
        self._run_test('iso8601')

class Log4PyAppenders(unittest.TestCase):
    def setUp(self):
        pass

        
    def tearDown(self):
        pass

    def test_fileappender_abs_path(self):
        import ossie.utils.log4py.config
        fname="/tmp/foo/bar/test.log"
        cfg = "log4j.rootLogger=ERROR,FILE\n" + \
            "log4j.appender.FILE=org.apache.log4j.FileAppender\n" + \
            "log4j.appender.FILE.File="+fname+"\n"

        ossie.utils.log4py.config.strConfig(cfg,None)

        fp = None
        fp = open(fname,'r')
        self.assertNotEquals(fp,None)
        try:
            os.remove(fname)
        except:
            pass
        
        # make sure rel directory not created
        floc = './'.join(fname.split('/')[:-1])
        self.assertRaises( OSError, os.stat, floc )

        try:
            os.rmdir('/tmp/foo/bar')
        except:
            pass

        try:
            os.rmdir('/tmp/foo')
        except:
            pass


    def test_fileappender_rel_path(self):
        import ossie.utils.log4py.config
        fname="tmp/foo/bar/test.log"
        cfg = "log4j.rootLogger=ERROR,FILE\n" + \
            "log4j.appender.FILE=org.apache.log4j.FileAppender\n" + \
            "log4j.appender.FILE.File="+fname+"\n"

        ossie.utils.log4py.config.strConfig(cfg,None)

        floc = os.getcwd()
        fp = None
        fp = open(fname,'r')
        self.assertNotEquals(fp,None)
        try:
            os.remove(fname)
        except Exception, e:
            pass

        try:
            os
            os.rmdir('tmp/foo/bar')
        except Exception, e:
            pass
        try:
            os.rmdir('tmp/foo')
        except:
            pass
        try:
            os.rmdir('tmp')
        except:
            pass



logcfg_ok_spaces="""
log4j.rootLogger=               INFO,                stdout
log4j.category.J1=    WARN   ,              pse
log4j.appender.stdout=                                    \
org.apache.log4j.ConsoleAppender
log4j.appender.pse=              org.ossie.logging.RH_LogEventAppender
"""
logcfg_continuation_1="""
log4j.rootLogger=INFO,stdout
log4j.appender.stdout=\
org.apache.log4j.ConsoleAppender
log4j.appender.stdout.Target=System.out
log4j.appender.stdout.layout=org.apache.log4j.PatternLayout
log4j.appender.stdout.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n
"""
logcfg_continuation_error="""
log4j.rootLogger=INFO,stdout
log4j.appender.stdout=\
org.apache.log4j.Console\Appender
log4j.appender.stdout.Target=System.out
log4j.appender.stdout.layout=org.apache.log4j.PatternLayout
log4j.appender.stdout.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n
"""
logcfg_error_1="""
log4j.rootLogger=INFO,stdout
log4j.logger.J1=WARN,pse
log4j.appender.stdout=\
org.apache.log4j.ConsoleAppender
log4j.appender.pse=org.ossei.logging.RH_LogEventAppender
"""
logcfg_error_2="""
log4j.rootLogger=INFO,stdout
log4j.logger.J1=WARN,pse
log4j.appender.stdoutx=org.apache.log4j.ConsoleAppender
log4j.appender.pse=org.ossie.logging.RH_LogEventAppender
"""
logcfg_error_3="""
log4j.rootLogger=INFO,stdout
log4j.logger.J1=WARN,pse
log4j.appender.stdout=org.apache.log4j.ConsoleAppender
log4j.appender.pse=org.ossei.logging.RH_LogEventAppender
"""
logcfg_error_4="""
log4j.rootLogger=INFO,stdout, missingappender
log4j.logger.J1=WARN,pse
log4j.appender.stdout=org.apache.log4j.ConsoleAppender
log4j.appender.pse=org.ossei.logging.RH_LogEventAppender
"""
logcfg_error_5="""
log4j.rootLogger=BAD,stdout
log4j.logger.J1=WARN,pse
log4j.appender.stdout=org.apache.log4j.ConsoleAppender
log4j.appender.pse=org.ossie.logging.RH_LogEventAppender
"""
logcfg_error_6="""
log4j.rootLogger=BAD,stdout, missingappender
log4j.logger.J1=WARN,pse
log4j.appender.stdout=org.apache.log4j.ConsoleAppender
log4j.appender.pse=org.ossei.logging.RH_LogEventAppender
"""
logcfg_error_7="""
log4j.rootLogger=BAD,stdout, missingappender
log4j.category.J1=WARN,pse
log4j.appender.stdout=org.apache.log4j.ConsoleAppender
log4j.appender.pse=org.ossei.logging.RH_LogEventAppender
"""
logcfg_error_8="""
log4j.rootLogger=INFO,stdout
log4j.category.J1=WARN,pse
"""
logcfg_error_9="""
-----log4j.rootLogger=INFO,stdout
log4j.category.J1=WARN,pse
log4j.appender.stdout=org.apache.log4j.ConsoleAppender
"""
logcfg_error_10="""
log4j.rootLogger=INFO,stdout
log4j.category.J1.WARN,pse
log4j.appender.stdout=org.apache.log4j.ConsoleAppender
"""
logcfg_error_11="""
log4j.rootLogger=INFO,stdout
log4j.category.J1=BAD,pse
log4j.appender.stdout=org.apache.log4j.ConsoleAppender
"""

class Log4PyConfigFile(unittest.TestCase):
    def setUp(self):
        pass


    def tearDown(self):
        pass

    def _try_config_test(self, logcfg, epattern, foundTest=None ):
        import ossie.utils.log4py.config

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

    def test_extra_spaces(self):
        self._try_config_test(logcfg_ok_spaces, "org.apache.log4j.ConsoleAppender", [] )

    def test_line_continuation(self):
        self._try_config_test(logcfg_continuation_1, "org.apache.log4j.ConsoleAppender", [] )

    def test_line_continuation_error(self):
        self._try_config_test(logcfg_continuation_error, r"org.apache.log4j.Console\\Appender" )

    def test_misspell_appender(self):
        self._try_config_test(logcfg_error_1, "error with appender:.*org.ossei.logging.RH_LogEventAppender" )

    def test_unknown_appender_root(self):
        self._try_config_test(logcfg_error_2, "Root.*unknown level or appender: stdout" )

    def test_unknown_appender_J1(self):
        self._try_config_test(logcfg_error_3,
                              [ "J1.*unknown handler: pse",
                                "error with appender:.*ossei.logging.RH_LogEventAppender",
                                ] )

    def test_missing_appender_multi(self):
        self._try_config_test(logcfg_error_4,
                              [ "unknown level or appender: missingappender",
                               "error with appender:.*ossei.logging.RH_LogEventAppender",
                                "J1.*unknown handler: pse"
                                ] )

    def test_badlevel(self):
        self._try_config_test(logcfg_error_5,
                              ["Root.*unknown level: BAD",
                               ] )

    def test_badlevel_multi_2(self):
        self._try_config_test(logcfg_error_6,
                              ["Root.*unknown level: BAD",
                               "unknown level or appender: missingappender",
                               "error with appender:.*ossei.logging.RH_LogEventAppender",
                               ] )

    def test_badlevel_multi_3(self):
        self._try_config_test(logcfg_error_7,
                              ["Root.*unknown level: BAD",
                               "unknown level or appender: missingappender",
                               "error with appender:.*ossei.logging.RH_LogEventAppender",
                               ] )

    def test_missing_appender(self):
        self._try_config_test(logcfg_error_8,
                              [ "Root.*unknown level or appender: stdout",
                                "J1.*unknown handler: pse",
                               ] )

    def test_line_format_error(self):
        self._try_config_test(logcfg_error_9,
                              [ "missing log4j.rootLogger line",
                               ] )

    def test_line_format_error_2(self):
        self._try_config_test(logcfg_error_10,
                              [ "error malformed log4py configuration",
                               ] )

    def test_badlevel_4(self):
        self._try_config_test(logcfg_error_11,
                              ["J1.*unknown level: BAD",
                               "J1.*unknown handler: pse",
                               ] )



class Log4PyConfigImport(unittest.TestCase):
    def setUp(self):
        pass


    def tearDown(self):
        pass

    def test_import_issue_file(self):
        from ossie.utils import sb
        import ossie.utils.log4py.config
        assert_raised=False
        try:
            ossie.utils.log4py.config.fileConfig(os.getcwd()+'/logconfig.cfg')
        except:
            assert_raised=True
        self.assertEqual( assert_raised, False )


    def test_import_issue_str(self):
        from ossie.utils import sb
        import ossie.utils.log4py.config
        assert_raised=False
        try:
            ossie.utils.log4py.config.strConfig(logcfg_ok_spaces)
        except:
            assert_raised=True
        self.assertEqual( assert_raised, False )
