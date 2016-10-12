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

"""
XML Test Runner for PyUnit that outputs JUnit-like reports.

Based on xmlrunner.py written by Sebasitan Rittau
"""

import os.path
import re
import sys
import time
import traceback
import unittest
from xml.sax.saxutils import escape
from StringIO import StringIO

class _TestInfo(object):
    def __init__(self, test, time):
        (self._class, self._method) = test.id().rsplit(".", 1)
        self._time = time
        self._error = None
        self._failure = None

    @staticmethod
    def create_success(test, time):
        return _TestInfo(test, time)

    @staticmethod
    def create_failure(test, time, failure):
        info = _TestInfo(test, time)
        info._failure = failure
        return info

    @staticmethod
    def create_error(test, time, error):
        info = _TestInfo(test, time)
        info._error = error
        return info

    def print_report(self, stream):
        stream.write('   <testcase classname="%(class)s" name="%(method)s" test="%(time).4f">' % \
            { "class": self._class,
              "method": self._method,
              "time": self._time,
            })
        if self._failure is not None:
            self._print_error(stream, 'failure', self._failure)
        if self._error is not None:
            self._print_error(stream, 'error', self._error)
        stream.write('</testcase>\n')

    def _print_error(self, stream, tagname, error):
        text = escape(str(error[1]))
        stream.write('\n')
        stream.write('   <%s type="%s">%s\n' % \
           (tagname, _clsname(error[0]), text))
        tb_stream = StringIO()
        traceback.print_tb(error[2], None, tb_stream)
        stream.write(escape(tb_stream.getvalue()))
        stream.write('   </%s>\n' % tagname)
        stream.write('  ')

def _clsname(cls):
    return cls.__module__ + "." + cls.__name__

class _XMLTestResult(unittest.TestResult):
    def __init__(self, classname):
        unittest.TestResult.__init__(self)
        self._test_name = classname
        self._start_time = None
        self._tests = []
        self._error = None
        self._failure = None

    def startTest(self, test):
        unittest.TestResult.startTest(self, test)
        self._error = None
        self._failure = None
        self._start_time = time.time()

    def stopTest(self, test):
        time_taken = time.time() - self._start_time
        unittest.TestResult.stopTest(self, test)
        if self._error:
            info = _TestInfo.create_error(test, time_taken, self._error)
        elif self._failure:
            info = _TestInfo.create_failure(test, time_taken, self._failure)
        else:
            info = _TestInfo.create_success(test, time_taken)
        self._tests.append(info)

    def addError(self, test, err):
        unittest.TestResult.addError(self, test, err)
        self._error = err

    def addFailure(self, test, err):
        unittest.TestResult.addFailure(self, test, err)
        self._failure = err

    def print_report(self, stream, time_taken, out, err):
        stream.write('<testsuite errors="%(e)d" failures="%(f)d" ' % \
           { "e": len(self.errors),
             "f": len(self.failures)
        })
        stream.write('name="%(n)s" tests="%(t)d" time="%(time).3f">\n' % \
          { "n": self._test_name,
            "t": self.testsRun,
            "time": time_taken,
          })
        for info in self._tests:
            info.print_report(stream)
        stream.write('  <system-out><![CDATA[%s]]></system-out>\n' % out)
        stream.write('  <system-err><![CDATA[%s]]></system-err>\n' % err)
        stream.write('</testsuite>\n')

class XMLTestRunner(object):
    def __init__(self, stream=None):
        self._stream = stream
        self._path = "."

    def run(self, test):
        class_ = test.__class__
        classname = class_.__module__ + "." + class_.__name__
        if self._stream == None:
            filename = "TEST-%s.xml" % classname
            stream = file(os.path.join(self._path, filename), "w")
            stream.write('<?xml version="1.0" encoding="utf-8"?>\n')
        else:
            stream = self._stream

        result = _XMLTestResult(classname)
        start_time = time.time()

        self._orig_stdout = sys.stdout
        self._orig_stderr = sys.stderr
        try:
            sys.stdout = StringIO()
            sys.stderror = StringIO()
            test(result)
            try:
                out_s = sys.stdout.getvalue()
            except AttributeError:
                out_s = ""
            try:
                err_s = sys.stderr.getvalue()
            except AttributeError:
                err_s = ""
        finally:
            sys.stdout = self._orig_stdout
            sys.stderr = self._orig_stderr

        time_taken = time.time() - start_time
        result.print_report(stream, time_taken, out_s, err_s)
        if self._stream is None:
            stream.close()

        return result

    def _set_path(self, path):
        self._path = path

    path = property(lambda self: self._path, _set_path, None, "")
