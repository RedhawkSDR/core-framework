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
               }

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

    def test_ap1(self):
        self._run_test('ap1')

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

        
